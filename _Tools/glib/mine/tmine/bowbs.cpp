/////////////////////////////////////////////////
// BagOfWords-Keywords-Set
void TBowKWordSet::GetWordStrV(TStrV& WordStrV) const {
  WordStrV.Clr(); int KWords=GetKWords();
  for (int KWordN=0; KWordN<KWords; KWordN++){
    WordStrV.Add(GetKWordStr(KWordN));
  }
}

TStr TBowKWordSet::GetKWordsStr() const {
  TChA ResChA; int KWords=GetKWords();
  for (int KWordN=0; KWordN<KWords; KWordN++){
    if (KWordN>0){ResChA+=", ";}
    ResChA+=GetKWordStr(KWordN);
  }
  return ResChA;
}

PBowKWordSet TBowKWordSet::GetTopKWords(const int& MxKWords, const double& WgtSumPrc) const {
  TFltStrPrV WWgtWStrPrV;
  WStrToWWgtH.GetDatKeyPrV(WWgtWStrPrV);
  WWgtWStrPrV.Sort(false);
  double WgtSum=0;
  for (int WordN=0; WordN<WWgtWStrPrV.Len(); WordN++){
    WgtSum+=WWgtWStrPrV[WordN].Val1;
  }
  PBowKWordSet KWordSet=TBowKWordSet::New(GetNm());
  double WgtSumSF=0;
  for (int WordN=0; WordN<WWgtWStrPrV.Len(); WordN++){
    KWordSet->AddKWord(WWgtWStrPrV[WordN].Val2, WWgtWStrPrV[WordN].Val1);
    WgtSumSF+=WWgtWStrPrV[WordN].Val1;
    if ((MxKWords!=-1)&&(KWordSet->GetKWords()>=MxKWords)){break;}
    if ((WgtSum>0)&&(WgtSumSF/WgtSum>WgtSumPrc)){break;}
  }
  return KWordSet;
}

void TBowKWordSet::SaveTxt(const PSOut& SOut) const {
  SOut->PutStr(TStr::Fmt("'%s':", GetNm().CStr()));
  int KWords=GetKWords();
  for (int KWordN=0; KWordN<KWords; KWordN++){
    TStr KWordStr=GetKWordStr(KWordN);
    double KWordWgt=GetKWordWgt(KWordN);
    SOut->PutStr(TStr::Fmt(" [%s:%.5f]", KWordStr.CStr(), KWordWgt));
  }
}

void TBowKWordSet::SaveXml(const PSOut& SOut) const {
  int KWords=GetKWords();
  SOut->PutStr(TStr::Fmt("<KeywordSet Nm=\"%s\">", TXmlLx::GetXmlStrFromPlainStr(GetNm()).CStr()));
  for (int KWordN=0; KWordN<KWords; KWordN++){
    TStr KWordStr=GetKWordStr(KWordN);
    TStr KWordXmlStr=TXmlLx::GetXmlStrFromPlainStr(KWordStr);
    double KWordWgt=GetKWordWgt(KWordN);
    SOut->PutStr(TStr::Fmt("<Keyword Str=\"%s\" Wgt=\"%g\"/>", KWordXmlStr.CStr(), KWordWgt));
  }
  SOut->PutStr("</KeywordSet>");
}

void TBowKWordBs::SaveTxt(const PSOut& SOut) const {
  int KWordSets=GetKWordSets();
  for (int KWordSetN=0; KWordSetN<KWordSets; KWordSetN++){
    PBowKWordSet KWordSet=GetKWordSet(KWordSetN);
    KWordSet->SaveTxt(SOut);
    SOut->PutLn();
  }
}

void TBowKWordBs::SaveXml(const PSOut& SOut) const {
  SOut->PutStr("<KeywordBase>"); SOut->PutLn();
  int KWordSets=GetKWordSets();
  for (int KWordSetN=0; KWordSetN<KWordSets; KWordSetN++){
    PBowKWordSet KWordSet=GetKWordSet(KWordSetN);
    KWordSet->SaveXml(SOut);
    SOut->PutLn();
  }
  SOut->PutStr("</KeywordBase>");
}

TBowSpV::TBowSpV(const int& _DId, const TFltV& FullVec,
        const double& Eps): DId(_DId) {

    double SqrSum = 0.0;
    for (int EltN = 0; EltN < FullVec.Len(); EltN++) {
        const double EltVal = FullVec[EltN];
        if (TFlt::Abs(EltVal) > Eps) {
            SqrSum += TMath::Sqr(EltVal);
            WIdWgtKdV.Add(TIntSFltKd(EltN, (sdouble)EltVal));
        }
    }
    Norm = sqrt(SqrSum);
}

TBowSpV::TBowSpV(const int& DId, const TIntFltKdV& SpV) {
    double SqrSum = 0.0;
    WIdWgtKdV.Gen(SpV.Len(), 0);
    for (int WIdN = 0; WIdN < SpV.Len(); WIdN++) {
        WIdWgtKdV.Add(TIntSFltKd(SpV[WIdN].Key, sdouble(SpV[WIdN].Dat.Val)));
    }
    Norm = sqrt(SqrSum);
}

void TBowSpV::PutUnitNorm(){
  int WIds=GetWIds();
  double SqWgtSum=0;
  for (int WIdN=0; WIdN<WIds; WIdN++){
    SqWgtSum+=TMath::Sqr(WIdWgtKdV[WIdN].Dat);
  }
  if (SqWgtSum>0){
    for (int WIdN=0; WIdN<WIds; WIdN++){
      WIdWgtKdV[WIdN].Dat=(sdouble)sqrt(TMath::Sqr(WIdWgtKdV[WIdN].Dat)/SqWgtSum);
    }
    Norm=1.0;
  } else {
    Norm=0.0;
  }
}

double TBowSpV::GetNorm(){
  if (double(Norm)==-1){
    double SqWgtSum=0;
    int WIds=GetWIds();
    for (int WIdN=0; WIdN<WIds; WIdN++){
      SqWgtSum+=TMath::Sqr(WIdWgtKdV[WIdN].Dat);}
    Norm=sqrt(SqWgtSum);
  }
  return Norm;
}

void TBowSpV::AddBowSpV(const PBowSpV& BowSpV)
{
	for (int WIdN=0; WIdN < BowSpV->GetWIds(); WIdN++)
	{
		int WId; double Wgt;
		BowSpV->GetWIdWgt(WIdN, WId, Wgt);
		IncreaseWIdWgt(WId, Wgt);
	}
	Sort();
}

void TBowSpV::GetWordStrWgtPrV(const PBowDocBs& BowDocBs,
 const int& TopWords, const double& TopWordsWgtPrc,
 TStrFltPrV& WordStrWgtPrV) const {
  int WIds=GetWIds(); double WordWgtSum=0;
  TFltIntKdV WgtWIdKdV(WIds, 0);
  for (int WIdN=0; WIdN<WIds; WIdN++){
    int WId; double Wgt; GetWIdWgt(WIdN, WId, Wgt);
    WgtWIdKdV.Add(TFltIntKd(Wgt, WId));
    WordWgtSum+=Wgt;
  }
  WgtWIdKdV.Sort(false);
  WordStrWgtPrV.Clr();
  double WordWgtSumSF=0;
  {for (int WIdN=0; WIdN<WIds; WIdN++){
    if ((TopWords!=-1)&&(WIdN>=TopWords)){break;}
    if ((WordWgtSum>0)&&(WordWgtSumSF/WordWgtSum>TopWordsWgtPrc)){break;}
    int WId=WgtWIdKdV[WIdN].Dat;
    double WordWgt=WgtWIdKdV[WIdN].Key;
    TStr WordStr;
    if (BowDocBs.Empty()){WordStr=TInt::GetStr(WId);}
    else {WordStr=BowDocBs->GetWordStr(WId);}
    WordWgtSumSF+=WordWgt;
    WordStrWgtPrV.Add(TStrFltPr(WordStr, WordWgt));
  }}
}

PBowKWordSet TBowSpV::GetKWordSet(const PBowDocBs& BowDocBs) const {
  TStrFltPrV WordStrWgtPrV; GetWordStrWgtPrV(BowDocBs, -1, 1.0, WordStrWgtPrV);
  PBowKWordSet KWordSet=TBowKWordSet::New();
  for (int WordN=0; WordN<WordStrWgtPrV.Len(); WordN++){
    TStr WStr=WordStrWgtPrV[WordN].Val1;
    double WWgt=WordStrWgtPrV[WordN].Val2;
    KWordSet->AddKWord(WStr, WWgt);
  }
  return KWordSet;
}

void TBowSpV::GetIntFltKdV(TIntFltKdV& SpV) const {
    const int Wds = WIdWgtKdV.Len(); SpV.Gen(Wds, 0);
    for (int WdN = 0; WdN < Wds; WdN++) {
        const TBowWIdWgtKd& WIdWgt = WIdWgtKdV[WdN];
        SpV.Add(TIntFltKd(WIdWgt.Key, WIdWgt.Dat.Val));
    }
}

void TBowSpV::DelWId(const int& WId){
	for (int WIdN = 0; WIdN < WIdWgtKdV.Len(); WIdN++)
	{
		if (WIdWgtKdV[WIdN].Key == WId)
		{
			WIdWgtKdV.Del(WIdN);
			return;
		}
	}
}

void TBowSpV::CutLowWgtWords(const double& CutWordWgtSumPrc){
  if (CutWordWgtSumPrc<=0){return;}
  int WIds=GetWIds();
  double WgtSum=0; TFltV WgtV(WIds, 0);
  for (int WIdN=0; WIdN<WIds; WIdN++){
    double Wgt=WIdWgtKdV[WIdN].Dat;
    WgtSum+=Wgt; WgtV.Add(Wgt);
  }
  WgtV.Sort();
  double CutWgtSum=CutWordWgtSumPrc*WgtSum;
  double CutWgt=-1; int NonCutWgts=-1;
  for (int WgtN=0; WgtN<WIds; WgtN++){
    CutWgtSum-=WgtV[WgtN];
    if (CutWgtSum<=0){
      CutWgt=WgtV[WgtN]; NonCutWgts=WIds-WgtN; break;}
  }
  if (NonCutWgts!=-1){
    TBowWIdWgtKdV NewWIdWgtKdV(NonCutWgts, 0);
    for (int WIdN=0; WIdN<WIds; WIdN++){
      double Wgt=WIdWgtKdV[WIdN].Dat;
      if (Wgt>=CutWgt){
        NewWIdWgtKdV.Add(WIdWgtKdV[WIdN]);
      }
    }
    Norm=-1;
    WIdWgtKdV.MoveFrom(NewWIdWgtKdV);
  }
}

TStr TBowSpV::GetStr(const PBowDocBs& BowDocBs,
 const int& TopWords, const double& TopWordsWgtPrc, const TStr& SepStr,
 const bool& ShowWeightsP, const bool& KeepUndelineP) const {
  TStrFltPrV WordStrWgtPrV;
  GetWordStrWgtPrV(BowDocBs, TopWords, TopWordsWgtPrc, WordStrWgtPrV);
  TChA ChA;
  for (int WordN=0; WordN<WordStrWgtPrV.Len(); WordN++){
    TStr WordStr=WordStrWgtPrV[WordN].Val1;
    if (!KeepUndelineP){
      WordStr.ChangeChAll('_', ' ');}
    double WordWgt=WordStrWgtPrV[WordN].Val2;
    if (!ShowWeightsP) WordStr.ToLc();
    if (!ChA.Empty()){ChA+=SepStr;}
    if (ShowWeightsP){ChA+='[';}
    ChA+=WordStr;
    if (ShowWeightsP) ChA+=TFlt::GetStr(WordWgt, ":%g]");
  }
  return ChA;
}

