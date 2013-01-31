#ifndef __ITEMSTORE_H__
#define __ITEMSTORE_H__

#include <qminer.h>

static const uchar ItemStoreId = 1;
static const uchar PersonStoreId = 2;
static const uchar ThreadStoreId = 3;
static const uchar LinkStoreId = 4;
static const uchar AttachmentStoreId = 5;
static const uchar TagStoreId = 6;

static const int InvalidPartVId = -1;

enum ItemTypeEnum
{
	Invalid = -1,
	Email = 0,
	Attachment,
	Document,
	FBWallPost,
	FBMessage
};

static const TStr AnyRoleNm("_ANY_");

template <class THash>
class TOgStoreIterHashKey : public TOgStoreIter {
private:
	const THash& Hash;
	int KeyId;

public:
	TOgStoreIterHashKey(const THash& _Hash): Hash(_Hash), KeyId(Hash.FFirstKeyId()) { }
	static POgStoreIter New(const THash& Hash) { 
		return new TOgStoreIterHashKey<THash>(Hash); }

	bool Next() 
	{ 
		return Hash.FNextKeyId(KeyId); 
	}
	uint64 GetRecId() const 
	{ 
		return (uint64) Hash.GetKey(KeyId); 
	}
};

template <class THashSet>
class TOgStoreIterHashSetKey : public TOgStoreIter {
private:
	const THashSet& HashSet;
	int KeyId;

public:
	TOgStoreIterHashSetKey(const THashSet& _HashSet): HashSet(_HashSet), KeyId(HashSet.FFirstKeyId()) { }
	static POgStoreIter New(const THashSet& HashSet) { 
		return new TOgStoreIterHashSetKey<THashSet>(HashSet); }

	bool Next() 
	{ 
		return HashSet.FNextKeyId(KeyId); 
	}
	uint64 GetRecId() const 
	{ 
		return (uint64) HashSet.GetKey(KeyId); 
	}
};

typedef TPair<TSInt, TInt> TPart;
typedef TVec<TPart> TPartV;

///////////////////////////////
// Item-Store
class TItemStore;
typedef TPt<TItemStore> PItemStore;
class TItemStore : public TOgStore
{
public:
	// Item
	class TItemRec	{
	public:
		TCh ItemType;
		TUInt EntryIdId;
		TUInt64 Time;
		TInt ThreadId;
		TInt TagsVId;
		TInt PartVId;
	
		TItemRec() { }
		TItemRec(const TCh _ItemType, const TUInt& _EntryIdId, const TUInt64& _Time, 
				const TInt& _ThreadId, const TInt& _TagsVId):
				ItemType(_ItemType), EntryIdId(_EntryIdId), Time(_Time), 
				ThreadId(_ThreadId), TagsVId(_TagsVId), PartVId(InvalidPartVId) { }

		TItemRec(TSIn& SIn): ItemType(SIn), EntryIdId(SIn), Time(SIn), ThreadId(SIn), TagsVId(SIn), PartVId(SIn) { }
		int64 GetMemUsed() const { return int64(sizeof(TCh) + 5*sizeof(TUInt) + sizeof(TUInt64)); }

		void Load(TSIn& SIn) { ItemType = TCh(SIn); EntryIdId.Load(SIn); Time.Load(SIn); ThreadId.Load(SIn); TagsVId.Load(SIn); PartVId.Load(SIn); }
		void Save(TSOut& SOut) const 
		{ 
			ItemType.Save(SOut);
			EntryIdId.Save(SOut);
			Time.Save(SOut);
			ThreadId.Save(SOut);
			TagsVId.Save(SOut);
			PartVId.Save(SOut);
		}
	};

public:
	TInt ItemTypeFieldId;
	TInt ItemIdFieldId;
	TInt EntryIdFieldId;
	TInt TimeFieldId;
	TInt ThreadIdFieldId;
	TInt TagsFieldId;

