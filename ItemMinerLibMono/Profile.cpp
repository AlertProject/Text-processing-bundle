#include "Profile.h"

#include "xmlParsing.h"
#include "Tokenizer.h"

TStr GetModulePath()
{
#ifdef WIN32
	char strExePath [255];
	GetModuleFileName(NULL, strExePath, 255);
	TStr Path(strExePath);
	int LastInd = Path.SearchChBack('\\');
	if (LastInd < 0 || LastInd > Path.Len()) LastInd = 0;
	return TStr(Path.GetSubStr(0, LastInd));
#endif

#ifdef UNIX
	char szTmp[1024];
	int len;
	if ((len = readlink("/modules/pass1", szTmp, sizeof(szTmp)-1)) != -1)
	{
		szTmp[len] = '\0';
		return TStr(szTmp);
	}
	printf("%s", szTmp);
#endif
	return TStr();
}

TProfile::TProfile(const TStr& _IndexFPath, const TStr& UnicodeDefFile, const int& MxNGramLen, const int& MxCachedNGrams, const int64& _IndexCacheSize, const int64& ItemCacheSize):
	IndexFPath(_IndexFPath), FAccess(faCreate), IndexCacheSize(_IndexCacheSize)
{
	OgBase = TOgBase::New();
	LastInformation = "";
	
	IndexVoc = TOgIndexVoc::New();
	Index = TOgIndex::New(IndexFPath, faCreate, IndexVoc, IndexCacheSize);

	if (!TUnicodeDef::IsDef()) { TUnicodeDef::Load(UnicodeDefFile); }
	
	SwSet = TSwSet::New(swstEn8);
	SwSet->AddEnMsdn();
	SwSet->AddGe();
	SwSet->AddSiYuAscii();
	Stemmer = TStemmer::New(stmtPorter, true);
	IndexVoc->PutTokenizer(TContextifyTokenizer::New(SwSet, Stemmer));
	
	TokenizerNoStemmingNoSwSet = TContextifyTokenizer::New(TSwSet::New(), TStemmer::New());

	ItemStore = new TItemStore(IndexFPath + "Item", ItemCacheSize, IndexVoc);
	PersonStore = new TPersonStore(IndexFPath + "Person", IndexVoc);
	ThreadStore = new TThreadStore(IndexFPath + "Thread", IndexVoc);
	LinkStore = new TLinkStore(IndexFPath + "Link", IndexVoc);
	AttachmentStore = new TAttachmentStore(IndexFPath + "Attachment", IndexVoc);

	BowDocBs = TBowDocBs::New(SwSet, Stemmer, NULL);
	BowDocBsSubjects = TBowDocBs::New(SwSet, Stemmer, NULL);
	BowDocBsWholeThreads = TBowDocBs::New(SwSet, Stemmer, NULL);
	BowDocBsConcepts = TBowDocBs::New(SwSet, Stemmer, NULL);
	BowDocBsConceptsByThread = TBowDocBs::New(SwSet, Stemmer, NULL);
		
	NGramBs = TStreamNGramBs::New(MxNGramLen, MxCachedNGrams);
	
	BowDocBs->AddWordStr("");
	BowDocBsSubjects->AddWordStr("");	

	InitData();

	IndexTextSearchKeyId = OgBase->NewIndexKey(ItemStore, "TextSearch", true);
	IndexConceptSearchKeyId = OgBase->NewIndexKey(ItemStore, "ConceptSearch", false);
	IndexTagSearchKeyId = OgBase->NewIndexKey(ItemStore, "TagSearch", false);
	IndexItemTypeSearchKeyId = OgBase->NewIndexKey(ItemStore, "ItemTypeSearch", false);
}

TProfile::TProfile(const TStr& _IndexFPath, const TStr& UnicodeDefFile, const TFAccess&_FAccess, const int64& _IndexCacheSize, const int64& _ItemCacheSize):
	IndexFPath(_IndexFPath), FAccess(_FAccess), IndexCacheSize(_IndexCacheSize)
{
	EAssert(FAccess == faRdOnly || FAccess == faUpdate || FAccess == faRestore);

	OgBase = TOgBase::New();
	LastInformation = "";
	
	IndexVoc = TOgIndexVoc::Load(*TFIn::New(IndexFPath + "IndexVoc.dat"));
	
	try 
	{
		Index = TOgIndex::New(IndexFPath, FAccess, IndexVoc, IndexCacheSize);
	}
	catch (...) 
	{ 
		TOgIndex::New(IndexFPath, faRestore, IndexVoc, IndexCacheSize);				
		Index = TOgIndex::New(IndexFPath, FAccess, IndexVoc, IndexCacheSize);
	}

	if (!TUnicodeDef::IsDef()) { TUnicodeDef::Load(UnicodeDefFile); }
	
	PTokenizer tokenizer = TContextifyTokenizer::Load(*TFIn::New(IndexFPath + "Tokenizer.dat"));
	Stemmer = ((TContextifyTokenizer*) tokenizer())->GetStemmmer();
	SwSet = ((TContextifyTokenizer*) tokenizer())->GetSwSet();
	IndexVoc->PutTokenizer(tokenizer);
	
	TokenizerNoStemmingNoSwSet = TContextifyTokenizer::New(TSwSet::New(), TStemmer::New());

	ItemStore = new TItemStore(IndexFPath + "Item", FAccess, _ItemCacheSize, IndexVoc);
	PersonStore = new TPersonStore(IndexFPath + "Person", FAccess, IndexVoc);
	ThreadStore = new TThreadStore(IndexFPath + "Thread", FAccess);
	LinkStore = new TLinkStore(IndexFPath + "Link", FAccess);
	AttachmentStore = new TAttachmentStore(IndexFPath + "Attachment", FAccess);

	BowDocBs = TBowDocBs::LoadBin(IndexFPath + "Bow.bin");
	BowDocBsSubjects = TBowDocBs::LoadBin(IndexFPath + "BowSubjects.bin");
	BowDocBsWholeThreads = TFile::Exists(IndexFPath + "BowWholeThreads.bin") ? 
		TBowDocBs::LoadBin(IndexFPath + "BowWholeThreads.bin") : 
		TBowDocBs::New(SwSet, Stemmer, NULL);
	BowDocBsConcepts = TFile::Exists(IndexFPath + "BowConcepts.bin") ? 
		TBowDocBs::LoadBin(IndexFPath + "BowConcepts.bin") : 
		TBowDocBs::New(SwSet, Stemmer, NULL);
	BowDocBsConceptsByThread = TFile::Exists(IndexFPath + "BowConceptsByThread.bin") ? 
		TBowDocBs::LoadBin(IndexFPath + "BowConceptsByThread.bin") : 
		TBowDocBs::New(SwSet, Stemmer, NULL);

	NGramBs = TStreamNGramBs::Load(*TFIn::New(IndexFPath + "NGrams.dat"));
	
	InitData();

	IndexTextSearchKeyId = IndexVoc->GetKeyId(ItemStoreId, "TextSearch");
	IndexConceptSearchKeyId = IndexVoc->GetKeyId(ItemStoreId, "ConceptSearch");
	IndexTagSearchKeyId = IndexVoc->GetKeyId(ItemStoreId, "TagSearch");
	IndexItemTypeSearchKeyId = IndexVoc->GetKeyId(ItemStoreId, "ItemTypeSearch");
}

void TProfile::InitData()
{
	NextQueryId = 0;
	
	OgBase->PutIndex(Index);
	OgBase->AddStore(ItemStore);
	OgBase->AddStore(PersonStore);
	OgBase->AddStore(ThreadStore);
	OgBase->AddStore(LinkStore);
	OgBase->AddStore(AttachmentStore);
	
	OgBase->Init();

	SettingUpdateThreadBow = 1;
	SettingUpdateNGrams = 0;
	SettingNGramsIgnoreSw = 0;
}

TProfile::~TProfile() 
{ 
	// save if necessary
	if (FAccess != faRdOnly)
		Save();
}

void TProfile::Save()
{
	printf("Started saving...\n");
	IndexVoc->GetTokenizer()->Save(*TFOut::New(IndexFPath + "Tokenizer.dat"));
	NGramBs->Save(*TFOut::New(IndexFPath + "NGrams.dat"));

	IndexVoc->Save(*TFOut::New(IndexFPath + "IndexVoc.dat"));

	BowDocBs->Save(*TFOut::New(IndexFPath + "Bow.bin"));
	BowDocBsSubjects->Save(*TFOut::New(IndexFPath + "BowSubjects.bin"));
	BowDocBsConcepts->Save(*TFOut::New(IndexFPath + "BowConcepts.bin"));

	if (SettingUpdateThreadBow)
	{
		BowDocBsWholeThreads->Save(*TFOut::New(IndexFPath + "BowWholeThreads.bin"));
		BowDocBsConceptsByThread->Save(*TFOut::New(IndexFPath + "BowConceptsByThread.bin"));
	}

	/*TagStore->Save(*TFOut::New(IndexFPath + "Tag.Store"));*/
	printf("Done saving\n");
}

void TProfile::UpdateBowWgts()
{
	if (BowDocWgtBs.Empty() || BowDocWgtBs->GetDocs() != BowDocBs->GetDocs())
		BowDocWgtBs = TBowDocWgtBs::New(BowDocBs, bwwtNrmTFIDF);
}
void TProfile::UpdateBowWgtsConcepts()
{
	if (BowDocWgtBsConcepts.Empty() || BowDocWgtBsConcepts->GetDocs() != BowDocBsConcepts->GetDocs())
		BowDocWgtBsConcepts = TBowDocWgtBs::New(BowDocBsConcepts, bwwtNrmTFIDF);
}

void TProfile::UpdateSettings(TStr Settings)
{
	PXmlDoc SettingsXml = TXmlDoc::LoadStr(Settings);
	if (!SettingsXml->IsOk())
	{
		LastInformation = "Invalid XML data: " + SettingsXml->GetMsgStr();
		return;
	}

	PXmlTok SettingsItem = SettingsXml->GetTagTok("Settings");
	SettingUpdateThreadBow = GetBoolArg(SettingsItem, "updateThreadBow", true);
	SettingUpdateNGrams = GetBoolArg(SettingsItem, "updateNGrams", false);
	SettingNGramsIgnoreSw = GetBoolArg(SettingsItem, "nGramsIgnoreSw", false);
}

TStr TProfile::AddItem(const TStr& ItemInfo, const TStr& ItemContent)
{
	LastInformation = "";
	PXmlDoc ItemDataXml = TXmlDoc::LoadStr(ItemInfo);
	if (!ItemDataXml->IsOk())
		return BuildErrorInfo("Invalid XML data: " + ItemDataXml->GetMsgStr());

	PXmlTok ItemXml = ItemDataXml->GetTagTok("item");
	TStr ItemTypeStr = ItemXml->GetStrArgVal("itemType", "-1");
	ItemTypeEnum ItemType = (ItemTypeEnum) ItemTypeStr.GetInt(-1);
	TUInt64 Time = ItemXml->GetArgVal("time").GetUInt64();
	TInt IndexText = GetBoolArg(ItemXml, "indexText", true);
	TInt ItemId = ItemXml->GetIntArgVal("itemId", -1);
	
	PXmlTok SubjectTok = ItemXml->GetTagTok("subject");
	TStr Subject = SubjectTok.Empty() ? "" : SubjectTok->GetTokStr(false);
	
	PXmlTok EntryIdTok = ItemXml->GetTagTok("entryId");
	TStr EntryId = EntryIdTok.Empty() ? "" : EntryIdTok->GetTokStr(false);
	
	bool ExistingThread = false;
	TInt ThreadId = -1;
	PXmlTok ThreadTok = ItemXml->GetTagTok("thread");
	TStr Thread = ThreadTok.Empty() ? "" : ThreadTok->GetTokStr(false);
	if (Thread != "") {
		ExistingThread = ThreadStore->IsRecNm(Thread);
		ThreadId = ThreadStore->AddRec(Thread);
	}

	PXmlTok TagsTok = ItemXml->GetTagTok("tags");
	TStr Tags = TagsTok.Empty() ? "" : TagsTok->GetTokStr(false);
	TStrV TagsV; Tags.SplitOnAllCh(',', TagsV);
		
	ItemStore->AddRec(ItemId, (TCh) ItemType, EntryId, Time, ThreadId, TagsV);
	
	if (ThreadId != -1)
		Index->IndexJoin(ThreadStore, ThreadStore->JoinHasItemsId, ThreadId, ItemId); 

	TPartV ParticipantsV;
	TXmlTokV emls;
	ItemXml->GetTagTokV("people|person", emls);
	for (int emlInd = 0; emlInd < emls.Len(); emlInd++) 
	{
		TInt IdInt = emls[emlInd]->GetIntArgVal("id", -1);
		int PersonId = PersonStore->AddPerson(IdInt);
		TStr Role = emls[emlInd]->GetArgVal("role");
		
		Index->IndexJoin(PersonStore, PersonStore->JoinHasAnyRoleId, PersonId, ItemId);	
		int RoleJoinId = PersonStore->AddRole(Role);							
		Index->IndexJoin(PersonStore, RoleJoinId, PersonId, ItemId);
				
		ParticipantsV.Add(TPart(RoleJoinId, PersonId));
		if (ThreadId != -1)
			Index->IndexJoin(PersonStore, PersonStore->JoinHasThreadsId, PersonId, ThreadId);		
	}

	if (ParticipantsV.Len() > 0)
		ItemStore->SetParticipants(ItemId, ParticipantsV);
	
	
	TXmlTokV LinksV;
	ItemXml->GetTagTokV("links|link", LinksV);
	for (int LinkInd = 0; LinkInd < LinksV.Len(); LinkInd++) 
	{
		TStr Href = LinksV[LinkInd]->GetTagTokStr("h");
		TStr Text = LinksV[LinkInd]->GetTagTokStr("t");
		int LinkId = LinkStore->AddRec(Href, Text);
		Index->IndexJoin(ItemStore, ItemStore->JoinHasLinksId, ItemId, LinkId);
	}
	
	
	TXmlTokV AttsV;
	ItemXml->GetTagTokV("attachments|att", AttsV);
	for (int AttInd = 0; AttInd < AttsV.Len(); AttInd++) 
	{
		TStr AttName = AttsV[AttInd]->GetArgVal("name", "");
		TInt AttSize = AttsV[AttInd]->GetIntArgVal("size", 0);
		TInt AttItemId =  AttsV[AttInd]->GetIntArgVal("attItemId", -1);
		int AttachmentId = AttachmentStore->AddRec(AttName, AttSize, ItemId, AttItemId);
		Index->IndexJoin(ItemStore, ItemStore->JoinHasAttachmentsId, ItemId, AttachmentId);	
	}

	Index->Index(IndexTagSearchKeyId, TagsV, ItemId);

	Index->Index(IndexItemTypeSearchKeyId, ItemTypeStr, ItemId);

	TXmlTokV RelatedItemsV;
	ItemXml->GetTagTokV("relatedItems|item", RelatedItemsV);
	for (int RelatedInd = 0; RelatedInd < RelatedItemsV.Len(); RelatedInd++)
	{
		int RelatedItemId = RelatedItemsV[RelatedInd]->GetIntArgVal("itemId", -1);
		if (RelatedItemId != -1)
			Index->IndexJoin(ItemStore, ItemStore->JoinHasRelatedItemsId, ItemId, RelatedItemId);	
	}
	
	TStrV ThreadTermV;
	TStrV ContentTermV;
	IndexVoc->GetTokenizer()->GetTokens(Subject, ThreadTermV);
	IndexVoc->GetTokenizer()->GetTokens(ItemContent, ContentTermV);
		
	TStrV FullTextTermV;
	FullTextTermV.AddV(ThreadTermV);
	FullTextTermV.AddV(ContentTermV);
	if (IndexText == 1)
		Index->Index(IndexTextSearchKeyId, FullTextTermV, ItemId);
	
	
	BowDocBs->AddDoc(ItemId.GetStr(), TStrV(), FullTextTermV);

	if (ThreadId != -1)
	{
		bool existingSubject = BowDocBsSubjects->IsDocNm(ThreadId.GetStr());
		if (!existingSubject)			
			BowDocBsSubjects->AddDoc(ThreadId.GetStr(), TStrV(), ThreadTermV);
		if (SettingUpdateThreadBow) 
			BowDocBsWholeThreads->AppendDoc(ThreadId.GetStr(), TStrV(), existingSubject ? ContentTermV : FullTextTermV);
	}	

	
	if (SettingUpdateNGrams)
	{
		TIntV ThreadIdV, ContentIdV;
		GetTokenIds(Thread, ThreadIdV);
		GetTokenIds(ItemContent, ContentIdV);
		TNGramDescV NGramDescV;
		NGramBs->AddDocTokIdV(ThreadIdV, 100, NGramDescV);
		NGramBs->AddDocTokIdV(ContentIdV, 100, NGramDescV);
	}

	
	TIntFltPrV ConceptsWIdWgtPrV;
	TIntFltPrV ConceptsThreadWIdWgtPrV;
	TXmlTokV ConceptV;
	ItemXml->GetTagTokV("concepts|concept", ConceptV);
	if (ConceptV.Len() > 0)
	{
		for (int conceptInd = 0; conceptInd < ConceptV.Len(); conceptInd++) 
		{
			TStr ConceptUrl = ConceptV[conceptInd]->GetArgVal("uri", "");
			int WId = BowDocBsConcepts->AddWordStr(ConceptUrl);
			TFlt ConceptWeight = ConceptV[conceptInd]->GetFltArgVal("weight", 0);
			ConceptsWIdWgtPrV.Add(TIntFltPr(WId, ConceptWeight));
			if (SettingUpdateThreadBow)
				ConceptsThreadWIdWgtPrV.Add(TIntFltPr(BowDocBsConceptsByThread->AddWordStr(ConceptUrl), ConceptWeight));
			Index->Index(IndexConceptSearchKeyId, ConceptUrl, ItemId);		
		}
	}
	BowDocBsConcepts->AddDoc(ItemId.GetStr(), TStrV(), ConceptsWIdWgtPrV);
	if (SettingUpdateThreadBow)
		BowDocBsConceptsByThread->AppendDoc(ThreadId.GetStr(), TStrV(), ConceptsThreadWIdWgtPrV);

	PXmlTok TextToIndex = ItemXml->GetTagTok("textToIndex");
	if (!TextToIndex.Empty())
	{
		TStr ExtraText = TextToIndex->GetTokStr(false);
		if (ExtraText != "")
		{
			TStrV ExtraTextTermV;
			IndexVoc->GetTokenizer()->GetTokens(ExtraText, ExtraTextTermV);
			if (IndexText == 1)
				Index->Index(IndexTextSearchKeyId, ExtraTextTermV, ItemId);
		}
	}

	return TStr(
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
"<addItemStatus itemId=\"" + ItemId.GetStr() + "\" threadId=\"" + ThreadId.GetStr() + "\" >"
"</addItemStatus>");
}


