#include "Stores.h"
#include "xmlParsing.h"

///////////////////////////////
TItemStore::TItemStore(const TStr& _StoreFNm, const int64& MxCacheSize, const POgIndexVoc &IndexVoc): 
	TOgStore(ItemStoreId, "Item"), StoreFNm(_StoreFNm), FAccess(faCreate), ItemsC(_StoreFNm + ".Cache", MxCacheSize, 1024)
{
	IndexVocInst = IndexVoc;
	EntryIdPool = TStrPool::New();
	// set fields
	ItemTypeFieldId = AddFieldDesc(TOgFieldDesc("ItemType", oftInt, offtNumeric, ofatNone, ofdtText));
	ItemIdFieldId = AddFieldDesc(TOgFieldDesc("ItemId", oftInt, offtNumeric, ofatNone, ofdtText));
	EntryIdFieldId = AddFieldDesc(TOgFieldDesc("EntryId", oftStr, offtToken, ofatNone, ofdtText));
	TimeFieldId = AddFieldDesc(TOgFieldDesc("Time", oftTm, offtTm, ofatTimeline, ofdtText));
	ThreadIdFieldId = AddFieldDesc(TOgFieldDesc("ThreadId", oftInt, offtToken, ofatNone, ofdtText));
	TagsFieldId = AddFieldDesc(TOgFieldDesc("Tags", oftStrV, offtToken, ofatNone, ofdtText));

	// set store description
	JoinHasLinksId = AddJoinDesc(TOgJoinDesc("hasLinks", LinkStoreId, ItemStoreId, IndexVoc));
	JoinHasAttachmentsId = AddJoinDesc(TOgJoinDesc("hasAttachments", AttachmentStoreId, ItemStoreId, IndexVoc));
	JoinHasRelatedItemsId = AddJoinDesc(TOgJoinDesc("hasRelatedItems", ItemStoreId, ItemStoreId, IndexVoc));
	JoinHasParentItemId = AddJoinDesc(TOgJoinDesc("hasParentItem", ItemStoreId, ItemStoreId, IndexVoc));
}

TItemStore::TItemStore(const TStr& _StoreFNm, const TFAccess& _FAccess, const int64& MxCacheSize, const POgIndexVoc & IndexVoc): 
	TOgStore(_StoreFNm + ".BaseStore"), StoreFNm(_StoreFNm), FAccess(_FAccess), ItemsC(_StoreFNm + ".Cache", _FAccess, MxCacheSize)
{
	IndexVocInst = IndexVoc;
	TFIn SIn(StoreFNm + ".Store");

	// fields
	ItemTypeFieldId.Load(SIn);
	ItemIdFieldId.Load(SIn);
	EntryIdFieldId.Load(SIn);
	TimeFieldId.Load(SIn);
	ThreadIdFieldId.Load(SIn);
	TagsFieldId.Load(SIn);

	// joins
	JoinHasLinksId.Load(SIn);
	JoinHasAttachmentsId.Load(SIn);
	JoinHasRelatedItemsId.Load(SIn);
	JoinHasParentItemId.Load(SIn);

	// data
	ItemsH.Load(SIn);
	PartsHS.Load(SIn);
	PartsToFqH.Load(SIn);
	TagsHS.Load(SIn);
	EntryIdPool = TStrPool::Load(SIn);
}

TItemStore::~TItemStore()
{
	// save if necessary
	if (FAccess != faRdOnly) {
		TOg::Logger->OnStatus("Saving item store ...");
		// save base store
		TFOut BaseFOut(StoreFNm + ".BaseStore"); SaveOgStore(BaseFOut);
		TFOut SOut(StoreFNm + ".Store");
		// fields
		ItemTypeFieldId.Save(SOut);
		ItemIdFieldId.Save(SOut);
		EntryIdFieldId.Save(SOut);
		TimeFieldId.Save(SOut);
		ThreadIdFieldId.Save(SOut);
		TagsFieldId.Save(SOut);
		// joins
		JoinHasLinksId.Save(SOut);
		JoinHasAttachmentsId.Save(SOut);
		JoinHasRelatedItemsId.Save(SOut);
		JoinHasParentItemId.Save(SOut);
		// data
		ItemsH.Save(SOut);
		PartsHS.Save(SOut);
		PartsToFqH.Save(SOut);
		TagsHS.Save(SOut);
		EntryIdPool->Save(SOut);
	}
}

