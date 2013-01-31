#ifndef OGTIME_H
#define OGTIME_H

#include "../qminer.h"

///////////////////////////////
// Current-Time-Provider
ClassTP(TCurTm, PCurTm)//{
private:
	TBool ActualP;
	TTm FakeTm;

	UndefDefaultCopyAssign(TCurTm);
	TCurTm(const bool& _ActualP): ActualP(_ActualP) { }
public:
	static PCurTm New(const bool& ActualP) { return new TCurTm(ActualP); }

	TTm Get() const { return ActualP ? TTm::GetCurUniTm() : FakeTm; }
	void Set(const TTm& _FakeTm) { FakeTm = _FakeTm; }
};

///////////////////////////////
// QMiner-Time-Slice
template <class TVal>
class TOgTmSlice {
private:
	// is the slice still active for adding
	TBool ActiveP;
	// time period
	TTm StartTm;
	TTm EndTm;
	// finished counts
	TInt Docs;
	TInt FqSum;
	TVec<TKeyDat<TVal, TInt> > FqV;

	// ongoing counts (temporary)
	THash<TVal, TInt> FqH;
public:
	TOgTmSlice(const TTm& _StartTm, const TTm& _EndTm): ActiveP(true), 
		StartTm(_StartTm), EndTm(_EndTm) { 
			EAssert(EndTm > StartTm);
	}

	// is the time slice still active (== elements can be added to it)
	bool IsActive() const { return ActiveP; }
	// is the given time within the time slice period
	bool IsTimeIn(const TTm& Tm) const { return (StartTm <= Tm) && (Tm < EndTm); }
	// return start and end time
	const TTm& GetStartTm() const { return StartTm; }
	const TTm& GetEndTm() const { return EndTm; }

	// add new features to the time slice
	void Add(const TVec<TVal>& ValV);
	// finish the time slice
	void Finish(const int& MnFq);

	// get counts 
	int GetDocs() const { return Docs; }
	int GetFqSum() const { return FqSum; }
	const TVec<TKeyDat<TVal, TInt> >& GetFqV() const { 
		Assert(!IsActive()); return FqV; }
};

template <class TVal>
void TOgTmSlice<TVal>::Add(const TVec<TVal>& ValV) {
	// check we can still add stuff
	EAssert(IsActive());
	// add stuff
	for (int ValN = 0; ValN < ValV.Len(); ValN++) {
		const TVal& Val = ValV[ValN];
		if (FqH.IsKey(Val)) {
			FqH.GetDat(Val)++;
		} else {
			FqH.AddDat(Val, 1);
		}
	}
	Docs++;
}

template <class TVal>
void TOgTmSlice<TVal>::Finish(const int& MnFq) {
	// no more adding
	ActiveP = false;
	// iterate over all the features
	int KeyId = FqH.FFirstKeyId();
	while (FqH.FNextKeyId(KeyId)) {
		// check if feature frequent enough
		const int Fq = FqH[KeyId];
		if (Fq < MnFq) { continue; }
		// get feature string
		const TVal& Val = FqH.GetKey(KeyId);
		// and remember the values
		FqV.Add(TKeyDat<TVal, TInt>(Val, Fq));
		FqSum += Fq;
	}
	// clean the temporoary hash table
	FqH.Clr(true);
	// sort the features by value
	FqV.Sort();
}

typedef TOgTmSlice<TUInt64> TOgRecTmSlice;
typedef TOgTmSlice<TStr> TOgFtrTmSlice;

///////////////////////////////
// QMiner-Time-Slice-Stat
ClassTP(TOgTmSliceStat, POgTmSliceStat)//{
private:
	class TOgTmSliceFtrStat {
	public:
		// past count
		TInt PastFq;
		// present count
		TInt PresentFq;
		// extra fields
		TIntH ExtraH;

	private:
		static double GetRel(const int& Fq, const int& Sum) { 
			return (Sum == 0) ? 0.0 : double(Fq) / double(Sum); }
		static double GetIDF(const int& Fq, const int& Docs) {
			return (Fq == 0.0) ? 0.0 : TMath::Log(double(Docs) / double(Fq)); }
	public:
		// relative share of feature in the time slice
		double GetRelPast(const int& PastFqSum) const { return GetRel(PastFq, PastFqSum); }
		double GetRelPresent(const int& PresentFqSum) const { return GetRel(PresentFq, PresentFqSum); }
		double GetRelDelta(const int& PastFqSum, const int& PresentFqSum) const { 
			return GetRelPast(PastFqSum) > 0 ? GetRelPresent(PresentFqSum) / GetRelPast(PastFqSum) : 0.0; }
		// IDF of featire in the time slice relative to number of documents
		double GetIDFPast(const int& PastDocs) const { return GetIDF(PastFq, PastDocs); }
		double GetIDFPresent(const int& PresentDocs) const { return GetIDF(PresentFq, PresentDocs); }
		double GetIDFDelta(const int& PastDocs, const int& PresentDocs) const { 
			return GetIDFPast(PastDocs) > 0 ? GetIDFPresent(PresentDocs) / GetIDFPast(PastDocs) : 0.0; }
	};

private:
	// time period
	TTm StartTm;
	TTm BreakTm;
	TTm EndTm;
	// statistics
	TInt PastDocs;
	TInt PastFqSum;
	TInt PresentDocs;
	TInt PresentFqSum;
	// temporary
	THash<TStr, TOgTmSliceFtrStat> FtrStatH;
	// final
	TVec<TKeyDat<TStr, TOgTmSliceFtrStat> > FtrStatV;
	// extra field
	TStrSet FieldSet;

	// measures overlap between features
	bool Overlap(const TStr& ShortFtr, const int& ShortFq,
		const TStr& LongStr, const int& LongFtr) const;