void TProfile::GetTokenIds(const TStr& Text, TIntV& TokenIdV)
{
	TStrV TokenV;
	TokenizerNoStemmingNoSwSet->GetTokens(Text, TokenV);
	for (int TokN = 0; TokN < TokenV.Len(); TokN++)
	{
		TStr& Tok = TokenV[TokN];
		
		if (SettingNGramsIgnoreSw && SwSet->IsIn(Tok.GetUc())) 
			TokenIdV.Add(0);
		else 
			TokenIdV.Add(BowDocBs->AddWordStr(Tok));
	}
}

void TProfile::SetTag(const TInt& ItemId, const TStr& TagId)
{
	
	if (ItemStore->SetTag(ItemId, TagId))
		Index->Index(IndexTagSearchKeyId, TagId, ItemId);
}

void TProfile::RemoveTag(const TInt& ItemId, const TStr& TagId)
{
	if (ItemStore->RemoveTag(ItemId, TagId))
		Index->Delete(IndexTagSearchKeyId, TagId, ItemId);
}

TStr TProfile::GetKeywordsFromConditions(const PXmlTok& Conditions)
{
	TStr Kws = "";
	if (!Conditions.Empty())
	{
		TXmlTokV XmlKeywordV;
		Conditions->GetTagTokV("keywords|kw", XmlKeywordV);
		for (int KwN=0; KwN < XmlKeywordV.Len(); KwN++)
			Kws += " " + XmlKeywordV[KwN]->GetTokStr(false);
	}
	return Kws;
}

POgRecSet TProfile::GetRecSetForQuery(const PXmlTok& Conditions, const PXmlTok& Ignores, const TStr& SortBy, const TStr& ResultData)
{
	POgRecSet ItemRecSet;
	
	TVec<POgRecSet> ConditionRecSetV;	
	
	if (!Conditions.Empty())
	{
		for (int TokN = 0; TokN < Conditions->GetSubToks(); TokN++)
		{
			PXmlTok ConditionTok = Conditions->GetSubTok(TokN);
			if (ConditionTok->GetSym() == xsyWs) continue;
			ConditionRecSetV.Add(GetRecSetForQueryItem(ConditionTok, SortBy, ResultData));
		}
	}
	
	if (ConditionRecSetV.Len() > 0)
		ItemRecSet = InsersectResultSets(ConditionRecSetV);
	else
		ItemRecSet = ItemStore->GetAllRecs();
	
	if (!Ignores.Empty() && Ignores->GetSubToks() > 0)
	{
		THashSet<TUInt64> IgnoreItemsH;
		TVec<POgRecSet> IgnoresRecSetV;
		for (int TokN = 0; TokN < Ignores->GetSubToks(); TokN++)
		{
			PXmlTok IgnoreTok = Ignores->GetSubTok(TokN);
			if (IgnoreTok->GetSym() == xsyWs) continue;
			POgRecSet IgnoreRecSet = GetRecSetForQueryItem(IgnoreTok, SortBy, ResultData);
			for (int i=0; i < IgnoreRecSet->GetRecs(); i++)
				IgnoreItemsH.AddKey(IgnoreRecSet->GetRecId(i));
		}
		
		if (IgnoreItemsH.Len() > 0)
		{
			TUInt64V FinalRecIdV;
			for (int i=0; i < ItemRecSet->GetRecs(); i++)
			{
				uint64 Id = ItemRecSet->GetRecId(i);
				if (!IgnoreItemsH.IsKey(Id))
					FinalRecIdV.Add(Id);
			}
			ItemRecSet = TOgRecSet::New(ItemStoreId, FinalRecIdV);
		}
	}
	return ItemRecSet;
}

POgRecSet TProfile::InsersectResultSets(const TVec<POgRecSet>& RecSetV)
{
	if (RecSetV.Len() == 0)
		return TOgRecSet::New();
	if (RecSetV.Len() == 1)
		return RecSetV[0];
	
	int IndexWithMinRecs = 0;
	int MinRecs = RecSetV[0]->GetRecs();
	for (int N=1; N < RecSetV.Len(); N++) {
		if (RecSetV[N]->GetRecs() < MinRecs) {
			MinRecs = RecSetV[N]->GetRecs(); IndexWithMinRecs = N; 
		}
	}
	THash<TUInt64, TInt> RecIdFqH; RecSetV[IndexWithMinRecs]->GetRecIdFqH(RecIdFqH);

	THash<TUInt64, TInt> RecIdInSets;				
	TUInt64V RecIdV; RecIdFqH.GetKeyV(RecIdV);
	int Recs = RecIdV.Len();
	for (int N=0; N < Recs; N++) {
		RecIdInSets.AddDat(RecIdV[N], 1);
	}
	for (int N=0; N < RecSetV.Len(); N++) {
		if (N == IndexWithMinRecs) continue;	
		POgRecSet Set = RecSetV[N];
		for (int RecN=0; RecN < Set->GetRecs(); RecN++) {
			uint64 RecId = Set->GetRecId(RecN);
			if (RecIdInSets.IsKey(RecId)) {
				RecIdInSets.AddDat(RecId, RecIdInSets.GetDat(RecId) + 1);					
				RecIdFqH.AddDat(RecId, RecIdFqH.GetDat(RecId) + Set->GetRecFq(RecN));		
			}
		}
	}

	TUInt64IntKdV RecIdFqV;
	int KeyId = RecIdInSets.FFirstKeyId();
	while (RecIdInSets.FNextKeyId(KeyId))
	{
		TUInt64 RecId; TInt Count;
		RecIdInSets.GetKeyDat(KeyId, RecId, Count);
		if (Count == RecSetV.Len()) {
			RecIdFqV.Add(TUInt64IntKd(RecId, RecIdFqH.GetDat(RecId)));
		}
	}
	return TOgRecSet::New(ItemStoreId, RecIdFqV, true);
}

TStr TProfile::GeneralQuery(const PXmlDoc& QueryXml)
{
	PXmlTok Conditions = QueryXml->GetTagTok("query|queryArgs|conditions");
	if (Conditions.Empty() || Conditions->GetSubToks() == 0)
	{
		LastInformation = "There were no conditions specified in the query";
		return "";
	}

	PXmlTok Ignores = QueryXml->GetTagTok("query|queryArgs|ignore");
	PXmlTok QueryParams = QueryXml->GetTagTok("query|params");
	
	TStr ResultType = GetStrArg(QueryParams, "resultData", "itemData");
	TStr Sorting = GetStrArg(QueryParams, "sortBy", "dateDesc");
	
	PXmlTok QueryArgs = QueryXml->GetTagTok("query|queryArgs");
	TStr QueryArgsStr = QueryArgs->GetTokStr(true);

	POgRecSet ItemRecSet = GetResultsForQuery(QueryArgsStr);
	if (ItemRecSet.Empty()) {
		ItemRecSet = GetRecSetForQuery(Conditions, Ignores);
		StoreResultsForQuery(QueryArgsStr, ItemRecSet);
	}
	
	if (ResultType == "itemData" && Sorting == "relevance" && ItemRecSet->GetRecs() > 0) {
		TStr Kws = GetKeywordsFromConditions(Conditions);
		if (Kws != "") {
			UpdateBowWgts();
			TUInt64Set ItemSet; ItemRecSet->GetRecIdSet(ItemSet);
			TVec<TFltStrPr> ItemsBySimStrV = GetSimilarityWithText(BowDocBs, BowDocWgtBs, ItemSet, Kws);
			TUInt64IntKdV ItemsBySimV;
			for (int i=0; i < ItemsBySimStrV.Len(); i++)
				ItemsBySimV.Add(TUInt64IntKd(ItemsBySimStrV[i].Val2.GetInt(1), (int) (10000 * ItemsBySimStrV[i].Val1)));
			ItemRecSet = TOgRecSet::New(ItemStoreId, ItemsBySimV, true);
		}
	}
	
	if (ResultType == "itemData")
	{
		if (Sorting == "dateDesc" || Sorting == "dateAsc")
			ItemRecSet->SortByField(OgBase, Sorting == "dateDesc" ? false : true, ItemStore->TimeFieldId);	
		else if (Sorting == "itemIdAsc" || Sorting == "itemIdDesc")
			ItemRecSet->SortById(Sorting == "itemIdAsc");
		else if (Sorting == "relevance")
			ItemRecSet->SortByFq(false);		
		return GetItemData(ItemRecSet, QueryXml);
	}
	else if (ResultType == "peopleData")
		return GetPeopleData(ItemRecSet, QueryXml);
	else if (ResultType == "timelineData")
		return GetTimelineData(ItemRecSet, QueryXml);
	else if (ResultType == "keywordData")
		return GetKeywordData(ItemRecSet, QueryXml);
	else if (ResultType == "itemIdData")
		return GetItemIdData(ItemRecSet, QueryXml);
	else 
	{
		LastInformation = "Unknown ResultType value";
		return "";
	}
}