int TItemStore::GetFieldInt(const uint64& ItemId, const int& FieldId) const 
{
	TItemRec Item; bool ok = GetItem((int)ItemId, Item); 
	if (!ok) return -1;
	if (FieldId == ItemIdFieldId) { return (int) ItemId; }
	else if (FieldId == ThreadIdFieldId) { return Item.ThreadId; }
	else if (FieldId == ItemTypeFieldId) { return (int) Item.ItemType; }
	FieldError(FieldId, "Int"); 
	return -1; 
}

TStr TItemStore::GetFieldStr(const uint64& ItemId, const int& FieldId) const 
{
	TItemRec Item; bool ok = GetItem((int)ItemId, Item); 
	if (!ok) return "Invalid ItemId";
	if (FieldId == EntryIdFieldId) { return GetEntryId(Item.EntryIdId); }
	FieldError(FieldId, "Str"); 
	return ""; 
}

void TItemStore::GetFieldStrV(const uint64& ItemId, const int& FieldId, TStrV& StrV) const
{
	TItemRec Item; bool ok = GetItem((int)ItemId, Item); 
	if (!ok) return;
	if (!ItemsH.IsKey((int) ItemId)) return;
	if (FieldId == TagsFieldId) { StrV = GetTagsV((int) ItemId); }
	else FieldError(FieldId, "StrV"); 
}

void TItemStore::GetFieldTm(const uint64& ItemId, const int& FieldId, TTm& Tm) const 
{
	TItemRec Item; bool ok = GetItem((int)ItemId, Item); 
	if (!ok) return;
	if (FieldId == TimeFieldId) { Tm = TTm::GetTmFromMSecs(Item.Time); }
	else FieldError(FieldId, "Tm");
}

uint64 TItemStore::GetFieldUInt64(const uint64& ItemId, const int& FieldId) const 
{
	TItemRec Item; bool ok = GetItem((int)ItemId, Item); 
	if (!ok) return 0;
	if (FieldId == TimeFieldId) { return Item.Time; }
	else FieldError(FieldId, "Tm");
	return 0;
}

uint64 TItemStore::AddRec(int ItemId, const TCh ItemType, const TStr& EntryId, const TUInt64& Time, 
				const TInt& ThreadId, const TStrV& TagsV)
{
	// store article content
	int TagVId = AddTagsV(TagsV);
	const uint64 CacheId = ItemsC.AddVal(TItemRec(ItemType, EntryIdPool->AddStr(EntryId), Time, ThreadId, TagVId));
	// index article URI and map it to cache id
	const uint64 RecId = (uint64)ItemsH.AddDat(ItemId, CacheId);
	
	// report new id
	return RecId;
}

void TItemStore::RemoveItem(const int ItemId)
{
	ItemsH.DelIfKey(ItemId);
}

// assign a TagId to ItemId
// if the tag was already assigned then ignore it
bool TItemStore::SetTag(int ItemId, const TStr& TagId)
{
	TItemRec Item; bool ok = GetItem(ItemId, Item); 
	if (!ok) return false;
	TStrV TagV = GetTagsV(ItemId);
	bool ret;
	if (!TagV.IsIn(TagId))
	{
		TagV.Add(TagId);
		Item.TagsVId = AddTagsV(TagV);
		UpdateItem(ItemId, Item);
		ret = true;
	}
	else
		ret = false;
	return ret;
}

bool TItemStore::RemoveTag(int ItemId, const TStr& TagId)
{
	TItemRec Item; bool ok = GetItem(ItemId, Item); 
	if (!ok) return false;
	TStrV TagV = GetTagsV(ItemId);
	bool ret;
	if (TagV.IsIn(TagId))
	{
		TagV.DelAll(TagId);
		Item.TagsVId = AddTagsV(TagV);
		UpdateItem(ItemId, Item);
		ret = true;
	}
	else
		ret = false;
	return ret;
}

void TItemStore::SetParticipants(const int ItemId, TPartV& PartV) 
{
	// sort the vector so that we won't have duplicated vectors just because of a different order
	PartV.Sort();
	int PartVId = PartsHS.AddKey(PartV);
	TItemRec Item; bool ok = GetItem(ItemId, Item); 
	if (!ok) return;
	Item.PartVId = PartVId;
	UpdateItem(ItemId, Item);

	// take the accounts only and increase the frequencies for them
	TIntV AccountV(PartV.Len());
	for (int i=0; i < PartV.Len(); i++)
		AccountV[i] = PartV[i].Val2;
	AccountV.Sort();
	int OldFq = PartsToFqH.IsKey(AccountV) ? (int) PartsToFqH.GetDat(AccountV) : 0;
	PartsToFqH.AddDat(AccountV, OldFq + 1);
}