void TBowSpV::SaveTxt(const PSOut& SOut, const PBowDocBs& BowDocBs,
 const int& TopWords, const double& TopWordsWgtPrc, const char& SepCh) const {
  TChA SepStr; SepStr += SepCh; 
  TStr Str=GetStr(BowDocBs, TopWords, TopWordsWgtPrc, SepStr);
  SOut->PutStr(Str);
  SOut->PutLn();
}

void TBowSpV::SaveXml(const PSOut& SOut, const PBowDocBs& BowDocBs) const {
  int WIds=GetWIds();
  TFltIntKdV WgtWIdKdV(WIds, 0);
  for (int WIdN=0; WIdN<WIds; WIdN++){
    int WId; double Wgt; GetWIdWgt(WIdN, WId, Wgt);
    WgtWIdKdV.Add(TFltIntKd(Wgt, WId));
  }
  WgtWIdKdV.Sort(false);
  SOut->PutStr("<SparseVec>");
  {for (int WIdN=0; WIdN<WIds; WIdN++){
    int WId=WgtWIdKdV[WIdN].Dat;
    double Wgt=WgtWIdKdV[WIdN].Key;
    TChA WordChA;
    WordChA+="<Word";
    WordChA+=" Id=\""; WordChA+=TInt::GetStr(WId); WordChA+='\"';
    if (!BowDocBs.Empty()){
      WordChA+=" Str=\""; WordChA+=BowDocBs->GetWordStr(WId); WordChA+='\"';
    }
    WordChA+=" Wgt=\""; WordChA+=TFlt::GetStr(Wgt); WordChA+='\"';
    WordChA+="/>";
    SOut->PutStr(WordChA);
  }}
  SOut->PutStr("</SparseVec>");
}

double TBowSimMtx::GetSim(const int& DId1, const int& DId2) const {
  if ((DId1==-1)||(DId2==-1)){return 0;}
  int MtxDId1=MtxDIdV[DId1]; int MtxDId2=MtxDIdV[DId2];
  if (MtxDId1>MtxDId2){TInt::Swap(MtxDId1, MtxDId2);}
  TFlt Sim=0;
  DIdPrToSimH.IsKeyGetDat(TIntPr(MtxDId1, MtxDId2), Sim);
  return Sim;
}

PBowSimMtx TBowSimMtx::LoadTxt(const TStr& FNm){
  PBowSimMtx BowSimMtx=TBowSimMtx::New();
  PSIn SIn=TFIn::New(FNm);
  TIntH MtxDIdH;
  TILx Lx(SIn, TFSet()|iloExcept);
  while (Lx.GetSym(syLBracket, syEof)!=syEof){
    int MtxDId1=Lx.GetInt(); Lx.GetSym(syColon);
    int MtxDId2=Lx.GetInt(); Lx.GetSym(syEq);
    double Sim=Lx.GetFlt(); Lx.GetSym(syRBracket);
    if (MtxDId1>MtxDId2){TInt::Swap(MtxDId1, MtxDId2);}
    BowSimMtx->DIdPrToSimH.AddDat(TIntPr(MtxDId1, MtxDId2), Sim);
    MtxDIdH.AddKey(MtxDId1); MtxDIdH.AddKey(MtxDId2);
  }
  int MtxDIds=MtxDIdH.Len();
  BowSimMtx->MtxDIdV.Gen(MtxDIdH.Len(), 0);
  for (int MtxDIdN=0; MtxDIdN<MtxDIds; MtxDIdN++){
    BowSimMtx->MtxDIdV.Add(MtxDIdH.GetKey(MtxDIdN));}
  BowSimMtx->MtxDIdV.Sort();
  return BowSimMtx;
}

double TBowSim::GetSim(const int& DId1, const int& DId2) const {
  IAssert(SimType==bstMtx);
  return SimMtx->GetSim(DId1, DId2);
}

double TBowSim::GetSim(const PBowSpV& SpV1, const PBowSpV& SpV2) const {
  switch (SimType){
    case bstBlock: return GetBlockSim(SpV1, SpV2);
    case bstEucl: return GetEuclSim(SpV1, SpV2);
    case bstCos: return GetCosSim(SpV1, SpV2);
    case bstMtx: return SimMtx->GetSim(SpV1->GetDId(), SpV2->GetDId());
    default: Fail; return 0;
  }
}

double TBowSim::GetSim(const TBowSpVV& SpVV1, const TBowSpVV& SpVV2) const {
  double Sim=0; int Sims=0;
  PMom SimMom=TMom::New();
  for (int SpVN1=0; SpVN1<SpVV1.Len(); SpVN1++){
    for (int SpVN2=0; SpVN2<SpVV2.Len(); SpVN2++){
      Sim+=GetSim(SpVV1[SpVN1], SpVV2[SpVN2]); Sims++;
      SimMom->Add(GetSim(SpVV1[SpVN1], SpVV2[SpVN2]));
    }
  }
  if (Sims>0){Sim/=Sims;}
  SimMom->Def();
  Sim=SimMom->GetMn();
  return Sim;
}

double TBowSim::GetBlockSim(const PBowSpV& SpV1, const PBowSpV& SpV2) {
  
  double Sim=0;
  
  int WIds1=SpV1->GetWIds();
  int WIds2=SpV2->GetWIds();
  
  int WIdN1=0; int WIdN2=0;
  while ((WIdN1<WIds1)&&(WIdN2<WIds2)){
    int WId1=SpV1->GetWId(WIdN1);
    int WId2=-1;
    forever {
      if (WIdN2>=WIds2){break;}
      WId2=SpV2->GetWId(WIdN2);
      if (WId2>=WId1){break;}
      WIdN2++;
    }
    if ((WIdN2<WIds2)&&(WId1==WId2)){
      double WordWgt1=SpV1->GetWgt(WIdN1); WIdN1++;
      double WordWgt2=SpV2->GetWgt(WIdN2); WIdN2++;
      if (WordWgt1==WordWgt2){Sim++;}
    } else {
      WIdN1++;
    }
  }
  return Sim;
}

double TBowSim::GetEuclSim(const PBowSpV& SpV1, const PBowSpV& SpV2) {
  int WIds1=SpV1->GetWIds();
  int WIds2=SpV2->GetWIds();
  double SqDifSum=0;
  int WIdN1=0; int WIdN2=0;
  while ((WIdN1<WIds1)&&(WIdN2<WIds2)){
    int WId1=SpV1->GetWId(WIdN1);
    int WId2=SpV2->GetWId(WIdN2);
    if (WId1==WId2){
      double WordWgt1=SpV1->GetWgt(WIdN1); WIdN1++;
      double WordWgt2=SpV2->GetWgt(WIdN2); WIdN2++;
      SqDifSum+=TMath::Sqr(WordWgt1-WordWgt2);
    } else
    if (WId1<WId2){
      double WordWgt1=SpV1->GetWgt(WIdN1); WIdN1++;
      double WordWgt2=0;
      SqDifSum+=TMath::Sqr(WordWgt1-WordWgt2);
    } else {
      
      double WordWgt1=0;
      double WordWgt2=SpV2->GetWgt(WIdN2); WIdN2++;
      SqDifSum+=TMath::Sqr(WordWgt1-WordWgt2);
    }
  }
  for (int RestWIdN1=WIdN1; RestWIdN1<WIds1; RestWIdN1++){
    double WordWgt1=SpV1->GetWgt(RestWIdN1);
    SqDifSum+=TMath::Sqr(WordWgt1);
  }
  for (int RestWIdN2=WIdN2; RestWIdN2<WIds2; RestWIdN2++){
    double WordWgt2=SpV2->GetWgt(RestWIdN2);
    SqDifSum+=TMath::Sqr(WordWgt2);
  }
  
  double Sim=-sqrt(SqDifSum);
  return Sim;
}

double TBowSim::GetCosSim(const PBowSpV& SpV1, const PBowSpV& SpV2) {
  // prepare shortcuts
  int WIds1=SpV1->GetWIds();
  int WIds2=SpV2->GetWIds();
  // search for equal words in both documents
  double WordWgtProdSum=0; int IntsWords=0;
  int WIdN1=0; int WIdN2=0;
  while ((WIdN1<WIds1)&&(WIdN2<WIds2)){
    int WId1=SpV1->GetWId(WIdN1);
    int WId2=-1;
    forever {
      if (WIdN2>=WIds2){break;}
      WId2=SpV2->GetWId(WIdN2);
      if (WId2>=WId1){break;}
      WIdN2++;
    }
    if ((WIdN2<WIds2)&&(WId1==WId2)){
      double WordWgt1=SpV1->GetWgt(WIdN1); WIdN1++;
      double WordWgt2=SpV2->GetWgt(WIdN2); WIdN2++;
      double WordWgtProd=WordWgt1*WordWgt2;
      WordWgtProdSum+=WordWgtProd; IntsWords++;
    } else {
      WIdN1++;
    }
  }
  // return results
  double Norm1=SpV1->GetNorm();
  double Norm2=SpV2->GetNorm();
  double Sim;
  if (Norm1*Norm2==0){
    Sim=0;
  } else {
    Sim=WordWgtProdSum/(Norm1*Norm2);
  }
  return Sim;
}

double TBowSim::GetCosSim(
 const PBowSpV& SpV1, const PBowSpV& SpV2, TFltIntPrV& WgtWIdPrV) {
  // prepare shortcuts
  int WIds1=SpV1->GetWIds();
  int WIds2=SpV2->GetWIds();
  double WordWgtProdSum=0; int IntsWords=0;
  int WIdN1=0; int WIdN2=0; WgtWIdPrV.Clr();
  while ((WIdN1<WIds1)&&(WIdN2<WIds2)){
    int WId1=SpV1->GetWId(WIdN1);
    int WId2=-1;
    forever {
      if (WIdN2>=WIds2){break;}
      WId2=SpV2->GetWId(WIdN2);
      if (WId2>=WId1){break;}
      WIdN2++;
    }
    if ((WIdN2<WIds2)&&(WId1==WId2)){
      double WordWgt1=SpV1->GetWgt(WIdN1); WIdN1++;
      double WordWgt2=SpV2->GetWgt(WIdN2); WIdN2++;
      double WordWgtProd=WordWgt1*WordWgt2;
      WordWgtProdSum+=WordWgtProd; IntsWords++;
      WgtWIdPrV.Add(TFltIntPr(WordWgtProd, WId1));
    } else {
      WIdN1++;
    }
  }
  double Norm1=SpV1->GetNorm();
  double Norm2=SpV2->GetNorm();
  double Sim;
  if (Norm1*Norm2==0){
    Sim=0;
  } else {
    Sim=WordWgtProdSum/(Norm1*Norm2);
  }
  return Sim;
}

TBowSimType TBowSim::GetSimType(const TStr& Nm){
  TStr UcNm=Nm.GetUc();
  if (UcNm=="UNDEF"){return bstUndef;}
  else if (UcNm=="BLOCK"){return bstBlock;}
  else if (UcNm=="EUCL"){return bstEucl;}
  else if (UcNm=="COS"){return bstCos;}
  else if (UcNm=="MTX"){return bstMtx;}
  else {return bstUndef;}
}