POgRecSet TProfile::GetRecSetForQueryItem(const PXmlTok& Token, const TStr& SortBy, const TStr& ResultData)
{
	TStr TagNm = Token->GetTagNm();
	TOgQueryItemV QueryItemV;
	if (TagNm == "accounts")
	{
		TXmlTokV XmlEmailIdV;
		Token->GetTagTokV("account", XmlEmailIdV);
		
		THashSet<TUInt64> ItemRecH;
		for (int EmailN=0; EmailN < XmlEmailIdV.Len(); EmailN++) {
			TInt PersonRecId = XmlEmailIdV[EmailN]->GetIntArgVal("id", -1);
			if (PersonRecId == -1 || !PersonStore->IsRecId(PersonRecId))
				continue;
			TStr RoleList = XmlEmailIdV[EmailN]->GetStrArgVal("role", AnyRoleNm);
			TStrV RoleV;			
			RoleList.SplitOnAllCh(',', RoleV);
			TOgRec PersonRec(PersonStoreId, PersonRecId);
			for (int RoleN = 0; RoleN < RoleV.Len(); RoleN++) {
				if (PersonStore->IsJoinNm(RoleV[RoleN])) {
					int JoinId = PersonStore->GetJoinId(RoleV[RoleN]);
					POgRecSet Set = PersonRec.DoJoin(OgBase, JoinId);
					TUInt64V RecIdV; Set->GetRecIdV(RecIdV);
					ItemRecH.AddKeyV(RecIdV);
				}
			}
		}
		TUInt64V AllRecIdV; ItemRecH.GetKeyV(AllRecIdV);
		return TOgRecSet::New(ItemStoreId, AllRecIdV);
	}
	else if (TagNm == "keywords")
	{
		TXmlTokV XmlKeywordV;
		Token->GetTagTokV("kw", XmlKeywordV);
		bool optional = GetBoolArg(Token, "optional", false);
		TOgQueryItemV QueryItemV;
		for (int KwN=0; KwN < XmlKeywordV.Len(); KwN++) {
			TStr Kw = XmlKeywordV[KwN]->GetTokStr(false);
			TStrV KwV; IndexVoc->GetTokenizer()->GetTokens(Kw, KwV);
			for (int i = 0; i < KwV.Len(); i++) {
				if (!IndexVoc->IsWordStr(IndexTextSearchKeyId, KwV[i]) && optional == false) 
					return TOgRecSet::New(ItemStoreId);
				QueryItemV.Add(TOgQueryItem(OgBase, IndexTextSearchKeyId, KwV[i], oqctEqual));
			}
		}
		
		if (QueryItemV.Len() == 0)
			return TOgRecSet::New(ItemStoreId);
		return OgBase->Search(TOgQuery::New(OgBase, TOgQueryItem(optional ? oqitOr : oqitAnd, QueryItemV)));
	}
	else if (TagNm == "concepts")
	{
		TXmlTokV XmlConceptV;
		Token->GetTagTokV("concept", XmlConceptV);
		for (int ConN=0; ConN < XmlConceptV.Len(); ConN++) {
			TStr Concept = XmlConceptV[ConN]->GetTokStr(false);
			if (IndexVoc->IsWordStr(IndexConceptSearchKeyId, Concept)) {
				uint64 WId = IndexVoc->GetWordId(IndexConceptSearchKeyId, Concept);
				QueryItemV.Add(TOgQueryItem(OgBase, IndexConceptSearchKeyId, WId, oqctEqual));
			}
		}
		return QueryItemV.Len() > 0 ? OgBase->Search(TOgQuery::New(OgBase, TOgQueryItem(oqitOr, QueryItemV))) : TOgRecSet::New(ItemStoreId);
	}
	
	else if (TagNm == "tagIds")
	{
		TStr TagIdsStr = Token->GetTokStr(false);
		TStrV TagIdsV;
		TagIdsStr.SplitOnAllCh(',', TagIdsV);
		for (int TagN=0; TagN < TagIdsV.Len(); TagN++) {
			if (IndexVoc->IsWordStr(IndexTagSearchKeyId, TagIdsV[TagN]))
				QueryItemV.Add(TOgQueryItem(OgBase, IndexTagSearchKeyId, TagIdsV[TagN], oqctEqual));
		}
		return QueryItemV.Len() > 0 ? OgBase->Search(TOgQuery::New(OgBase, TOgQueryItem(oqitOr, QueryItemV))) : TOgRecSet::New(ItemStoreId);
	}
	
	else if (TagNm == "itemIds")
	{
		TStr ItemIdsStr = Token->GetTokStr(false);
		TStrV ItemIdsV;
		ItemIdsStr.SplitOnAllCh(',', ItemIdsV);
		TUInt64IntKdV RecIdFqV;
		for (int i=0; i < ItemIdsV.Len(); i++) {
			TUInt64 id = ItemIdsV[i].GetUInt64(-1);
			if (id != -1 && ItemStore->IsItem((int) id))
				RecIdFqV.Add(TUInt64IntKd(id, 1));
		}
		return TOgRecSet::New(ItemStoreId, RecIdFqV, false);
	}
	else if (TagNm == "threads")
	{
		TStr ItemIdsStr = Token->GetTokStr(false);
		TStrV ItemIdsV;
		ItemIdsStr.SplitOnAllCh(',', ItemIdsV);
		TUInt64IntKdV RecIdFqV;
		for (int i=0; i < ItemIdsV.Len(); i++) {
			TUInt64 ItemId = ItemIdsV[i].GetUInt64(-1);
			TInt ThreadId = ItemStore->GetFieldInt(ItemId, ItemStore->ThreadIdFieldId);	
			if (ThreadId != -1) {
				TUInt64V ThreadItemsIdV = GetItemIdsForThread(ThreadId);
				for (int j=0; j < ThreadItemsIdV.Len(); j++)						
					RecIdFqV.Add(TUInt64IntKd(ThreadItemsIdV[j], 1));
			}
		}
		return TOgRecSet::New(ItemStoreId, RecIdFqV, false);
	}
	else if (TagNm == "threadIds")
	{
		TStr ThreadIdsStr = Token->GetTokStr(false);
		TStrV ThreadIdsV; ThreadIdsStr.SplitOnAllCh(',', ThreadIdsV);
		TUInt64IntKdV RecIdFqV;
		for (int i=0; i < ThreadIdsV.Len(); i++) {
			TInt ThreadId = ThreadIdsV[i].GetUInt64(-1);
			if (ThreadId != -1) {
				TUInt64V ThreadItemsIdV = GetItemIdsForThread(ThreadId);
				for (int j=0; j < ThreadItemsIdV.Len(); j++)						
					RecIdFqV.Add(TUInt64IntKd(ThreadItemsIdV[j], 1));
			}
		}
		return TOgRecSet::New(ItemStoreId, RecIdFqV, false);
	}
	else if (TagNm == "threadStr")
	{
		TStr ThreadStr = Token->GetTokStr(false);
		TUInt64IntKdV RecIdFqV;
		if (!ThreadStore->IsRecNm(ThreadStr))
			return TOgRecSet::New(ItemStoreId, RecIdFqV, false);
		int ThreadId = ThreadStore->AddRec(ThreadStr);
		TUInt64V ThreadItemsIdV = GetItemIdsForThread(ThreadId);
		for (int j=0; j < ThreadItemsIdV.Len(); j++)		
			RecIdFqV.Add(TUInt64IntKd(ThreadItemsIdV[j], 1));
		return TOgRecSet::New(ItemStoreId, RecIdFqV, false);
	}
	else if (TagNm == "timeline")
	{
		TUInt64 Start = Token->GetArgVal("start", "0").GetInt64(0);
		TUInt64 End = Token->GetArgVal("end", "0").GetInt64(0);
		if (Start > 0 && End > 0) {
			TOgOpLinSearch LinSearch;
			return LinSearch.Exec(OgBase, ItemStoreId, ItemStore->TimeFieldId, Start, End);
		}
	}
	else if (TagNm == "itemTypes")
	{
		TStr ItemTypesStr = Token->GetTokStr(false);
		TStrV ItemTypesV;
		ItemTypesStr.SplitOnAllCh(',', ItemTypesV);
		
		for (int TypesN=0; TypesN < ItemTypesV.Len(); TypesN++) {
			if (IndexVoc->IsWordStr(IndexItemTypeSearchKeyId, ItemTypesV[TypesN]))
				QueryItemV.Add(TOgQueryItem(OgBase, IndexItemTypeSearchKeyId, ItemTypesV[TypesN], oqctEqual));
		}
		return QueryItemV.Len() > 0 ? OgBase->Search(TOgQuery::New(OgBase, TOgQueryItem(oqitOr, QueryItemV))) : TOgRecSet::New(ItemStoreId);
	}
	else {
		TStr error = "Unknown tag name: " + TagNm + "!";
		printf(error.CStr());
		TOgExcept::New(error);
	}
	return TOgRecSet::New(ItemStoreId);
}

TStr TProfile::GetItemData(const POgRecSet& RecSet, const PXmlDoc& QueryXml)
{
	TInt Offset = GetIntArg(QueryXml->GetTagTok("query|params"), "offset", 0);
	TInt MaxCount = GetIntArg(QueryXml->GetTagTok("query|params"), "maxCount", 100);
	bool IncludeAttachments = GetBoolArg(QueryXml->GetTagTok("query|params"), "includeAttachments", true);

	TChA ResultChA;
	ResultChA += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	ResultChA += "<results type=\"itemData\">\n";
	ResultChA += TStr::FmtBf(Bf, BfSize, "	<info totalCount=\"%d\" offset=\"%d\" maxCount=\"%d\" />\n", (int) RecSet->GetRecs(), (int) Offset, (int) MaxCount);

	ResultChA += "	<items>\n";
	for (int RecN = Offset; RecN < RecSet->GetRecs() && RecN < Offset+MaxCount; RecN++) {
		const int ItemId = (int) RecSet->GetRecId(RecN);
		if (ItemId >= 0)
			AddItemResult(ItemId, ResultChA, IncludeAttachments);
	}
	ResultChA += "	</items>\n";

	ResultChA += "</results>";
	return ResultChA;
}

TStr TProfile::GetPeopleData(const POgRecSet& RecSet, const PXmlDoc& QueryXml)
{
	TChA ResultChA;

	TInt MaxCountItems = GetIntArg(QueryXml->GetTagTok("query|params"), "maxCountItems", 1000);
	
	ResultChA += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	ResultChA += "<results type=\"peopleData\">\n";

	ResultChA += TStr::FmtBf(Bf, BfSize, "	<info totalCount=\"%d\" />\n", (int) RecSet->GetRecs());

	TTm StartTm = TTm::GetCurLocTm(); 

	THash<TStr, TInt> EmailIdToCountH;
	THash<TInt, TInt> FromCountH;
	TStr Key;
	
	TInt SenderId;
	TIntV RecipientIdV;
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
		SenderId = -1;
		RecipientIdV.Clr();
				
		const int ItemId = (int) RecSet->GetRecId(RecN);		
		TPartV PartV = ItemStore->GetPartV(ItemId);
		for (int i=0; i < PartV.Len(); i++) {
			if (PartV[i].Val1 == PersonStore->JoinHasFromId) SenderId = PartV[i].Val2;
			else if (PartV[i].Val1 == PersonStore->JoinHasToId || PartV[i].Val1 == PersonStore->JoinHasCCId || PartV[i].Val1 == PersonStore->JoinHasBCCId) 
				RecipientIdV.Add(PartV[i].Val2);
		}
		
		if (SenderId != -1) {
			FromCountH.AddDat(SenderId, FromCountH.IsKey(SenderId) ? FromCountH.GetDat(SenderId) + 1 : 1);
			for (int RecipN = 0; RecipN < RecipientIdV.Len(); RecipN++) {
				Key = TStr::FmtBf(Bf, BfSize, "%d-%d", (int) SenderId, (int) RecipientIdV[RecipN]);
				EmailIdToCountH.AddDat(Key, EmailIdToCountH.IsKey(Key) ? EmailIdToCountH.GetDat(Key) + 1 : 1);
			}
		}
	}

	TTm EndTm = TTm::GetCurLocTm(); 

	TVec<TPair<TInt, TStr> > CountToPairV;
	for (int KeyId=0; KeyId < EmailIdToCountH.Len(); KeyId++) {
		TStr Pair;
		TInt Count;
		EmailIdToCountH.GetKeyDat(KeyId, Pair, Count);
		CountToPairV.Add(TPair<TInt, TStr>(Count, Pair));
	}
	CountToPairV.Sort(false);

	ResultChA += "	<fromCounts>\n";
	for (int i=0; i < FromCountH.Len(); i++) {
		if (i > 0) ResultChA += ",";
		ResultChA += TStr::FmtBf(Bf, BfSize, "%d-%d", (int) FromCountH.GetKey(i), (int) FromCountH[i]);
	}
	ResultChA += "</fromCounts>\n";

	ResultChA += "	<fromToCounts>\n";
	for (int i=0; i < CountToPairV.Len() && i < MaxCountItems; i++) {
		if (i > 0) ResultChA += ",";
		ResultChA += TStr::FmtBf(Bf, BfSize, "%s-%d", CountToPairV[i].Val2.CStr(), (int) CountToPairV[i].Val1);
	}
	ResultChA += "</fromToCounts>\n";

	ResultChA += "</results>";	
	return ResultChA;
}

// return the timeline information for the items in the result set
TStr TProfile::GetTimelineData(const POgRecSet& RecSet, const PXmlDoc& QueryXml)
{
	TChA ResultChA;
	
	ResultChA += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	ResultChA += "<results type=\"timelineData\">\n";

	ResultChA += TStr::FmtBf(Bf, BfSize, "	<info totalCount=\"%d\" />\n", (int) RecSet->GetRecs());

	THash<TStr, TIntV> MonthToCountH;		
	THash<TStr, TInt> EmailIdToCountH;

	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
		const int ItemId = (int) RecSet->GetRecId(RecN);

		TUInt64 Time = ItemStore->GetFieldUInt64(ItemId, ItemStore->TimeFieldId);
		
		TTm docTime = TTm::GetTmFromMSecs(Time);
		int day = docTime.GetDay() - 1;		
		TStr Key = TStr::FmtBf(Bf, BfSize, "%d-%d", (int) docTime.GetYear(), (int) docTime.GetMonth());
		TIntV MonthV;
		if (MonthToCountH.IsKey(Key))
			MonthV = MonthToCountH.GetDat(Key);
		else
			MonthV = TIntV(31);
		MonthV[day] = MonthV[day] + 1;
		MonthToCountH.AddDat(Key, MonthV);
	}

	ResultChA += "	<dates>\n";
	for (int KeyId=0; KeyId < MonthToCountH.Len(); KeyId++)	{
		TStr Key;
		TIntV MonthV;
		MonthToCountH.GetKeyDat(KeyId, Key, MonthV);
		ResultChA += TStr::FmtBf(Bf, BfSize, "		<month val=\"%s\">", Key.CStr());
		ResultChA += TStr::FmtBf(Bf, BfSize, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", (int)MonthV[0],(int)MonthV[1],(int)MonthV[2],(int)MonthV[3],(int)MonthV[4],(int)MonthV[5],(int)MonthV[6],(int)MonthV[7],(int)MonthV[8],(int)MonthV[9],(int)MonthV[10],(int)MonthV[11],(int)MonthV[12],(int)MonthV[13],(int)MonthV[14],(int)MonthV[15],(int)MonthV[16],(int)MonthV[17],(int)MonthV[18],(int)MonthV[19],(int)MonthV[20],(int)MonthV[21],(int)MonthV[22],(int)MonthV[23],(int)MonthV[24],(int)MonthV[25],(int)MonthV[26],(int)MonthV[27],(int)MonthV[28],(int)MonthV[29],(int)MonthV[30]);
		ResultChA += "</month>\n";
	}
	ResultChA += "	</dates>\n";

	ResultChA += "</results>";
	return ResultChA;
}

// return the keywords for the items in the result set
TStr TProfile::GetKeywordData(const POgRecSet& RecSet, const PXmlDoc& QueryXml)
{
	TChA ResultChA;
	PXmlTok ParamsTok = QueryXml->GetTagTok("query|params");
	TInt SampleSize = ParamsTok->GetIntArgVal("sampleSize", 1000);
	TInt KeywordCount = ParamsTok->GetIntArgVal("keywordCount", 30);
	TStr MethodUsed = ParamsTok->GetStrArgVal("keywordMethod", "localConceptSpV");
	TStr SVMInterestingClass = ParamsTok->GetStrArgVal("SVMInterestingClass", "positive");
	TStr KeywordSource = ParamsTok->GetStrArgVal("keywordSource", "text");
	
	if (SampleSize <= 0) 
		SampleSize = 1000;
	SampleSize = min((int) SampleSize, 1000);
	POgRecSet SampleRecSet = RecSet->GetSampleRecSet(SampleSize, false);
	PBowKWordSet KWordSet = ComputeKWordSet(SampleRecSet, MethodUsed, KeywordSource, SVMInterestingClass);
	
	ResultChA += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	ResultChA += "<results type=\"keywordData\">\n";

	ResultChA += TStr::FmtBf(Bf, BfSize, "	<info totalCount=\"%d\" usedRecCount=\"%d\" />\n", RecSet->GetRecs(), SampleRecSet->GetRecs());
	if (!KWordSet.Empty())
		WriteKeywordSet(ResultChA, KWordSet, KeywordCount);
	ResultChA += "</results>";
	return ResultChA;
}

