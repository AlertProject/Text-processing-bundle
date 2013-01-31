#ifndef OGSEQ_H
#define OGSEQ_H

#include "../qminer.h"
#include "../qminer_srv.h"

///////////////////////////////
// OntoGen-StateExtractor
ClassTP(TOgSeqFtrExt, POgSeqFtrExt)//{
private:
	static const int HOverlap = 1;
private:

	POgBase OgBase;
	TInt FilterStId;
	TIntPrV FilterJoinIdV; 	
	TInt FilterJStId;

	TOgFtrExt FilterFtrExt;
	TOgFtrExt ObsFtrExt;
	TVec<TOgFtrExt> TransFtrExtV; 
	TVec<TOgFtrExt> StateFtrExtV; 	

	TInt ElementNo;

private:
	void TrimToItemNo(TStrV& TempV);
public:	
	TOgSeqFtrExt(POgBase _OgBase, 
		const TInt& _FilterStId, 
		const TIntPrV& _FilterJoinIdV, 
		const TInt& _FilterJStId, 
		const TOgFtrExt& _St1FrtExt,
		const TOgFtrExt& _St2FrtExt,
		const TInt& _ElementNo);

	static POgSeqFtrExt New(POgBase _OgBase, 
		const TInt& _FilterStId, 
		const TIntPrV& _FilterJoinIdV, 
		const TInt& _FilterJStId, 
		const TOgFtrExt& _St1FrtExt,
		const TOgFtrExt& _St2FrtExt,
		const TInt& _ElementNo) { 
			return new TOgSeqFtrExt(_OgBase, _FilterStId, _FilterJoinIdV, _FilterJStId, 
				_St1FrtExt, _St2FrtExt, _ElementNo); }

	POgRecSet GetOwnerSet(const TStr& FilterQueryType, 
		const TStr& FilterQuery, const TInt& FilterRecSmpl);
	POgRecSet GetObsSet(const POgRecSet& SeqRSet, const TStr& FilterJQueryType, 
		const TStr& FilterJQuery, const TInt& FilterJoinSmpl);
	POgRecSet GetObservations(const int& OwnerId, const int& FilterJoinSmpl);

	TIntV GetTmFiledIdV(const int& StoreId);
	void GetTmSorted(const int& TmFieldId, const POgRecSet& SeqRSet);
	int GetTmDifSecs(const int& StoreId, const int& Rec1Id, 
		const int& Rec2Id, const int& TmFieldId);
	void GetTransV(const TInt& StoreId,  const POgRecSet& Sequence, TStrV& TransV);

	void CheckInvalidTrans(POgRecSet OwnerSet, const int& OwnerIdx, POgRecSet SeqRSet);

};

typedef enum { bbptDrudge, bbptGoogle, bbptYahoo, bbptExternal, bbptEmpty} TBBPgType;
// handles operation related to action sequences retrieved from stores
ClassTP(TOgSeqBs, POgSeqBs)//{
private:	
	POgRecSet OwnerSet;

	TVec<POgRecSet> SeqV; //vector of sequences, elements are pointers to records in stores
	TVec<TStrVIntH> StateHV; //may contain per sequence states, case in which len>1 or aggregated states, len=1
	TVec<TIntIntHH> DirEdgeHV; //may contain per sequence trans probab, case in which len>1 or aggregated trans probab, len=1
	
	PBowDocBs BowDocBs;
	PBowDocPart BowDocPart;	
	
	TVec<PMom> SeqMomV; // statistics

	TIntV ActionNoV;
	TIntIntH SeqOwnerH;
	TIntIntH BowSeqH;
	
	TOgSeqBs(){};
	TOgSeqBs(TSIn& SIn);

	int GetStateId (const int& ClustIdx, TStr& StateNm);	
	TStr GetOutStateStr(const TStrV& StStrV);
	
	void GenSeqByTmSlice(const POgSeqFtrExt& Extractor, const POgRecSet& SeqRSet, 
	const int& OwnerIdx, const int& TmIntervalSec);
public:	
	static POgSeqBs New() {return new TOgSeqBs; }
	
	//get
	int GetActionNo(int ActionIdx) const { return ActionNoV[ActionIdx]; }
	void GetNGramStates(const int& N, const int& SeqIdx, 
		const POgSeqFtrExt& Extractor, TStrV& StateStrV);

	void ExtractSeqV(const POgSeqFtrExt& Extractor, const TStr& FilterQueryType,
	const TStr& FilterQuery, const TStr& FilterJQueryType, const TStr& FilterJQuery, 
	const int& FilterRecSmpl, const int& FilterJoinSmpl);

	//generate	
	void GenBowBs(const int& N, const POgSeqFtrExt& Extractor);
	void GenTrans(const POgSeqFtrExt& Extractor);	


	//cluster
	void ClustSeq(const PBowSim& BowSim, const int& Seed, const int& Clusts, const int& ClustTrials,
		const double& ConvergEps, const int& MnDocsPerClust, const TBowWordWgtType& WordWgtType, 
		const double& CutWordWgtSumPrc, const int& MnWordFq);

	//load
	static POgSeqBs Load(const TStr& InFNm){ return new TOgSeqBs(*TFIn::New(InFNm)); }

	//save
	void SaveActionGraph(const TStr& OutFNm, const int& MnTransFq);
	void SaveStateHist(const TStr& OutFNm);	
	void Save(const TStr& OutFNm);
	void SaveXml(const TStr& PartialClustFNm, const PXmlTok& TopTok);
	//TODO: serialize
};

#endif
