#include "qminer_time.h"

///////////////////////////////
// QMiner-Time-Slice-Stat
bool TOgTmSliceStat::Overlap(const TStr& ShortFtr, const int& ShortFq,
		const TStr& LongStr, const int& LongFq) const {

	if (LongStr.IsPrefix(ShortFtr)) {
		// get present frequency difference
		const double FqDiff = double(TInt::Abs(LongFq - ShortFq));
		// get 10% of longftrstat
		const double MxDiff = 0.1 * double(LongFq);
		// if smaller then then percent of longftr, then overlap
		return (FqDiff < MxDiff);
	} 
	return false;
}

void TOgTmSliceStat::AddExtra(const TStr& Ftr, const TStr& Field, const int& Val) {
	const int FieldId = FieldSet.AddKey(Field);
	FtrStatH.AddDat(Ftr).ExtraH.AddDat(FieldId, Val);
}

void TOgTmSliceStat::Finish() {
	// transform from hash to vec
	TVec<TKeyDat<TStr, TOgTmSliceFtrStat> > _FtrStatV;
	FtrStatH.GetKeyDatKdV(_FtrStatV); _FtrStatV.Sort();
	FtrStatH.Clr();
	//HACK filter features for repetitions
	for (int FtrStatN = 0; FtrStatN < _FtrStatV.Len(); FtrStatN++) {
		const TStr& Ftr = _FtrStatV[FtrStatN].Key;
		const TOgTmSliceFtrStat& Stat = _FtrStatV[FtrStatN].Dat;
		if (Stat.PresentFq < 10) { continue; }
		if (Ftr.Len() < 30) { continue; }
		if (!TCh::IsAlpha(Ftr[0])) { continue; }
		// check if there is overlap
		if (!FtrStatV.Empty() && Overlap(FtrStatV.Last().Key, 
				FtrStatV.Last().Dat.PresentFq, Ftr, Stat.PresentFq)) {

			// to much overlap, always keep the longer one
			PastFqSum -= FtrStatV.Last().Dat.PastFq;
			PresentFqSum -= FtrStatV.Last().Dat.PresentFq;
			PastFqSum += Stat.PastFq;
			PresentFqSum += Stat.PresentFq;
			FtrStatV.Last() = _FtrStatV[FtrStatN];
			continue;
		}
		FtrStatV.Add(_FtrStatV[FtrStatN]);
		PastFqSum += Stat.PastFq;
		PresentFqSum += Stat.PresentFq;
	}
}

void TOgTmSliceStat::SaveStat(const TStr& FNm) {
	TFOut FOut(FNm);
	FOut.PutStrLn("Start time: " + StartTm.GetWebLogDateTimeStr());
	FOut.PutStrLn("Break time: " + BreakTm.GetWebLogDateTimeStr());
	FOut.PutStrLn("End time: " + EndTm.GetWebLogDateTimeStr());

	FOut.PutLn();
	// output header
	FOut.PutStr("Feature\tPastFq\tPresentFq\tPastFqRel\tPresentFqRel\tRelDelta\tPastIDF\tPresentIDF\tIDFDelta");
	{int FieldId = FieldSet.FFirstKeyId();
	while (FieldSet.FNextKeyId(FieldId)) {
		FOut.PutCh('\t'); FOut.PutStr(FieldSet.GetKey(FieldId));
	}}
	FOut.PutLn();
	// output features
	for (int FtrStatN = 0; FtrStatN < FtrStatV.Len(); FtrStatN++) {
		const TStr& Ftr = FtrStatV[FtrStatN].Key;
		const TOgTmSliceFtrStat& Stat = FtrStatV[FtrStatN].Dat;
		// get features
		const int PastFtrFq = Stat.PastFq;
		const int PresentFtrFq = Stat.PresentFq;
		const double RelPast = Stat.GetRelPast(PastFqSum);
		const double RelPresent = Stat.GetRelPresent(PresentFqSum);
		const double RelDelta = Stat.GetRelDelta(PastFqSum, PresentFqSum);
		const double IDFPast = Stat.GetIDFPast(PastDocs);
		const double IDFPresent = Stat.GetIDFPresent(PresentDocs);
		const double IDFDelta = Stat.GetIDFDelta(PastDocs, PresentDocs);
		// output features
		FOut.PutStr(TStr::Fmt("%s\t%d\t%d\t%.6f\t%.6f\t%.2f\t%.6f\t%.6f\t%.2f", 
			Ftr.CStr(), PastFtrFq, PresentFtrFq, RelPast, RelPresent,
			RelDelta, IDFPast, IDFPresent, IDFDelta));
		// extra fields
		int FieldId = FieldSet.FFirstKeyId();
		while (FieldSet.FNextKeyId(FieldId)) {
			FOut.PutCh('\t');
			if (Stat.ExtraH.IsKey(FieldId)) {
				FOut.PutInt(Stat.ExtraH.GetDat(FieldId));
			}
		}
		FOut.PutLn();
	}
}