void TProfile::WriteKeywordSet(TChA& ResultChA, const PBowKWordSet KWordSet, int KeywordCount)
{
	KWordSet->SortByWgt();
	ResultChA += "<keywords>\n		";
	for (int i=0; i < KWordSet->GetKWords() && i < KeywordCount; i++)
		ResultChA += TStr::FmtBf(Bf, BfSize, "<kw str=\"%s\" wgt=\"%.3f\"/>", TXmlLx::GetXmlStrFromPlainStr(KWordSet->GetKWordStr(i)).CStr(), KWordSet->GetKWordWgt(i));
	ResultChA += "</keywords>\n";
}

PBowKWordSet TProfile::ComputeKWordSet(const POgRecSet& RecSet, const TStr& MethodUsed, const TStr& KeywordSource, const TStr& SVMInterestingClass)
{
	PBowSpV BowSpV;
	PBowKWordSet KWordSet = NULL;

	if (RecSet->GetRecs() == 0)
		return KWordSet;

	PBowDocBs BowData;
	PBowDocWgtBs BowWgtData;
	if (KeywordSource == "concepts") {
		UpdateBowWgtsConcepts();
		BowData = BowDocBsConcepts;
		BowWgtData = BowDocWgtBsConcepts;
	}
	else {
		UpdateBowWgts();
		BowData = BowDocBs;
		BowWgtData = BowDocWgtBs;
	}

	if (MethodUsed == "globalConceptSpV") {
		TIntV DIdV; GetBowDocIdV(RecSet, BowData, DIdV);
		BowSpV = TBowClust::GetConceptSpV(BowWgtData, TBowSim::New(bstCos), DIdV);
		KWordSet = BowSpV->GetKWordSet(BowData);
	}
	else if (MethodUsed == "localConceptSpV") {
		TIntV DIdV; GetBowDocIdV(RecSet, BowData, DIdV);
		PBowDocBs TempBowDocBs = BowData->GetSubDocSet(DIdV);
		TIntV AllDocIdV; TempBowDocBs->GetAllDIdV(AllDocIdV);
		PBowDocWgtBs TempBowDocWgtBs = TBowDocWgtBs::New(TempBowDocBs, bwwtNrmTFIDF);
		BowSpV = TBowClust::GetConceptSpV(TempBowDocWgtBs, TBowSim::New(bstCos), AllDocIdV);
		KWordSet = BowSpV->GetKWordSet(TempBowDocBs);
	}
	else if (MethodUsed == "SVM") {
		TIntV AllDocIds;
		BowData->GetAllDIdV(AllDocIds);
		
		TIntV PosBowDIdV;
		GetBowDocIdV(RecSet, BowData, PosBowDIdV);

		TStr CatNm("SVM-PositiveExamples");
		BowData->SetCatToBowDIds(CatNm, PosBowDIdV);

		int classSign = SVMInterestingClass.GetLc() == "positive" ? 1 : -1;
		BowSpV = TBowSVMMd::GetKeywords(BowData, BowWgtData, AllDocIds, CatNm, 200, 1, 1, 10, classSign, 1, true);
		BowData->RemoveCatFromBowDIds(CatNm, PosBowDIdV);
		KWordSet = BowSpV->GetKWordSet(BowData);
	}
	return KWordSet;
}

void TProfile::GetBowDocIdV(const POgRecSet& RecSet, const PBowDocBs& BowDocBs, TIntV& BowDocIdV)
{
	for (int i=0; i < RecSet->GetRecs(); i++) {
		TInt ItemId = (int) RecSet->GetRecId(i);
		if (BowDocBs->IsDocNm(ItemId.GetStr())) {
			int BowDocId = BowDocBs->GetDId(ItemId.GetStr());
			BowDocIdV.Add(BowDocId);
		}
	}
}

POgRecSet TProfile::GetRecSetForBowDocIdV(const TIntV& DocIdV, const PBowDocBs& BowDocBs) 
{
	TUInt64V ItemIdV;
	for (int i=0; i < DocIdV.Len(); i++) {
		TInt DId = DocIdV[i];
		TStr ItemIdStr = BowDocBs->GetDocNm(DId);
		uint64 ItemId = ItemIdStr.GetUInt64(TUInt64::Mx);
		if (ItemId != TUInt64::Mx)
			ItemIdV.Add(ItemId);
	}
	return TOgRecSet::New(ItemStoreId, ItemIdV);
}

TStr TProfile::GetItemIdData(const POgRecSet& RecSet, const PXmlDoc& QueryXml)
{
	TChA ResultChA;

	ResultChA += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
	ResultChA += "<results type=\"itemIdData\">\n";
	ResultChA += TStr::FmtBf(Bf, BfSize, "	<info totalCount=\"%d\" />\n", (int)RecSet->GetRecs());
	ResultChA += "	<itemIds>";
	for (int RecN=0; RecN < RecSet->GetRecs(); RecN++) {
		const int ItemId = (int) RecSet->GetRecId(RecN);
		if (ItemId >= 0) {
			if (RecN > 0) ResultChA += ",";
			ResultChA += TStr::FmtBf(Bf, BfSize, "%d", ItemId);
		}
	}
	ResultChA += "</itemIds>\n";
	ResultChA += "</results>";

	return ResultChA;
}

int TProfile::StoreResults(const POgRecSet& RecSet, const bool Save)
{
	int QueryId = GetNextQueryId();
	if (Save)
		SavedResults.AddDat(QueryId, RecSet);
	else {
		CachedResults.AddDat(QueryId, RecSet);
		int i=0; 
		while (CachedResults.Len() > 100) {
			CachedResults.DelIfKey(i);
			i++;
		}
	}
	return QueryId;
}

POgRecSet TProfile::GetResults(const int QueryId)
{ 
	if (CachedResults.IsKey(QueryId))
		return CachedResults.GetDat(QueryId)->Clone();
	else if (SavedResults.IsKey(QueryId))
		return SavedResults.GetDat(QueryId)->Clone();
	return NULL;
}

TStr TProfile::UpdateItem(const TStr& ItemInfo, const TStr& ItemContent)
{
	LastInformation = "";
	PXmlDoc ItemDataXml = TXmlDoc::LoadStr(ItemInfo);
	if (!ItemDataXml->IsOk())
		return BuildErrorInfo("Invalid XML data: " + ItemDataXml->GetMsgStr());
	
	PXmlTok ItemXml = ItemDataXml->GetTagTok("item");
	
	TInt ItemId = ItemXml->GetIntArgVal("itemId", -1);
	if (ItemId == -1) {
		return BuildErrorInfo("ItemId was not provided. Updating was not performed.");
	}
	TInt ThreadId = ItemXml->GetIntArgVal("threadId", -1);

	TStr ItemTypeStr = ItemXml->GetStrArgVal("itemType", "-1");
	ItemTypeEnum ItemType = (ItemTypeEnum) ItemTypeStr.GetInt(-1);
	TItemStore::TItemRec Item;
	ItemStore->GetItem(ItemId, Item);

	if (ItemType != Invalid && ItemType != Item.ItemType) {
		TInt OldItemType = (int) Item.ItemType.Val;
		Index->Delete(IndexItemTypeSearchKeyId, OldItemType.GetStr(), ItemId);
		Item.ItemType = ItemType;
		Index->Index(IndexItemTypeSearchKeyId, ItemTypeStr, ItemId);
	}

	TStr TimeStr = ItemXml->GetArgVal("time", "");
	if (TimeStr != "")
		Item.Time = TimeStr.GetUInt64();

	bool updateItemSubject = GetBoolArg(ItemXml, "updateItemSubject", "True");
	PXmlTok SubjectTok = ItemXml->GetTagTok("subject");
	TStr Subject = SubjectTok.Empty() ? "" : SubjectTok->GetTokStr(false);
	if (updateItemSubject)
		RemoveItemSubjectFromIndex(ItemId, ThreadId);

	bool updateItemContent = GetBoolArg(ItemXml, "updateItemContent", "True");
	if (updateItemContent)
		RemoveItemContentFromIndex(ItemId);
	
	TStrV ThreadTermV;  IndexVoc->GetTokenizer()->GetTokens(Subject, ThreadTermV);
	TStrV ContentTermV; IndexVoc->GetTokenizer()->GetTokens(ItemContent, ContentTermV);
		
	TStrV FullTextTermV;
	if (updateItemSubject) FullTextTermV.AddV(ThreadTermV);
	if (updateItemContent) FullTextTermV.AddV(ContentTermV);
	
	if (updateItemSubject) {
		BowDocBsSubjects->DelDoc(ThreadId.GetStr());
		BowDocBsSubjects->AddDoc(ThreadId.GetStr(), TStrV(), ThreadTermV);
		if (SettingUpdateThreadBow) {
			int DId = BowDocBs->GetDId(ItemId.GetStr());
			PBowSpV BowSpV = BowDocBs->GetDocSpV(DId);
			int TDId = BowDocBsWholeThreads->GetDId(ThreadId.GetStr());
			PBowSpV TBowSpV = BowDocBsWholeThreads->GetDocSpV(TDId);
			for (int i=0; i < BowSpV->Len(); i++) {
				int WId = BowSpV->GetWId(i);
				TStr Word = BowDocBs->GetWordStr(WId);
				int TWId = BowDocBsWholeThreads->GetWId(Word);			
				TBowSpV->DecreaseWIdWgt(TWId, BowSpV->GetWgt(i));
			}
			BowDocBsWholeThreads->AppendDoc(ThreadId.GetStr(), TStrV(), ContentTermV);
		}
	}

	if (updateItemContent) {
		BowDocBs->DelDoc(ItemId.GetStr());
		BowDocBs->AddDoc(ItemId.GetStr(), TStrV(), FullTextTermV);
	}

	// index new content
	Index->Index(IndexTextSearchKeyId, FullTextTermV, ItemId);

	TPartV ParticipantsV = ItemStore->GetPartV(Item.PartVId);
	TXmlTokV addedAccounts;
	ItemXml->GetTagTokV("addedPeople|person", addedAccounts);
	if (addedAccounts.Len() > 0) {
		for (int accInd = 0; accInd < addedAccounts.Len(); accInd++) 
		{
			TInt IdInt = addedAccounts[accInd]->GetIntArgVal("id", -1);		
			int PersonId = PersonStore->AddPerson(IdInt);
			TStr Role = addedAccounts[accInd]->GetArgVal("role");		
		
			Index->IndexJoin(PersonStore, PersonStore->JoinHasAnyRoleId, PersonId, ItemId);	
			int RoleJoinId = PersonStore->AddRole(Role);									
			Index->IndexJoin(PersonStore, RoleJoinId, PersonId, ItemId);
				
			ParticipantsV.Add(TPart(RoleJoinId, PersonId));
			if (ThreadId != -1)
				Index->IndexJoin(PersonStore, PersonStore->JoinHasThreadsId, PersonId, ThreadId);				
		}
	}

	TXmlTokV removedAccounts;
	ItemXml->GetTagTokV("removedPeople|person", removedAccounts);
	if (removedAccounts.Len() > 0) {
		for (int accInd = 0; accInd < removedAccounts.Len(); accInd++) 
		{
			TInt IdInt = removedAccounts[accInd]->GetIntArgVal("id", -1);		
			int PersonId = PersonStore->AddPerson(IdInt);
			TStr Role = removedAccounts[accInd]->GetArgVal("role");		
		
			int RoleJoinId = PersonStore->AddRole(Role);								
			Index->DeleteJoin(PersonStore, RoleJoinId, PersonId, ItemId);				
			Index->DeleteJoin(PersonStore, PersonStore->JoinHasAnyRoleId, PersonId, ItemId);	
	
			ParticipantsV.DelAll(TPart(RoleJoinId, PersonId));
		}
	}
	if (addedAccounts.Len() > 0 || removedAccounts.Len() > 0)
		ItemStore->SetParticipants(ItemId, ParticipantsV);

	
	// update the concepts bow
	int DId = BowDocBsConcepts->GetDId(ItemId.GetStr());
	PBowSpV BowConceptSpV = BowDocBsConcepts->IsDId(DId) ? BowDocBsConcepts->GetDocSpV(DId) : NULL;
	int ThreadDId = BowDocBsConceptsByThread->GetDId(ThreadId.GetStr());
	PBowSpV BowConceptThreadSpV = BowDocBsConceptsByThread->IsDId(ThreadDId) ? BowDocBsConceptsByThread->GetDocSpV(ThreadDId) : NULL;
	TXmlTokV ConceptV;
	
	ItemXml->GetTagTokV("updateConcepts|concept", ConceptV);
	if (ConceptV.Len() > 0 && !BowConceptSpV.Empty()) 
	{
		for (int i=0; i < ConceptV.Len(); i++)
		{
			TStr ConceptUrl = ConceptV[i]->GetArgVal("id", "");
			TFlt Wgt = ConceptV[i]->GetFltArgVal("weight", 0);
			int WId = BowDocBsConcepts->AddWordStr(ConceptUrl);
			BowConceptSpV->IncreaseWIdWgt(WId, Wgt);
			if (SettingUpdateThreadBow && !BowConceptThreadSpV.Empty()) {
				int TWId = BowDocBsConceptsByThread->AddWordStr(ConceptUrl);
				BowConceptThreadSpV->IncreaseWIdWgt(WId, Wgt);
			}
		}
	}

	ItemStore->UpdateItem(ItemId, Item);
	return TStr(
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<addItemStatus itemId=\"" + ItemId.GetStr() + "\" threadId=\"" + ThreadId.GetStr() + "\" >"
		"</addItemStatus>");
}