bool TItemStore::GetItem(const TInt& ItemId, TItemRec& Item) const 
{ 
	TUInt64 CacheId;
	bool found = ItemsH.IsKeyGetDat(ItemId, CacheId); 
	if (!found)
		return false;
	ItemsC.GetVal(CacheId, Item); 
	return true;
}

bool TItemStore::UpdateItem(const TInt& ItemId, const TItemRec& Item)
{
	TUInt64 CacheId;
	bool found = ItemsH.IsKeyGetDat(ItemId, CacheId); 
	if (!found)
		return false;
	ItemsC.SetVal(CacheId, Item); 
	return true;
}

TPartV TItemStore::GetPartV(const int ItemId) const
{ 
	TPartV PartV;
	TItemRec Item; bool ok = GetItem(ItemId, Item); 
	if (!ok) return PartV;
	if (Item.PartVId != InvalidPartVId && PartsHS.IsKeyId(Item.PartVId))
		PartV = PartsHS.GetKey(Item.PartVId); 
	return PartV; 
}

TInt TItemStore::AddTagsV(TStrV TagV) 
{ 
	// first sort tags so that we won't have duplicated lists in the TagsHS
	TagV.Sort();
	return TagsHS.AddKey(TagV);
}

TStrV TItemStore::GetTagsV(const int ItemId) const
{ 
	TStrV TagV;
	TItemRec Item; bool ok = GetItem(ItemId, Item); 
	if (!ok) return TagV;
	if (TagsHS.IsKeyId(Item.TagsVId)) 
		TagV = TagsHS.GetKey(Item.TagsVId); 
	return TagV; 
}

void TItemStore::GetFrequentSocialGroups(int MaxCount, TVec<TPair<TInt, TIntV> > FqAccsV)
{
	// iterate over THash
	int KeyId = PartsToFqH.FFirstKeyId();
	TVec<TPair<TInt, TIntV> > LargeFqAccsV;
	while (PartsToFqH.FNextKeyId(KeyId))
	{
		TIntV AccsV; TInt Fq;
		PartsToFqH.GetKeyDat(KeyId, AccsV, Fq);
		if (Fq >= 5)
			LargeFqAccsV.Add(TPair<TInt, TIntV>(Fq, AccsV));
	}
	LargeFqAccsV.Sort(false);
	LargeFqAccsV.GetSubValV(0, MaxCount, FqAccsV);
}

void TItemStore::PrintStatus(TChA& StatusChA)
{
	StatusChA += "Item Store:\n";
	StatusChA += "Nr. of items: " + TUInt64(GetRecs()).GetStr() + "\n";
	StatusChA += "Item id to miner id hash size: " + TInt(ItemsH.GetMemUsed() / TInt::Kilo).GetStr() + "KB\n";
	StatusChA += "Participants hash set size: " + TInt(PartsHS.GetMemUsed() / TInt::Kilo).GetStr() + "KB\n";
	StatusChA += "Account frequencies hash size: " + TInt(PartsToFqH.GetMemUsed() / TInt::Kilo).GetStr() + "KB\n";
	StatusChA += "Tags hash set items: " + TInt(TagsHS.Len()).GetStr() + "\n";
	StatusChA += "Tags hash set size: " + TInt(TagsHS.GetMemUsed() / TInt::Kilo).GetStr() + "KB\n";
	StatusChA += "EntryId string pool size: " + TInt(EntryIdPool->Size() / TInt::Kilo).GetStr() + "KB\n";
	StatusChA += "\n";
}

///////////////////////////////
// Person Store
TPersonStore::TPersonStore(const TStr& _StoreFNm, const POgIndexVoc &IndexVoc) : 
	TOgStore(PersonStoreId, "Person"), StoreFNm(_StoreFNm), FAccess(faCreate)
{
	IndexVocInst = IndexVoc;
	// set fields
	PersonIdFieldId = AddFieldDesc(TOgFieldDesc("PersonId", oftStr, offtToken, ofatNone, ofdtText));
	
	// join
	JoinHasThreadsId = AddJoinDesc(TOgJoinDesc("hasThreads", ThreadStoreId, PersonStoreId, IndexVoc));

	JoinHasAnyRoleId = AddRole(AnyRoleNm);
	JoinHasFromId = AddRole("from");
	JoinHasToId = AddRole("to");
	JoinHasCCId = AddRole("cc");
	JoinHasBCCId = AddRole("bcc");
	JoinHasAuthorId = AddRole("author");
}

