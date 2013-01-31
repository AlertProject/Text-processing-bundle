#include "qminer_seq.h"

//feature extractor for sequences
TOgSeqFtrExt::TOgSeqFtrExt(POgBase _OgBase, 
		const TInt& _FilterStId, 
		const TIntPrV& _FilterJoinIdV, 
		const TInt& _FilterJStId, 
		const TOgFtrExt& _St1FrtExt,
		const TOgFtrExt& _St2FrtExt,
		const TInt& _ElementNo):  OgBase(_OgBase), FilterJoinIdV(_FilterJoinIdV){

	FilterStId = _FilterStId;
	FilterJStId = _FilterJStId;
	ElementNo = _ElementNo;	

	StateFtrExtV.Add(_St1FrtExt);
	StateFtrExtV.Add(_St2FrtExt);
}

POgRecSet TOgSeqFtrExt::GetOwnerSet(const TStr& FilterQueryType, 
	const TStr& FilterQuery, const TInt& FilterRecSmpl){

	if (FilterQuery == ""){
		return OgBase->GetStoreByStoreId(FilterStId)->GetRndRecs(FilterRecSmpl); 
	} else {
		TStrKdV NmValPrV; NmValPrV.Add(TStrKd("storeid", FilterStId.GetStr()));
		NmValPrV.Add(TStrKd("qtype", FilterQueryType)); 
		NmValPrV.Add(TStrKd("q", FilterQuery)); 
		NmValPrV.Add(TStrKd("sample", FilterRecSmpl.GetStr())); 
		return OgBase->GetSearchOp()->Exec(OgBase, TOgRecSetV(), NmValPrV);
	}
}

POgRecSet TOgSeqFtrExt::GetObsSet(const POgRecSet& SeqRSet, const TStr& FilterJQueryType, 
	const TStr& FilterJQuery, const TInt& FilterJoinSmpl){

	/*TStrKdV NmValPrV; NmValPrV.Add(TStrKd("storeid", FilterJStId.GetStr()));
	NmValPrV.Add(TStrKd("q", FilterJQuery)); 
	NmValPrV.Add(TStrKd("qtype", FilterJQueryType)); 
	NmValPrV.Add(TStrKd("sample", FilterJoinSmpl.GetStr())); 
	return OgBase->GetSearchOp()->Exec(OgBase, TOgRecSetV(), NmValPrV);*/
	TOgOpLinSearch LinSrch;
	return LinSrch.Exec(OgBase, SeqRSet, 0, oolstIsInRange, "Morning");
}

POgRecSet TOgSeqFtrExt::GetObservations(const int& OwnerId, const int& FilterJoinSmpl){
	
	POgRecSet OwnerSet = TOgRecSet::New(FilterStId, OwnerId);
	// get sequences of states for current owner
	if (FilterJoinIdV.Len() > 0){
		return OwnerSet->DoJoin(OgBase, FilterJoinIdV[0].Val1, FilterJoinSmpl, false);
	} else { return OwnerSet; }
}

TIntV TOgSeqFtrExt::GetTmFiledIdV(const int& StoreId){
	TIntV FieldIdV = OgBase->GetStoreByStoreId(StoreId)->GetFieldIdV(oftTm);
	return FieldIdV;
}

void TOgSeqFtrExt::GetTmSorted(const int& TmFieldId, const POgRecSet& SeqRSet){
	SeqRSet->SortByField(OgBase, true, TmFieldId);
}

int TOgSeqFtrExt::GetTmDifSecs(const int& StoreId, const int& Rec1Id, 
	const int& Rec2Id, const int& TmFieldId){
	
	TTm Tm1; 
	OgBase->GetStoreByStoreId(StoreId)->GetFieldTm(Rec1Id, TmFieldId, Tm1);
	TTm Tm2; 
	OgBase->GetStoreByStoreId(StoreId)->GetFieldTm(Rec2Id, TmFieldId, Tm2);
	return (int) TTm::GetDiffMSecs(Tm1, Tm2) / 60000;
}