bool TProfile::RemoveItem(const TInt& ItemId)
{
	TItemStore::TItemRec Item;
	bool ok = ItemStore->GetItem(ItemId, Item);
	if (!ok)
		return false;
	TOgRec ItemRec(ItemStoreId, ItemId);
	TInt ThreadId = Item.ThreadId;
	TInt ItemType = (int) Item.ItemType;

	if (ThreadId != -1)
		Index->DeleteJoin(ThreadStore, ThreadStore->JoinHasItemsId, ThreadId, ItemId);

	Index->Delete(IndexItemTypeSearchKeyId, ItemType.GetStr(), ItemId);

	TPartV PartV = ItemStore->GetPartV(ItemId);
	for (int i=0; i < PartV.Len(); i++)	{
		TSInt RoleJoinId = PartV[i].Val1;
		TInt PersonId = PartV[i].Val2;
		Index->DeleteJoin(PersonStore, RoleJoinId, PersonId, ItemId);				
		Index->DeleteJoin(PersonStore, PersonStore->JoinHasAnyRoleId, PersonId, ItemId);	
	}
	
	POgRecSet AttachmentsSet = ItemRec.DoJoin(OgBase, ItemStore->JoinHasAttachmentsId);
	TUInt64V AttachmentIdV;
	AttachmentsSet->GetRecIdV(AttachmentIdV);
	for (int AttN = 0; AttN < AttachmentIdV.Len(); AttN++)	{
		AttachmentStore->RemoveItem((int) AttachmentIdV[AttN]);
		Index->DeleteJoin(ItemStore, ItemStore->JoinHasAttachmentsId, ItemId, AttachmentIdV[AttN]);	
	}

	TStrV TagsV; ItemStore->GetFieldStrV(ItemId, ItemStore->TagsFieldId, TagsV);
	for (int TagInd = 0; TagInd < TagsV.Len(); TagInd++) 
		Index->Delete(IndexTagSearchKeyId, TagsV[TagInd], ItemStoreId, ItemId);

	POgRecSet RelatedSet = ItemRec.DoJoin(OgBase, ItemStore->JoinHasRelatedItemsId);
	TUInt64V RelatedIdV;
	RelatedSet->GetRecIdV(RelatedIdV);
	for (int RelatedN = 0; RelatedN < RelatedIdV.Len(); RelatedN++)	{
		int RelatedItemId = (int) RelatedIdV[RelatedN];
		Index->DeleteJoin(ItemStore, ItemStore->JoinHasRelatedItemsId, ItemId, RelatedItemId);	
	}

	RemoveItemContentFromIndex(ItemId);
	RemoveItemSubjectFromIndex(ItemId, ThreadId);
	
	if (BowDocBs->IsDocNm(ItemId.GetStr())) {
		if (SettingUpdateThreadBow) {
			int DId = BowDocBs->GetDId(ItemId.GetStr());
			PBowSpV BowSpV = BowDocBs->GetDocSpV(DId);
			int TDId = BowDocBsWholeThreads->GetDId(ThreadId.GetStr());
			PBowSpV TBowSpV = BowDocBsWholeThreads->GetDocSpV(TDId);
			for (int i=0; i < BowSpV->Len(); i++) {
				int WId = BowSpV->GetWId(i);
				TStr Word = BowDocBs->GetWordStr(WId);
				int TWId = BowDocBsWholeThreads->GetWId(Word);			
				TBowSpV->DecreaseWIdWgt(TWId, BowSpV->GetWgt(i));
			}
		}
		BowDocBs->DelDoc(ItemId.GetStr());
	}
	
	if (BowDocBsConcepts->IsDocNm(ItemId.GetStr())) {
		int DId = BowDocBsConcepts->GetDId(ItemId.GetStr());
		PBowSpV ConceptBowSpV = BowDocBsConcepts->GetDocSpV(DId);
		for (int i=0; i < ConceptBowSpV->Len(); i++) {
			TStr ConceptUrl = BowDocBsConcepts->GetWordStr(ConceptBowSpV->GetWId(i));
			Index->Delete(IndexConceptSearchKeyId, ConceptUrl, ItemId);		// index the concept to make it searchable
		}
		if (SettingUpdateThreadBow) {
			int TDId = BowDocBsConceptsByThread->GetDId(ThreadId.GetStr());
			PBowSpV TConceptBowSpV = BowDocBsConceptsByThread->GetDocSpV(TDId);
			for (int i=0; i < ConceptBowSpV->Len(); i++) {
				int WId = ConceptBowSpV->GetWId(i);
				TStr Word = BowDocBsConcepts->GetWordStr(WId);
				int TWId = BowDocBsWholeThreads->GetWId(Word);			
				TConceptBowSpV->DecreaseWIdWgt(TWId, ConceptBowSpV->GetWgt(i));
			}
		}
		BowDocBsConcepts->DelDoc(ItemId.GetStr());
	}

	
	if (ItemType == Email) {
		POgRecSet AttachmentsSet = ItemRec.DoJoin(OgBase, ItemStore->JoinHasAttachmentsId);
		TUInt64V AttachmentIdV;
		AttachmentsSet->GetRecIdV(AttachmentIdV);
		for (int AttN = 0; AttN < AttachmentsSet->GetRecs(); AttN++) {
			uint64 recId = AttachmentsSet->GetRec(AttN).GetRecId();
			int AttItemId = AttachmentStore->GetFieldInt(recId, AttachmentStore->AttItemIdFieldId);
			RemoveItem(AttItemId);
		}
	}

	
	ItemStore->RemoveItem(ItemId);

	return true;
}


void TProfile::RemoveItemSubjectFromIndex(const TInt& ItemId, const TInt& ThreadId)
{
	TStr ThreadIdStr = ThreadId.GetStr();
	if (ThreadId != -1 && BowDocBsSubjects->IsDocNm(ThreadIdStr)) {
		int BowDId = BowDocBsSubjects->GetDId(ThreadIdStr);
		PBowSpV BowSpV = BowDocBsSubjects->GetDocSpV(BowDId);
		for (int WIdN = 0; WIdN < BowSpV->GetWIds(); WIdN++) {
			int WId = BowSpV->GetWId(WIdN);
			TStr WordStr = BowDocBsSubjects->GetWordStr(WId);
			Index->Delete(IndexTextSearchKeyId, WordStr, ItemId);
		}
		// note: we cannot remove the subject by calling BowDocBsSubjects->DelDoc since there are likely other emails with the same subject. Also, we don't update the BowDocBsWholeThreads.
	}
}

void TProfile::RemoveItemContentFromIndex(const TInt& ItemId)
{
	TStr ItemIdStr = ItemId.GetStr();
	if (!BowDocBs->IsDocNm(ItemIdStr))
		return;
	
	int BowDId = BowDocBs->GetDId(ItemIdStr);
	PBowSpV BowSpV = BowDocBs->GetDocSpV(BowDId);
	for (int WIdN = 0; WIdN < BowSpV->GetWIds(); WIdN++) {
		int WId = BowSpV->GetWId(WIdN);
		TStr WordStr = BowDocBs->GetWordStr(WId);
		Index->Delete(IndexTextSearchKeyId, WordStr, ItemId);
	}
}

bool TProfile::SetTagData(const TStr& TagData)
{
	LastInformation = "";
	PXmlDoc TagDataXmlDoc = TXmlDoc::LoadStr(TagData);
	if (!TagDataXmlDoc->IsOk()) {
		LastInformation = "Invalid XML data";
		return false;
	}

	PXmlTok TagDataXml = TagDataXmlDoc->GetTagTok("tagData");
	TStr TagIdStr = TagDataXml->GetArgVal("tagId", "");
	if (TagIdStr == "") {
		LastInformation = "tag id was not specified.";
		return false;		
	}
	TStr Operation = TagDataXml->GetArgVal("operation", "");
	if (Operation == "") {
		LastInformation = "operation on the tag data was not specified.";
		return false;		
	}
	TStr IdsStr = TagDataXml->GetTokStr(false);
	TStrV IdStrV;
	IdsStr.SplitOnAllCh(',', IdStrV);
	
	if (Operation == "add") {
		for (int IdN = 0; IdN < IdStrV.Len(); IdN++) {
			TInt ItemId = IdStrV[IdN].GetInt(-1);
			if (ItemId != -1)
				SetTag(ItemId, TagIdStr);
		}
	}
	else if (Operation == "remove") {
		for (int IdN = 0; IdN < IdStrV.Len(); IdN++) {
			TInt ItemId = IdStrV[IdN].GetInt(-1);
			if (ItemId != -1)
				RemoveTag(ItemId, TagIdStr);
		}
	}

	return true;
}

TStr TProfile::GetTagData(const TStr& TagData)
{
	LastInformation = "";
	PXmlDoc TagDataXmlDoc = TXmlDoc::LoadStr(TagData);
	if (!TagDataXmlDoc->IsOk()) {
		LastInformation = "Invalid XML data";
		return "";
	}

	TChA ResultChA;
	PXmlTok TagDataXml = TagDataXmlDoc->GetTagTok("tagData");
	TStr TagIdStr = TagDataXml->GetArgVal("tagId", "");
	if (TagIdStr != "") {
		
		if (IndexVoc->IsWordStr(IndexTagSearchKeyId, TagIdStr)) {
			POgRecSet RecSet = OgBase->Search(TOgQuery::New(OgBase, 
					TOgQueryItem(OgBase, IndexTagSearchKeyId, TagIdStr, oqctEqual)));
			for (int i=0; i < RecSet->GetRecs(); i++) {
				int ItemId = (int) RecSet->GetRec(i).GetRecId();
				ResultChA += TStr::FmtBf(Bf, BfSize, "%d,", ItemId);
			}
		}

		return ResultChA;
	}
	
	TStr ItemIdStr = TagDataXml->GetArgVal("itemId", "");
	if (ItemIdStr != "") {
		TInt ItemId = ItemIdStr.GetInt(-1);
		if (ItemStore->IsItem(ItemId)) {
			TStrV TagV = ItemStore->GetTagsV(ItemId);
			return TStr::GetStr(TagV, ",");
		}
	}
	
	return "";
}

void TProfile::AddItemResult(const TUInt64 ItemId, TChA& ResultChA, const bool IncludeAttachments, const TStr& ExtraItemInfo)
{
	if (!ItemStore->IsItem((int) ItemId))
		return;
	TStr fmtStr;
	
	TInt ItemType = ItemStore->GetFieldInt(ItemId, ItemStore->ItemTypeFieldId);
	TStr EntryIdStr = ItemStore->GetFieldStr(ItemId, ItemStore->EntryIdFieldId);
	TUInt64 Time = ItemStore->GetFieldUInt64(ItemId, ItemStore->TimeFieldId);
	TInt ThreadId = ItemStore->GetFieldInt(ItemId, ItemStore->ThreadIdFieldId);
	TStrV TagsV; ItemStore->GetFieldStrV(ItemId, ItemStore->TagsFieldId, TagsV);
	TStr TagsStr = TStr::GetStr(TagsV, ",");

	TOgRec ItemRec(ItemStoreId, ItemId);
	
	int ThreadItemCount = 1;
	if (ThreadId >= 0) {
		TOgRec ThreadRec(ThreadStoreId, ThreadId);
		TUInt64V ThreadItemsIdV;
		POgRecSet ThreadItemsSet = ThreadRec.DoJoin(OgBase, ThreadStore->JoinHasItemsId);
		ThreadItemCount = ThreadItemsSet->GetRecs();
	}

	ResultChA += TStr::FmtBf(Bf, BfSize, "		<item id=\"%s\" itemType=\"%d\" time=\"%s\" tags=\"%s\"", ItemId.GetStr().CStr(), (int)ItemType, Time.GetStr().CStr(), TagsStr.CStr());
	if (EntryIdStr != "") ResultChA += TStr::FmtBf(Bf, BfSize, " entryId=\"%s\"", TXmlLx::GetXmlStrFromPlainStr(EntryIdStr).CStr());
	/*if (EntryIdStr != "") ResultChA += TStr::FmtBf(Bf, BfSize, " entryId=\"%s\"", EntryIdStr.CStr());*/
	
	
	if (ItemType == Attachment && AttachmentStore->IsRecNm(TInt((int)ItemId).GetStr()))  
		ResultChA += TStr::FmtBf(Bf, BfSize, " emailItemId=\"%s\"", AttachmentStore->GetFieldStr(AttachmentStore->GetRecId(TInt((int)ItemId).GetStr()), AttachmentStore->EmailItemIdFieldId).CStr());
	if (ThreadId >= 0) ResultChA += TStr::FmtBf(Bf, BfSize, " threadId=\"%d\" count=\"%d\"", (int)ThreadId, (int)ThreadItemCount);
		
	for (int RoleN=0; RoleN < PersonStore->GetRoles(); RoleN++) {
		TStr RoleNm = PersonStore->GetRole(RoleN);
		if (RoleNm == AnyRoleNm) continue;
		int RoleJoinId = PersonStore->AddRole(RoleNm);
		bool found = false;
		TPartV PartV = ItemStore->GetPartV((int) ItemId);
		for (int i=0; i < PartV.Len(); i++) {
			if (PartV[i].Val1 == RoleJoinId) {
				if (found == false)			
					ResultChA += " " + RoleNm + "=\"";
				if (found == true)			
					ResultChA += ",";
				ResultChA += TStr::FmtBf(Bf, BfSize, "%d", (int)PartV[i].Val2);
				found = true;
			}
		}
		if (found == true)
			ResultChA += "\"";
	}
	ResultChA += ">\n";

	TIntV AttItemIdV;
	if (ItemType == Email) {
		POgRecSet AttachmentsSet = ItemRec.DoJoin(OgBase, ItemStore->JoinHasAttachmentsId);
		TUInt64V AttachmentIdV;
		AttachmentsSet->GetRecIdV(AttachmentIdV);

		if (AttachmentsSet->GetRecs() > 0) {
			ResultChA += "			<attachments>\n";
			for (int a=0; a < AttachmentsSet->GetRecs(); a++) {
				uint64 recId = AttachmentsSet->GetRec(a).GetRecId();
				ResultChA += "				<att name=\"" + TXmlLx::GetXmlStrFromPlainStr(AttachmentStore->GetFieldStr(recId, AttachmentStore->NameFieldId)) +
				//ResultChA += "				<att name=\"" + AttachmentStore->GetFieldStr(recId, AttachmentStore->NameFieldId) + 
					"\" size=\"" + AttachmentStore->GetFieldStr(recId, AttachmentStore->SizeFieldId) + 
					"\" attItemId=\"" + AttachmentStore->GetFieldStr(recId, AttachmentStore->AttItemIdFieldId) + "\" />\n";
				AttItemIdV.Add(AttachmentStore->GetFieldInt(recId, AttachmentStore->AttItemIdFieldId));
			}
			ResultChA += "			</attachments>\n";
		}
	}

	// write links
	POgRecSet LinksSet = ItemRec.DoJoin(OgBase, ItemStore->JoinHasLinksId);
	TUInt64V LinkIdV;
	LinksSet->GetRecIdV(LinkIdV);
	
	if (LinksSet->GetRecs() > 0) {
		ResultChA += "			<links>\n";
		for (int a=0; a < LinksSet->GetRecs(); a++)
			ResultChA += "<a><h>" + TXmlLx::GetXmlStrFromPlainStr(LinkStore->GetFieldStr(LinksSet->GetRec(a).GetRecId(), LinkStore->LinkHrefFieldId)) + "</h><t>" + TXmlLx::GetXmlStrFromPlainStr(LinkStore->GetFieldStr(LinksSet->GetRec(a).GetRecId(), LinkStore->LinkTextFieldId)) + "</t></a>\n";
			//ResultChA += "<a><h>" + LinkStore->GetFieldStr(LinksSet->GetRec(a).GetRecId(), LinkStore->LinkHrefFieldId) + "</h><t>" + LinkStore->GetFieldStr(LinksSet->GetRec(a).GetRecId(), LinkStore->LinkTextFieldId) + "</t></a>\n";
		ResultChA += "			</links>\n";
	}

	POgRecSet RelatedSet = ItemRec.DoJoin(OgBase, ItemStore->JoinHasRelatedItemsId);
	TUInt64V RelatedIdV;
	RelatedSet->GetRecIdV(RelatedIdV);
	if (RelatedSet->GetRecs() > 0) {
		ResultChA += "			<relatedItems>";
		for (int RelN=0; RelN < RelatedSet->GetRecs(); RelN++) {
			if (RelN > 0) ResultChA += ",";
			ResultChA += TStr::FmtBf(Bf, BfSize, "%d", (int) RelatedSet->GetRec(RelN).GetRecId());
		}	
		ResultChA += "</relatedItems>\n";
	}
	ResultChA += ExtraItemInfo;		
	ResultChA += "		</item>\n";

	if (IncludeAttachments) {
		for (int i=0; i < AttItemIdV.Len(); i++) {
			if (AttItemIdV[i] >= 0)		
				AddItemResult(TUInt64(AttItemIdV[i]), ResultChA, false);
		}
	}
}


