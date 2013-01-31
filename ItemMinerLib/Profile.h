#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <qminer.h>
#include "Stores.h"

ClassTP(TProfile, PProfile)
//{
public:
	TStr IndexFPath;
	TFAccess FAccess;
	int64 IndexCacheSize;
	POgBase OgBase;
	POgIndexVoc IndexVoc;
	POgIndex Index;
	
	PSwSet SwSet;
	PStemmer Stemmer;
	PTokenizer TokenizerNoStemmingNoSwSet;

	PBowDocBs BowDocBs;
	PBowDocWgtBs BowDocWgtBs;
	
	PBowDocBs BowDocBsConcepts;
	PBowDocWgtBs BowDocWgtBsConcepts;

	PBowDocBs BowDocBsSubjects;		
	//PBowDocWgtBs BowDocWgtBsSubjects;

	PBowDocBs BowDocBsWholeThreads;			
	PBowDocWgtBs BowDocWgtBsWholeThreads;
	
	PBowDocBs BowDocBsConceptsByThread;
	PBowDocWgtBs BowDocWgtBsConceptsByThread;

	PStreamNGramBs NGramBs; 


	TItemStore* ItemStore;
	TPersonStore* PersonStore;
	TThreadStore* ThreadStore;
	TLinkStore* LinkStore;
	TAttachmentStore* AttachmentStore;
		
	int IndexTextSearchKeyId;		
	int IndexConceptSearchKeyId;	
	int IndexTagSearchKeyId;		
	int IndexItemTypeSearchKeyId;   
	/*int SubjectKeyId;
	int BodyKeyId;*/
	
	static PProfile New(const TStr& IndexFPath, const TStr& UnicodeDefFile, const int& MxNGramLen, const int& MxCachedNGrams, const int64& IndexCacheSize, const int64& ItemCacheSize)
	{ return new TProfile(IndexFPath, UnicodeDefFile, MxNGramLen, MxCachedNGrams, IndexCacheSize, ItemCacheSize); }

	static PProfile Load(const TStr& IndexFPath, const TStr& UnicodeDefFile, const TFAccess& FAccess, const int64& IndexCacheSize, const int64& ItemCacheSize)
	{ return new TProfile(IndexFPath, UnicodeDefFile, FAccess, IndexCacheSize, ItemCacheSize); }

	TStr AddItem(const TStr& ItemInfo, const TStr& ItemContent);
	TStr UpdateItem(const TStr& ItemInfo, const TStr& ItemContent);
	bool RemoveItem(const TInt& ItemId);

	void SetTag(const TInt& ItemId, const TStr& TagId);
	void RemoveTag(const TInt& ItemId, const TStr& TagId);
	
	bool SetTagData(const TStr& TagData);
	TStr GetTagData(const TStr& TagData);

	TStr GeneralQuery(const PXmlDoc& QueryXml);		
	TStr CustomQuery(const PXmlDoc& QueryXml);			
	TStr ExecuteCommand(const TStr& CommandStr);
	
	TStr LastInformation;
	void UpdateSettings(TStr Settings);
	void ClearResults() { CachedResults.Clr(); SavedResults.Clr(); }		
	TStr TokenizeText(const TStr& Text)
	{
		TStrV TokenV;
		IndexVoc->GetTokenizer()->GetTokens(Text, TokenV);
		TStr Output;
		for (int i=0; i < TokenV.Len(); i++)
			Output += TokenV[i] + " ";
		return Output;
	}
	void PrintStatus(TChA& StatusChA);