void TOgSeqFtrExt::GetTransV(const TInt& StoreId,  const POgRecSet& Sequence, TStrV& TransV){	

	if ((StoreId == FilterJStId) || (StoreId == FilterStId)){
		for (int VIdx = 0; VIdx < Sequence->GetRecs(); VIdx++){	
			TStrV TempV;
			TOgRec Rec = Sequence->GetRec(VIdx);
			int i = (VIdx == 0) ? 0 : HOverlap;
			for (; i < StateFtrExtV.Len(); i++){
				StateFtrExtV[i].ExtractAggr(OgBase, Rec, TempV);
				if (TempV.Empty()){
					TransV.Add("empty");					
				}else{
					TrimToItemNo(TempV);
					TStr State = "";
					for (int i = 0; i < TempV.Len(); i++){
						if (i > 0){State += "_";}
						State += TempV[i];
					}
					TransV.Add(State);	
					/*if (State == "external"){ 
 						printf("external, ");
					} */
				}
			}
		}
		//printf("\n");
	}else {
		TOgExcept::Throw("Wrong transition store id");
	}
}

void TOgSeqFtrExt::TrimToItemNo(TStrV& TempV){
	while (TempV.Len() > ElementNo){ TempV.DelLast(); }	
}

void TOgSeqFtrExt::CheckInvalidTrans(POgRecSet OwnerSet, const int& OwnerIdx, POgRecSet SeqRSet){
	for (int RecIdx = 0; RecIdx < SeqRSet->GetRecs(); RecIdx++){
		TStrV StateV1, StateV2;
		StateFtrExtV[0].ExtractAggr(OgBase, SeqRSet->GetRec(RecIdx), StateV1);
		StateFtrExtV[1].ExtractAggr(OgBase, SeqRSet->GetRec(RecIdx), StateV2);
		if (StateV2.IsIn("external")){
			if (StateV1.Len() == 0){StateV1.Add("empty");}
			TStr UserNm = OgBase->GetStoreByStoreId(OwnerSet->GetStoreId())->GetRecNm(OwnerSet->GetRecId(OwnerIdx));
			//TStr Referrer = OgBase->GetStoreByStoreId(SeqRSet->GetStoreId())->GetFieldStr(SeqRSet->GetRecId(RecIdx), 0);
			//TStr Requested = OgBase->GetStoreByStoreId(SeqRSet->GetStoreId())->GetFieldStr(SeqRSet->GetRecId(RecIdx), 0);
			printf("%d %lu %s %s %s \n", OwnerIdx, SeqRSet->GetRecId(RecIdx), UserNm.CStr(), StateV1[0].CStr(), StateV2[0].CStr());
		}
	}
}

///sequence
TOgSeqBs::TOgSeqBs(TSIn& SIn){
	SeqV.Load(SIn); 	
	BowDocBs = TBowDocBs::New()->Load(SIn);
	BowDocPart = TBowDocPart::New()->Load(SIn); 
	StateHV.Load(SIn); 
	DirEdgeHV.Load(SIn); 
	//SeqMomV.Load(SIn); 
	ActionNoV.Load(SIn);
}

void TOgSeqBs::ExtractSeqV(const POgSeqFtrExt& Extractor, const TStr& FilterQueryType,
	const TStr& FilterQuery, const TStr& FilterJQueryType, const TStr& FilterJQuery, 
	const int& FilterRecSmpl, const int& FilterJoinSmpl){

	//get owner set
	OwnerSet = Extractor->GetOwnerSet(FilterQueryType, FilterQuery, FilterRecSmpl);	
	 for (int OwnerIdx = 0; OwnerIdx < OwnerSet->GetRecs(); OwnerIdx++){
		//int OwnerId = OwnerSet->GetRecId(OwnerIdx);
		// get observations for current owner 
		POgRecSet SeqRSet = Extractor->GetObservations(OwnerIdx, FilterJoinSmpl);
		if (FilterJQuery != ""){	
			//SeqRSet = TOgOpLinSearch::Exec(OgBase, SeqRSet, 0, oolstIsInRange, "Morning");
			SeqRSet = Extractor->GetObsSet(SeqRSet, FilterJQueryType, FilterJQuery, FilterJoinSmpl);
			//SeqRSet = SeqRSet->Intersect(ObsSet);
		}
		//get sequences split by time
		GenSeqByTmSlice(Extractor, SeqRSet, OwnerIdx, 30);			   	
	}
}

int TOgSeqBs::GetStateId (const int& ClustIdx, TStr& StateNm){
	//int StateId = 0;
	TStrV StateNmV; StateNmV.Add(StateNm);
	if (StateHV[ClustIdx].IsKey(StateNmV)){ 		
			return StateHV[ClustIdx].GetKeyId(StateNmV);					
	}
	return -1;
}