void TProfile::WriteVector(TChA& OutChA, const TUInt64V& DataV, const char separator)
{
	for (int i=0; i < DataV.Len(); i++) {
		if (i > 0) OutChA += separator;
		OutChA += TStr::FmtBf(Bf, BfSize, "%s", DataV[i].GetStr().CStr());
	}
}

void TProfile::WriteVector(TChA& OutChA, const TIntV& DataV, const char separator)
{
	for (int i=0; i < DataV.Len(); i++)	{
		if (i > 0) OutChA += separator;
		OutChA += TStr::FmtBf(Bf, BfSize, "%d", (int)DataV[i].Val);
	}
}

//TStr TProfile::GetTopWords(int KeywordCount, bool GroupByThreads, int MaxNGramLen, int MinNGramFq)
//{
//	
//	PSwSet SwSet=TSwSet::GetSwSet(swstEn523);				// prepare stop-words
//	PStemmer Stemmer=TStemmer::GetStemmer(stmtPorter);		// prepare stemmer
//	
//	TStrV HtmlStrV;
//	POgStoreIter Iter = ItemStore->GetIter();
//	TIntSet SeenThreadIdsH;
//	while (Iter->Next()) 
//	{
//		const uint64 RecId = Iter->GetRecId();
//		int ThreadId = ItemStore->GetFieldInt(RecId, ItemStore->ThreadIdFieldId);
//		if (GroupByThreads && SeenThreadIdsH.IsKey(ThreadId))
//			continue;
//		
//		TStr Content = "";
//		// if we are groupping to threads then store the whole thread as a single document
//		if (ThreadId != -1 && GroupByThreads == true)
//		{
//			TOgRec ThreadRec(ThreadStoreId, ThreadId);
//			TUInt64V ThreadItemsIdV;
//			POgRecSet ThreadItemsSet = ThreadRec.DoJoin(OgBase, ThreadStore->JoinHasItemsId);
//			ThreadItemsSet->GetRecIdV(ThreadItemsIdV);			
//			
//			Content = ItemStore->GetFieldStr(RecId, ItemStore->SubjectFieldId);
//			for (int i=0; i < ThreadItemsIdV.Len(); i++)
//				Content += " " + ItemStore->GetFieldStr(ThreadItemsIdV[i], ItemStore->ContentFieldId);
//			SeenThreadIdsH.AddKey(ThreadId);
//		}
//		else 
//			Content = ItemStore->GetFieldStr(RecId, ItemStore->SubjectFieldId) + ItemStore->GetFieldStr(RecId, ItemStore->ContentFieldId);
//		
//		HtmlStrV.Add(Content);
//	}
//
//	PNGramBs NGramBs = TNGramBs::GetNGramBsFromHtmlStrV(HtmlStrV, MaxNGramLen, MinNGramFq, SwSet, Stemmer);
//
//	// create bow
//	PBowDocBs BowDocBs=TBowDocBs::New(SwSet, Stemmer, NGramBs);
//	
//	for (int DocN=0; DocN < HtmlStrV.Len(); DocN++)
//		BowDocBs->AddHtmlDoc(TInt::GetStr(DocN), TStrV(), HtmlStrV[DocN]);
//
//	PBowDocWgtBs BowDocWgtBs = TBowDocWgtBs::New(BowDocBs, bwwtNrmTFIDF);
//
//	// get keywords
//	TIntV AllDIdV; 
//	BowDocBs->GetAllDIdV(AllDIdV);
//	PBowSpV BowSpV = TBowClust::GetConceptSpV(BowDocWgtBs, TBowSim::New(bstCos), AllDIdV);
//
//	// store top keywords
//	PBowKWordSet KWordSet = BowSpV->GetKWordSet(BowDocBs);
//	KWordSet->SortByWgt();
//	TChA ResultChA;
//	ResultChA += "<keywords>\n    ";
//	for (int i=0; i < KWordSet->GetKWords() && i < KeywordCount; i++)
//		ResultChA += TStr::FmtBf(Bf, BfSize, "<kw str=\"%s\" wgt=\"%.3f\"/>", KWordSet->GetKWordStr(i).CStr(), KWordSet->GetKWordWgt(i));
//	ResultChA += "</keywords>\n";
//	return ResultChA;
//}


void TProfile::UpdateBowByThread()
{
	if (!BowDocBsWholeThreads.Empty() && BowDocBsWholeThreads->GetDocs() == ThreadStore->GetRecs() && 
		!BowDocBsConceptsByThread.Empty() && BowDocBsConceptsByThread->GetDocs() == ThreadStore->GetRecs()) 
		return;

	// create the bag of words with all the treads
	TStrV ThreadNmV;
	TVec<TIntV> DIdVV;
	TVec<TInt> DIdCountV;
	POgStoreIter Iter = ThreadStore->GetIter();
	while (Iter->Next()) {
		TInt ThreadId = (int) Iter->GetRecId();
		ThreadNmV.Add(ThreadId.GetStr());

		TOgRec ThreadRec(ThreadStoreId, ThreadId);
		TUInt64V ThreadItemsIdV;
		POgRecSet ThreadItemsSet = ThreadRec.DoJoin(OgBase, ThreadStore->JoinHasItemsId);
		ThreadItemsSet->GetRecIdV(ThreadItemsIdV);
		TIntV DIdV;
		GetBowDocIdV(ThreadItemsSet, BowDocBs, DIdV);
		DIdVV.Add(DIdV);
		DIdCountV.Add(ThreadItemsSet->GetRecs());		
	}

	BowDocBsWholeThreads = BowDocBs->GetMergedDocs(DIdVV, ThreadNmV);
	BowDocBsConceptsByThread = BowDocBsConcepts->GetMergedDocs(DIdVV, ThreadNmV);

	for (int N=0; N < ThreadNmV.Len(); N++) {
		TStr ThreadNm = ThreadNmV[N];
		int ThreadItemCount = DIdCountV[N];
		TStr Thread = ThreadStore->GetFieldStr(TStr(ThreadNm).GetInt(0), ThreadStore->ThreadFieldId);
		
		if (!BowDocBsSubjects->IsDocNm(ThreadNm) || !BowDocBsWholeThreads->IsDocNm(ThreadNm))
			continue;
		PBowSpV SubjectSpV = BowDocBsSubjects->GetDocSpV(BowDocBsSubjects->GetDId(ThreadNm));
		PBowSpV ThreadSpV = BowDocBsWholeThreads->GetDocSpV(BowDocBsWholeThreads->GetDId(ThreadNm));
		/*for (int i=0; i < SubjectSpV->GetWIds(); i++)
		{
			int spvId = SubjectSpV->GetWId(i);
			int thrId = ThreadSpV->GetWId(i);
			TStr Word = BowDocBsSubjects->GetWordStr(spvId);
			TStr Word2 = BowDocBsWholeThreads->GetWordStr(thrId);
			LastInformation = "asdf";
		}*/
		for (int WIdN=0; WIdN < SubjectSpV->GetWIds(); WIdN++) {
			int WId; double Wgt;
			SubjectSpV->GetWIdWgt(WIdN, WId, Wgt);				
			TStr Word = BowDocBsSubjects->GetWordStr(WId);
			int WId2 = BowDocBsWholeThreads->GetWId(Word);
			WAssert(ThreadSpV->GetWIdN(WId2) != -1, "The word should be present in the thread SpV");		
			double oldWgt = ThreadSpV->GetWgt(ThreadSpV->GetWIdN(WId2));
			WAssert(oldWgt >= -0.1, "The bow weight for the thread has a negative value");		
			if (oldWgt < Wgt*ThreadItemCount)
				ThreadSpV->UpdateWIdWgt(WId2, 0.0);
			else
				ThreadSpV->DecreaseWIdWgt(WId2, Wgt * ThreadItemCount);		
			/*if (ThreadSpV->GetWgt(WIdN).Val < 0.0)
				ThreadSpV->SetWIdWgt(WId, 0.0);*/
			//IAssert(ThreadSpV->GetWgt(WIdN) > -1);						// the number should be above or equal 0 but because of rounding errors we allow small deviations
		}
	}
	UpdateBowWgtsByThread();
}

void TProfile::UpdateBowWgtsByThread()
{
	if (BowDocBsWholeThreads.Empty() || BowDocBsWholeThreads->GetDocs() == 0) {
		LastInformation = "Unable to compute BOWWgts for threads since BOW for threads is not generated yet.";
		return;
	}

	if (!BowDocWgtBsWholeThreads.Empty() && BowDocWgtBsWholeThreads->GetDocs() == ThreadStore->GetRecs() && 
		!BowDocWgtBsConceptsByThread.Empty() && BowDocWgtBsConceptsByThread->GetDocs() == ThreadStore->GetRecs())
		return;

	BowDocWgtBsWholeThreads = TBowDocWgtBs::New(BowDocBsWholeThreads, bwwtNrmTFIDF);
	BowDocWgtBsConceptsByThread = TBowDocWgtBs::New(BowDocBsConceptsByThread, bwwtNrmTFIDF);
}


TVec<TPair<TFlt, TStr> > TProfile::GetSimilarityWithItem(const PBowDocBs& BowDoc, const PBowDocWgtBs& BowDocWgt, const TUInt64Set& CandidateItemIdsH, const TInt TestItemId)
{
	PBowSim BowSim = TBowSim::New(bstCos);

	TVec<TPair<TFlt, TStr> > SimilarityItemIdPairV;
	int TestItemIdDocId = BowDoc->GetDId(TestItemId.GetStr());
	if (TestItemIdDocId == -1)				
		return SimilarityItemIdPairV;
	
	
	PBowSpV TestItemSpV = BowDocWgt->GetSpV(TestItemIdDocId);

	for (int DId = 0; DId < BowDoc->GetDocs(); DId++) {
		if (CandidateItemIdsH.Len() > 0 && !CandidateItemIdsH.IsKey(BowDoc->GetDocNm(DId).GetInt(-1))) 
			continue;
		if (DId == TestItemIdDocId) continue;
		PBowSpV ItemSpV = BowDocWgt->GetSpV(DId);		// get the SpV of the item content

		double Sim = BowSim->GetSim(TestItemSpV, ItemSpV);
		TStr ItemId = BowDoc->GetDocNm(DId);
		SimilarityItemIdPairV.Add(TFltStrPr(Sim, ItemId));
	}

	return SimilarityItemIdPairV;
}


// compute a list of most similar items to the given SpV
TVec<TPair<TFlt, TStr> > TProfile::GetSimilarityWithText(const PBowDocBs& BowDoc, const PBowDocWgtBs& BowDocWgt, const TUInt64Set& CandidateItemIdsH, const TStr& Text)
{
	TVec<TPair<TFlt, TStr> > SimilarityItemIdPairV;
	PBowSim BowSim = TBowSim::New(bstCos);
	
	TStrV WIdV, WordV;
	THash<TInt, TInt> WIdsToCountH;
	IndexVoc->GetTokenizer()->GetTokens(Text, WordV);
	for (int WordN = 0; WordN < WordV.Len(); WordN++) {
		TStr WordStr = WordV[WordN];
		if (BowDoc->IsWordStr(WordStr)) {
			int WId = BowDoc->GetWId(WordStr);
			WIdsToCountH.AddDat(WId, WIdsToCountH.IsKey(WId) ? WIdsToCountH.GetDat(WId) + 1 : 1);
		}
	}

	TIntFltKdV WIdWgtV;
	for (int WIdN = 0; WIdN < WIdsToCountH.Len(); WIdN++) {
		int WId = WIdsToCountH.GetKey(WIdN);
		double DocWordFq = (double) WIdsToCountH.GetDat(WId);
		TFlt WordIDF;
		double WordDf = BowDocWgt->GetWordFq(WId);
		if (WordDf > 0)
			WordIDF = log(double(BowDocWgt->GetDocs())/WordDf);
		double WordWgt = DocWordFq * WordIDF;
		WIdWgtV.Add(TIntFltKd(WId, WordWgt));
	}
	PBowSpV TestItemSpV = TBowSpV::New(-1, WIdWgtV);
	TestItemSpV->PutUnitNorm();

	for (int DId = 0; DId < BowDoc->GetDocs(); DId++) {
		if (CandidateItemIdsH.Len() > 0 && !CandidateItemIdsH.IsKey(BowDoc->GetDocNm(DId).GetInt(-1))) 
			continue;
		PBowSpV ItemSpV = BowDocWgt->GetSpV(DId);		

		double Sim = BowSim->GetSim(TestItemSpV, ItemSpV);
		TStr ItemId = BowDoc->GetDocNm(DId);
		SimilarityItemIdPairV.Add(TFltStrPr(Sim, ItemId));
	}

	return SimilarityItemIdPairV;
}