///////////////////////////////
// QMiner-Time-Slice-Base
TTm TOgTmSliceBs::GetEndTm(const TTm& StartTm) {
	TTm EndTm = StartTm;
	EndTm.AddTime(0, 0, 0, (int)PeriodMSec);
	return EndTm;
}

void TOgTmSliceBs::UpdateFtr() {
	// check if current time slice still valid
	TTm NowTm = CurTm->Get();
	if (FtrTmSliceL.Empty()) {
		// compute the end time and create first slice
		FtrTmSliceL.AddBack(TOgFtrTmSlice(NowTm, GetEndTm(NowTm)));
	} else {
		bool NewTmSliceP = false;
		while (!FtrTmSliceL.LastVal().IsTimeIn(NowTm)) {
			printf("Adding new TmSlice ... ");
			// start a new one if out of date
			TOgFtrTmSlice& TmSlice = FtrTmSliceL.LastVal();
			// finish last timeslice
			TmSlice.Finish(MnFtrFq);
			// get last slice timeperiod
			//const TTm& OldStartTm = TmSlice.GetStartTm();
			const TTm& OldEndTm = TmSlice.GetEndTm();
			// make sure it's in the past (otherwise something went wrong...)
			EAssert(NowTm >= OldEndTm);
			// create next time slice
			FtrTmSliceL.AddBack(TOgFtrTmSlice(OldEndTm, GetEndTm(OldEndTm)));
			// delete a slice, if buffer to big
			printf("deleting old ... ");
			if (FtrTmSliceL.Len() > MxTmSlices) { FtrTmSliceL.DelFirst(); }
			// mark that we added new slice
			NewTmSliceP = true;
			printf("Done\n");
		}

		// update statistics, if enough evidence
		if (NewTmSliceP && FtrTmSliceL.Len() >= 3) {
			printf("Preparing TmSlice Statistics ... ");
			// get present (second from the back
			TLstNd<TOgFtrTmSlice>* PresentLstNd = FtrTmSliceL.Last()->Prev();
			const int PresentDocs = PresentLstNd->GetVal().GetDocs();
			const TStrIntKdV& PresentFtrFqV = PresentLstNd->GetVal().GetFqV();
			// get time
			TTm EndTm = PresentLstNd->GetVal().GetEndTm();
			TTm BreakTm = PresentLstNd->GetVal().GetStartTm();
			// get past
			TLstNd<TOgFtrTmSlice>* PastLstNd = PresentLstNd->Prev();
			int PastDocs = 0; TStrIntKdV PastFtrFqV; TTm StartTm;
			while (PastLstNd != NULL) {
				// merge slice info with the average
				PastDocs += PastLstNd->GetVal().GetDocs();
				TStrIntKdV SumFtrFqV;
				TSparseOps<TStr, TInt>::SparseMerge(PastFtrFqV, 
					PastLstNd->GetVal().GetFqV(), SumFtrFqV);
				PastFtrFqV = SumFtrFqV;
				StartTm = PastLstNd->GetVal().GetStartTm();
				// move back in time
				PastLstNd = PastLstNd->Prev();
			}
			// compute statistics
			printf("computing ... ");
			POgTmSliceStat OgTmSliceStat = TOgTmSliceStat::New(
				StartTm, BreakTm, EndTm, PastDocs, PresentDocs);
			for (int PresentN = 0; PresentN < PresentFtrFqV.Len(); PresentN++) {
				const TStrIntKd& FtrFq = PresentFtrFqV[PresentN];
				OgTmSliceStat->AddPresent(FtrFq.Key, FtrFq.Dat);
			}
			for (int PastN = 0; PastN < PastFtrFqV.Len(); PastN++) {
				const TStrIntKd& FtrFq = PastFtrFqV[PastN];
				OgTmSliceStat->AddPast(FtrFq.Key, FtrFq.Dat);
			}
			OgTmSliceStat->Finish();
			// save statistics
			printf("saving ... ");
			OgTmSliceStat->SaveStat(TFile::GetUniqueFNm("./temp/slice/stat.txt"));
			printf("Done\n");
		}
	}
}