TStr TOgSeqBs::GetOutStateStr(const TStrV& StStrV){
	
	TStr OutStr = "";
	for(int Idx = 0; Idx < StStrV.Len(); Idx++){
		if (Idx > 0){ OutStr += "_"; }
		OutStr += StStrV[Idx];
		if (OutStr.IsStrIn("NICAT:")){OutStr.DelSubStr(0, OutStr.SearchCh(':'));}
	}	
	if (OutStr == ""){OutStr = "empty";}
	return OutStr;
}

void TOgSeqBs::GenSeqByTmSlice(const POgSeqFtrExt& Extractor, 
	const POgRecSet& SeqRSet, const int& OwnerIdx, const int& TmIntervalSec){
	
	int StoreId = SeqRSet->GetStoreId();
	int TmFieldId = Extractor->GetTmFiledIdV(StoreId)[0];
	Extractor->GetTmSorted(TmFieldId, SeqRSet);  //sort by server time

	Extractor->CheckInvalidTrans(OwnerSet, OwnerIdx, SeqRSet);

	TVec<TUInt64> ActionV; int ActionId = 0;
	if (SeqRSet->GetRecs() > 0){
		ActionV.Add(SeqRSet->GetRecId(ActionId++));
		while (ActionId < SeqRSet->GetRecs()){				
			int TimeDif = Extractor->GetTmDifSecs(StoreId, 
				SeqRSet->GetRecId(ActionId - 1), SeqRSet->GetRecId(ActionId), TmFieldId);

			if (TimeDif >= TmIntervalSec){
				SeqV.Add(TOgRecSet::New(StoreId, ActionV));
				SeqOwnerH.AddDat(SeqV.LastValN(), OwnerIdx);	
				ActionV.Clr();								
			}
			ActionV.Add(SeqRSet->GetRecId(ActionId++));			
		}
		SeqV.Add(TOgRecSet::New(StoreId, ActionV));
		SeqOwnerH.AddDat(SeqV.LastValN(), OwnerIdx);		
	}
}

void TOgSeqBs::GetNGramStates(const int& N, const int& SeqIdx, 
	  const POgSeqFtrExt& Extractor, TStrV& StateStrV){

	  int SeqPos = 0; TStr StateStr; TStrV StateV;
	  if (SeqV[SeqIdx]->GetRecs() >= N){
		  Extractor->GetTransV(SeqV[SeqIdx]->GetStoreId(), SeqV[SeqIdx], StateV);
		  ///
		  if (StateV.IsIn("external")){
		  for (int i = 0; i < StateV.Len(); i++){
				  printf("%s, ", StateV[i].GetCStr());
			}
		  printf("\n");
		  }
		  ///
		  while ((SeqPos + N) <= StateV.Len()){
			  for (int StateIdx = SeqPos; StateIdx < (N + SeqPos); StateIdx++){
				  if (StateIdx > SeqPos){StateStr += "__";}
				  StateStr += StateV[StateIdx];			
			  }
			  StateStrV.Add(StateStr);			  
			  StateStr = "";
			  SeqPos++;
		  }
	  }
}

void TOgSeqBs::GenBowBs(const int& N, const POgSeqFtrExt& Extractor){

	BowDocBs =  TBowDocBs::New();	
	for (int SeqIdx = 0; SeqIdx < SeqV.Len(); SeqIdx++){
		TStrV StateStrV;
		GetNGramStates(N, SeqIdx, Extractor, StateStrV);
		int BowId = BowDocBs->AddDoc(TInt(SeqIdx).GetStr(), "", StateStrV);
		BowSeqH.AddDat(BowId, SeqIdx);
	}
}

void TOgSeqBs::ClustSeq(const PBowSim& BowSim, const int& Seed, const int& Clusts, 
	const int& ClustTrials,	const double& ConvergEps, const int& MnDocsPerClust, 
	const TBowWordWgtType& WordWgtType, const double& CutWordWgtSumPrc, const int& MnWordFq){

		ActionNoV.Gen(Clusts);
		// get doc-ids
		TIntV AllDIdV; BowDocBs->GetAllDIdV(AllDIdV);

		TRnd Rnd(Seed);
		TSecTm StartTm=TSecTm::GetCurTm(); // get start-time
		BowDocPart = TBowClust::GetKMeansPart(
			TNotify::StdNotify, 
			BowDocBs,
			BowSim, // similarity function
			Rnd, // random generator
			Clusts, // number of clusters
			ClustTrials, // trials per k-means
			ConvergEps, // convergence epsilon for k-means
			1, // min. documents per cluster
			WordWgtType, // word weighting
			CutWordWgtSumPrc, // cut-word-weights percentage
			MnWordFq, // minimal word frequency
			AllDIdV); // training documents

		TSecTm EndTm=TSecTm::GetCurTm(); // get end-time
		printf("Duration: %d secs\n", TSecTm::GetDSecs(StartTm, EndTm));	
		BowDocPart->SaveTxt("./output/clusters.txt", BowDocBs, true, 15, 0.5, false);
}