TStr TProfile::ExecuteCommand(const TStr& CommandStr)
{
	PXmlDoc CommandXml = TXmlDoc::LoadStr(CommandStr);
	if (!CommandXml->IsOk()) {
		LastInformation = "Invalid XML data: " + CommandXml->GetMsgStr();
		return "";
	}

	LastInformation = "";
	PXmlTok CommandToken = CommandXml->GetTagTok("commandData");
	TStr CommandType = CommandToken->GetStrArgVal("command", "");
	
	TChA ResultChA;
	// compute bowwgts only
	if (CommandType == "computeBowWgt")
		UpdateBowWgts();
	else if (CommandType == "computeBowByThread")
		UpdateBowByThread();
	else if (CommandType == "ignoreBowWgtWords") {
		if (BowDocBs.Empty()) {
			LastInformation = "BowDocBs is invalid. Unable to compute new BowDocWgtBs.";
			return "";
		}
		TStr IgnoredWordsStr = CommandToken->GetTokStr(false);
		TStrV IgnoredWordsV;
		THashSet<TInt> IgnoredWIdsH;
		IndexVoc->GetTokenizer()->GetTokens(IgnoredWordsStr, IgnoredWordsV);
		for (int WIdN = 0; WIdN < IgnoredWordsV.Len(); WIdN++) {
			TStr WordStr = IgnoredWordsV[WIdN];
			if (BowDocBs->IsWordStr(WordStr)) {
				int WId = BowDocBs->GetWId(WordStr);
				IgnoredWIdsH.AddKey(WId);
			}
		}
		BowDocWgtBs = TBowDocWgtBs::New(BowDocBs, bwwtNrmTFIDF, 0, 0, TIntV(), TIntV(), IgnoredWIdsH);
		return TStr("Successfully created new BowWgts where " + TInt(IgnoredWordsV.Len()).GetStr() + " words were ignored");
	}
	
	else if (CommandType == "classSeparation") {
		TInt Folds = CommandToken->GetIntArgVal("folds", 10);
		TInt Seed = CommandToken->GetIntArgVal("rndSeed", 1);
		TInt TimeLimit = CommandToken->GetIntArgVal("timeLimit", 20);		
		double j = CommandToken->GetFltArgVal("j", 1.0);						
		float c = 1.0;
		int subSize = 100;
		double eps_ter = 1e-3;
		bool shrink = true;

		PXmlTok PositiveExamplesTok = CommandToken->GetTagTok("positiveExamples");
		if (PositiveExamplesTok.Empty() || PositiveExamplesTok->GetSubToks() == 0) {
			LastInformation = "The set of items that should be positive examples was not specified. Unable to build a classifier.";
			return "";
		}
		POgRecSet PosExRecSet = GetRecSetForQuery(PositiveExamplesTok, NULL);		
		TIntV BowDIdV;
		GetBowDocIdV(PosExRecSet, BowDocBs, BowDIdV);
		
		TIntV TrainBowDocIdV;
		PXmlTok TrainExTok = CommandToken->GetTagTok("trainExamples");
		if (!TrainExTok.Empty() && TrainExTok->GetSubToks() > 0) {
			POgRecSet TrainItems = GetRecSetForQuery(TrainExTok, NULL);	
			GetBowDocIdV(TrainItems, BowDocBs, TrainBowDocIdV);
		}
		else		// take all bow documents as training examples
			BowDocBs->GetAllDIdV(TrainBowDocIdV);
		
		TStr CatNm("SVM-PositiveExamples");
		BowDocBs->SetCatToBowDIds(CatNm, BowDIdV);
		
		UpdateBowWgts();
				
		TCfyRes Results  = TBowSVMMd::CrossValidClsLinear(Folds, Seed, BowDocBs, BowDocWgtBs, CatNm, TrainBowDocIdV, c, j, TSVMLearnParam::Lin(TimeLimit));
		LastInformation = Results.GetStatStr(TStr::Fmt("Classification results (%d positive examples, %d negative examples):", (int)BowDIdV.Len(), (int)(TrainBowDocIdV.Len()-BowDIdV.Len())));		

		BowDocBs->RemoveCatFromBowDIds(CatNm, BowDIdV);
		
		return TStr(LastInformation);
	}
	
	else {
		LastInformation = "Unknown command " + CommandType;
		TStr(LastInformation);
	}
	return "";
}

TStr TProfile::CustomQuery(const PXmlDoc& QueryXml)
{
	LastInformation = "";
	PXmlTok QueryTok = QueryXml->GetTagTok("query");
	PXmlTok ParamsTok = QueryXml->GetTagTok("query|params");
	TStr QueryType = ParamsTok->GetStrArgVal("type", "");
	TInt MaxCount = ParamsTok->GetIntArgVal("maxCount", 100);
	TInt Offset = ParamsTok->GetIntArgVal("offset", 0);

	PXmlTok ConditionsTok = QueryTok->GetTagTok("queryArgs|conditions");
	PXmlTok IgnoresTok = QueryTok->GetTagTok("queryArgs|ignore");
	
	PXmlTok ConditionsNegativeTok = QueryTok->GetTagTok("queryArgsNegative|conditions");
	PXmlTok IgnoresNegativeTok = QueryTok->GetTagTok("queryArgsNegative|ignore");
	
	TChA ResultChA;

	if (QueryType == "frequentSocialGroups") {
		TVec<TPair<TInt, TIntV> > FqAccsV;
		ItemStore->GetFrequentSocialGroups(MaxCount, FqAccsV);
		ResultChA += "<results>\n";
		for (int i=0; i < FqAccsV.Len(); i++) {
			ResultChA += TStr::Fmt("<group fq=\"%d\">", FqAccsV[i].Val1);
			WriteVector(ResultChA, FqAccsV[i].Val2);
			ResultChA += "</group>\n";
		}
		ResultChA += "</results>\n";
	}

	if (QueryType == "threadId") {
		TStr Thread = ParamsTok->GetTagTok("thread")->GetTokStr(false);
		if (!ThreadStore->IsRecNm(Thread))
		{
			LastInformation = "Thread with the specified name was not indexed yet.";
			return "";
		}
		TInt ThreadId = (int) ThreadStore->GetRecId(Thread); 
		return ThreadId.GetStr();
	}
	else if (QueryType == "thread") {
		TInt ThreadId = ParamsTok->GetIntArgVal("threadId", -1);
		return ThreadStore->GetFieldStr(ThreadId, ThreadStore->ThreadFieldId);
	}
	else if (QueryType == "similarThreads") {
		TInt TestThreadId = ParamsTok->GetIntArgVal("threadId", -1);
		bool IncludeItemData = GetBoolArg(ParamsTok, "includeItemData", false);						
		bool IncludeOnlyFirstInThread = GetBoolArg(ParamsTok, "includeOnlyFirstInThread", true);	
		bool IncludeItemIds = GetBoolArg(ParamsTok, "includeItemIds", false);						

		if (TestThreadId == -1) { LastInformation = "Test thread id was not specified in the query."; return ""; }
		//if (BowDocBsWholeThreads.Empty() || BowDocWgtBsWholeThreads.Empty()) { LastInformation = "BOW or BOW weights by thread were not created yet for threads. Please create them first."; return ""; }
				
		POgRecSet CandidateItems;
		TUInt64Set CandidateThreadIdsH;
		if (!ConditionsTok.Empty() && ConditionsTok->GetSubToks() > 0) {
			CandidateItems = GetRecSetForQuery(ConditionsTok, IgnoresTok);
			for (int i=0; i < CandidateItems->GetRecs(); i++) {
				int ItemId = (int) CandidateItems->GetRecId(i);
				TInt ThreadId = ItemStore->GetFieldInt(ItemId, ItemStore->ThreadIdFieldId);
				CandidateThreadIdsH.AddKey((TUInt64) ThreadId);
			}
		}
		
		UpdateBowByThread();
		UpdateBowWgtsByThread();

		TVec<TPair<TFlt, TStr> > SimilarityThreadIdPairV = GetSimilarityWithItem(BowDocBsWholeThreads, BowDocWgtBsWholeThreads, CandidateThreadIdsH, TestThreadId);
		TVec<TPair<TFlt, TStr> > TopSimilarityThreadIdPairV;
		if (MaxCount != -1) {
			for (int i=0; i < SimilarityThreadIdPairV.Len(); i++)
				TopSimilarityThreadIdPairV.AddSorted(SimilarityThreadIdPairV[i], false, Offset + MaxCount);		
		}
		else
			TopSimilarityThreadIdPairV = SimilarityThreadIdPairV;		
		
		
		ResultChA += "<results>\n";
		ResultChA += TStr::FmtBf(Bf, BfSize, "<info totalCount=\"%d\" offset=\"%d\" maxCount=\"%d\" />\n", (int) SimilarityThreadIdPairV.Len(), (int) Offset, (int) MaxCount);
		ResultChA += TStr::Fmt("<similarities comparedThreadId=\"%d\">\n", (int)TestThreadId);
		TChA ItemIdsChA = "";
		TUInt64V ItemIdV;		
		THash<TUInt64, TStr> ItemIdToSimilarityH;
		for (int i = Offset; i < TopSimilarityThreadIdPairV.Len(); i++) {
			ItemIdsChA.Clr();
			TStr Similarity = TStr::Fmt("%.4f", (float) TopSimilarityThreadIdPairV[i].Val1);
			if (IncludeItemIds || IncludeItemData) {
				uint64 ThreadId = TopSimilarityThreadIdPairV[i].Val2.GetUInt64(-1);
				TUInt64V ThreadItemsIdV = GetItemIdsForThread(ThreadId);
				ItemIdsChA = " itemIds=\"";
				for (int itemIdN = 0; itemIdN < ThreadItemsIdV.Len(); itemIdN++)
				{
					if (itemIdN == 0 || IncludeOnlyFirstInThread == false) ItemIdV.Add(ThreadItemsIdV[itemIdN]);
					if (itemIdN > 0) ItemIdsChA += ",";
					ItemIdsChA += ThreadItemsIdV[itemIdN].GetStr();
					ItemIdToSimilarityH.AddDat(ThreadItemsIdV[itemIdN], Similarity);
				}
				ItemIdsChA += "\" ";
			}
			if (IncludeItemIds == 0) 
				ItemIdsChA.Clr();
			ResultChA += TStr::Fmt("<thread id=\"%s\" sim=\"%s\" %s/>\n", TopSimilarityThreadIdPairV[i].Val2.CStr(), Similarity.CStr(), ItemIdsChA.CStr());
		}
		ResultChA += "</similarities>\n";
		
		if (IncludeItemData == true) {
			ResultChA += "	<items>\n";
			for (int ItemN = 0; ItemN < MaxCount && ItemN < ItemIdV.Len(); ItemN++)
				AddItemResult(ItemIdV[ItemN], ResultChA, false, TStr::Fmt("			<similarity>%s</similarity>", ItemIdToSimilarityH.GetDat(ItemIdV[ItemN]).CStr()));
			ResultChA += "	</items>\n";
		}
		ResultChA += "</results>\n";
		return ResultChA;
	}
	
	else if (QueryType == "similarItems") {
		TStr TextToCompare;
		TInt IncludeItemData = GetBoolArg(ParamsTok, "includeItemData", false);		
		TInt TestItemId = ParamsTok->GetIntArgVal("itemId", -1);
		if (TestItemId == -1) {
			PXmlTok TextToCompareToken = ParamsTok->GetTagTok("textToCompare");
			TextToCompare = TextToCompareToken->GetTokStr(false);
		}

		if (TestItemId == -1 && TextToCompare == "") { LastInformation = "Nor item id nor text to compare were not specified in the query. Unable to compute similar items."; return ""; }
				
		POgRecSet CandidateItems;
		TUInt64Set CandidateItemIdsH;		
		
		if (!ConditionsTok.Empty() && ConditionsTok->GetSubToks() > 0) {
			CandidateItems = GetRecSetForQuery(ConditionsTok, IgnoresTok);
			CandidateItems->GetRecIdSet(CandidateItemIdsH);
		}
		
		UpdateBowWgts();		

		TVec<TPair<TFlt, TStr> > SimilarityItemIdPairV;
		
		if (TestItemId != -1)
			SimilarityItemIdPairV = GetSimilarityWithItem(BowDocBs, BowDocWgtBs, CandidateItemIdsH, TestItemId);
		
		else if (TextToCompare != "")
			SimilarityItemIdPairV = GetSimilarityWithText(BowDocBs, BowDocWgtBs, CandidateItemIdsH, TextToCompare);
		TVec<TPair<TFlt, TStr> > TopSimilarityItemIdPairV;
		if (MaxCount != -1) {
			for (int i=0; i < SimilarityItemIdPairV.Len(); i++)
				TopSimilarityItemIdPairV.AddSorted(SimilarityItemIdPairV[i], false, Offset + MaxCount);		
		}
		else
			TopSimilarityItemIdPairV = SimilarityItemIdPairV;			
		
		
		ResultChA += "<results>\n";
		ResultChA += TStr::FmtBf(Bf, BfSize, "<info totalCount=\"%d\" offset=\"%d\" maxCount=\"%d\" />\n", (int) SimilarityItemIdPairV.Len(), (int) Offset, (int) MaxCount);
		ResultChA += TStr::Fmt("<similarities comparedItemId=\"%d\">\n", (int)TestItemId);
		for (int i = Offset; i < TopSimilarityItemIdPairV.Len(); i++)
			ResultChA += TStr::Fmt("<item id=\"%s\" sim=\"%.4f\" />", TopSimilarityItemIdPairV[i].Val2.CStr(), (float)TopSimilarityItemIdPairV[i].Val1);
		ResultChA += "</similarities>\n";
		
		if (IncludeItemData) {
			ResultChA += "	<items>\n";
			for (int ItemN = Offset; ItemN < Offset + MaxCount && ItemN < TopSimilarityItemIdPairV.Len(); ItemN++)
				AddItemResult(TopSimilarityItemIdPairV[ItemN].Val2.GetUInt64(-1), ResultChA, false, TStr::Fmt("			<similarity>%.4f</similarity>", (float)TopSimilarityItemIdPairV[ItemN].Val1));
			ResultChA += "	</items>\n";
		}
		
		ResultChA += "</results>\n";
		return ResultChA;
	}
	
	else if (QueryType == "similarItemsUsingAnnotations") {
		TStr TextToCompare;
		TInt TestItemId = ParamsTok->GetIntArgVal("itemId", -1);
		if (TestItemId == -1) { 
			LastInformation = "Item id was not specified in the query. Unable to compute similar items."; 
			return ""; 
		}
		
		POgRecSet CandidateItems;
		TUInt64Set CandidateItemIdsH;	
		if (!ConditionsTok.Empty() && ConditionsTok->GetSubToks() > 0) {
			CandidateItems = GetRecSetForQuery(ConditionsTok, IgnoresTok);
			CandidateItems->GetRecIdSet(CandidateItemIdsH);
		}
		
		UpdateBowWgtsConcepts();		

		TVec<TPair<TFlt, TStr> > SimilarityItemIdPairV = GetSimilarityWithItem(BowDocBsConcepts, BowDocWgtBsConcepts, CandidateItemIdsH, TestItemId);
		TVec<TPair<TFlt, TStr> > TopSimilarityItemIdPairV;
		if (MaxCount != -1) {
			for (int i=0; i < SimilarityItemIdPairV.Len(); i++)
				TopSimilarityItemIdPairV.AddSorted(SimilarityItemIdPairV[i], false, MaxCount);		
		}
		else
			TopSimilarityItemIdPairV = SimilarityItemIdPairV;			
		
		
		ResultChA += "<results>\n";
		ResultChA += TStr::FmtBf(Bf, BfSize, "<info totalCount=\"%d\" offset=\"%d\" maxCount=\"%d\" />\n", (int) SimilarityItemIdPairV.Len(), (int) Offset, (int) MaxCount);
		ResultChA += "<similarities>\n";
		for (int i=0; i < TopSimilarityItemIdPairV.Len(); i++)
			ResultChA += TStr::Fmt("<item id=\"%s\" sim=\"%.4f\" />", TopSimilarityItemIdPairV[i].Val2.CStr(), (float) TopSimilarityItemIdPairV[i].Val1);
		ResultChA += "</similarities>\n";
		ResultChA += "</results>\n";
		return ResultChA;
	}
	
	else if (QueryType == "keywordsUsingKMeans") {
		TInt K = ParamsTok->GetIntArgVal("k", 5);
		TInt RndSeed = ParamsTok->GetIntArgVal("rndSeed", 1);
		TInt ClustTrials = ParamsTok->GetIntArgVal("clustTrials", 1);
		TInt ConvergEps = ParamsTok->GetIntArgVal("convergEps", 10);
		double CutWordWgtSumPrc = ParamsTok->GetFltArgVal("cutWordWgtSumPrc", 0.5);
		TInt MnWordFq = ParamsTok->GetIntArgVal("mnWordFq", 5);
		TInt KeywordCount = ParamsTok->GetIntArgVal("keywordCount", 20);
		bool ComputeOnThreads = GetBoolArg(ParamsTok, "computeOnThreads", false);
		TStr MethodUsed = ParamsTok->GetStrArgVal("keywordMethod", "localConceptSpV");
				
		PBowSim BowSim=TBowSim::New(bstCos); 
		TBowWordWgtType WordWgtType = bwwtNrmTFIDF; 

		PBowDocBs Bow;
		PBowDocWgtBs BowWgts;
		if (ComputeOnThreads) {
			UpdateBowByThread();
			UpdateBowWgtsByThread();
			Bow = BowDocBsWholeThreads;
			BowWgts = BowDocWgtBsWholeThreads;
		}
		
		else if (!ConditionsTok.Empty() || !IgnoresTok.Empty()) {
			POgRecSet ItemRecSet = GetRecSetForQuery(ConditionsTok, IgnoresTok);
			TIntV DIdV; 
			GetBowDocIdV(ItemRecSet, BowDocBs, DIdV);
			Bow = BowDocBs->GetSubDocSet(DIdV);
			BowWgts = TBowDocWgtBs::New(Bow, bwwtNrmTFIDF);
		}
		else {
			UpdateBowWgts();
			Bow = BowDocBs;
			BowWgts = BowDocWgtBs;
		}
			
		ResultChA += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
		ResultChA += "<results>\n";
		if (Bow->GetDocs() > 0) {
			TRnd rnd = TRnd(RndSeed);
			PBowDocPart BowDocPart=TBowClust::GetKMeansPartForDocWgtBs(
				TNotify::StdNotify, 
				BowWgts,
				Bow, // document data
				BowSim, // similarity function
				rnd, // random generator
				K.Val, // number of clusters
				ClustTrials.Val, // trials per k-means
				ConvergEps.Val, // convergence epsilon for k-means
				1); // min. documents per cluster

		
			ResultChA += "<keywords>\n";
			for (int ClustN=0; ClustN < BowDocPart->GetClusts(); ClustN++) {
				PBowDocPartClust Clust = BowDocPart->GetClust(ClustN);
				ResultChA += TStr::FmtBf(Bf, BfSize, "<cluster index=\"%d\" meanSim=\"%.5f\" documents=\"%d\" >", ClustN, Clust->GetMeanSim(), Clust->GetDocs() );
			
				TIntV DocIdV; Clust->GetDIdV(DocIdV);
				POgRecSet ClustRecSet = GetRecSetForBowDocIdV(DocIdV, Bow);
				PBowKWordSet KWordSet = ComputeKWordSet(ClustRecSet, MethodUsed);
				for (int KwN=0; KwN < KWordSet->GetKWords() && KwN < KeywordCount; KwN++)
					ResultChA += TStr::FmtBf(Bf, BfSize, "<kw str=\"%s\" wgt=\"%.3f\"/>", KWordSet->GetKWordStr(KwN).CStr(), KWordSet->GetKWordWgt(KwN));
				ResultChA += "</cluster>\n";
			}
			ResultChA += "</keywords>\n";
		}
		else
			LastInformation = "The BOW was empty. Unable to compute clusters";
		
		ResultChA += "</results>\n";
		return ResultChA;
	}
	else if (QueryType == "keywordsUsingHKMeans") {
		TInt K = ParamsTok->GetIntArgVal("k", 5);
		TInt RndSeed = ParamsTok->GetIntArgVal("rndSeed", 1);
		TInt ClustTrials = ParamsTok->GetIntArgVal("clustTrials", 1);
		TInt ConvergEps = ParamsTok->GetIntArgVal("convergEps", 10);
		double CutWordWgtSumPrc = ParamsTok->GetFltArgVal("cutWordWgtSumPrc", 0.5);
		TInt MnWordFq = ParamsTok->GetIntArgVal("mnWordFq", 5);
		TInt MnDocsPerCluster = ParamsTok->GetIntArgVal("mnDocsPerCluster", 50);
		TInt MxDocsPerCluster = ParamsTok->GetIntArgVal("mxDocsPerCluster", 1000);
		TInt KeywordCount = ParamsTok->GetIntArgVal("keywordCount", 20);
		TInt ComputeOnThreads = GetBoolArg(ParamsTok, "computeOnThreads", false);
		TStr MethodUsed = ParamsTok->GetStrArgVal("keywordMethod", "localConceptSpV");

		PBowSim BowSim=TBowSim::New(bstCos); 
		TBowWordWgtType WordWgtType = bwwtNrmTFIDF; 

		PBowDocBs Bow;
		PBowDocWgtBs BowWgts;

		if (ComputeOnThreads) {
			UpdateBowByThread();
			UpdateBowWgtsByThread();
			Bow = BowDocBsWholeThreads;
			BowWgts = BowDocWgtBsWholeThreads;
		}
		
		else if (!ConditionsTok.Empty() || !IgnoresTok.Empty()) {
			POgRecSet ItemRecSet = GetRecSetForQuery(ConditionsTok, IgnoresTok);
			TIntV DIdV; 
			GetBowDocIdV(ItemRecSet, BowDocBs, DIdV);
			Bow = BowDocBs->GetSubDocSet(DIdV);
			BowWgts = TBowDocWgtBs::New(Bow, bwwtNrmTFIDF);
		}
		else {
			UpdateBowWgts();
			Bow = BowDocBs;
			BowWgts = BowDocWgtBs;
		}
		
		TIntV AllDIdV; 
		Bow->GetAllDIdV(AllDIdV);
				
		ResultChA += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
		ResultChA += "<results>\n";

		if (Bow->GetDocs() > 0) {
			TRnd rnd = TRnd(RndSeed);
			PBowDocPart BowDocPart=TBowClust::GetHKMeansPart(
				TNotify::StdNotify, // log output
				Bow, // document data
				BowSim, // similarity function
				rnd, // random generator
				MxDocsPerCluster, // number of clusters
				ClustTrials, // trials per k-means
				ConvergEps, // convergence epsilon for k-means
				MnDocsPerCluster, // min. documents per cluster
				WordWgtType, // word weighting
				CutWordWgtSumPrc, // cut-word-weights percentage
				MnWordFq, // minimal word frequency
				AllDIdV, // training documents
				false,
				BowWgts);

		
			ResultChA += "<keywords>\n";
			TVec<PBowDocPartClust> LeafClustV;
			TBowClust::GetHKMeansLeafClustV(BowDocPart, LeafClustV);

			for (int ClustN=0; ClustN < LeafClustV.Len(); ClustN++) {
				PBowDocPartClust Clust = LeafClustV[ClustN];
				ResultChA += TStr::FmtBf(Bf, BfSize, "<cluster index=\"%d\" meanSim=\"%.5f\" >", ClustN, Clust->GetMeanSim() );
			
				TIntV DocIdV; Clust->GetDIdV(DocIdV);
				POgRecSet ClustRecSet = GetRecSetForBowDocIdV(DocIdV, Bow);
				PBowKWordSet KWordSet = ComputeKWordSet(ClustRecSet, MethodUsed);
				for (int KwN=0; KwN < KWordSet->GetKWords() && KwN < KeywordCount; KwN++)
					ResultChA += TStr::FmtBf(Bf, BfSize, "<kw str=\"%s\" wgt=\"%.3f\"/>", KWordSet->GetKWordStr(KwN).CStr(), KWordSet->GetKWordWgt(KwN));
				ResultChA += "</cluster>\n";
			}
			ResultChA += "</keywords>\n";
		}
		else
			LastInformation = "The BOW was empty. Unable to compute clusters";

		ResultChA += "</results>\n";
		return ResultChA;
	}
	else if (QueryType == "keywordsUsingSVM") {
		TInt KeywordCount = ParamsTok->GetIntArgVal("keywordCount", 100);
		TInt TimeLimit = ParamsTok->GetIntArgVal("timeLimit", 20);		// the number of seconds before we stop learning the model

		PXmlTok PositiveExamplesTok = ParamsTok->GetTagTok("positiveExamples");
		if (ConditionsTok.Empty() || ConditionsTok->GetSubToks() == 0) { LastInformation = "Positive examples were not specified."; return "";	}
		POgRecSet PosExRecSet = GetRecSetForQuery(ConditionsTok, IgnoresTok);		
				
		PXmlTok NegativeExamplesTok = ParamsTok->GetTagTok("negativeExamples");
		if (ConditionsNegativeTok.Empty() || ConditionsNegativeTok->GetSubToks() == 0) { LastInformation = "Negative examples were not specified."; return "";	}
		POgRecSet NegExRecSet = GetRecSetForQuery(ConditionsNegativeTok, IgnoresNegativeTok);		
				
		PBowSpV BowSpV;
		PBowKWordSet KWordSet = NULL;

		TIntV OrigAllDocIdV;		
		TIntV OrigPosDocIdV;
		GetBowDocIdV(PosExRecSet, BowDocBs, OrigPosDocIdV);
		GetBowDocIdV(PosExRecSet, BowDocBs, OrigAllDocIdV);
		GetBowDocIdV(NegExRecSet, BowDocBs, OrigAllDocIdV);
		PBowDocBs SubBowDocBs = BowDocBs->GetSubDocSet(OrigAllDocIdV);
		
		TIntV PosBowDIdV;
		for (int i=0; i < OrigPosDocIdV.Len(); i++)
			PosBowDIdV.Add(i);
		TIntV AllDocIdV;
		for (int i=0; i < SubBowDocBs->GetDocs(); i++)
			AllDocIdV.Add(i);

		TStr CatNm("SVM-PositiveExamples");
		SubBowDocBs->SetCatToBowDIds(CatNm, PosBowDIdV);
		PBowDocWgtBs SubBowDocWgtBs = TBowDocWgtBs::New(SubBowDocBs, bwwtNrmTFIDF);
		
		if (SubBowDocBs->GetDocs() > 0) {
			BowSpV = TBowSVMMd::GetKeywords(SubBowDocBs, SubBowDocWgtBs, AllDocIdV, CatNm, KeywordCount, 1, 1, TimeLimit, 1, 1, true);
			KWordSet = BowSpV->GetKWordSet(SubBowDocBs);
		}
		ResultChA += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
		ResultChA += "<results>\n";
		if (!KWordSet.Empty())
			WriteKeywordSet(ResultChA, KWordSet, KeywordCount);
		ResultChA += "</results>";
		return ResultChA;
	}
	else if (QueryType == "nGrams") {
		TInt MnNGramFq = ParamsTok->GetIntArgVal("mnNGramFq", 10);
		TInt MnNGramLen = ParamsTok->GetIntArgVal("mnNGramLen", 2);
		TInt MxNGramCount = ParamsTok->GetIntArgVal("mxNGramCount", 500);
		
		TVec<TPair<TUInt, TIntV> > NGramFqV;
		
		NGramBs->GetFrequentNGrams(NGramFqV, MnNGramFq, MnNGramLen);
		
		ResultChA += "<results>\n";
		ResultChA += "<ngrams>\n";
		
		for (int N=0; N < NGramFqV.Len(); N++) {
			TUInt Fq = NGramFqV[N].Val1;
			TIntV NGramV = NGramFqV[N].Val2;
			TChA NGramStr(GetTokenStr(NGramV[0]));
			for (int NGramN = 1; NGramN < NGramV.Len(); NGramN++)
				NGramStr += " " + GetTokenStr(NGramV[NGramN]);
			ResultChA += TStr::FmtBf(Bf, BfSize, "<ngram text=\"%s\" fq=\"%d\" />", NGramStr.CStr(), (int)Fq);
		}
		ResultChA += "</ngrams>\n";
		ResultChA += "</results>\n";
		return ResultChA;
	}
	/*else if (QueryType == "similarityBetweenThreads")
	{
		return GetSimilarityBetweenThreads();
	}*/
	else {
		LastInformation = "Unknown query type " + QueryType;
	}
	
	return "";
}