TPersonStore::TPersonStore(const TStr& _StoreFNm, const TFAccess& _FAccess, const POgIndexVoc & IndexVoc):
	TOgStore(_StoreFNm + ".BaseStore"), StoreFNm(_StoreFNm), FAccess(_FAccess)
{
	IndexVocInst = IndexVoc;
	TFIn SIn(StoreFNm + ".Store");
	// fields
	PersonIdFieldId.Load(SIn);
	//joins
	JoinHasThreadsId.Load(SIn);
	JoinHasAnyRoleId.Load(SIn);
	JoinHasFromId.Load(SIn);
	JoinHasToId.Load(SIn);
	JoinHasCCId.Load(SIn);
	JoinHasBCCId.Load(SIn);
	JoinHasAuthorId.Load(SIn);

	RolesV.Load(SIn);
	PersonH.Load(SIn);
}

TPersonStore::~TPersonStore()
{
	if (FAccess != faRdOnly) {
		TOg::Logger->OnStatus("Saving person store ...");
		// save base store
		TFOut BaseFOut(StoreFNm + ".BaseStore"); SaveOgStore(BaseFOut);
		TFOut SOut(StoreFNm + ".Store");
		// fields
		PersonIdFieldId.Save(SOut);
		
		// joins
		JoinHasThreadsId.Save(SOut);

		JoinHasAnyRoleId.Save(SOut);
		JoinHasFromId.Save(SOut);
		JoinHasToId.Save(SOut);
		JoinHasCCId.Save(SOut);
		JoinHasBCCId.Save(SOut);
		JoinHasAuthorId.Save(SOut);
		
		// data
		RolesV.Save(SOut);
		PersonH.Save(SOut);
	}
}

TStr TPersonStore::GetFieldStr(const uint64 &RecId, const int &FieldId) const
{
	if(FieldId == PersonIdFieldId)	{ return TUInt64(RecId).GetStr();	}
	else 
	{
		FieldError(FieldId, "Str");
		return "";
	}
}

void TPersonStore::PrintStatus(TChA& StatusChA)
{
	StatusChA += "Person Store:\n";
	StatusChA += "Nr. of people: " + TUInt64(GetRecs()).GetStr() + "\n";
	TUInt64 MemUsed = PersonH.GetMemUsed() + RolesV.GetMemUsed();
	StatusChA += "Memory used: " + TInt((int) (MemUsed / TInt::Kilo)).GetStr() + "KB\n";
	StatusChA += "\n";
}


///////////////////////////////
// Thread Store
TThreadStore::TThreadStore(const TStr& _StoreFNm, const POgIndexVoc &IndexVoc): 
	TOgStore(ThreadStoreId, "Thread"), StoreFNm(_StoreFNm), FAccess(faCreate)
{
	// set fields
	ThreadFieldId = AddFieldDesc(TOgFieldDesc("Thread", oftStr, offtToken, ofatKeywords, ofdtText));
	
	// set store description
	JoinHasItemsId = AddJoinDesc(TOgJoinDesc("hasItems", ItemStoreId, ThreadStoreId, IndexVoc));
	JoinHasParticipantsId = AddJoinDesc(TOgJoinDesc("hasParticipants", PersonStoreId, ThreadStoreId, IndexVoc));
}

TThreadStore::TThreadStore(const TStr& _StoreFNm, const TFAccess& _FAccess):
	TOgStore(_StoreFNm + ".BaseStore"), StoreFNm(_StoreFNm), FAccess(_FAccess)
{
	TFIn SIn(StoreFNm + ".Store");
	// fields
	ThreadFieldId.Load(SIn);
	// joins
	JoinHasItemsId.Load(SIn);
	JoinHasParticipantsId.Load(SIn);
	// data
	ThreadH.Load(SIn);
}

TThreadStore::~TThreadStore()
{ 
	if (FAccess != faRdOnly) {
		TOg::Logger->OnStatus("Saving thread store ...");
		// save base store
		TFOut BaseFOut(StoreFNm + ".BaseStore"); SaveOgStore(BaseFOut);
		TFOut SOut(StoreFNm + ".Store");
	
		// fields
		ThreadFieldId.Save(SOut); 
		// joins
		JoinHasItemsId.Save(SOut);
		JoinHasParticipantsId.Save(SOut);
		// data
		ThreadH.Save(SOut); 
	}
} 