void TOgSeqBs::GenTrans(const POgSeqFtrExt& Extractor){	

	//generate per cluster transitions
	for (int ClustIdx = 0; ClustIdx < BowDocPart->GetClusts(); ClustIdx++){
		printf("Cluster %d \n", ClustIdx);
		StateHV.Add(TStrVIntH());
		DirEdgeHV.Add(TIntIntHH());
		SeqMomV.Add(TMom::New());
		PBowDocPartClust Clust = BowDocPart->GetClust(ClustIdx);	
		TIntV DocIdV; Clust->GetDIdV(DocIdV); 
		//for all transition sequences in a cluster
		for (int CSeqIdx = 0; CSeqIdx < DocIdV.Len(); CSeqIdx++){
			int SeqIdx = DocIdV[CSeqIdx];			
			//go through all actions/events and build state frequency H 
			TStrV StateV;
			Extractor->GetTransV(SeqV[SeqIdx]->GetStoreId(), SeqV[SeqIdx], StateV);
			SeqMomV[ClustIdx]->Add(StateV.Len());
			for (int StateIdx = 0; StateIdx < StateV.Len(); StateIdx++){
				TStrV TempV; TempV.Add(StateV[StateIdx]);
				//StateH is a hash of states (which are vectors of strings) and their frequency. 
				//StateHV is the vector of StateHs, one per cluster
				if (StateHV[ClustIdx].IsKey(TempV)){
					StateHV[ClustIdx].GetDat(TempV)++;
				} else {StateHV[ClustIdx].AddDat(TempV, 1); }							
			}	
			
			//go through all actions/events and build transition model
			for (int StateIdx = 0; StateIdx < StateV.Len() - 1; StateIdx++){
				TIntH Dst; 
				//keyids in the state vector
				int LeftKey = GetStateId(ClustIdx, StateV[StateIdx]);				
				int RightKey = GetStateId(ClustIdx, StateV[StateIdx + 1]);					
				if ((LeftKey >= 0) && (RightKey >= 0)){	
					ActionNoV[ClustIdx]++;
					//row id is always LeftKey
					if (DirEdgeHV[ClustIdx].IsKeyGetDat(LeftKey, Dst)){
						//column id is always RightKey
						if (Dst.IsKey(RightKey)){
							DirEdgeHV[ClustIdx].GetDat(LeftKey).GetDat(RightKey)++;				
						} else {
							DirEdgeHV[ClustIdx].GetDat(LeftKey).AddDat(RightKey, 1);				
						}
					} else {
						Dst.AddDat(RightKey, 1);
						DirEdgeHV[ClustIdx].AddDat(LeftKey, Dst);
					}
					TStrV OutStr1 = StateHV[ClustIdx].GetKey(LeftKey);
					TStrV OutStr2 = StateHV[ClustIdx].GetKey(RightKey);	
					if (OutStr2.IsIn("external")){
						printf("%s -> %s \n", GetOutStateStr(OutStr1).CStr(), GetOutStateStr(OutStr2).CStr());					
					}
				}
			}
		}
		SeqMomV[ClustIdx]->Def();
	}
}