void TProfile::PrintStatus(TChA& StatusChA)
{
	ItemStore->PrintStatus(StatusChA);
	PersonStore->PrintStatus(StatusChA);
	ThreadStore->PrintStatus(StatusChA);
	AttachmentStore->PrintStatus(StatusChA);	
	LinkStore->PrintStatus(StatusChA);

	if (!BowDocBs.Empty()) 
		StatusChA += "BowDocBs size: " + TUInt64(BowDocBs->GetMemUsed() / TInt::Mega).GetStr() + "MB\n";
	if (!BowDocBsConcepts.Empty()) 
		StatusChA += "BowDocBsConcepts size: " + TUInt64(BowDocBsConcepts->GetMemUsed() / TInt::Mega).GetStr() + "MB\n";
	if (!BowDocBsConceptsByThread.Empty()) 
		StatusChA += "BowDocBsConceptsByThread size: " + TUInt64(BowDocBsConceptsByThread->GetMemUsed() / TInt::Mega).GetStr() + "MB\n";
	if (!BowDocBsSubjects.Empty()) 
		StatusChA += "BowDocBsSubjects size: " + TUInt64(BowDocBsSubjects->GetMemUsed() / TInt::Mega).GetStr() + "MB\n";
	if (!BowDocBsWholeThreads.Empty()) 
		StatusChA += "BowDocBsWholeThreads size: " + TUInt64(BowDocBsWholeThreads->GetMemUsed() / TInt::Mega).GetStr() + "MB\n";
	if (!BowDocWgtBs.Empty()) 
		StatusChA += "BowDocWgtBs size: " + TUInt64(BowDocWgtBs->GetMemUsed() / TInt::Mega).GetStr() + "MB\n";
	if (!BowDocWgtBsConcepts.Empty()) 
		StatusChA += "BowDocWgtBsConcepts size: " + TUInt64(BowDocWgtBsConcepts->GetMemUsed() / TInt::Mega).GetStr() + "MB\n";
	if (!BowDocWgtBsConceptsByThread.Empty()) 
		StatusChA += "BowDocWgtBsConceptsByThread size: " + TUInt64(BowDocWgtBsConceptsByThread->GetMemUsed() / TInt::Mega).GetStr() + "MB\n";
	if (!BowDocWgtBsWholeThreads.Empty()) 
		StatusChA += "BowDocWgtBsWholeThreads size: " + TUInt64(BowDocWgtBsWholeThreads->GetMemUsed() / TInt::Mega).GetStr() + "MB\n";

	OgBase->PrintIndex("Index.txt", true);
	OgBase->PrintIndexVoc("IndexVoc.txt");
}

POgRecSet TProfile::GetResultsForQuery(TStr QueryArgs)
{
	TUInt64 ItemCount = ItemStore->GetRecs();
	for (int i=0; i < LastQueryResV.Len(); i++) {
		if (LastQueryResV[i].Val1 == QueryArgs  && ItemCount == LastQueryResV[i].Val2)
			return LastQueryResV[i].Val3;
	}
	return NULL;
}