public:
	TOgTmSliceStat(const TTm& _StartTm, const TTm& _BreakTm, const TTm& _EndTm, 
		const int& _PastDocs, const int& _PresentDocs): StartTm(_StartTm), 
			BreakTm(_BreakTm), EndTm(_EndTm), PastDocs(_PastDocs), 
			PresentDocs(_PresentDocs) { }
	static POgTmSliceStat New(const TTm& StartTm, const TTm& BreakTm, 
		const TTm& EndTm, const int& PastDocs, const int& PresentDocs) { 
			return new TOgTmSliceStat(StartTm, BreakTm, EndTm, PastDocs, PresentDocs); }

	// adding feature measurements
	void AddPast(const TStr& Ftr, const int& Fq) { FtrStatH.AddDat(Ftr).PastFq = Fq; }
	void AddPresent(const TStr& Ftr, const int& Fq) { FtrStatH.AddDat(Ftr).PresentFq = Fq; }
	void AddExtra(const TStr& Ftr, const TStr& Field, const int& Val);
	// finalize statstics
	void Finish();

	void SaveStat(const TStr& FNm);
};

///////////////////////////////
// QMiner-Time-Slice-Base
ClassTP(TOgTmSliceBs, POgTmSliceBs)//{
private:
	typedef enum { otstRec, otstFtr } TOgTmSliceType;

private:
	// type of slices in this base
	TOgTmSliceType Type;
  // when aggregatin features over time
	// feature extractor for feature slices
	POgFtrExt FtrExt;
  // when aggregating recored over time
	// joins for record slices
	TIntPrV JoinIdV;
	// feature extractors for extra record slices ranking features
	TVec<TPair<TStr, TIntPrV> > ExtraNmJoinIdVV;
  // shared parameters
	// time slice buffer
	TInt MxTmSlices; 
	TLst<TOgFtrTmSlice> FtrTmSliceL;
	TLst<TOgRecTmSlice> RecTmSliceL;
	// reports
	TLst<TOgTmSliceStat> StatL;
	// time slice parameters
	TUInt64 PeriodMSec;
	// mn feature count for time slice
	TInt MnFtrFq;
	// current time provider
	PCurTm CurTm;
	
	TTm GetEndTm(const TTm& StartTm);
	void UpdateFtr();
	void UpdateRec(const POgBase& OgBase, const POgStore& Store);

	UndefDefaultCopyAssign(TOgTmSliceBs);

	TOgTmSliceBs(const POgFtrExt& _FtrExt, const int& _MxTmSlices, 
		const uint64& _PeriodMSec, const int& _MnFtrFq);
	TOgTmSliceBs(const TIntPrV& _JoinIdV, const int& _MxTmSlices,
		const uint64& _PeriodMSec, const int& _MnFtrFq);
public:
	static POgTmSliceBs New(const POgFtrExt& FtrExt, const int& MxTmSlices, 
		const uint64& PeriodMSec, const int& MnFtrFq) {  
			return new TOgTmSliceBs(FtrExt, MxTmSlices, PeriodMSec, MnFtrFq); }
	static POgTmSliceBs New(const TIntPrV& JoinIdV, const int& MxTmSlices, 
		const uint64& PeriodMSec, const int& MnFtrFq) {  
			return new TOgTmSliceBs(JoinIdV, MxTmSlices, PeriodMSec, MnFtrFq); }

	// add extra fields
	void AddExtraJoin(const TStr& FieldNm, const TIntPrV& JoinIdV) {
		ExtraNmJoinIdVV.Add(TPair<TStr, TIntPrV>(FieldNm, JoinIdV)); }
	// add record
	void Add(const POgBase& OgBase, const TOgRec& Rec);
	// set different time provider
	void SetCurTm(const PCurTm& _CurTm) { CurTm = _CurTm; }
};

///////////////////////////////
// QMiner-Time-Slice-Trigger
class TOgTmSliceTrigger : public TOgStoreTrigger {
private:
	POgTmSliceBs TmSliceBs;

	TOgTmSliceTrigger(const POgTmSliceBs& _TmSliceBs): TmSliceBs(_TmSliceBs) { }
public:
	static POgStoreTrigger New(const POgTmSliceBs& TmSliceBs) {
		return new TOgTmSliceTrigger(TmSliceBs); }

    void OnAdd(const POgBase& OgBase, const TOgStore* Store, const uint64& RecId) {
		TmSliceBs->Add(OgBase, TOgRec(Store->GetStoreId(), RecId)); }
};

// Example: prepare time slicer
//TIntPrV JoinIdV; JoinIdV.Add(TIntPr(ArticleStore->JoinArticleConceptId, 1000));
//TOgFtrExt TmSliceFtrExt(ArticleStore, JoinIdV, ConceptStore, ConceptStore->LabelFieldId, true);
//TmSliceBs = TOgTmSliceBs::New(TmSliceFtrExt, 12, 4*TTmInfo::GetHourMSecs(), 10);
//TmSliceBs = TOgTmSliceBs::New(JoinIdV, 12, 4*TTmInfo::GetHourMSecs(), 10);
//TIntPrV ExtraJoinIdV; 
//ExtraJoinIdV.Add(TIntPr(ConceptStore->JoinConceptArticleId, 100));
//TmSliceBs->AddExtraJoin("articles", ExtraJoinIdV);
//ExtraJoinIdV.Add(TIntPr(ArticleStore->JoinArticleSourceId, 100));
//TmSliceBs->AddExtraJoin("sources", ExtraJoinIdV);
//ConceptStore->AddTrigger(TOgTmSliceTrigger::New(TmSliceBs));


#endif