PBowDocWgtBs TBowDocWgtBs::New(
 const PBowDocBs& BowDocBs, const TBowWordWgtType& _WordWgtType,
 const double& _CutWordWgtSumPrc, const int& _MnWordFq,
 const TIntV& _DIdV, const TIntV& _BaseDIdV, const THashSet<TInt>& IgnoreWIds, const PNotify& Notify){
  PBowDocWgtBs DocWgtBs=TBowDocWgtBs::New(BowDocBs->GetSig());
  DocWgtBs->WordWgtType=_WordWgtType;
  DocWgtBs->CutWordWgtSumPrc=_CutWordWgtSumPrc;
  DocWgtBs->MnWordFq=_MnWordFq;
  if (_DIdV.Empty()){BowDocBs->GetAllDIdV(DocWgtBs->DIdV);}
  else {DocWgtBs->DIdV=_DIdV;}
  int Docs=DocWgtBs->GetDocs();
  int AllDocs=BowDocBs->GetDocs();
  int AllWords=BowDocBs->GetWords();
  if ((DocWgtBs->WordWgtType==bwwtEq)||(DocWgtBs->WordWgtType==bwwtNrmEq)){
    DocWgtBs->WordFqV.Gen(AllWords);
    DocWgtBs->DocSpVV.Gen(AllDocs);
    for (int DIdN=0; DIdN<Docs; DIdN++){
      TNotify::OnNotify(Notify, ntInfo, TStr("Computing weights (")+TInt::GetStr(DIdN+1)+"/"+TInt::GetStr(Docs)+")...");
      int DId=DocWgtBs->GetDId(DIdN);
      int DocWIds=BowDocBs->GetDocWIds(DId);
      DocWgtBs->DocSpVV[DId]=BowDocBs->GetDocSpV(DId);
      for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
        int DocWId; double DocWordFq;
        BowDocBs->GetDocWIdFq(DId, DocWIdN, DocWId, DocWordFq);
		if (IgnoreWIds.IsKey(DocWId))		// skip the words that we want to ignore
			continue;
        DocWgtBs->WordFqV[DocWId]+=DocWordFq;
      }
      if (DocWgtBs->WordWgtType==bwwtNrmEq) {
        DocWgtBs->DocSpVV[DId]->PutUnitNorm();
      }
    }
  } else if ((DocWgtBs->WordWgtType==bwwtBin)||(DocWgtBs->WordWgtType==bwwtNrmBin)) {
    DocWgtBs->WordFqV.Gen(AllWords);
    DocWgtBs->DocSpVV.Gen(AllDocs);
    for (int DIdN=0; DIdN<Docs; DIdN++){
      TNotify::OnNotify(Notify, ntInfo, TStr("Computing weights (")+TInt::GetStr(DIdN+1)+"/"+TInt::GetStr(Docs)+")...");
      int DId=DocWgtBs->GetDId(DIdN);
      int DocWIds=BowDocBs->GetDocWIds(DId);
      DocWgtBs->DocSpVV[DId]=TBowSpV::New(DId, DocWIds);
      for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
        int DocWId; double DocWordFq;
        BowDocBs->GetDocWIdFq(DId, DocWIdN, DocWId, DocWordFq);
		if (IgnoreWIds.IsKey(DocWId))		
			continue;
        DocWgtBs->WordFqV[DocWId]+=DocWordFq;
        DocWgtBs->DocSpVV[DId]->AddWIdWgt(DocWId, 1.0);
      }
      if (DocWgtBs->WordWgtType==bwwtNrmBin) {
        DocWgtBs->DocSpVV[DId]->PutUnitNorm();
      }
    }
  } else if (DocWgtBs->WordWgtType==bwwtNrm01){
    DocWgtBs->WordFqV.Gen(AllWords);
    DocWgtBs->DocSpVV.Gen(AllDocs);
    for (int DIdN=0; DIdN<Docs; DIdN++){
      TNotify::OnNotify(Notify, ntInfo, TStr("Computing weights (")+TInt::GetStr(DIdN+1)+"/"+TInt::GetStr(Docs)+")...");
      int DId=DocWgtBs->GetDId(DIdN);
      int DocWIds=BowDocBs->GetDocWIds(DId);
      DocWgtBs->DocSpVV[DId]=TBowSpV::New(DId, DocWIds);
      for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
        int DocWId; double DocWordFq;
        BowDocBs->GetDocWIdFq(DId, DocWIdN, DocWId, DocWordFq);
		if (IgnoreWIds.IsKey(DocWId))		
			continue;
        DocWgtBs->WordFqV[DocWId]+=DocWordFq;
        double MnWIdFq=BowDocBs->GetWordMnVal(DocWId);
        double MxWIdFq=BowDocBs->GetWordMxVal(DocWId);
        if (MnWIdFq!=MxWIdFq){
          double DocWordWgt=(DocWordFq-MnWIdFq)/(MxWIdFq-MnWIdFq);
          DocWgtBs->DocSpVV[DId]->AddWIdWgt(DocWId, DocWordWgt);
        }
      }
    }
  } else if (
   (DocWgtBs->WordWgtType==bwwtNrmTFIDF)||
   (DocWgtBs->WordWgtType==bwwtLogDFNrmTFIDF)){
    TIntV BaseDIdV;
    if (_BaseDIdV.Empty()){BaseDIdV=DocWgtBs->DIdV;}
    else {BaseDIdV=_BaseDIdV;}
    int BaseDocs=BaseDIdV.Len();
    int Words=BowDocBs->GetWords();
    DocWgtBs->WordFqV.Gen(Words);
    for (int DIdN=0; DIdN<BaseDocs; DIdN++){
      TNotify::OnNotify(Notify, ntInfo, TStr("Computing weights (")+TInt::GetStr(DIdN+1)+"/"+TInt::GetStr(BaseDocs)+")...");
      int DId=BaseDIdV[DIdN];
      int DocWIds=BowDocBs->GetDocWIds(DId);
      for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
        int DocWId; double DocWordFq;
        BowDocBs->GetDocWIdFq(DId, DocWIdN, DocWId, DocWordFq);
        DocWgtBs->WordFqV[DocWId]++;
      }
    }
    TFltV WordIDFV(Words);
    for (int WId=0; WId<Words; WId++){
      if (WId%100==0){
        TNotify::OnNotify(Notify, ntInfo, TStr("Computing IDF values (")+TInt::GetStr(WId+1)+"/"+TInt::GetStr(Words)+")...");}
      double WordDf=DocWgtBs->WordFqV[WId];
      if (WordDf>0){
        WordIDFV[WId]=log(double(Docs)/WordDf);}
    }
    DocWgtBs->DocSpVV.Gen(AllDocs);
    {for (int DIdN=0; DIdN<Docs; DIdN++){
      TNotify::OnNotify(Notify, ntInfo, TStr("Computing weights (")+TInt::GetStr(DIdN+1)+"/"+TInt::GetStr(BaseDocs)+")...");
      int DId=DocWgtBs->GetDId(DIdN);
      int DocWIds=BowDocBs->GetDocWIds(DId);
      PBowSpV DocSpV=TBowSpV::New(DId, DocWIds);
      DocWgtBs->DocSpVV[DId]=DocSpV;
      // calculate & add words weights
      for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
        int DocWId; double DocWordFq;
        BowDocBs->GetDocWIdFq(DId, DocWIdN, DocWId, DocWordFq);
		if (IgnoreWIds.IsKey(DocWId))		// skip the words that we want to ignore
			continue;
        if (BowDocBs->GetWordFq(DocWId)>=_MnWordFq){
          double DocWordWgt=DocWordFq*WordIDFV[DocWId];
          if (DocWgtBs->WordWgtType==bwwtLogDFNrmTFIDF){
            double WordDf=DocWgtBs->WordFqV[DocWId];
            DocWordWgt=log(1+WordDf)*DocWordWgt;
          }
          DocSpV->AddWIdWgt(DocWId, DocWordWgt);
        }
      }
      DocSpV->CutLowWgtWords(_CutWordWgtSumPrc);
      DocSpV->PutUnitNorm();
    }}
  } else if (DocWgtBs->WordWgtType==bwwtNrmTFICF){
    TIntV BaseDIdV;
    if (_BaseDIdV.Empty()){BaseDIdV=DocWgtBs->DIdV;}
    else {BaseDIdV=_BaseDIdV;}
    const int BaseDocs=BaseDIdV.Len();
    const int Words=BowDocBs->GetWords();
    TVec<TIntH> WordCIdHV(Words, 0);
    for (int WId = 0; WId < Words; WId++) { WordCIdHV.Add(TIntH()); }
    for (int DIdN=0; DIdN<BaseDocs; DIdN++){
      const int DId=BaseDIdV[DIdN];
      const int DocWIds=BowDocBs->GetDocWIds(DId);
      const int DocCIds = BowDocBs->GetDocCIds(DId);
      for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
        int DocWId; double DocWordFq;
        BowDocBs->GetDocWIdFq(DId, DocWIdN, DocWId, DocWordFq);
		if (IgnoreWIds.IsKey(DocWId))		// skip the words that we want to ignore
			continue;
        for (int DocCIdN = 0; DocCIdN < DocCIds; DocCIdN++) {
            const int DocCId = BowDocBs->GetDocCId(DId, DocCIdN);
            if (!WordCIdHV[DocWId].IsKey(DocCId)) {
                WordCIdHV[DocWId].AddKey(DocCId);
            }
        }
      }
    }
    const int Cats=BowDocBs->GetCats();
    TFltV WordICFV(Words);
    for (int WId=0; WId<Words; WId++){
      double WordCf=WordCIdHV[WId].Len();
      if (WordCf>0){
        WordICFV[WId]=log(double(Cats)/WordCf);
      } else {
        WordICFV[WId]=0.0;
      }
    }
    DocWgtBs->WordFqV.Gen(Words);
    DocWgtBs->DocSpVV.Gen(AllDocs);
    {for (int DIdN=0; DIdN<Docs; DIdN++){
      int DId=DocWgtBs->GetDId(DIdN);
      int DocWIds=BowDocBs->GetDocWIds(DId);
      PBowSpV DocSpV=TBowSpV::New(DId, DocWIds);
      DocWgtBs->DocSpVV[DId]=DocSpV;
      for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
        int DocWId; double DocWordFq;
        BowDocBs->GetDocWIdFq(DId, DocWIdN, DocWId, DocWordFq);
		if (IgnoreWIds.IsKey(DocWId))		// skip the words that we wan't to ignore
			continue;
        DocWgtBs->WordFqV[DocWId]+=DocWordFq;
        TStr WordStr=BowDocBs->GetWordStr(DocWId); // for debugging
        if (BowDocBs->GetWordFq(DocWId)>=_MnWordFq){
          double DocWordWgt=DocWordFq*WordICFV[DocWId];
          DocSpV->AddWIdWgt(DocWId, DocWordWgt);
        }
      }
      DocSpV->CutLowWgtWords(_CutWordWgtSumPrc);
      DocSpV->PutUnitNorm();
    }}
  } else {
    Fail;
  }
  return DocWgtBs;
}