void TOgSeqBs::SaveActionGraph(const TStr& OutFNm, const int& MnTransFq){

	for (int ClustIdx = 0; ClustIdx < StateHV.Len(); ClustIdx++){
		printf("Cluster %d \n", ClustIdx);
		TStr CrtFNm = OutFNm + TInt(ClustIdx).GetStr();
		TFOut FOut (CrtFNm + ".dot", false);	
		FOut.PutStrLn("digraph finite_state_machine {");	
		FOut.PutStrLn("fontsize=12;");	

		int initEdgId = DirEdgeHV[ClustIdx].FFirstKeyId();
		while(DirEdgeHV[ClustIdx].FNextKeyId(initEdgId)){
			int initStateId = DirEdgeHV[ClustIdx].GetKey(initEdgId);		
			TIntH DstH = DirEdgeHV[ClustIdx].GetDat(initStateId);		
			int dstEdgId = DstH.FFirstKeyId();
			while(DstH.FNextKeyId(dstEdgId)){
				int dstStateId = DstH.GetKey(dstEdgId);		
				TStrV OutStr1 = StateHV[ClustIdx].GetKey(initStateId); 
				TStrV OutStr2 = StateHV[ClustIdx].GetKey(dstStateId); 
				TFlt TransFq = TFlt(DstH.GetDat(dstStateId))/TFlt(ActionNoV[ClustIdx]);
				if ((TransFq * 100) > MnTransFq){
					printf("%s -> %s \n", GetOutStateStr(OutStr1).CStr(), GetOutStateStr(OutStr2).CStr());					
					FOut.PutStr(GetOutStateStr(OutStr1) + " -> " + GetOutStateStr(OutStr2) + "  [ label = \""); 
					FOut.PutFlt(TransFq);
					if (TransFq > 0.1){ FOut.PutStrLn("\", color=orangered ];");
					}else { FOut.PutStrLn("\" ];"); }
				}
			}
		}	
		FOut.PutStr("\n");
		FOut.PutStrLn("}");
		//generate jpegs of graphs
		TStr OutParamStr = " -Tjpg -o\"" + CrtFNm + ".jpg\" -Kdot " + CrtFNm + ".dot";
		#ifdef GLib_WIN
		ShellExecute(NULL, "open", "dot.exe", OutParamStr.CStr(), NULL, SW_HIDE);
		#else
		TSysProc::ExeProc("./dot.exe", OutParamStr);
		#endif

	}	
}

void TOgSeqBs::SaveStateHist(const TStr& OutFNm){

	for (int ClustIdx = 0; ClustIdx < StateHV.Len(); ClustIdx++){
		TFOut FOut (OutFNm + TInt(ClustIdx).GetStr() + ".txt", false);	
		int KeyId = StateHV[ClustIdx].FFirstKeyId();	
		while (StateHV[ClustIdx].FNextKeyId(KeyId)){		
			FOut.PutStr(GetOutStateStr(StateHV[ClustIdx].GetKey(KeyId))); FOut.PutStr(", "); 
			FOut.PutStr(StateHV[ClustIdx].GetDat(StateHV[ClustIdx].GetKey(KeyId)).GetStr()); FOut.PutStr("\n");
		}
	}
}

void TOgSeqBs::Save(const TStr& OutFNm){

	TFOut FOut (OutFNm, false);	
	SeqV.Save(FOut); 	
	BowDocBs->Save(FOut);
	BowDocPart->Save(FOut);
	StateHV.Save(FOut); 
	DirEdgeHV.Save(FOut); 
	//SeqMomV.Save(FOut); 
	ActionNoV.Save(FOut);
}

void TOgSeqBs::SaveXml(const TStr& PartialClustFNm, const PXmlTok& TopTok){
	for (int ClustIdx = 0; ClustIdx < StateHV.Len(); ClustIdx++){
		PXmlTok SubTok = TXmlTok::New("cluster");
		SubTok->AddArg("id", ClustIdx);

		PXmlTok InTok = TXmlTok::New("in");
		InTok->AddArg("User-smpl", OwnerSet->GetRecs());
		InTok->AddArg("User-seq", SeqV.Len());
		PBowDocPartClust Clust = BowDocPart->GetClust(ClustIdx);	
		TIntV SeqIdV; Clust->GetDIdV(SeqIdV); TIntV ClustUsrIdV;
		for (int CSeqIdx = 0; CSeqIdx < SeqIdV.Len(); CSeqIdx++){
			int SeqIdx = SeqIdV[CSeqIdx];
			ClustUsrIdV.AddMerged(SeqOwnerH.GetDat(SeqIdx));
		}

		InTok->AddArg("User-clust", ClustUsrIdV.Len());
		InTok->AddArg("Clust-seq", SeqIdV.Len());
		SubTok->AddSubTok(InTok);

		PXmlTok StatsTok = TXmlTok::New("stats");
		if (SeqMomV[ClustIdx]->GetVals() > 1){				
			StatsTok->AddArg("Min-State", SeqMomV[ClustIdx]->GetMn());
			StatsTok->AddArg("Max-State", SeqMomV[ClustIdx]->GetMx());
			StatsTok->AddArg("Mean-State", SeqMomV[ClustIdx]->GetMean());
			StatsTok->AddArg("Median-State", SeqMomV[ClustIdx]->GetMedian());
			StatsTok->AddArg("StdDev-State", SeqMomV[ClustIdx]->GetSDev());
			StatsTok->AddArg("Url", PartialClustFNm + TInt(ClustIdx).GetStr() + ".jpg");	
			SubTok->AddSubTok(StatsTok);
		} 		
		TopTok->AddSubTok(SubTok);
	}
}