	//join ids
	TInt JoinHasLinksId;
	TInt JoinHasAttachmentsId;
	TInt JoinHasRelatedItemsId;
	TInt JoinHasParentItemId;

private:
	// map from article URI to article Id in cache
	//TStrHash<TUInt64> ItemsH;
	// store for articles
	TBlockCache<TItemRec> ItemsC;
	// map from itemId to rec id in cache
	THash<TInt, TUInt64> ItemsH;	
	THashSet<TPartV> PartsHS;		// participants hash set
	THash<TIntV, TInt> PartsToFqH;	// how frequently the same group of participants occurs in the data
	THashSet<TStrV> TagsHS;
	PStrPool EntryIdPool;
	
	TStr StoreFNm;
	TFAccess FAccess;
	POgIndexVoc IndexVocInst;

public:
	TItemStore(const TStr& _StoreFNm, const int64& MxCacheSize, const POgIndexVoc & IndexVoc);
	TItemStore(const TStr& _StoreFNm, const TFAccess& _FAccess, const int64& MxCacheSize, const POgIndexVoc & IndexVoc);	
	~TItemStore();
	
	POgStore GetOgStore() { return POgStore(this); }

	// records (inherited)
	/*bool IsRecId(const uint64& RecId) const { return ArticleH.IsKeyId((int)RecId); }
	bool IsRecNm(const TStr& RecNm) const { return ArticleH.IsKey(RecNm); }
    TStr GetRecNm(const uint64& RecId) const { return ArticleH.GetKey((int)RecId); }
    uint64 GetRecId(const TStr& RecNm) const { return (uint64)ArticleH.GetKeyId(RecNm); }
    uint64 GetRecs() const { return (uint64)ArticleH.Len(); }
	POgStoreIter GetIter() const { return TOgStoreIterHash<TStrHash<TUInt64> >::New(ArticleH); }*/

	bool IsRecId(const uint64 & ItemId) const { return ItemsH.IsKey((int) ItemId); }
	bool IsRecNm(const TStr & RecNm) const { return ItemsH.IsKey(RecNm.GetInt()); }
	TStr GetRecNm(const uint64 & ItemId) const { return TUInt64(ItemId).GetStr(); }
	uint64 GetRecId(const TStr& RecNm) const { return (uint64) RecNm.GetInt(); }
	POgStoreIter GetIter() const { return TOgStoreIterHashKey<THash<TInt, TUInt64> >::New(ItemsH); }
	uint64 GetRecs() const { return ItemsH.Len(); }
	int GetItemId(const uint64 ItemId) const { return ItemsH.GetKey((int) ItemId); }

	bool SetTag(int ItemId, const TStr& TagId);
	bool RemoveTag(int ItemId, const TStr& TagId);
	
	// field value retrieval
	int GetFieldInt(const uint64& ItemId, const int& FieldId) const;
	TStr GetFieldStr(const uint64& ItemId, const int& FieldId) const;
	void GetFieldStrV(const uint64& ItemId, const int& FieldId, TStrV& StrV) const;
	void GetFieldTm(const uint64& ItemId, const int& FieldId, TTm& Tm) const;
	uint64 GetFieldUInt64(const uint64& ItemId, const int& FieldId) const;

	// custom functions
	uint64 AddRec(int ItemId, const TCh ItemType, const TStr& EntryId, const TUInt64& Time, 
				const TInt& ThreadId, const TStrV& TagsV);
	void RemoveItem(const int ItemId);
	bool GetItem(const TInt& ItemId, TItemRec& Item) const ;
	bool UpdateItem(const TInt& ItemId, const TItemRec& Item);
	bool IsItem(const TInt& ItemId) { return ItemsH.IsKey(ItemId); }
	
	TPartV GetPartV(const int PartVId) const;
	TInt AddTagsV(TStrV TagV);
	TStrV GetTagsV(const int TagVId) const;
	
	void SetParticipants(const int ItemId, TPartV& PartV);
	TUInt AddEntryId(const TStr& EntryId) { return EntryIdPool->AddStr(EntryId); }
	TStr GetEntryId(const TUInt& EntryIdId) const	{ return EntryIdPool->GetStr(EntryIdId); }
		