PBowDocWgtBs TBowDocWgtBs::New(const TVec<PBowSpV>& BowSpVV) {
    PBowDocWgtBs DocWgtBs=TBowDocWgtBs::New(0);
    DocWgtBs->WordWgtType=bwwtPreCalc;
    DocWgtBs->CutWordWgtSumPrc=0.0;
    DocWgtBs->MnWordFq=0;
    int AllWords=0; DocWgtBs->DIdV.Gen(BowSpVV.Len(), 0);
    for (int DocN = 0; DocN < BowSpVV.Len(); DocN++) {
        DocWgtBs->DIdV.Add(BowSpVV[DocN]->GetDId());
        AllWords = TInt::GetMx(AllWords, BowSpVV[DocN]->GetLastWId());
    }
    DocWgtBs->WordFqV.Gen(AllWords); 
    DocWgtBs->WordFqV.PutAll(0.0);
    DocWgtBs->DocSpVV = BowSpVV;
    return DocWgtBs;
}

PBowDocWgtBs TBowDocWgtBs::NewPreCalcWgt(const PBowDocBs& BowDocBs,
 const TFltV& WordWgtV, const bool& PutUniteNorm,
 const double& _CutWordWgtSumPrc, const int& _MnWordFq,
 const TIntV& _DIdV) {
    PBowDocWgtBs DocWgtBs=TBowDocWgtBs::New(BowDocBs->GetSig());
    DocWgtBs->WordWgtType=bwwtPreCalc;
    DocWgtBs->CutWordWgtSumPrc=_CutWordWgtSumPrc;
    DocWgtBs->MnWordFq=_MnWordFq;
    if (_DIdV.Empty()){BowDocBs->GetAllDIdV(DocWgtBs->DIdV);}
    else {DocWgtBs->DIdV=_DIdV;}
    int Docs=DocWgtBs->GetDocs();
    int AllDocs=BowDocBs->GetDocs();
    int AllWords=BowDocBs->GetWords();
    EAssert(AllWords == WordWgtV.Len());
    DocWgtBs->WordFqV.Gen(AllWords);
    DocWgtBs->DocSpVV.Gen(AllDocs);
    for (int DIdN=0; DIdN<Docs; DIdN++) {
        int DId=DocWgtBs->GetDId(DIdN);
        int DocWIds=BowDocBs->GetDocWIds(DId);
        PBowSpV DocSpV=TBowSpV::New(DId, DocWIds);
        DocWgtBs->DocSpVV[DId]=DocSpV;
        for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
            int DocWId; double DocWordFq;
            BowDocBs->GetDocWIdFq(DId, DocWIdN, DocWId, DocWordFq);
            TStr WordStr=BowDocBs->GetWordStr(DocWId); // for debugging
            if (BowDocBs->GetWordFq(DocWId)>=_MnWordFq){
                DocSpV->AddWIdWgt(DocWId, DocWordFq*WordWgtV[DocWId]);
            }
            DocWgtBs->WordFqV[DocWId] += DocWordFq;
        }
        DocSpV->CutLowWgtWords(_CutWordWgtSumPrc);
        if (PutUniteNorm) { DocSpV->PutUnitNorm(); }
    }
    return DocWgtBs;
}

PBowDocWgtBs TBowDocWgtBs::NewSvmWgt(
   const PBowDocBs& BowDocBs,
   const PBowDocWgtBs& BowDocWgtBs,
   const TIntV& _TrainDIdV,
   const double& SvmCostParam,
   const int& MxTimePerCat,
   const bool& NegFeaturesP,
   const TIntV& _DIdV,
   const bool& PutUniteNormP,
   const double& _CutWordWgtSumPrc,
   const int& _MnWordFq) {

    PBowDocWgtBs DocWgtBs=TBowDocWgtBs::New(BowDocBs->GetSig());
    DocWgtBs->WordWgtType=bwwtSvm;
    DocWgtBs->CutWordWgtSumPrc=_CutWordWgtSumPrc;
    DocWgtBs->MnWordFq=_MnWordFq;
    if (_DIdV.Empty()){BowDocBs->GetAllDIdV(DocWgtBs->DIdV);}
    else {DocWgtBs->DIdV=_DIdV;}
    const int Docs=DocWgtBs->DIdV.Len();
    const int AllDocs=BowDocBs->GetDocs();
    const int AllWords=BowDocBs->GetWords();
    EAssert(BowDocBs->IsCats());
    const int Cats = BowDocBs->GetCats();

    TIntV TrainDIdV;
    if (_TrainDIdV.Empty()){
        TrainDIdV.Gen(BowDocWgtBs->GetDocs(), 0);
        for (int DIdN = 0; DIdN < BowDocWgtBs->GetDocs(); DIdN++) {
            TrainDIdV.Add(BowDocWgtBs->GetDId(DIdN));
        }
    } else { TrainDIdV = TrainDIdV; }
    TVec<TFltV> CatWgtVV(Cats);
    for (int CId = 0; CId < Cats; CId++) {
        printf("Cat %d/%d\r", CId+1, Cats);
        PSVMTrainSet CatTrainSet = TBowDocBs2TrainSet::NewBowAllCat(
            BowDocBs, BowDocWgtBs, CId, TrainDIdV);
        TIntFltKdV PPCatWIdWgtV, NNCatWIdWgtV;
        if (CatTrainSet->HasPosNegVecs(5)) {
            const double SvmUnbalanceParam = 5.0;
            PSVMModel CatSvmModel = TSVMModel::NewClsLinear(
                CatTrainSet, SvmCostParam, SvmUnbalanceParam,
                TIntV(), TSVMLearnParam::Lin(MxTimePerCat));
            TFltV NormalV; CatSvmModel->GetWgtV(NormalV);
            CatTrainSet->GetKeywords(NormalV, PPCatWIdWgtV, TIntV(), -1, 1.0, 1.0, true);
            CatTrainSet->GetKeywords(NormalV, NNCatWIdWgtV, TIntV(), -1, -1.0, -1.0, true);
            TLinAlg::NormalizeLinf(PPCatWIdWgtV); TLinAlg::NormalizeLinf(NNCatWIdWgtV);
        }
        TFltV& CatWgtV = CatWgtVV[CId];
        CatWgtV.Gen(AllWords); CatWgtV.PutAll(0.0);

        for (int WdN = 0; WdN < PPCatWIdWgtV.Len(); WdN++) {
            const int CatWId = PPCatWIdWgtV[WdN].Key;
            const double CatWordWgt = pow(2*PPCatWIdWgtV[WdN].Dat+1.0, 1.0/4.0);
            CatWgtV[CatWId] += CatWordWgt;
        }
        if (NegFeaturesP) {
            for (int WdN = 0; WdN < NNCatWIdWgtV.Len(); WdN++) {
                const int CatWId = NNCatWIdWgtV[WdN].Key;
                const double CatWordWgt = pow(2*NNCatWIdWgtV[WdN].Dat+1.0, 1.0/4.0);
                CatWgtV[CatWId] += CatWordWgt;
            }
        }
        TLinAlg::Normalize(CatWgtV);
    }
    printf("\n");

    DocWgtBs->WordFqV.Gen(AllWords);
    DocWgtBs->DocSpVV.Gen(AllDocs);
    for (int DIdN=0; DIdN<Docs; DIdN++) {
        printf("Doc %d/%d\r", DIdN+1, Docs);
        const int DId=DocWgtBs->GetDId(DIdN);
        const int DocWIds=BowDocBs->GetDocWIds(DId);
        PBowSpV DocSpV=TBowSpV::New(DId, DocWIds);
        DocWgtBs->DocSpVV[DId]=DocSpV;
        const int DocCIds = BowDocBs->GetDocCIds(DId);
        for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
            int DocWId; double DocWordFq;
            BowDocBs->GetDocWIdFq(DId, DocWIdN, DocWId, DocWordFq);
            TStr WordStr=BowDocBs->GetWordStr(DocWId); // for debugging
            if (BowDocBs->GetWordFq(DocWId)>=_MnWordFq){
                double WordWgt = 0.0;
                for (int DocCatN = 0; DocCatN < DocCIds; DocCatN++) {
                    const int DocCatId = BowDocBs->GetDocCId(DId, DocCatN);
                    WordWgt += CatWgtVV[DocCatId][DocWId];
                }
                DocSpV->AddWIdWgt(DocWId, DocWordFq*WordWgt);
            }
            DocWgtBs->WordFqV[DocWId] += DocWordFq;
        }
        DocSpV->CutLowWgtWords(_CutWordWgtSumPrc);
        if (PutUniteNormP) { DocSpV->PutUnitNorm(); }
    }
    printf("\n");
    return DocWgtBs;
}

PBowDocWgtBs TBowDocWgtBs::NewBinSvmWgt(
   const PBowDocBs& BowDocBs,
   const PBowDocWgtBs& BowDocWgtBs,
   const TStr& CatNm,
   const TIntV& TrainDIdV,
   const double& SvmCostParam,
   const double& SvmUnbalanceParam,
   const double& MnWgt,
   const bool& NegFeaturesP,
   const bool& PutUniteNormP,
   const bool& AvgNormalP,
   const TIntV& _DIdV,
   const double& _CutWordWgtSumPrc,
   const int& _MnWordFq) {

    PBowDocWgtBs DocWgtBs=TBowDocWgtBs::New(BowDocBs->GetSig());
    DocWgtBs->WordWgtType=bwwtSvm;
    DocWgtBs->CutWordWgtSumPrc=_CutWordWgtSumPrc;
    DocWgtBs->MnWordFq=_MnWordFq;
    if (_DIdV.Empty()){BowDocBs->GetAllDIdV(DocWgtBs->DIdV);}
    else {DocWgtBs->DIdV=_DIdV;}
    const int Docs=DocWgtBs->DIdV.Len();
    const int AllDocs=BowDocBs->GetDocs();
    const int AllWords=BowDocBs->GetWords();
    EAssert(BowDocBs->IsCatNm(CatNm));
    const int CId = BowDocBs->GetCId(CatNm);

    PSVMTrainSet CatTrainSet = TBowDocBs2TrainSet::NewBowAllCat(
        BowDocBs, BowDocWgtBs, CId, TrainDIdV);
    TIntFltKdV PPCatWIdWgtV, NNCatWIdWgtV;
    EAssert(CatTrainSet->HasPosNegVecs(5));
    PSVMModel CatSvmModel = TSVMModel::NewClsLinear(
        CatTrainSet, SvmCostParam, SvmUnbalanceParam);
    TFltV NormalV; CatSvmModel->GetWgtV(NormalV);
    CatTrainSet->GetKeywords(NormalV, PPCatWIdWgtV, TIntV(), -1, 1.0, 1.0, AvgNormalP);
    CatTrainSet->GetKeywords(NormalV, NNCatWIdWgtV, TIntV(), -1, -1.0, -1.0, AvgNormalP);
    TLinAlg::NormalizeLinf(PPCatWIdWgtV); TLinAlg::NormalizeLinf(NNCatWIdWgtV);
    // save calculated word weights
    TFltV CatWgtV; CatWgtV.Gen(AllWords); CatWgtV.PutAll(MnWgt);
    for (int WdN = 0; WdN < PPCatWIdWgtV.Len(); WdN++) {
        const int CatWId = PPCatWIdWgtV[WdN].Key;
        const double CatWordWgt = pow(2*PPCatWIdWgtV[WdN].Dat+1.0, 1.0/4.0);
        CatWgtV[CatWId] += CatWordWgt;
    }
    if (NegFeaturesP) {
        for (int WdN = 0; WdN < NNCatWIdWgtV.Len(); WdN++) {
            const int CatWId = NNCatWIdWgtV[WdN].Key;
            const double CatWordWgt = pow(2*NNCatWIdWgtV[WdN].Dat+1.0, 1.0/4.0);
            CatWgtV[CatWId] += CatWordWgt;
        }
    }

    DocWgtBs->WordFqV.Gen(AllWords);
    DocWgtBs->DocSpVV.Gen(AllDocs);
    for (int DIdN=0; DIdN<Docs; DIdN++) {
        const int DId=DocWgtBs->GetDId(DIdN);
        const int DocWIds=BowDocBs->GetDocWIds(DId);
        
        PBowSpV DocSpV=TBowSpV::New(DId, DocWIds);
        DocWgtBs->DocSpVV[DId]=DocSpV;
        for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
            int DocWId; double DocWordFq;
            BowDocBs->GetDocWIdFq(DId, DocWIdN, DocWId, DocWordFq);
            TStr WordStr=BowDocBs->GetWordStr(DocWId); // for debugging
            const double WordWgt = CatWgtV[DocWId];
            DocSpV->AddWIdWgt(DocWId, DocWordFq*WordWgt);
            DocWgtBs->WordFqV[DocWId] += DocWordFq;
        }
        
        DocSpV->CutLowWgtWords(_CutWordWgtSumPrc);
        
        if (PutUniteNormP) { DocSpV->PutUnitNorm(); }
    }
    
    return DocWgtBs;
}