void TOgTmSliceBs::UpdateRec(const POgBase& OgBase, const POgStore& Store) {
	// check if current time slice still valid
	TTm NowTm = CurTm->Get();
	if (RecTmSliceL.Empty()) {
		// compute the end time and create first slice
		RecTmSliceL.AddBack(TOgRecTmSlice(NowTm, GetEndTm(NowTm)));
	} else {
		bool NewTmSliceP = false;
		while (!RecTmSliceL.LastVal().IsTimeIn(NowTm)) {
			printf("Adding new TmSlice ... ");
			// start a new one if out of date
			TOgRecTmSlice& TmSlice = RecTmSliceL.LastVal();
			// finish last timeslice
			TmSlice.Finish(MnFtrFq);
			// get last slice timeperiod
			//const TTm& OldStartTm = TmSlice.GetStartTm();
			const TTm& OldEndTm = TmSlice.GetEndTm();
			// make sure it's in the past (otherwise something went wrong...)
			EAssert(NowTm >= OldEndTm);
			// create next time slice
			RecTmSliceL.AddBack(TOgRecTmSlice(OldEndTm, GetEndTm(OldEndTm)));
			// delete a slice, if buffer to big
			printf("deliting old ... ");
			if (RecTmSliceL.Len() > MxTmSlices) { RecTmSliceL.DelFirst(); }
			// mark that we added new slice
			NewTmSliceP = true;
			printf("Done\n");
		}

		// update statistics, if enough evidence
		if (NewTmSliceP && RecTmSliceL.Len() >= 3) {
			printf("Preparing TmSlice Statistics ... ");
			// get present (second from the back
			TLstNd<TOgRecTmSlice>* PresentLstNd = RecTmSliceL.Last()->Prev();
			const int PresentDocs = PresentLstNd->GetVal().GetDocs();
			const TUInt64IntKdV& PresentRecFqV = PresentLstNd->GetVal().GetFqV();
			// get time
			TTm EndTm = PresentLstNd->GetVal().GetEndTm();
			TTm BreakTm = PresentLstNd->GetVal().GetStartTm();
			// get past
			TLstNd<TOgRecTmSlice>* PastLstNd = PresentLstNd->Prev();
			int PastDocs = 0; TUInt64IntKdV PastRecFqV; TTm StartTm;
			while (PastLstNd != NULL) {
				// merge slice info with the average
				PastDocs += PastLstNd->GetVal().GetDocs();
				TUInt64IntKdV SumFtrFqV;
				TSparseOps<TUInt64, TInt>::SparseMerge(PastRecFqV, 
					PastLstNd->GetVal().GetFqV(), SumFtrFqV);
				PastRecFqV = SumFtrFqV;
				StartTm = PastLstNd->GetVal().GetStartTm();
				// move back in time
				PastLstNd = PastLstNd->Prev();
			}
			// compute statistics
			printf("computing ... ");
			POgTmSliceStat OgTmSliceStat = TOgTmSliceStat::New(
				StartTm, BreakTm, EndTm, PastDocs, PresentDocs);
			for (int PresentN = 0; PresentN < PresentRecFqV.Len(); PresentN++) {
				const TUInt64IntKd& RecIdFq = PresentRecFqV[PresentN];
				TStr RecNm = Store->GetRecNm(RecIdFq.Key);
				OgTmSliceStat->AddPresent(RecNm, RecIdFq.Dat);
				// get extra features
				for (int ExtraN = 0; ExtraN < ExtraNmJoinIdVV.Len(); ExtraN++) {
					TOgRec Rec(Store->GetStoreId(), RecIdFq.Key);
					const TStr& FieldNm = ExtraNmJoinIdVV[ExtraN].Val1;
					const int Val = Rec.DoJoin(OgBase, ExtraNmJoinIdVV[ExtraN].Val2)->GetRecs();
					OgTmSliceStat->AddExtra(RecNm, FieldNm, Val);
				}
			}
			for (int PastN = 0; PastN < PastRecFqV.Len(); PastN++) {
				const TUInt64IntKd& RecIdFq = PastRecFqV[PastN];
				TStr RecNm = Store->GetRecNm(RecIdFq.Key);
				OgTmSliceStat->AddPast(RecNm, RecIdFq.Dat);
			}
			OgTmSliceStat->Finish();
			// save statistics
			printf("saving ... ");
			OgTmSliceStat->SaveStat(TFile::GetUniqueFNm("./temp/slice/stat.txt"));
			printf("Done\n");
		}
	}
}