	int GetCustomJoinId(const TStr& JoinNm, const uchar JoinStoreId, const uchar StoreId)
	{
		if (!IsJoinNm(JoinNm))
			AddJoinDesc(TOgJoinDesc(JoinNm, JoinStoreId, StoreId, IndexVocInst));
		return GetJoinDesc(GetJoinId(JoinNm)).GetJoinKeyId();
	}
	void GetFrequentSocialGroups(int MaxCount, TVec<TPair<TInt, TIntV> > FqAccsV);
	void PrintStatus(TChA& StatusChA);
};

///////////////////////////////
// Person-Store
class TPersonStore;
typedef TPt<TPersonStore> PPersonStore;
class TPersonStore : public TOgStore
{
public:
	//enum Role { Sender = 0, Recipient };
	TInt PersonIdFieldId;

	//join ids
	TInt JoinHasThreadsId;
	
	TInt JoinHasAnyRoleId;
	TInt JoinHasFromId;
	TInt JoinHasToId;
	TInt JoinHasCCId;
	TInt JoinHasBCCId;
	TInt JoinHasAuthorId;

	TStr StoreFNm;
	TFAccess FAccess;
private:	
	THashSet<TInt> PersonH;	
	TStrV RolesV;
	POgIndexVoc IndexVocInst;

public:
	TPersonStore(const TStr& _StoreFNm, const POgIndexVoc & IndexVoc);
	TPersonStore(const TStr& _StoreFNm, const TFAccess& _FAccess, const POgIndexVoc & IndexVoc);	
	~TPersonStore();

	POgStore GetOgStore() { return POgStore(this); }

	// records (inherited)
	bool IsRecId(const uint64& RecId) const { return PersonH.IsKey((int) RecId); }
	bool IsRecNm(const TStr & RecNm) const { return PersonH.IsKey(RecNm.GetInt()); }
	TStr GetRecNm(const uint64 & RecId) const { return TUInt64(RecId).GetStr(); }
	uint64 GetRecId(const TStr& RecNm) const { return RecNm.GetInt(); }
	uint64 GetRecs() const { return PersonH.Len(); }
	POgStoreIter GetIter() const { return TOgStoreIterHash<THashSet<TInt> >::New(PersonH); }

	// field value retrieval
	TStr GetFieldStr(const uint64& RecId, const int& FieldId) const;
	
	// custom functions
	int AddPerson(const TInt& ItemId) { if (!PersonH.IsKey(ItemId)) PersonH.AddKey(ItemId);	return ItemId; }
	
	int GetRoles() { return RolesV.Len(); }
	bool IsRole(const TStr& RoleNm) { return RolesV.IsIn(RoleNm); }
	TStr GetRole(int RoleN) { Assert(RoleN < RolesV.Len()); return RolesV[RoleN]; }

	// add a role if not existing and return the join id
	int AddRole(const TStr& RoleNm)
	{
		Assert(ItemStoreId >= 0);
		Assert(PersonStoreId >= 0);
		if (!IsJoinNm(RoleNm))
		{
			AddJoinDesc(TOgJoinDesc(RoleNm, (uchar) ItemStoreId, (uchar) PersonStoreId, IndexVocInst));
			RolesV.AddUnique(RoleNm);
		}
		return GetJoinId(RoleNm);
	}
	void PrintStatus(TChA& StatusChA);
private:
	void AddItemToSortedList(TInt& ItemId, TIntV& IntV, TItemStore* ItemStore);
};


///////////////////////////////
// Thread-Store
class TThreadStore;
typedef TPt<TThreadStore> PThreadStore;
class TThreadStore : public TOgStore
{
public:
	TInt ThreadFieldId;
	//Join ids
	TInt JoinHasItemsId;
	TInt JoinHasParticipantsId;