TBowWordWgtType TBowDocWgtBs::GetWordWgtTypeFromStr(const TStr& Nm){
  TStr UcNm=Nm.GetUc();
  if (UcNm=="UNDEF"){return bwwtUndef;}
  else if (UcNm=="EQ"){return bwwtEq;}
  else if (UcNm=="NRMEQ"){return bwwtNrmEq;}
  else if (UcNm=="BIN"){return bwwtNrmBin;}
  else if (UcNm=="NRM01"){return bwwtNrm01;}
  else if (UcNm=="NRMTFIDF"){return bwwtNrmTFIDF;}
  else if (UcNm=="LOGDFNRMTFIDF"){return bwwtLogDFNrmTFIDF;}
  else if (UcNm=="NRMTFICF"){return bwwtNrmTFICF;}
  else if (UcNm=="PRECALC"){return bwwtPreCalc;}
  else if (UcNm=="SVM"){return bwwtSvm;}
  else {return bwwtUndef;}
}

void TBowDocWgtBs::GetSimDIdV(
 const PBowSpV& RefBowSpV, const PBowSim& BowSim,
 TFltIntKdV& SimDIdKdV, const bool& RefBowSpVInclude) const {
  int Docs=GetDocs();
  SimDIdKdV.Gen(Docs, 0);
  for (int DIdN=0; DIdN<Docs; DIdN++){
    int DId=GetDId(DIdN);
    PBowSpV DocBowSpV=GetSpV(DId);
    //B: dodan (RefBowSpVInclude)|| na zacetek if-a
    if ((RefBowSpVInclude)||(!RefBowSpV->IsDId())||(RefBowSpV->GetDId()!=DocBowSpV->GetDId())){
      double Sim=BowSim->GetSim(RefBowSpV, DocBowSpV);
      SimDIdKdV.Add(TFltIntKd(Sim, DId));
    }
  }
  SimDIdKdV.Sort(false);
}

void TBowDocWgtBs::SaveTxtSimDIdV(
 const PSOut& SOut, const PBowDocBs& BowDocBs,
 const PBowSpV& RefBowSpV, const TFltIntKdV& SimDIdKdV,
 const int& TopHits, const double& MnSim, const int& TopDocWords,
 const char& SepCh) const {
  
  SOut->PutStr("Query Document:\n");
  if (RefBowSpV->IsDId()){
    int RefDId=RefBowSpV->GetDId();
    TStr RefDocNm=BowDocBs->GetDocNm(RefDId);
    SOut->PutStr(RefDocNm, "   Document '%s'");
    SOut->PutLn();
  }
  RefBowSpV->SaveTxt(SOut, BowDocBs, TopDocWords, SepCh);
  SOut->PutLn();
  // output result-set
  int OutDocs=TopHits;
  if ((OutDocs==-1)||(OutDocs>=SimDIdKdV.Len())){OutDocs=SimDIdKdV.Len();}
  for (int OutDocN=0; OutDocN<OutDocs; OutDocN++){
    
    double Sim=SimDIdKdV[OutDocN].Key;
    int DId=SimDIdKdV[OutDocN].Dat;
    TStr DocNm=BowDocBs->GetDocNm(DId);
    PBowSpV DocSpV=GetSpV(DId);
    if (Sim<MnSim){break;}
    
    SOut->PutInt(OutDocN+1, "%d.");
    SOut->PutStr(DocNm, "   Document '%s'");
    SOut->PutFlt(Sim, "   Similarity: %g");
    SOut->PutLn();
    DocSpV->SaveTxt(SOut, BowDocBs, TopDocWords, SepCh);
    SOut->PutLn();
  }
}

void TBowDocWgtBs::SaveXmlSimDIdV(
 const PSOut& SOut, const PBowDocBs& BowDocBs,
 const PBowSpV& RefBowSpV, const TFltIntKdV& SimDIdKdV,
 const int& TopHits, const double& MnSim) const {
  SOut->PutStr("<SimilaritySearchResults>");
  
  SOut->PutStr("<QueryDocument");
  if (RefBowSpV->IsDId()){
    int RefDId=RefBowSpV->GetDId();
    TStr RefDocNm=BowDocBs->GetDocNm(RefDId);
    SOut->PutStr(TInt::GetStr(RefDId, "  DocId=\"%d\""));
    SOut->PutStr(TStr::GetStr(RefDocNm, "  DocNm=\"%s\""));
  }
  SOut->PutStr(">"); SOut->PutLn();
  RefBowSpV->SaveXml(SOut, BowDocBs);
  SOut->PutStr("</QueryDocument>"); SOut->PutLn();
  
  int OutDocs=TopHits;
  if ((OutDocs==-1)||(OutDocs>=SimDIdKdV.Len())){OutDocs=SimDIdKdV.Len();}
  for (int OutDocN=0; OutDocN<OutDocs; OutDocN++){
    
    double Sim=SimDIdKdV[OutDocN].Key;
    int DId=SimDIdKdV[OutDocN].Dat;
    TStr DocNm=BowDocBs->GetDocNm(DId);
    PBowSpV DocSpV=GetSpV(DId);
    if (Sim<MnSim){break;}
    
    SOut->PutStr("<Hit");
    SOut->PutStr(TInt::GetStr(OutDocN+1, " Num=\"%d\""));
    SOut->PutStr(TInt::GetStr(DId, "  DocId=\"%d\""));
    SOut->PutStr(TStr::GetStr(DocNm, "  DocNm=\"%s\""));
    SOut->PutStr(TFlt::GetStr(Sim, "  Sim=\"%g\""));
    SOut->PutStr(">"); SOut->PutLn();
    DocSpV->SaveXml(SOut, BowDocBs);
    SOut->PutStr("</Hit>"); SOut->PutLn();
  }
  SOut->PutStr("</SimilaritySearchResults>");
}

void TBowDocWgtBs::SaveTxtStat(
 const TStr& StatFNm, const PBowDocBs& BowDocBs,
 const bool& SaveWordsP, const bool& SaveCatsP, const bool& SaveDocsP) const {
  
  TFOut StatSOut(StatFNm); FILE* fStat=StatSOut.GetFileId();
  
  if (SaveWordsP){
    int DIds=GetDocs();
    int WIds=GetWords();
    // collect word-weights
    TIntIntFltPrH WIdToWFqWWgtPrH(WIds);
    for (int DIdN=0; DIdN<DIds; DIdN++){
      int DId=GetDId(DIdN);
      PBowSpV DocSpV=GetSpV(DId);
      int DocWIds=DocSpV->GetWIds();
      for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
        int DocWId; double DocWgt;
        DocSpV->GetWIdWgt(DocWIdN, DocWId, DocWgt);
        TIntFltPr& WFqWWgtPr=WIdToWFqWWgtPrH.AddDat(DocWId);
        WFqWWgtPr.Val1++; WFqWWgtPr.Val2+=DocWgt;
      }
    }
    
    TFltIntPrV AvgWWgtWIdPrV(WIdToWFqWWgtPrH.Len(), 0);
    for (int WIdN=0; WIdN<WIdToWFqWWgtPrH.Len(); WIdN++){
      int WId=WIdToWFqWWgtPrH.GetKey(WIdN);
      TIntFltPr& WFqWWgtPr=WIdToWFqWWgtPrH.AddDat(WId);
      double AvgWWgt=0;
      if (WFqWWgtPr.Val1>0){AvgWWgt=WFqWWgtPr.Val2/WFqWWgtPr.Val1;}
      AvgWWgtWIdPrV.Add(TFltIntPr(AvgWWgt, WId));
    }
    AvgWWgtWIdPrV.Sort(false);
    
    fprintf(fStat, "\nRank Word-Weight Document-Frequency Word-String\n\n");
    {for (int WIdN=0; WIdN<AvgWWgtWIdPrV.Len(); WIdN++){
      double AvgWWgt=AvgWWgtWIdPrV[WIdN].Val1;
      int WId=AvgWWgtWIdPrV[WIdN].Val2;
      TStr WordStr=BowDocBs->GetWordStr(AvgWWgtWIdPrV[WIdN].Val2);
      double WordDFq=GetWordFq(WId);
      fprintf(fStat, "%d.\t%.3f\t%g\t'%s'\n",
       1+WIdN, AvgWWgt, WordDFq, WordStr.CStr());
    }}
  }
  
  if (SaveCatsP){}
  
  if (SaveDocsP){
    fprintf(fStat, "\n\nDocument-Statistics\n\n");
    int Docs=GetDocs();
    for (int DIdN=0; DIdN<Docs; DIdN++){
      int DId=GetDId(DIdN);
      TStr DocNm=BowDocBs->GetDocNm(DId);
      PBowSpV DocSpV=GetSpV(DId);
      int DocWIds=DocSpV->GetWIds();
      fprintf(fStat, "DId:%d   Name:'%s' (%d Words):", DId, DocNm.CStr(), DocWIds);
      for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
        int DocWId; double DocWgt;
        DocSpV->GetWIdWgt(DocWIdN, DocWId, DocWgt);
        TStr WordStr=BowDocBs->GetWordStr(DocWId);
        fprintf(fStat, " '%s':%.3f", WordStr.CStr(), DocWgt);
      }
      fprintf(fStat, "\n");
    }
  }
}

const TStr TBowDocWgtBs::BowDocWgtBsFExt=".Boww";


PBowDocBs TBowDocBs::New(
 const PSwSet& SwSet, const PStemmer& Stemmer, const PNGramBs& NGramBs){
  PBowDocBs BowDocBs=New();
  if (NGramBs.Empty()){
    BowDocBs->PutSwSet(SwSet); BowDocBs->PutStemmer(Stemmer);
  } else {
    BowDocBs->PutNGramBs(NGramBs);
  }
  return BowDocBs;
}