void TThreadStore::PrintStatus(TChA& StatusChA)
{
	StatusChA += "Thread Store:\n";
	StatusChA += "Nr. of threads: " + TUInt64(GetRecs()).GetStr() + "\n";
	StatusChA += "Memory used: " + TUInt64(ThreadH.GetMemUsed()/TInt::Kilo).GetStr() + "KB\n";
	StatusChA += "\n";
}


///////////////////////////////
// Link Store
TLinkStore::TLinkStore(const TStr& _StoreFNm, const POgIndexVoc &IndexVoc):
	TOgStore(LinkStoreId, "Link"), StoreFNm(_StoreFNm), FAccess(faCreate)
{
	// set fields
	LinkHrefFieldId = AddFieldDesc(TOgFieldDesc("Link", oftStr, offtToken, ofatNone, ofdtText));
	LinkTextFieldId = AddFieldDesc(TOgFieldDesc("Text", oftInt, offtNumeric, ofatNone, ofdtText));

	// set store description
	JoinHasItemsId = AddJoinDesc(TOgJoinDesc("hasItems", ItemStoreId, LinkStoreId, IndexVoc));
}

TLinkStore::TLinkStore(const TStr& _StoreFNm, const TFAccess& _FAccess) : 
	TOgStore(_StoreFNm + ".BaseStore"), StoreFNm(_StoreFNm), FAccess(_FAccess)
{ 
	TFIn SIn(StoreFNm + ".Store");
	// fields	
	LinkHrefFieldId.Load(SIn);
	LinkTextFieldId.Load(SIn);
	// joins
	JoinHasItemsId.Load(SIn);
	// data
	LinkH.Load(SIn);
}

TLinkStore::~TLinkStore()
{ 
	if (FAccess != faRdOnly) {
		TOg::Logger->OnStatus("Saving link store ...");
		// save base store
		TFOut BaseFOut(StoreFNm + ".BaseStore"); SaveOgStore(BaseFOut);
		TFOut SOut(StoreFNm + ".Store");
		// fields
		LinkHrefFieldId.Save(SOut); 
		LinkTextFieldId.Save(SOut); 
		// joins
		JoinHasItemsId.Save(SOut);
		// data
		LinkH.Save(SOut); 
	}
} 

void TLinkStore::PrintStatus(TChA& StatusChA)
{
	StatusChA += "Link Store:\n";
	StatusChA += "Nr. of links: " + TUInt64(GetRecs()).GetStr() + "\n";
	StatusChA += "Memory used: " + TUInt64(LinkH.GetMemUsed()/TInt::Kilo).GetStr() + "KB\n";
	StatusChA += "\n";
}


///////////////////////////////
// Attachment Store
TAttachmentStore::TAttachmentStore(const TStr& _StoreFNm, const POgIndexVoc &IndexVoc):
	TOgStore(AttachmentStoreId, "Attachment"), StoreFNm(_StoreFNm), FAccess(faCreate)
{
	// set fields
	AttItemIdFieldId = AddFieldDesc(TOgFieldDesc("Id", oftInt, offtNumeric, ofatNone, ofdtText));
	NameFieldId = AddFieldDesc(TOgFieldDesc("Name", oftStr, offtToken, ofatNone, ofdtText));
	SizeFieldId = AddFieldDesc(TOgFieldDesc("Size", oftInt, offtNumeric, ofatNone, ofdtText));
	EmailItemIdFieldId = AddFieldDesc(TOgFieldDesc("EmailId", oftInt, offtNumeric, ofatNone, ofdtText));
}

TAttachmentStore::TAttachmentStore(const TStr& _StoreFNm, const TFAccess& _FAccess):
	TOgStore(_StoreFNm + ".BaseStore"), StoreFNm(_StoreFNm), FAccess(_FAccess)
{ 
	TFIn SIn(StoreFNm + ".Store");
	// fields
	AttItemIdFieldId.Load(SIn);
	NameFieldId.Load(SIn);
	SizeFieldId.Load(SIn);
	EmailItemIdFieldId.Load(SIn);
	// data
	AttachmentH.Load(SIn);
}

TAttachmentStore::~TAttachmentStore()
{ 
	if (FAccess != faRdOnly) {
		TOg::Logger->OnStatus("Saving attachment store ...");
		// save base store
		TFOut BaseFOut(StoreFNm + ".BaseStore"); SaveOgStore(BaseFOut);
		TFOut SOut(StoreFNm + ".Store");
		// fields
		AttItemIdFieldId.Save(SOut); 
		NameFieldId.Save(SOut); 
		SizeFieldId.Save(SOut); 
		EmailItemIdFieldId.Save(SOut); 
		// data
		AttachmentH.Save(SOut); 
	}
} 