	TStr StoreFNm;
	TFAccess FAccess;

private:	
	THashSet<TStr> ThreadH;	
	
public:
	TThreadStore(const TStr& _StoreFNm, const POgIndexVoc & IndexVoc);
	TThreadStore(const TStr& _StoreFNm, const TFAccess& _FAccess);	
	~TThreadStore();

	POgStore GetOgStore() { return POgStore(this); }

	int AddRec(const TStr& Thread) { return ThreadH.AddKey(Thread); }
	// records (inherited)
	bool IsRecId(const uint64& RecId) const { return ThreadH.IsKeyId((int) RecId); }
	bool IsRecNm(const TStr & RecNm) const { return ThreadH.IsKey(RecNm); }
	TStr GetRecNm(const uint64 & RecId) const { return ThreadH.GetKey((int) RecId); }
	uint64 GetRecId(const TStr& RecNm) const { return ThreadH.GetKeyId(RecNm); }
	uint64 GetRecs() const { return ThreadH.Len(); }
	POgStoreIter GetIter() const { return TOgStoreIterHash<THashSet<TStr> >::New(ThreadH); }

	// field value retrieval
	TStr GetFieldStr(const uint64& RecId, const int& FieldId) const 
	{ 
		if (FieldId == ThreadFieldId)	{ return ThreadH.GetKey((int) RecId);	}
		else  {	FieldError(FieldId, "Str");	return "";	}
	}
	void PrintStatus(TChA& StatusChA);
};


///////////////////////////////
// Link-Store
class TLinkStore;
typedef TPt<TLinkStore> PTLinkStore;
class TLinkStore : public TOgStore
{
public:
	//TInt LinkFieldId;
	TInt LinkHrefFieldId;
	TInt LinkTextFieldId;
	// Join ids
	TInt JoinHasItemsId;

	TStr StoreFNm;
	TFAccess FAccess;
private:	
	THashSet<TPair<TStr, TStr> > LinkH;	// (link href, link text)

public:
	TLinkStore(const TStr& _StoreFNm, const POgIndexVoc & IndexVoc);
	TLinkStore(const TStr& _StoreFNm, const TFAccess& _FAccess);
	~TLinkStore();

	POgStore GetOgStore() { return POgStore(this); }

	int AddRec(const TStr& Href, const TStr& Text) 
	{ 
		TPair<TStr, TStr> Key = TPair<TStr, TStr>(Href, Text);
		if (LinkH.IsKey(Key))
			return LinkH.GetKeyId(Key);
		else
			return LinkH.AddKey(Key); 
	}

	// records (inherited)
	bool IsRecId(const uint64& RecId) const { return LinkH.IsKeyId((int) RecId); }
	bool IsRecNm(const TStr & RecNm) const { return IsRecId(RecNm.GetInt64()); }
	TStr GetRecNm(const uint64 & RecId) const { return TUInt64(RecId).GetStr(); }
	uint64 GetRecId(const TStr& RecNm) const { return RecNm.GetUInt64(); }
	uint64 GetRecs() const { return LinkH.Len(); }
	POgStoreIter GetIter() const { return TOgStoreIterHash<THashSet<TPair<TStr, TStr> > >::New(LinkH); }

	// field value retrieval
	TStr GetFieldStr(const uint64& RecId, const int& FieldId) const 
	{ 
		if (FieldId == LinkHrefFieldId) 
		{ 
			TPair<TStr, TStr> Key = LinkH.GetKey((int) RecId);
			return Key.Val1;
		}
		else if (FieldId == LinkTextFieldId) 
		{ 
			TPair<TStr, TStr> Key = LinkH.GetKey((int) RecId);
			return Key.Val2;
		}
		else  {	FieldError(FieldId, "Str");	return "";	}
	}
	void PrintStatus(TChA& StatusChA);
};

///////////////////////////////
// Attachment-Store
class TAttachmentStore;
typedef TPt<TAttachmentStore> PAttachmentStore;
class TAttachmentStore : public TOgStore
{
public:
	TInt NameFieldId;
	TInt SizeFieldId;
	TInt EmailItemIdFieldId;
	TInt AttItemIdFieldId;