TOgTmSliceBs::TOgTmSliceBs(const POgFtrExt& _FtrExt, const int& _MxTmSlices, 
		const uint64& _PeriodMSec, const int& _MnFtrFq): Type(otstFtr), 
			FtrExt(_FtrExt), MxTmSlices(_MxTmSlices), PeriodMSec(_PeriodMSec), 
			MnFtrFq(_MnFtrFq), CurTm(TCurTm::New(true)) {

	// must have at leats three slices (past, present, future)
	EAssert(MxTmSlices >= 3);
}

TOgTmSliceBs::TOgTmSliceBs(const TIntPrV& _JoinIdV, const int& _MxTmSlices,
		const uint64& _PeriodMSec, const int& _MnFtrFq):
			Type(otstRec), JoinIdV(_JoinIdV), MxTmSlices(_MxTmSlices), 
			PeriodMSec(_PeriodMSec), MnFtrFq(_MnFtrFq), CurTm(TCurTm::New(true)) {

	// must have at leats three slices (past, present, future)
	EAssert(MxTmSlices >= 3);
}

void TOgTmSliceBs::Add(const POgBase& OgBase, const TOgRec& Rec) {
	// add accordig to the type
	if (Type == otstFtr) {
		// bring slices up-to-date
		UpdateFtr();
		// extract features
		TStrV FtrValV; FtrExt->ExtractStrV(OgBase, Rec, FtrValV);
		// add them to the time slices
		FtrTmSliceL.LastVal().Add(FtrValV);
	} else {
		// extract records
		TUInt64V RecIdV; 
		uchar StoreId = Rec.GetStoreId();
		if (JoinIdV.Empty()) {
			RecIdV.Add(Rec.GetRecId());
		} else {
			POgRecSet RecSet = Rec.DoJoin(OgBase, JoinIdV);
			RecSet->GetRecIdV(RecIdV);
			StoreId = RecSet->GetStoreId();
		}
		// bring slices up-to-date
		UpdateRec(OgBase, OgBase->GetStoreByStoreId(StoreId));	
		// add them to the time slices
		RecTmSliceL.LastVal().Add(RecIdV);
	}
}