TTriple<TStr, TInt, TInt> TAttachmentStore::GetAtt(TStr RecNm)
{
	IAssert(IsRecNm(RecNm));
	return AttachmentH.GetDat(RecNm);
}
// field value retrieval
TStr TAttachmentStore::GetFieldStr(const TUInt64& RecId, const int& FieldId) const 
{ 
	TStr key = GetRecNm(RecId);
	TTriple<TStr, TInt, TInt> Dat = AttachmentH.GetDat(key);
	//if (FieldId == AttachmentIdFieldId)	{ return RecId.GetStr();	}
	if (FieldId == NameFieldId) { return Dat.Val1; }
	if (FieldId == EmailItemIdFieldId) { return Dat.Val2.GetStr(); }
	if (FieldId == AttItemIdFieldId) 
	{ 
		if (key[0] == 'X') return "-1";
		else return key;
	}
	if (FieldId == SizeFieldId) { return Dat.Val3.GetStr(); }
	else  {	FieldError(FieldId, "Str");	return "";	}
}

TInt TAttachmentStore::GetFieldInt(const TUInt64& RecId, const int& FieldId) const
{
	TStr key = GetRecNm(RecId);
	TTriple<TStr, TInt, TInt> Dat = AttachmentH.GetDat(key);
	if (FieldId == EmailItemIdFieldId) return Dat.Val2;
	else if (FieldId == AttItemIdFieldId)
	{
		if (key[0] == 'X') return -1;
		else return key.GetInt(-1);
	}
	else if (FieldId == SizeFieldId) return Dat.Val3;
	else  {	FieldError(FieldId, "Int");	return -1;	}
}

void TAttachmentStore::RemoveItem(const int RecId)
{
	TStr key = GetRecNm(RecId);
	// TODO: add removing of the attachment. Currently I can't find any function to remove data from StringHash
}

void TAttachmentStore::PrintStatus(TChA& StatusChA)
{
	StatusChA += "Attachment Store:\n";
	StatusChA += "Nr. of attachments: " + TUInt64(GetRecs()).GetStr() + "\n";
	uint64 MemUsed = 0;
	for (int i=0; i < GetRecs(); i++) {
		TStr Key = GetRecNm(i);
		TTriple<TStr, TInt, TInt> Att = GetAtt(Key);
		MemUsed += Att.GetMemUsed();
	}
	StatusChA += "Memory used: " + TUInt64(MemUsed/ TInt::Kilo).GetStr() + "KB\n";
	StatusChA += "\n";
}

/////////////////////////////////
//// Tag Store
//TTagStore::TTagStore(const TStr& _StoreFNm, const POgIndexVoc &IndexVoc): 
//	TOgStore(TagStoreId, "Tag"), StoreFNm(_StoreFNm), FAccess(faCreate)
//{
//	// set fields
//	TagIdFieldId = AddFieldDesc(TOgFieldDesc("TagId", oftInt, offtNumeric, ofatNone, ofdtText));
//}
//
//TTagStore::TTagStore(const TStr& _StoreFNm, const TFAccess& _FAccess): 
//	TOgStore(_StoreFNm + ".BaseStore"), StoreFNm(_StoreFNm), FAccess(_FAccess)
//{
//	TFIn SIn(StoreFNm + ".Store");
//	// fields
//	TagIdFieldId.Load(SIn);
//	// data
//	TagsH.Load(SIn);
//}
//
//TTagStore::~TTagStore()
//{
//	if (FAccess != faRdOnly) {
//		TOg::Logger->OnStatus("Saving tag store ...");
//		// save base store
//		TFOut BaseFOut(StoreFNm + ".BaseStore"); SaveOgStore(BaseFOut);
//		TFOut SOut(StoreFNm + ".Store");
//		// fields
//		TagIdFieldId.Save(SOut);
//		// data
//		TagsH.Save(SOut);
//	}
//}
//
//int TTagStore::AddItem(int TagId)
//{
//	TagsH.AddKey(TagId);
//	return TagId;
//}
//
//int TTagStore::GetFieldInt(const uint64& RecId, const int& FieldId) const 
//{
//	if (!TagsH.IsKey((int) RecId)) return -1;
//	if (FieldId == TagIdFieldId) { return (int) RecId; }
//	FieldError(FieldId, "Int"); 
//	return -1; 
//}