	TStr StoreFNm;
	TFAccess FAccess;
private:
	TStrHash<TTriple<TStr, TInt, TInt> > AttachmentH;	// attachment name, email item id, ,attachment size

public:
	TAttachmentStore(const TStr& _StoreFNm, const POgIndexVoc & IndexVoc);
	TAttachmentStore(const TStr& _StoreFNm, const TFAccess& _FAccess);	
	~TAttachmentStore();

	POgStore GetOgStore() { return POgStore(this); }

	int AddRec(const TStr& AttachmentName, TInt AttachmentSize, TInt EmailItemId, TInt AttItemId)
	{
		TTriple<TStr, TInt, TInt> Dat = TTriple<TStr, TInt, TInt>(AttachmentName, EmailItemId, AttachmentSize);
		if (AttItemId != -1)
			return AttachmentH.AddDat(AttItemId.GetStr(), Dat);
		else
			return AttachmentH.AddDat("X"+AttachmentH.Len(), Dat);
	}

	// records (inherited)
	bool IsRecId(const uint64& RecId) const { return AttachmentH.IsKeyId((int) RecId); }
	bool IsRecNm(const TStr & RecNm) const { return AttachmentH.IsKey(RecNm); }
	TStr GetRecNm(const uint64 & RecId) const { return AttachmentH.GetKey((int)RecId); }
	uint64 GetRecId(const TStr& RecNm) const { return AttachmentH.GetKeyId(RecNm); }
	uint64 GetRecs() const { return AttachmentH.Len(); }
	POgStoreIter GetIter() const { return TOgStoreIterHash<TStrHash<TTriple<TStr, TInt, TInt> > >::New(AttachmentH); }
	
	TTriple<TStr, TInt, TInt> GetAtt(TStr RecNm);
	TStr GetFieldStr(const TUInt64& RecId, const int& FieldId) const ;
	TInt GetFieldInt(const TUInt64& RecId, const int& FieldId) const;
	void RemoveItem(const int RecId);
	void PrintStatus(TChA& StatusChA);
};

///////////////////////////////
// Tag Store
//class TTagStore;
//typedef TPt<TTagStore> PTagStore;
//class TTagStore : public TOgStore
//{
//public:
//	TInt TagIdFieldId;
//
//	TStr StoreFNm;
//	TFAccess FAccess;
//private:
//	THashSet<TInt> TagsH;	
//	
//public:
//	TTagStore(const TStr& _StoreFNm, const POgIndexVoc &IndexVoc);
//	TTagStore(const TStr& _StoreFNm, const TFAccess& _FAccess);	
//	~TTagStore();
//	POgIndexVoc IndexVocInst;
//	POgStore GetOgStore() { return POgStore(this); }
//
//	// records (inherited)
//	bool IsRecId(const uint64 & RecId) const { return TagsH.IsKey((int) RecId); }
//	bool IsRecNm(const TStr & RecNm) const { return TagsH.IsKey(RecNm.GetInt()); }
//	TStr GetRecNm(const uint64 & RecId) const { return TUInt64(RecId).GetStr(); }
//	uint64 GetRecId(const TStr& RecNm) const { return (uint64) RecNm.GetInt(); }
//	POgStoreIter GetIter() const { return TOgStoreIterHashSetKey<THashSet<TInt> >::New(TagsH); }
//	uint64 GetRecs() const { return TagsH.Len(); }
//
//	// field value retrieval
//	int GetFieldInt(const uint64& RecId, const int& FieldId) const;
//	int AddItem(int TagId);
//	int GetCustomJoinId(const TStr& JoinNm, const uchar JoinStoreId, const uchar StoreId)
//	{
//		if (!IsJoinNm(JoinNm))
//			AddJoinDesc(TOgJoinDesc(JoinNm, JoinStoreId, StoreId, IndexVocInst));
//		return GetJoinDesc(GetJoinId(JoinNm)).GetJoinKeyId();
//	}
//};

#endif