private:
	
	TProfile(const TStr& _IndexFPath, const TStr& UnicodeDefFile, const int& MxNGramLen, const int& MxCachedNGrams, const int64& _IndexCacheSize, const int64& ItemCacheSize);
	
	TProfile(const TStr& _IndexFPath, const TStr& UnicodeDefFile, const TFAccess&_FAccess, const int64& _IndexCacheSize, const int64& _ItemCacheSize);

	TInt NextQueryId;
	
	static const int BfSize = 256*1024;
	char Bf[BfSize];

	void Save();
	void InitData();

	int GetSwSize() { return SwSet->Len(); }
	
	/*static PProfile New(const TStr& IndexFPath, const int64& IndexCacheSize)
	{ return new TProfile(IndexFPath, IndexCacheSize); }*/

	~TProfile();
	
	TInt GetNextQueryId() { return NextQueryId++; }		
	THash<TInt, POgRecSet> CachedResults;		
	THash<TInt, POgRecSet> SavedResults;		
	POgRecSet GetResults(const int QueryId);
	int StoreResults(const POgRecSet& RecSet, const bool Save);
	
	
	POgRecSet InsersectResultSets(const TVec<POgRecSet>& RecSetV);
	POgRecSet GetRecSetForQuery(const PXmlTok& Conditions, const PXmlTok& Ignores, const TStr& SortBy = "", const TStr& ResultData = "");

	POgRecSet GetRecSetForQueryItem(const PXmlTok& Token, const TStr& SortBy = "", const TStr& ResultData = "");
	TStr GetKeywordsFromConditions(const PXmlTok& Conditions);
	
	TStr GetItemData(const POgRecSet& RecSet, const PXmlDoc& QueryXml);
	TStr GetPeopleData(const POgRecSet& RecSet, const PXmlDoc& QueryXml);
	TStr GetTimelineData(const POgRecSet& RecSet, const PXmlDoc& QueryXml);
	TStr GetKeywordData(const POgRecSet& RecSet, const PXmlDoc& QueryXml);
	TStr GetItemIdData(const POgRecSet& RecSet, const PXmlDoc& QueryXml);

	TUInt64V GetItemIdsForThread(uint64 ThreadId)
	{
		TUInt64V ThreadItemsIdV;
		TOgRec ThreadRec(ThreadStoreId, ThreadId);
		POgRecSet ThreadItemsSet = ThreadRec.DoJoin(OgBase, ThreadStore->JoinHasItemsId);
		ThreadItemsSet->GetRecIdV(ThreadItemsIdV);
		return ThreadItemsIdV;
	}
	
	
	void AddItemResult(const TUInt64 ItemId, TChA& ResultChA, bool IncludeAttachments, const TStr& ExtraItemInfo = "");
	void WriteVector(TChA& OutChA, const TUInt64V& DataV, const char separator=',');
	void WriteVector(TChA& OutChA, const TIntV& DataV, const char separator=',');

	void UpdateBowWgts();
	void UpdateBowWgtsConcepts();
	
	void UpdateBowByThread();
	void UpdateBowWgtsByThread();

	void GetTokenIds(const TStr& Text, TIntV& TokenIdV);
	TStr GetTokenStr(const int& WId) { return BowDocBs->GetWordStr(WId); }

	TVec<TPair<TFlt, TStr> > GetSimilarityWithText(const PBowDocBs& BowDoc, const PBowDocWgtBs& BowDocWgt, const TUInt64Set& CandidateItemIdsH, const TStr& Text);
	TVec<TPair<TFlt, TStr> > GetSimilarityWithItem(const PBowDocBs& BowDoc, const PBowDocWgtBs& BowDocWgt, const TUInt64Set& CandidateItemIdsH, const TInt TestItemId);
	
	void GetBowDocIdV(const POgRecSet& RecSet, const PBowDocBs& BowDocBs, TIntV& BowDocIdV);
	POgRecSet GetRecSetForBowDocIdV(const TIntV& DocIdV, const PBowDocBs& BowDocBs);
	PBowKWordSet ComputeKWordSet(const POgRecSet& RecSet, const TStr& MethodUsed, const TStr& KeywordSource = "text", const TStr& SVMInterestingClass = "positive");
	void WriteKeywordSet(TChA& ResultChA, const PBowKWordSet KWordSet, int KeywordCount);

	void RemoveItemSubjectFromIndex(const TInt& ItemId, const TInt& ThreadId);
	void RemoveItemContentFromIndex(const TInt& ItemId);

	bool SettingUpdateThreadBow;		
	bool SettingUpdateNGrams;			
	bool SettingNGramsIgnoreSw;

	TVec<TTriple<TStr, TUInt64, POgRecSet> > LastQueryResV;
	POgRecSet GetResultsForQuery(TStr QueryArgs);
	void StoreResultsForQuery(TStr QueryArgs, POgRecSet Results) 
	{
		TUInt64 RecCount = ItemStore->GetRecs();
		LastQueryResV.Ins(0, TTriple<TStr, TUInt64, POgRecSet>(QueryArgs, RecCount, Results));
		if (LastQueryResV.Len() > 20)
			LastQueryResV.Trunc(20);
	}
public:
	static TStr BuildErrorInfo(const TStr& Error, const TStr& ErrorAdditionalInfo = "") { return TStr("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<error message=\"" + TXmlLx::GetXmlStrFromPlainStr(Error) + "\" additionalInfo=\"" + TXmlLx::GetXmlStrFromPlainStr(ErrorAdditionalInfo) + "\" />"); }
};

#endif