void TBowDocBs::AddDocs(const PBowDocBs& BowDocBs)
{
  
  for (int WordN = 0; WordN < BowDocBs->GetWords(); WordN++)
  {
    TStr WordStr = BowDocBs->GetWordStr(WordN);
    int WordId = -1;
    if (!this->IsWordStr(WordStr, WordId))
    {
      this->AddWordStr(WordStr);
    }
  }
  
  for (int DocId = 0; DocId < BowDocBs->GetDocs(); DocId++)
  {
    TIntFltPrV NewDoc;
    for (int WIdN = 0; WIdN < BowDocBs->GetDocWIds(DocId); WIdN++)
    {
      int WId = BowDocBs->GetDocWId(DocId, WIdN);
      double WFq = BowDocBs->GetDocWFq(DocId, WIdN);
      TStr WordStr = BowDocBs->GetWordStr(WId);
      int WIdCmn = this->GetWId(WordStr);
      NewDoc.Add(TIntFltPr(WIdCmn, WFq));
    }
    this->AddDoc("", TStrV(), NewDoc);
  }
  
}

void TBowDocBs::AssertOk() const {
  
  int Docs=GetDocs();
  for (int DId=0; DId<Docs; DId++){
    PBowSpV DocSpV=GetDocSpV(DId);
    IAssert(DId==DocSpV->GetDId());
    TStr DocNm=GetDocNm(DId);
    // check document words
    for (int WIdN=0; WIdN<DocSpV->GetWIds(); WIdN++){
      int WId; double Wgt; DocSpV->GetWIdWgt(WIdN, WId, Wgt);
      TStr WordStr=GetWordStr(WId);
    }
    // check document categories
    int CIds=GetDocCIds(DId);
    for (int CIdN=0; CIdN<CIds; CIdN++){
      int CId=GetDocCId(DId, CIdN);
      TStr CatNm=GetCatNm(CId);
    }
  }
  int TrainDocs=GetTrainDocs();
  for (int TrainDIdN=0; TrainDIdN<TrainDocs; TrainDIdN++){
    int TrainDId=GetTrainDId(TrainDIdN);
    PBowSpV TrainDocSpV=GetDocSpV(TrainDId);
  }
  int TestDocs=GetTestDocs();
  for (int TestDIdN=0; TestDIdN<TestDocs; TestDIdN++){
    int TestDId=GetTestDId(TestDIdN);
    PBowSpV TestDocSpV=GetDocSpV(TestDId);
  }
}

void TBowDocBs::GetWordStrVFromHtml(const TStr& HtmlStr, TStrV& WordStrV) const {
  WordStrV.Clr();
  if (!NGramBs.Empty()){
    NGramBs->GetNGramStrV(HtmlStr, WordStrV);
  } else {
    PSIn HtmlSIn=TStrIn::New(HtmlStr);
    THtmlLx HtmlLx(HtmlSIn);
    // traverse html string symbols
    while (HtmlLx.Sym!=hsyEof){
      if (HtmlLx.Sym==hsyStr){
        TStr WordStr=HtmlLx.UcChA;
        if ((SwSet.Empty())||(!SwSet->IsIn(WordStr))){
          if (!Stemmer.Empty()){
            WordStr=Stemmer->GetStem(WordStr);}
          WordStrV.Add(WordStr);
        }
      }
      HtmlLx.GetSym();
    }
  }
}

int TBowDocBs::AddDoc(const TStr& _DocNm,
 const TStrV& CatNmV, const TIntFltPrV& WIdWgtPrV){
  TStr DocNm=_DocNm;
  if (DocNm.Empty()){DocNm=TInt::GetStr(GetDocs());}
  int DId=-1;
  if (!DocNmToDescStrH.IsKey(DocNm, DId)){
    DId=DocNmToDescStrH.AddKey(DocNm);
    DocSpVV.Add(TBowSpV::New(DId)); IAssert(DId==DocSpVV.Len()-1);
    DocStrV.Add();
    DocCIdVV.Add(); IAssert(DId==DocCIdVV.Len()-1);
  }

  TIntV& DocCIdV=DocCIdVV[DId];
  DocCIdV.Gen(CatNmV.Len(), 0);
  for (int CatNmN=0; CatNmN<CatNmV.Len(); CatNmN++){
    int CId=CatNmToFqH.AddKey(CatNmV[CatNmN]);
    CatNmToFqH[CId]++; DocCIdV.Add(CId);
  }
  DocCIdV.Sort();

  TIntFltH DocWIdToWgtH;
  for (int WordN=0; WordN<WIdWgtPrV.Len(); WordN++){
    int WId=WIdWgtPrV[WordN].Val1;
    IAssert(IsWId(WId));
    DocWIdToWgtH.AddDat(WId, WIdWgtPrV[WordN].Val2);
  }
  PBowSpV DocSpV=DocSpVV[DId];
  DocSpV->GenMx(DocWIdToWgtH.Len());
  for (int DocWordN=0; DocWordN<DocWIdToWgtH.Len(); DocWordN++){
    int WId=DocWIdToWgtH.GetKey(DocWordN);
    double Wgt=DocWIdToWgtH[DocWordN];
    WordStrToDescH[WId].Fq++;
    DocSpV->AddWIdWgt(WId, Wgt);
  }
  DocSpV->Sort();

  return DId;
}

int TBowDocBs::AddDoc(const TStr& _DocNm,
 const TStrV& CatNmV, const TStrV& WordStrV, const TStr& DocStr){
  // create doc-id
  TStr DocNm=_DocNm;
  if (DocNm.Empty()){DocNm=TInt::GetStr(GetDocs());}
  int DId=-1;
  if (!DocNmToDescStrH.IsKey(DocNm, DId)){
    DId=DocNmToDescStrH.AddKey(DocNm);
    DocSpVV.Add(TBowSpV::New(DId)); IAssert(DId==DocSpVV.Len()-1);
    DocStrV.Add(DocStr);
    DocCIdVV.Add(); IAssert(DId==DocCIdVV.Len()-1);
  }

  TIntV& DocCIdV=DocCIdVV[DId];
  DocCIdV.Gen(CatNmV.Len(), 0);
  for (int CatNmN=0; CatNmN<CatNmV.Len(); CatNmN++){
    int CId=CatNmToFqH.AddKey(CatNmV[CatNmN]);
    CatNmToFqH[CId]++; DocCIdV.Add(CId);
  }
  DocCIdV.Sort();

  TStrIntH DocWordStrToFqH;
  for (int WordStrN=0; WordStrN<WordStrV.Len(); WordStrN++){
    DocWordStrToFqH.AddDat(WordStrV[WordStrN])++;}
  
  PBowSpV DocSpV=DocSpVV[DId];
  DocSpV->GenMx(DocWordStrToFqH.Len());
  for (int DocWordStrN=0; DocWordStrN<DocWordStrToFqH.Len(); DocWordStrN++){
    TStr WordStr=DocWordStrToFqH.GetKey(DocWordStrN);
    int Fq=DocWordStrToFqH[DocWordStrN];
    int WId=WordStrToDescH.AddKey(WordStr);
    WordStrToDescH[WId].Fq++;
    DocSpV->AddWIdWgt(WId, Fq);
  }
  DocSpV->Sort();

  
  return DId;
}

void TBowDocBs::DelDoc(const TStr& DocNm)
{
	int DId=-1;
	if (!DocNmToDescStrH.IsKey(DocNm, DId))
		return;
	
	
	DocCIdVV[DId].Clr();
	PBowSpV DocSpV=DocSpVV[DId];
  
	for (int WIdN = 0; WIdN < DocSpV->GetWIds(); WIdN++)
	{
		int WId = DocSpV->GetWId(WIdN);
		WordStrToDescH[WId].Fq--;
	}
	DocSpV->Clr();
}

int TBowDocBs::AppendDoc(const TStr& _DocNm, const TStrV& CatNmV, const TStrV& WordStrV, const TStr& DocStr)
{

	TStr DocNm=_DocNm;
	if (DocNm.Empty()){DocNm=TInt::GetStr(GetDocs());}
	int DId=-1;

	if (!DocNmToDescStrH.IsKey(DocNm, DId))
		return AddDoc(DocNm, CatNmV, WordStrV, DocStr);

	TIntV& DocCIdV=DocCIdVV[DId];
	DocCIdV.Gen(CatNmV.Len(), 0);
	for (int CatNmN=0; CatNmN<CatNmV.Len(); CatNmN++)
	{
		int CId=CatNmToFqH.AddKey(CatNmV[CatNmN]);
		CatNmToFqH[CId]++; DocCIdV.Add(CId);
	}
	DocCIdV.Sort();

	TStrIntH DocWordStrToFqH;
	for (int WordStrN=0; WordStrN<WordStrV.Len(); WordStrN++)
		DocWordStrToFqH.AddDat(WordStrV[WordStrN])++;
	
	PBowSpV DocSpV=DocSpVV[DId];
	for (int DocWordStrN=0; DocWordStrN<DocWordStrToFqH.Len(); DocWordStrN++)
	{
		TStr WordStr=DocWordStrToFqH.GetKey(DocWordStrN);
		int Fq=DocWordStrToFqH[DocWordStrN];
		int WId=WordStrToDescH.AddKey(WordStr);
		WordStrToDescH[WId].Fq++;
		if (DocSpV->IsWId(WId))
			DocSpV->IncreaseWIdWgt(WId, Fq);	
		else
			DocSpV->AddWIdWgt(WId, Fq);		
	}
	DocSpV->Sort();

	return DId;
}

int TBowDocBs::AppendDoc(const TStr& _DocNm, const TStrV& CatNmV, const TIntFltPrV& WIdWgtPrV)
{
	TStr DocNm=_DocNm;
	if (DocNm.Empty()){DocNm=TInt::GetStr(GetDocs());}
	int DId=-1;
	
	if (!DocNmToDescStrH.IsKey(DocNm, DId))
		return AddDoc(DocNm, CatNmV, WIdWgtPrV);

	TIntV& DocCIdV=DocCIdVV[DId];
	DocCIdV.Gen(CatNmV.Len(), 0);
	for (int CatNmN=0; CatNmN<CatNmV.Len(); CatNmN++)
	{
		int CId=CatNmToFqH.AddKey(CatNmV[CatNmN]);
		CatNmToFqH[CId]++; DocCIdV.Add(CId);
	}
	DocCIdV.Sort();

	PBowSpV DocSpV=DocSpVV[DId];
	for (int WordN=0; WordN<WIdWgtPrV.Len(); WordN++)
	{
		int WId = WIdWgtPrV[WordN].Val1;
		double Fq = WIdWgtPrV[WordN].Val2;
		WordStrToDescH[WId].Fq++;
		if (DocSpV->IsWId(WId))
			DocSpV->IncreaseWIdWgt(WId, Fq);		
		else
			DocSpV->AddWIdWgt(WId, Fq);			// for new WIds add them to the vector
	}
	DocSpV->Sort();

	// return doc-id
	return DId;
}

void TBowDocBs::AppendWord(const TStr& DocNm, const TStr& Word, const float Wgt)
{
	int DId=-1;
	
	if (!DocNmToDescStrH.IsKey(DocNm, DId))
		return;
	AppendWord(DId, Word, Wgt);
}

void TBowDocBs::AppendWord(const int DId, const TStr& Word, const float Wgt)
{
	PBowSpV DocSpV = DocSpVV[DId];
	int WId = WordStrToDescH.AddKey(Word);
	WordStrToDescH[WId].Fq++;
	if (DocSpV->IsWId(WId))
		DocSpV->IncreaseWIdWgt(WId, Wgt);		
	else
		DocSpV->AddWIdWgt(WId, Wgt);			
}


// NOTE: Categories are ignored
PBowDocBs TBowDocBs::GetMergedDocs(const TVec<TIntV>& DIdVV, const TStrV& DocNmV) const {
	PBowDocBs BowDocBs=TBowDocBs::New();
	BowDocBs->DocSpVV.Gen(DIdVV.Len(), 0);
	BowDocBs->DocCIdVV.Gen(DIdVV.Len(), 0);
	

	for (int N=0; N < GetWords(); N++)
		BowDocBs->AddWordStr(GetWordStr(N));

	int Groups=DIdVV.Len();
	for (int GroupIdN=0; GroupIdN<Groups; GroupIdN++)
	{
		TIntV DIdV = DIdVV[GroupIdN];
		TStr DocNm;
		if (DocNmV.Empty()){DocNm=TInt::GetStr(GetDocs());}
		else DocNm = DocNmV[GroupIdN];
		int NewDId = BowDocBs->DocNmToDescStrH.AddKey(DocNm);

		PBowSpV NewSpV=TBowSpV::New(NewDId);
		THash<TInt, double> WIdWgtH;
		for (int DIdN = 0; DIdN < DIdVV[GroupIdN].Len(); DIdN++)
		{
			int DId = DIdVV[GroupIdN][DIdN];
			PBowSpV DocSpV=DocSpVV[DId];
			for (int WIdN=0; WIdN < DocSpV->GetWIds(); WIdN++)
			{
				int WId; double Wgt;
				DocSpV->GetWIdWgt(WIdN, WId, Wgt);
				TStr Word = GetWordStr(WId);
				double ExWgt = 0;
				if (WIdWgtH.IsKey(WId))
					ExWgt = WIdWgtH.GetDat(WId);
				WIdWgtH.AddDat(WId, ExWgt + Wgt);
			}
		}
		for (int KeyId=0; KeyId < WIdWgtH.Len(); KeyId++)
		{
			TInt WId; double Wgt;
			WIdWgtH.GetKeyDat(KeyId, WId, Wgt);
			NewSpV->AddWIdWgt(WId, Wgt);
		}
		NewSpV->Sort();
		BowDocBs->DocSpVV.Add(NewSpV); IAssert(NewDId==BowDocBs->DocSpVV.Len()-1);
	}
	return BowDocBs;
}


int TBowDocBs::AddHtmlDoc(const TStr& DocNm, const TStrV& CatNmV,
 const TStr& HtmlDocStr, const bool& SaveDocP){
  TStrV WordStrV; GetWordStrVFromHtml(HtmlDocStr, WordStrV);
  int DocId;
  if (SaveDocP){DocId=AddDoc(DocNm, CatNmV, WordStrV, HtmlDocStr);}
  else {DocId=AddDoc(DocNm, CatNmV, WordStrV, "");}
  // return doc-id
  return DocId;
}

void TBowDocBs::GetAllDIdV(TIntV& DIdV) const {
  int Docs=GetDocs();
  DIdV.Gen(Docs);
  for (int DId=0; DId<Docs; DId++){
    DIdV[DId]=DId;}
}

bool TBowDocBs::IsDocWordStr(const int& DId, const TStr& WordStr) const {
  int WId;
  if (IsWordStr(WordStr, WId)){
    return DocSpVV[DId]->IsWId(WId);
  } else {
    return false;
  }
}

void TBowDocBs::SetCatToBowDIds(const TStr& CatNm, const TIntV& BowDIdV)
{
	int CId = AddCatNm(CatNm);
	for (int i=0; i < BowDIdV.Len(); i++)
		AddDocCId(BowDIdV[i], CId);
}

void TBowDocBs::RemoveCatFromBowDIds(const TStr& CatNm, TIntV& BowDIdV)
{
	int CId = GetCId(CatNm);
	for (int i=0; i < BowDIdV.Len(); i++)
		RemoveDocCId(BowDIdV[i], CId);
}

void TBowDocBs::GetTopCatV(const int& TopCats, TIntStrPrV& FqCatNmPrV) const {
  CatNmToFqH.GetDatKeyPrV(FqCatNmPrV);
  FqCatNmPrV.Sort(false);
  FqCatNmPrV.Trunc(TopCats);
}

void TBowDocBs::SetHOTrainTestDIdV(
 const double& TestDocsPrc, TRnd& Rnd){
  TIntV AllDIdV; GetAllDIdV(AllDIdV);
  AllDIdV.Shuffle(Rnd);
  int TestDocs=int(AllDIdV.Len()*TestDocsPrc);
  int MxTrainDIdN=AllDIdV.Len()-1-TestDocs;
  // get train doc-ids
  AllDIdV.GetSubValV(0, MxTrainDIdN, TrainDIdV);
  // get test doc-ids
  AllDIdV.GetSubValV(MxTrainDIdN+1, AllDIdV.Len()-1, TestDIdV);
}

void TBowDocBs::SetCVTrainTestDIdV(
 const int& Folds, const int& FoldN, TRnd& Rnd){
  TIntV AllDIdV; GetAllDIdV(AllDIdV);
  if (Folds==1){
    TrainDIdV=AllDIdV;
    TestDIdV=AllDIdV;
  } else {
    AllDIdV.Shuffle(Rnd);
    int MnTestDIdN=(FoldN*AllDIdV.Len())/Folds;
    int MxTestDIdN=(((FoldN+1)*AllDIdV.Len())/Folds)-1;
    // get train doc-ids
    AllDIdV.GetSubValV(0, MnTestDIdN-1, TrainDIdV);
    TIntV UpTrainDIdV;
    AllDIdV.GetSubValV(MxTestDIdN+1, AllDIdV.Len()-1, UpTrainDIdV);
    TrainDIdV.AddV(UpTrainDIdV);
    // get test doc-ids
    AllDIdV.GetSubValV(MnTestDIdN, MxTestDIdN, TestDIdV);
  }
}

PBowDocBs TBowDocBs::GetLimWordRelFqDocBs(
 const double& MnWordFqPrc, const double& MxWordFqPrc) const {
  PBowDocBs BowDocBs=TBowDocBs::New();

  BowDocBs->DocNmToDescStrH=DocNmToDescStrH;
  BowDocBs->CatNmToFqH=CatNmToFqH;
  BowDocBs->DocCIdVV=DocCIdVV;
  BowDocBs->TrainDIdV=TrainDIdV;
  BowDocBs->TestDIdV=TestDIdV;
  
  int Docs=GetDocs();
  int Words=GetWords();
  
  for (int WId=0; WId<Words; WId++){
    TStr WordStr=GetWordStr(WId);
    int WordFq=GetWordFq(WId);
    double WordFqPrc=double(WordFq)/double(Docs);
    if ((MnWordFqPrc<WordFqPrc)&&(WordFqPrc<MxWordFqPrc)){
      BowDocBs->WordStrToDescH.AddDat(WordStr).Fq=WordFq;
    }
  }
  
  for (int DId=0; DId<Docs; DId++){
    PBowSpV DocSpV=DocSpVV[DId]; int WIds=DocSpV->GetWIds();
    BowDocBs->DocSpVV.Add(TBowSpV::New(DId));
    for (int WIdN=0; WIdN<WIds; WIdN++){
      int WId; double WordFq; DocSpV->GetWIdWgt(WIdN, WId, WordFq);
      TStr WordStr=GetWordStr(WId);
      int NewWId;
      if (BowDocBs->IsWordStr(WordStr, NewWId)){
        BowDocBs->DocSpVV.Last()->AddWIdWgt(NewWId, WordFq);
      }
    }
    BowDocBs->DocSpVV.Last()->Trunc();
  }
  
  return BowDocBs;
}

PBowDocBs TBowDocBs::GetLimWordAbsFqDocBs(const int& MnWordFq) const {
  
  PBowDocBs BowDocBs=TBowDocBs::New();
  
  BowDocBs->DocNmToDescStrH=DocNmToDescStrH;
  BowDocBs->CatNmToFqH=CatNmToFqH;
  BowDocBs->DocCIdVV=DocCIdVV;
  BowDocBs->TrainDIdV=TrainDIdV;
  BowDocBs->TestDIdV=TestDIdV;
  
  int Docs=GetDocs();
  int Words=GetWords();
  
  for (int WId=0; WId<Words; WId++){
    TStr WordStr=GetWordStr(WId);
    int WordFq=GetWordFq(WId);
    if (MnWordFq<=WordFq){
      BowDocBs->WordStrToDescH.AddDat(WordStr).Fq=WordFq;
    }
  }
  
  for (int DId=0; DId<Docs; DId++){
    PBowSpV DocSpV=DocSpVV[DId]; int WIds=DocSpV->GetWIds();
    BowDocBs->DocSpVV.Add(TBowSpV::New(DId));
    for (int WIdN=0; WIdN<WIds; WIdN++){
      int WId; double WordFq; DocSpV->GetWIdWgt(WIdN, WId, WordFq);
      TStr WordStr=GetWordStr(WId);
      int NewWId;
      if (BowDocBs->IsWordStr(WordStr, NewWId)){
        BowDocBs->DocSpVV.Last()->AddWIdWgt(NewWId, WordFq);
      }
    }
    BowDocBs->DocSpVV.Last()->Trunc();
  }
  
  return BowDocBs;
}

PBowDocBs TBowDocBs::GetSubDocSet(const TIntV& DIdV) const {
  
  PBowDocBs BowDocBs=TBowDocBs::New();
  BowDocBs->DocSpVV.Gen(DIdV.Len(), 0);
  BowDocBs->DocCIdVV.Gen(DIdV.Len(), 0);
  
  int Docs=DIdV.Len();
  
  for (int DIdN=0; DIdN<Docs; DIdN++){
    int DId=DIdV[DIdN];
    TStr DocNm=DocNmToDescStrH.GetKey(DId);
    PBowSpV SpV=DocSpVV[DId]; int WIds=SpV->GetWIds();
    const TIntV& DocCIdV=DocCIdVV[DId];
    
    int NewDId=BowDocBs->DocNmToDescStrH.AddKey(DocNm);
    PBowSpV NewSpV=TBowSpV::New(NewDId, WIds);
    BowDocBs->DocSpVV.Add(NewSpV); IAssert(NewDId==BowDocBs->DocSpVV.Len()-1);
    for (int WIdN=0; WIdN<WIds; WIdN++){
      int WId; double WordFq; SpV->GetWIdWgt(WIdN, WId, WordFq);
      TStr WordStr=GetWordStr(WId);
      int NewWId=BowDocBs->WordStrToDescH.AddKey(WordStr);
      BowDocBs->WordStrToDescH[NewWId].Fq++;
      NewSpV->AddWIdWgt(NewWId, WordFq);
    }
    NewSpV->Sort();
    
    BowDocBs->DocCIdVV.Add(); IAssert(NewDId+1==BowDocBs->DocCIdVV.Len());
    BowDocBs->DocCIdVV.Last().Gen(DocCIdV.Len(), 0);
    for (int DocCIdN=0; DocCIdN<DocCIdV.Len(); DocCIdN++){
      int CId=DocCIdV[DocCIdN];
      TStr CatNm=GetCatNm(CId);
      const int NewCId=BowDocBs->CatNmToFqH.AddKey(CatNm);
      BowDocBs->CatNmToFqH[NewCId]++;
      BowDocBs->DocCIdVV.Last().Add(NewCId);
    }
  }
  
  return BowDocBs;
}

PBowDocBs TBowDocBs::GetInvDocBs() const {
  
  int Docs=GetDocs();
  int Words=GetWords();
  
  PBowDocBs InvBowDocBs=TBowDocBs::New();
  
  InvBowDocBs->WordStrToDescH.Gen(Docs);
  TIntV WordFqV(Words);
  for (int DId=0; DId<Docs; DId++){
    TStr DocNm=GetDocNm(DId);
    PBowSpV DocSpV=GetDocSpV(DId);
    int DocWIds=DocSpV->GetWIds();
    InvBowDocBs->WordStrToDescH.AddDat(DocNm)=TBowWordDesc(DocWIds, 1, 1);
    for (int WIdN=0; WIdN<DocWIds; WIdN++){
      int WId; double Wgt; DocSpV->GetWIdWgt(WIdN, WId, Wgt);
      WordFqV[WId]++;
    }
  }
  
  InvBowDocBs->DocNmToDescStrH.Gen(Words);
  InvBowDocBs->DocSpVV.Gen(Words, 0);
  InvBowDocBs->DocCIdVV.Gen(Words, 0);
  for (int WId=0; WId<Words; WId++){
    TStr WordStr=GetWordStr(WId);
    int InvDId=InvBowDocBs->DocNmToDescStrH.AddKey(WordStr); IAssert(WId==InvDId);
    int InvDocWIds=WordFqV[WId];
    PBowSpV BowSpV=TBowSpV::New(InvDId, InvDocWIds);
    InvBowDocBs->DocSpVV.Add(BowSpV);
  }
  
  {for (int DId=0; DId<Docs; DId++){
    PBowSpV DocSpV=GetDocSpV(DId);
    int DocWIds=DocSpV->GetWIds();
    for (int WIdN=0; WIdN<DocWIds; WIdN++){
      int WId; double Wgt; DocSpV->GetWIdWgt(WIdN, WId, Wgt);
      InvBowDocBs->DocSpVV[WId]->AddWIdWgt(DId, 1);
    }
  }}
  
  {for (int WId=0; WId<Words; WId++){
    InvBowDocBs->DocSpVV[WId]->Sort();
    IAssert(InvBowDocBs->DocSpVV[WId]->Len()==InvBowDocBs->DocSpVV[WId]->Reserved());
  }}
  
  InvBowDocBs->AssertOk();
  return InvBowDocBs;
}

PBowDocBs TBowDocBs::GetVocBs() const {
  PBowDocBs BowDocBs = TBowDocBs::New(SwSet, Stemmer, NGramBs);
  BowDocBs->Sig = Sig;
  BowDocBs->WordStrToDescH = WordStrToDescH;
  return BowDocBs;
}

void TBowDocBs::SaveTxtStat(const TStr& StatFNm,
 const bool& SaveWordsP, const bool& SaveCatsP, const bool& SaveDocsP) const {
  
  TFOut StatSOut(StatFNm); FILE* fStat=StatSOut.GetFileId();
  
  if (SaveWordsP){
    fprintf(fStat, "\nDocument-Frequency Word-Statistics\n\n");
    TBowWordDescStrPrV WordDescStrPrV;
    WordStrToDescH.GetDatKeyPrV(WordDescStrPrV);
    WordDescStrPrV.Sort(false);
    for (int WordN=0; WordN<WordDescStrPrV.Len(); WordN++){
      fprintf(fStat, "%d.\t%d\t'%s'\n",
       1+WordN, (int)WordDescStrPrV[WordN].Val1.Fq, WordDescStrPrV[WordN].Val2.CStr());
    }
  }
  
  if (SaveCatsP){
    fprintf(fStat, "\nCategory-Statistics\n\n");
    TIntStrPrV FqCatNmPrV; CatNmToFqH.GetDatKeyPrV(FqCatNmPrV);
    FqCatNmPrV.Sort(false);
    for (int FqCatNmPrN=0; FqCatNmPrN<FqCatNmPrV.Len(); FqCatNmPrN++){
      fprintf(fStat, "%d\t'%s'\n",
       (int)FqCatNmPrV[FqCatNmPrN].Val1, FqCatNmPrV[FqCatNmPrN].Val2.CStr());
    }
  }
  
  if (SaveDocsP){
    fprintf(fStat, "Document-Statistics\n\n");
    int Docs=GetDocs();
    for (int DId=0; DId<Docs; DId++){
      TStr DocNm=GetDocNm(DId);
      TStr DescStr=GetDocDescStr(DId);
      int DocWIds=GetDocWIds(DId);
      fprintf(fStat, "DId:%d   Name:'%s' Desc: '%s' (%d Words):",
       DId, DocNm.CStr(), DescStr.CStr(), DocWIds);
      int DocCIds=GetDocCIds(DId);
      if (DocCIds>0){
        fprintf(fStat, "Cats:[");
        for (int DocCIdN=0; DocCIdN<DocCIds; DocCIdN++){
          int CId=GetDocCId(DId, DocCIdN);
          TStr CatNm=GetCatNm(CId);
          if (DocCIdN>0){fprintf(fStat, ", ");}
          fprintf(fStat, "'%s'", CatNm.CStr());
        }
        fprintf(fStat, "] ");
      }
      for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
        int WId; double WordFq;
        GetDocWIdFq(DId, DocWIdN, WId, WordFq);
        TStr WordStr=GetWordStr(WId);
        
        fprintf(fStat, " '%s':%g", WordStr.CStr(), WordFq);
      }
      fprintf(fStat, "\n");
    }
  }
}

PBowSpV TBowDocBs::GetSpVFromHtmlStr(
 const TStr& HtmlStr, const PBowDocWgtBs& BowDocWgtBs) const {
  
  TStrV WordStrV; GetWordStrVFromHtml(HtmlStr, WordStrV);
  
  TIntH DocWIdToFqH(100);
  for (int WordStrN=0; WordStrN<WordStrV.Len(); WordStrN++){
    int WId;
    if (IsWordStr(WordStrV[WordStrN], WId)){
      DocWIdToFqH.AddDat(WId)++;
    }
  }
  
  PBowSpV DocSpV;
  if (BowDocWgtBs.Empty()){
    
    int DocWIds=DocWIdToFqH.Len();
    DocSpV=TBowSpV::New(-1, DocWIds);
    
    TInt DocWId; TInt DocWordFq;
    for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
      
      DocWIdToFqH.GetKeyDat(DocWIdN, DocWId, DocWordFq);
      DocSpV->AddWIdWgt(DocWId, DocWordFq);
    }
    DocSpV->Sort();
  } else {
    
    int MnWordFq=BowDocWgtBs->GetMnWordFq();
    int Docs=BowDocWgtBs->GetDocs();
    int DocWIds=DocWIdToFqH.Len();
    DocSpV=TBowSpV::New(-1, DocWIds);
    
    TInt DocWId; TInt DocWordFq;
    for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
      
      DocWIdToFqH.GetKeyDat(DocWIdN, DocWId, DocWordFq);
      
      if (GetWordFq(DocWId)>=MnWordFq){
        double WordDf=BowDocWgtBs->GetWordFq(DocWId);
        double WordIDf=0;
        if (WordDf>0){WordIDf=log(double(Docs)/WordDf);}
        double DocWordWgt=DocWordFq*WordIDf;
        DocSpV->AddWIdWgt(DocWId, DocWordWgt);
      }
    }
    DocSpV->Sort();
  
    DocSpV->CutLowWgtWords(BowDocWgtBs->GetCutWordWgtSumPrc());
    
    DocSpV->PutUnitNorm();
  }
  // return sparse vector
  return DocSpV;
}

PBowSpV TBowDocBs::GetSpVFromHtmlStr(const TStr& HtmlStr, 
        const int& Docs, const TFltV& WordFqV) const {
    TStrV WordStrV; GetWordStrVFromHtml(HtmlStr, WordStrV);
    
    TIntH DocWIdToFqH(100);
    for (int WordStrN=0; WordStrN<WordStrV.Len(); WordStrN++){
        int WId;
        if (IsWordStr(WordStrV[WordStrN], WId)){
            DocWIdToFqH.AddDat(WId)++;
        }
    }
  
    PBowSpV DocSpV;
    int DocWIds=DocWIdToFqH.Len();
    DocSpV=TBowSpV::New(-1, DocWIds);
    
    TInt DocWId; TInt DocWordFq;
    for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
        // get word id & freq.
        DocWIdToFqH.GetKeyDat(DocWIdN, DocWId, DocWordFq);
        // calculate & add TFIDF weight
        double WordDf = WordFqV[DocWId];
        double WordIDf = (WordDf>0) ? WordIDf=log(double(Docs)/WordDf) : 0.0;
        double DocWordWgt = DocWordFq*WordIDf;
        DocSpV->AddWIdWgt(DocWId, DocWordWgt);
    }
    DocSpV->Sort();
    // put unit-norm
    DocSpV->PutUnitNorm();
    // return sparse vector
    return DocSpV;
}

PBowSpV TBowDocBs::GetSpVFromHtmlFile(
 const TStr& HtmlFNm, const PBowDocWgtBs& BowDocWgtBs) const {
  PSIn HtmlSIn=TFIn::New(HtmlFNm);
  TStr HtmlStr=TStr::LoadTxt(HtmlSIn);
  return GetSpVFromHtmlStr(HtmlStr, BowDocWgtBs);
}

PBowSpV TBowDocBs::GetSpVFromWIdWgtPrV(
 const TIntFltPrV& WIdWgtPrV, const PBowDocWgtBs& BowDocWgtBs) const {
  
  PBowSpV DocSpV;
  if (BowDocWgtBs.Empty()){
    
    int DocWIds=WIdWgtPrV.Len();
    DocSpV=TBowSpV::New(-1, DocWIds);
    
    TInt DocWId; TInt DocWordFq;
    for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
      
      DocSpV->AddWIdWgt(WIdWgtPrV[DocWIdN].Val1, WIdWgtPrV[DocWIdN].Val2);
    }
    DocSpV->Sort();
  } else {
    
    int MnWordFq=BowDocWgtBs->GetMnWordFq();
    int Docs=BowDocWgtBs->GetDocs();
    int DocWIds=WIdWgtPrV.Len();
    DocSpV=TBowSpV::New(-1, DocWIds);
    
    TInt DocWId; double DocWordFq;
    for (int DocWIdN=0; DocWIdN<DocWIds; DocWIdN++){
      // get word id & freq.
      DocWId=WIdWgtPrV[DocWIdN].Val1;
      DocWordFq=WIdWgtPrV[DocWIdN].Val2;
      //TStr WordStr=GetWordStr(DocWId); // for debugging
      if (GetWordFq(DocWId)>=MnWordFq){
        // calculate & add TFIDF weight
        double WordDf=BowDocWgtBs->GetWordFq(DocWId);
        double WordIDf=0;
        if (WordDf>0){WordIDf=log(double(Docs)/WordDf);}
        double DocWordWgt=DocWordFq*WordIDf;
        DocSpV->AddWIdWgt(DocWId, DocWordWgt);
      }
    }
    DocSpV->Sort();
    // cut low weight words
    DocSpV->CutLowWgtWords(BowDocWgtBs->GetCutWordWgtSumPrc());
    
    DocSpV->PutUnitNorm();
  }
  
  return DocSpV;
}

const TStr TBowDocBs::BowDocBsFExt=".Bow";

