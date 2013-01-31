#ifndef QMINER_H
#define QMINER_H

#include <base.h>
#include <mine.h>

class TOg {
public:
	static PNotify Logger;

private:
	static TChA ValidFirstCh;
	static TChA ValidCh;
public:
	static void AssertValidNm(const TStr& NmStr);
};

class TOgRec;
ClassHdTP(TOgIndexVoc, POgIndexVoc)
ClassHdTP(TOgIndex, POgIndex)
ClassHdTP(TOgStore, POgStore)
ClassHdTP(TOgRecSet, POgRecSet)
ClassHdTP(TOgBase, POgBase)
ClassHdTP(TOgFtrExt, POgFtrExt)
ClassHdTP(TOgAggr, POgAggr)

class TOgExcept : public TExcept {
private:
	TOgExcept(const TStr& MsgStr): TExcept(MsgStr) { }
	TOgExcept(const TStr& MsgStr, const TStr& LocStr): TExcept(MsgStr, LocStr) { }
public:
	static PExcept New(const TStr& MsgStr) { 
		return PExcept(new TOgExcept(MsgStr));
	}
	static PExcept New(const TStr& MsgStr, const TStr& LocStr) { 
		return PExcept(new TOgExcept(MsgStr, LocStr));
	}
	static void Throw(const TStr& MsgStr) { 
		throw PExcept(new TOgExcept(MsgStr));
	}
	static void Throw(const TStr& MsgStr, const TStr& LocStr) { 
		throw PExcept(new TOgExcept(MsgStr, LocStr));
	}
};

#define OgAssert(Cond) \
  ((Cond) ? static_cast<void>(0) : TOgExcept::Throw(TStr(__FILE__) + " line " + TInt::GetStr(__LINE__) + ": " + TStr(#Cond)))

#define OgAssertR(Cond, MsgStr) \
  ((Cond) ? static_cast<void>(0) : TOgExcept::Throw(MsgStr, TStr(__FILE__) + " line "  +TInt::GetStr(__LINE__) + ": " + TStr(#Cond)))

typedef enum { 
	osjtUndef,
	osjtIndex,
	osjtField
} TOgStoreJoinType;

ClassTV(TOgJoinDesc, TOgJoinDescV)//{
private:
    TInt JoinId;
	TStr JoinNm;
	TUCh JoinStoreId;
	TOgStoreJoinType JoinType;
	TInt JoinKeyId;
	TInt JoinFieldId;

public:
	TOgJoinDesc(): JoinId(-1), JoinStoreId(255), JoinType(osjtUndef) { } 
	TOgJoinDesc(const TStr& _JoinNm, const uchar& _JoinStoreId,
		const uchar& StoreId, const POgIndexVoc& IndexVoc);
	TOgJoinDesc(const TStr& _JoinNm, const uchar& _JoinStoreId,
		const int& _JoinFieldId): JoinId(-1), JoinNm(_JoinNm), 
			JoinStoreId(_JoinStoreId), JoinType(osjtField), 
			JoinKeyId(-1), JoinFieldId(_JoinFieldId) { TOg::AssertValidNm(JoinNm); }

	TOgJoinDesc(TSIn& SIn);
	void Save(TSOut& SOut) const;

	void PutJoinId(const int& _JoinId) { JoinId = _JoinId; }
    int GetJoinId() const { return JoinId; };
	const TStr& GetJoinNm() const { return JoinNm; }
	uchar GetJoinStoreId() const { return JoinStoreId; }
	POgStore GetJoinStore(const POgBase& OgBase) const;
	bool IsIndexJoin() const { return JoinType == osjtIndex; }
	int GetJoinKeyId() const { return JoinKeyId; }
	bool IsFieldJoin() const { return JoinType == osjtField; }
	int GetJoinFieldId() const { return JoinFieldId; }
};

ClassTV(TOgJoinSeq, TOgJoinSeqV)//{
private:
	TUCh StartStoreId;
	TIntPrV JoinIdV;

public:
	TOgJoinSeq(): StartStoreId(TUCh::Mx) { }
	TOgJoinSeq(const uchar& _StartStoreId): StartStoreId(_StartStoreId) { }
	TOgJoinSeq(const uchar& _StartStoreId, const int& JoinId, const int& Sample = -1);
	TOgJoinSeq(const uchar& _StartStoreId, const TIntPrV& _JoinIdV);

	bool IsEmpty() const { return (StartStoreId.Val == TUCh::Mx); }
	bool IsJoin() const { return JoinIdV.Empty(); }

	POgStore GetStartStore(const POgBase& OgBase) const;
	uchar GetStartStoreId() const { return StartStoreId.Val; }
	POgStore GetEndStore(const POgBase& OgBase) const;
	uchar GetEndStoreId(const POgBase& OgBase) const;
	const TIntPrV& GetJoinIdV() const { return JoinIdV; }
};

typedef enum { 
	oftInt		= 0,
	oftIntV		= 9,
	oftUInt64	= 8,
	oftStr		= 1, 
	oftStrV		= 2, 
	oftBool		= 4, 
	oftFlt		= 5,
	oftFltPr	= 6,
	oftFltV		= 10,
	oftTm		= 7, 
	oftNumSpV	= 11, 
	oftBowSpV	= 12 
} TOgFieldType;

typedef enum { 
	offtNone		= 0, 
	offtNumeric		= 1, 
	offtNominal		= 2, 
	offtMultiNom	= 3,
	offtToken		= 4,
	offtSpNum		= 5,
	offtTm			= 6
} TOgFieldFtrType;

typedef enum { 
	ofatNone		= 0, 
	ofatHistogram	= 1, 
	ofatPiechart	= 2, 
	ofatTimeline	= 3, 
	ofatKeywords	= 4, 
	ofatScalar		= 5 
} TOgFieldAggrType;

typedef enum { 
	ofdtNone	= 0, 
	ofdtText	= 1, 
	ofdtMap		= 2 
} TOgFieldDisplayType;

ClassTV(TOgFieldDesc, TOgFieldDescV)//{
private:
    TInt FieldId;
    TStr FieldNm;
	TOgFieldType FieldType;
    TOgFieldFtrType DefFtrType;
	TOgFieldAggrType AggrType;
	TOgFieldDisplayType DisplayType;
	TIntV KeyIdV;

public:
	TOgFieldDesc(): FieldId(-1) { }
    TOgFieldDesc(const TStr& _FieldNm, TOgFieldType _FieldType, const TOgFieldFtrType& _DefFtrType,
        const TOgFieldAggrType _AggrType, const TOgFieldDisplayType _DisplayType): FieldId(-1),
			FieldNm(_FieldNm), FieldType(_FieldType), DefFtrType(_DefFtrType), AggrType(_AggrType), 
			DisplayType(_DisplayType) { TOg::AssertValidNm(FieldNm); }
	TOgFieldDesc(TSIn& SIn);
	void Save(TSOut& SOut);

	void PutFieldId(const int& _FieldId) { FieldId = _FieldId; }
    int GetFieldId() const { return FieldId; };
    const TStr& GetFieldNm() const { return FieldNm; }

	// field data-type
    TOgFieldType GetFieldType() const { return FieldType; }
	TStr GetFieldTypeStr() const;
    bool IsInt() const { return FieldType == oftInt; }
    bool IsIntV() const { return FieldType == oftIntV; }
    bool IsUInt64() const { return FieldType == oftUInt64; }
    bool IsStr() const { return FieldType == oftStr; }
    bool IsStrV() const { return FieldType == oftStrV; }
    bool IsBool() const { return FieldType == oftBool; }
    bool IsFlt() const { return FieldType == oftFlt; }
    bool IsFltPr() const { return FieldType == oftFltPr; }
    bool IsFltV() const { return FieldType == oftFltV; }
    bool IsTm() const { return FieldType == oftTm; }
    bool IsNumSpV() const { return FieldType == oftNumSpV; }
    bool IsBowSpV() const { return FieldType == oftBowSpV; }

	// default field feature type
	bool HasDefFtrType() const { return (DefFtrType != offtNone); }
    TOgFieldFtrType GetDefFtrType() const { return DefFtrType; }
    TStr GetDefFtrTypeStr() const;
	bool IsDefFtrNumeric() const { return (DefFtrType == offtNumeric); }
	bool IsDefFtrNominal() const { return (DefFtrType == offtNominal); }
	bool IsDefFtrMultiNom() const { return (DefFtrType == offtMultiNom); }
	bool IsDefFtrToken() const { return (DefFtrType == offtToken); }
	bool IsDefFtrSpNum() const { return (DefFtrType == offtSpNum); }
	bool IsDefFtrTm() const { return (DefFtrType == offtTm); }

	// field aggregation type
	bool HasDefAggr() const { return (AggrType != ofatNone); }
    TOgFieldAggrType GetAggrType() const { return AggrType; }
    TStr GetAggrTypeStr() const;
	bool IsAggrHistogram() const { return (AggrType == ofatHistogram); }
	bool IsAggrKeywords() const { return (AggrType == ofatKeywords); }
	bool IsAggrPiechart() const { return (AggrType == ofatPiechart); }
	bool IsAggrScalar() const { return (AggrType == ofatScalar); }
	bool IsAggrTimeline() const { return (AggrType == ofatTimeline); }

	// field display type
	bool HasDisplay() const { return (DisplayType != ofdtNone); }
    TOgFieldDisplayType GetDisplayType() const { return DisplayType; }
    TStr GetDisplayTypeStr() const;
	bool IsDisplayText() const { return (DisplayType == ofdtText); }
	bool IsDisplayMap() const { return (DisplayType == ofdtMap); }

	// linked keys
	void AddKey(const int& KeyId) { KeyIdV.Add(KeyId); }
	bool IsKeys() const { return !KeyIdV.Empty(); }
	int GetKeys() const { return KeyIdV.Len(); }
	int GetKeyId(const int& KeyIdN) const { return KeyIdV[KeyIdN]; }
	int GetKeyId() const { OgAssert(IsKeys()); return KeyIdV[0]; }
};

ClassTP(TOgStoreIter, POgStoreIter)//{
public:
	virtual ~TOgStoreIter() { }
	virtual bool Next() = 0;
	virtual uint64 GetRecId() const = 0;
};

class TOgStoreIterVec : public TOgStoreIter {
private:
	bool FirstP;
	uint64 RecId;
	uint64 RecIds;

	TOgStoreIterVec(const uint64& _RecIds);
public:
	static POgStoreIter New(const uint64& RecIds) { 
		return new TOgStoreIterVec(RecIds); }

	bool Next();
	uint64 GetRecId() const { Assert(!FirstP); return RecId; }
};

template <class THash>
class TOgStoreIterHash : public TOgStoreIter {
private:
	const THash& Hash;
	int KeyId;

public:
	TOgStoreIterHash(const THash& _Hash): Hash(_Hash), KeyId(Hash.FFirstKeyId()) { }
	static POgStoreIter New(const THash& Hash) { 
		return new TOgStoreIterHash<THash>(Hash); }

	bool Next() { return Hash.FNextKeyId(KeyId); }
	uint64 GetRecId() const { return (uint64)KeyId; }
};

ClassTPV(TOgStoreTrigger, POgStoreTrigger, TOgStoreTriggerV)//{
public:
	virtual ~TOgStoreTrigger() { }
    virtual void Init(const TOgStore* Store) { }
    virtual void OnAdd(const POgBase& OgBase, const TOgStore* Store, const uint64& RecId) { }
    virtual void OnUpdate(const POgBase& OgBase, const TOgStore* Store, const uint64& RecId) { }
    virtual void OnDelete(const POgBase& OgBase, const TOgStore* Store, const uint64& RecId) { }
};

ClassTP(TOgStore, POgStore)//{
private:
	// store meta-data
    TUCh StoreId;
    TStr StoreNm;
	// joins meta-data
    TOgJoinDescV JoinDescV;
    TStrH JoinNmToIdH;
    TStrH JoinNmToKeyIdH;
	// fields meta-data
    TOgFieldDescV FieldDescV;
    TStrH FieldNmToIdH;
    // triggers
    TOgStoreTriggerV TriggerV;

    void LoadOgStore(TSIn& SIn);
protected:
	TOgStore(uchar _StoreId, const TStr& _StoreNm): StoreId(_StoreId),
		StoreNm(_StoreNm) {  TOg::AssertValidNm(StoreNm); }
	TOgStore(TSIn& SIn) { LoadOgStore(SIn); }
	TOgStore(const TStr& FNm) { TFIn FIn(FNm); LoadOgStore(FIn); }
    virtual ~TOgStore() { }

	void SaveOgStore(TSOut& SOut) const;

    int AddFieldDesc(const TOgFieldDesc& FieldDesc);
    void FieldError(const int& FieldId, const TStr& TypeStr) const;

    void OnAdd(const POgBase& OgBase, const uint64& RecId);
    void OnUpdate(const POgBase& OgBase, const uint64& RecId);
    void OnDelete(const POgBase& OgBase, const uint64& RecId);

    void StrVToIntV(const TStrV& StrV, TStrHash<TInt, TBigStrPool>& WordH, TIntV& IntV);
    void IntVToStrV(const TIntV& IntV, const TStrHash<TInt, TBigStrPool>& WordH, TStrV& StrV) const;
	void IntVToStrV(const TIntV& IntV, TStrV& StrV) const;

public:
    uchar GetStoreId() const { return StoreId; }
    TStr GetStoreNm() const { return StoreNm; }

    int AddJoinDesc(const TOgJoinDesc& JoinDesc);
	int GetJoins() const { return JoinDescV.Len(); }
	bool IsJoinId(const int& JoinId) const { return (JoinId >=0) && (JoinId < JoinDescV.Len()); }
	bool IsJoinNm(const TStr& JoinNm) const { return JoinNmToIdH.IsKey(JoinNm); }
	const TStr& GetJoinNm(const int JoinId) const { return JoinDescV[JoinId].GetJoinNm(); }
	int GetJoinId(const TStr& JoinNm) const { return JoinNmToIdH.GetDat(JoinNm); }
	int GetJoinKeyId(const TStr& JoinNm) const { return JoinNmToKeyIdH.GetDat(JoinNm); }
	int GetJoinKeyId(const int& JoinId) const { return JoinDescV[JoinId].GetJoinKeyId(); }
	const TOgJoinDesc& GetJoinDesc(const int& JoinId) const { return JoinDescV[JoinId]; }

    int GetFields() const { return FieldDescV.Len(); }
	bool IsFieldId(const int& FieldId) const { return (FieldId >=0) && (FieldId < FieldDescV.Len()); }
    const TStr& GetFieldNm(const int& FieldId) const { return FieldDescV[FieldId].GetFieldNm(); }
    bool IsFieldNm(const TStr& FieldNm) const { return FieldNmToIdH.IsKey(FieldNm); }
    int GetFieldId(const TStr& FieldNm) const { return FieldNmToIdH.GetDat(FieldNm); }
    const TOgFieldDesc& GetFieldDesc(const int& FieldId) const { return FieldDescV[FieldId]; }
	TIntV GetFieldIdV(const TOgFieldType& Type);
	void AddFieldKey(const int& FieldId, const int& KeyId) { FieldDescV[FieldId].AddKey(KeyId); }
	int GetFieldKeyId(const int& FieldId) const { return FieldDescV[FieldId].GetKeyId(); }
    
    void AddTrigger(const POgStoreTrigger& Trigger);
	
    virtual bool IsRecId(const uint64& RecId) const = 0;
	virtual bool IsRecNm(const TStr& RecNm) const = 0;
	virtual TStr GetRecNm(const uint64& RecId) const = 0;
    virtual uint64 GetRecId(const TStr& RecNm) const = 0;
	TOgRec GetRec(const uint64& RecId) const;
	TOgRec GetRec(const TStr& RecNm) const;
	virtual uint64 GetRecs() const = 0; 
	virtual POgStoreIter GetIter() const = 0;
	virtual POgRecSet GetAllRecs() const;
	virtual POgRecSet GetRndRecs(const uint64& SampleSize) const;
	bool Empty() const { return (GetRecs() == uint64(0)); }

    virtual int GetFieldInt(const uint64& RecId, const int& FieldId) const;
    virtual void GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const;
    virtual uint64 GetFieldUInt64(const uint64& RecId, const int& FieldId) const;
	virtual TStr GetFieldStr(const uint64& RecId, const int& FieldId) const;
	virtual void GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const;
	virtual bool GetFieldBool(const uint64& RecId, const int& FieldId) const;
    virtual double GetFieldFlt(const uint64& RecId, const int& FieldId) const;
    virtual TFltPr GetFieldFltPr(const uint64& RecId, const int& FieldId) const;
    virtual void GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const;
    virtual void GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const;
    virtual void GetFieldNumSpV(const uint64& RecId, const int& FieldId, TIntFltKdV& SpV) const;
    virtual void GetFieldBowSpV(const uint64& RecId, const int& FieldId, PBowSpV& SpV) const;

    int GetFieldNmInt(const uint64& RecId, const TStr& FieldNm) const;
    void GetFieldNmIntV(const uint64& RecId, const TStr& FieldNm, TIntV& IntV) const;
    uint64 GetFieldNmUInt64(const uint64& RecId, const TStr& FieldNm) const;
	TStr GetFieldNmStr(const uint64& RecId, const TStr& FieldNm) const;
	void GetFieldNmStrV(const uint64& RecId, const TStr& FieldNm, TStrV& StrV) const;
	bool GetFieldNmBool(const uint64& RecId, const TStr& FieldNm) const;
    double GetFieldNmFlt(const uint64& RecId, const TStr& FieldNm) const;
    TFltPr GetFieldNmFltPr(const uint64& RecId, const TStr& FieldNm) const;
    void GetFieldNmFltV(const uint64& RecId, const TStr& FieldNm, TFltV& FltV) const;
    void GetFieldNmTm(const uint64& RecId, const TStr& FieldNm, TTm& Tm) const;
    void GetFieldNmNumSpV(const uint64& RecId, const TStr& FieldNm, TIntFltKdV& SpV) const;
    void GetFieldNmBowSpV(const uint64& RecId, const TStr& FieldNm, PBowSpV& SpV) const;

	virtual TStr GetDisplayText(const uint64& RecId, const int& FieldId) const;
	virtual TFltPr GetDisplayMap(const uint64& RecId, const int& FieldId) const;
	virtual TStr GetDisplayText(const uint64& RecId, const TStr& FieldNm) const;
	virtual TFltPr GetDisplayMap(const uint64& RecId, const TStr& FieldNm) const;

	void PrintRecSet(const POgBase& OgBase, const POgRecSet& RecSet, TSOut& SOut) const;
	void PrintRecSet(const POgBase& OgBase, const POgRecSet& RecSet, const TStr& FNm) const;
    void PrintAll(const POgBase& OgBase, TSOut& SOut) const;
    void PrintAll(const POgBase& OgBase, const TStr& FNm) const;
    void PrintTypes(const POgBase& OgBase, TSOut& SOut) const;
    void PrintTypes(const POgBase& OgBase, const TStr& FNm) const;
};
typedef THash<TUCh, POgStore> TUChOgStoreH;

class TOgRec {
private:
    TUCh StoreId;
  // when passed by reference
	TBool ByRefP;
	TUInt64 RecId;
  // when passed by value
    // field values
    THash<TInt, TInt> FieldIdIntH;
    THash<TInt, TIntV> FieldIdIntVH;
    THash<TInt, TUInt64> FieldIdUInt64H;
    THash<TInt, TStr> FieldIdStrH;
    THash<TInt, TStrV> FieldIdStrVH;
    THash<TInt, TBool> FieldIdBoolH;
    THash<TInt, TFlt> FieldIdFltH;
    THash<TInt, TFltV> FieldIdFltVH;
    THash<TInt, TFltPr> FieldIdFltPrH;
    THash<TInt, TTm> FieldIdTmH;
    THash<TInt, TIntFltKdV> FieldIdNumSpVH;
    THash<TInt, PBowSpV> FieldIdBowSpVH;

private:
    void FieldError(const int& FieldId, const TStr& TypeStr) const;

public:
    TOgRec(): StoreId(TUCh::Mx), ByRefP(false), RecId(TUInt64::Mx) { }
    TOgRec(const uchar& _StoreId): StoreId(_StoreId), ByRefP(false), RecId(TUInt64::Mx) { }
    TOgRec(const uchar& _StoreId, const uint64& _RecId): StoreId(_StoreId), ByRefP(true), RecId(_RecId) { }

    bool IsDef() const { return ((StoreId.Val != TUCh::Mx) && !ByRefP) || (ByRefP && (RecId != TUInt64::Mx)); }
	uchar GetStoreId() const { return StoreId; }
    bool IsByRef() const { return ByRefP; }
    bool IsByVal() const { return !ByRefP; }
    uint64 GetRecId() const { return RecId; }

    int GetFieldInt(const int& FieldId) const;
    void GetFieldIntV(const int& FieldId, TIntV& IntV) const;
    uint64 GetFieldUInt64(const int& FieldId) const;
	TStr GetFieldStr(const int& FieldId) const;
	void GetFieldStrV(const int& FieldId, TStrV& StrV) const;
	bool GetFieldBool(const int& FieldId) const;
    double GetFieldFlt(const int& FieldId) const;
    TFltPr GetFieldFltPr(const int& FieldId) const;
    void GetFieldFltV(const int& FieldId, TFltV& FltV) const;
    void GetFieldTm(const int& FieldId, TTm& Tm) const;
    void GetFieldNumSpV(const int& FieldId, TIntFltKdV& NumSpV) const;
    void GetFieldBowSpV(const int& FieldId, PBowSpV& BowSpV) const;
	TStr GetDisplayText(const POgStore& Store, const int& FieldId) const;
	TFltPr GetDisplayMap(const POgStore& Store, const int& FieldId) const;
    
    void AddFieldInt(const int& FieldId, const int& Int) { FieldIdIntH.AddDat(FieldId, Int); }
    void AddFieldIntV(const int& FieldId, const TIntV& IntV) { FieldIdIntVH.AddDat(FieldId, IntV); }
    void AddFieldUInt64(const int& FieldId, const uint64& UInt64) { FieldIdUInt64H.AddDat(FieldId, UInt64); }
	void AddFieldStr(const int& FieldId, const TStr& Str) { FieldIdStrH.AddDat(FieldId, Str); }
	void AddFieldStrV(const int& FieldId, const TStrV& StrV) { FieldIdStrVH.AddDat(FieldId, StrV); }
	void AddFieldBool(const int& FieldId, const bool& Bool) { FieldIdBoolH.AddDat(FieldId, Bool); }
    void AddFieldFlt(const int& FieldId, const double& Flt) { FieldIdFltH.AddDat(FieldId, Flt); }
    void AddFieldFltV(const int& FieldId, const TFltV& FltV) { FieldIdFltVH.AddDat(FieldId, FltV); }
    void AddFieldFltPr(const int& FieldId, const TFltPr& FltPr) { FieldIdFltPrH.AddDat(FieldId, FltPr); }
    void AddFieldTm(const int& FieldId, const TTm& Tm) { FieldIdTmH.AddDat(FieldId, Tm); }
    void AddFieldNumSpV(const int& FieldId, const TIntFltKdV& NumSpV) { FieldIdNumSpVH.AddDat(FieldId, NumSpV); }
    void AddFieldBowSpV(const int& FieldId, const PBowSpV& BowSpV) { FieldIdBowSpVH.AddDat(FieldId, BowSpV); }

    POgRecSet ToRecSet() const;
	POgRecSet DoJoin(const POgBase& OgBase, const int& JoinId) const;
	POgRecSet DoJoin(const POgBase& OgBase, const TStr& JoinNm) const;
	POgRecSet DoJoin(const POgBase& OgBase, const TIntPrV& JoinIdV) const;
	POgRecSet DoJoin(const POgBase& OgBase, const TOgJoinSeq& JoinSeq) const;
	TOgRec DoSingleJoin(const POgBase& OgBase, const int& JoinId) const;
	TOgRec DoSingleJoin(const POgBase& OgBase, const TStr& JoinNm) const;
	TOgRec DoSingleJoin(const POgBase& OgBase, const TIntPrV& JoinIdV) const;
	TOgRec DoSingleJoin(const POgBase& OgBase, const TOgJoinSeq& JoinSeq) const;

	PXmlTok GetXmlTok(const POgBase& OgBase, const bool& FieldsP, const bool& StoreInfoP = true) const;
	PXmlDoc SaveXml(const POgBase& OgBase, const bool& FieldsP = true, const bool& StoreInfoP = true) const;
	PJsonVal SaveJson(const POgBase& OgBase, const bool& FieldsP = true, const bool& StoreInfoP = true) const;
};

ClassTPV(TOgRecSet, POgRecSet, TOgRecSetV)//{
private:
	typedef TVec<TPair<TInt, POgStore> > TIntOgStorePrV;
private:
	TUCh StoreId;
    TBool WgtP;
	TUInt64IntKdV RecIdFqV;
    TVec<POgAggr> AggrV;

private:
	void GetSampleRecIdV(const int& SampleSize, 
		const bool& SortedP, TUInt64IntKdV& SampleRecIdFqV) const;
	void LimitToSampleRecIdV(const TUInt64IntKdV& SampleRecIdFqV);

	TOgRecSet() { }
    TOgRecSet(const uchar& _StoreId, const TUInt64V& _RecIdV);
	TOgRecSet(const uchar& _StoreId, const POgStore& OgStore);
	TOgRecSet(const uchar& _StoreId, const TUInt64IntKdV& _RecIdFqV, const bool& _WgtP):
		StoreId(_StoreId), WgtP(_WgtP), RecIdFqV(_RecIdFqV) { }
	TOgRecSet(TSIn& SIn);

public:
	static POgRecSet New() { 
		return new TOgRecSet(); }
	static POgRecSet New(const uchar& StoreId) { 
		return new TOgRecSet(StoreId, TUInt64V()); }
	static POgRecSet New(const uchar& StoreId, const uint64& RecId) {
		return new TOgRecSet(StoreId, TUInt64V::GetV(RecId)); }
	static POgRecSet New(const uchar& StoreId, const TUInt64V& RecIdV) {
		return new TOgRecSet(StoreId, RecIdV); }
	static POgRecSet New(const uchar& StoreId, const POgStore& OgStore) {
		return new TOgRecSet(StoreId, OgStore); }
	static POgRecSet New(const uchar& StoreId, const TUInt64IntKdV& RecIdFqV,
		const bool& WgtP) { return new TOgRecSet(StoreId, RecIdFqV, WgtP); }

	static POgRecSet Load(TSIn& SIn){ return new TOgRecSet(SIn); }
	void Save(TSOut& SOut);

    bool IsWgt() const { return WgtP; }
	bool Empty() const { return RecIdFqV.Empty(); }
	uchar GetStoreId() const { return StoreId; }

	int GetRecs() const { return RecIdFqV.Len(); }
    TOgRec GetRec(const int& RecN) const { return TOgRec(StoreId, RecIdFqV[RecN].Key); }
	uint64 GetRecId(const int& RecN) const { return RecIdFqV[RecN].Key; }
	int GetRecFq(const int& RecN) const { return WgtP ? RecIdFqV[RecN].Dat.Val : 1; }
	uint64 GetLastRecId() const { return RecIdFqV.Last().Key; }
    const TUInt64IntKdV& GetRecIdFqV() const { return RecIdFqV; }

    void GetRecIdV(TUInt64V& RecIdV) const;
	void GetRecIdSet(TUInt64Set& RecIdSet) const;
	void GetRecIdFqH(THash<TUInt64, TInt>& RecIdFqH) const;

	void PutRecFq(const int& RecN, const int& Fq) { RecIdFqV[RecN].Dat = Fq; }
	void PutAllRecFq(const THash<TUInt64, TInt>& RecIdFqH);
    void DelLastRec() { RecIdFqV.DelLast(); }
    void Shuffle(TRnd& Rnd) { RecIdFqV.Shuffle(Rnd); }
	void Reverse() { RecIdFqV.Reverse(); }
	void Trunc(const int& Recs) { RecIdFqV.Trunc(Recs); }
	void SortById(const bool& Asc = true) { if (!RecIdFqV.IsSorted(Asc)) { RecIdFqV.Sort(Asc); } }
	void SortByFq(const bool& Asc = true);
	void SortByField(const POgBase& OgBase, const bool& Asc, const int& SortFieldId);

	void FilterByRecId(const uint64& MinRecId, const uint64& MaxRecId);
	void FilterByFq(const int& MinFq, const int& MaxFq);
	void FilterByIntField(const POgBase& OgBase, const int& FieldId, const int& MinVal, const int& MaxVal);
	void FilterByTmField(const POgBase& OgBase, const int& FieldId, const uint64& MinVal, const uint64& MaxVal);
	void FilterByTmField(const POgBase& OgBase, const int& FieldId, const TTm& MinVal, const TTm& MaxVal);
	void RemoveRecId(const TUInt64& RecId);
	void RemoveRecIdSet(THashSet<TUInt64>& RemoveItemIdH);
	
    POgRecSet Clone() const;
	POgRecSet GetSampleRecSet(const int& SampleSize, const bool& SortedP) const;
	POgRecSet GetLimit(const int& Limit, const int& Offset) const;

	POgRecSet GetMerge(const POgRecSet& RecSet) const;
	void Merge(const POgRecSet& RecSet);
	void Merge(const TVec<POgRecSet>& RecSetV);
	POgRecSet GetIntersect(const POgRecSet& RecSet);

	POgRecSet DoJoin(const POgBase& OgBase, const int& JoinId, 
		const int& SampleSize = -1, const bool& SortedP = false) const;
	POgRecSet DoJoin(const POgBase& OgBase, const TStr& JoinNm, 
		const int& SampleSize = -1, const bool& SortedP = false) const;
	POgRecSet DoJoin(const POgBase& OgBase, const TIntPrV& JoinIdV, const bool& SortedP) const;
	POgRecSet DoJoin(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const bool& SortedP) const;

	int GetAggrs() const { return AggrV.Len(); }
	const POgAggr& GetAggr(const int& AggrN) const { return AggrV[AggrN]; }
	void AddAggr(const POgAggr& Aggr) { AggrV.Add(Aggr); }

	static TStr GetJoinPathStr(const POgBase& OgBase, 
		const POgStore& StartStore, const TIntPrV& JoinIdV);

	void Print(const POgBase& OgBase, TSOut& SOut);
	void Print(const POgBase& OgBase, TStr& FNm);

	PXmlDoc SaveXml(const POgBase& OgBase, const int& _MxHits = -1, 
		const int& Offset = 0, const bool& FieldsP = false) const;
	PJsonVal SaveJson(const POgBase& OgBase, const int& _MxHits = -1, 
		const int& Offset = 0, const bool& FieldsP = false) const;
};

typedef TIntUInt64Pr TOgKeyWord;
typedef TIntUInt64PrV TOgKeyWordV;

typedef enum { oikstUndef, oikstByStr, oikstById, oikstByFlt } TOgIndexKeySortType;

class TOgIndexKey {
private:
	TUCh StoreId;
	TInt KeyId;
	TStr KeyNm;
	TInt WordVocId;
	TBool TextP;
	TBool AggrP;
	TBool InternalP;
	TOgIndexKeySortType SortType;
	TIntV FieldIdV;
	TStr JoinNm;

public:
	TOgIndexKey(): StoreId(TUCh::Mx), KeyId(-1), KeyNm(""), 
		WordVocId(-1), SortType(oikstUndef), InternalP(true) { }
	TOgIndexKey(const uchar& _StoreId, const TStr& _KeyNm, const TStr& _JoinNm): StoreId(_StoreId), KeyNm(_KeyNm), 
		WordVocId(-1), SortType(oikstUndef), InternalP(true), JoinNm(_JoinNm) { TOg::AssertValidNm(KeyNm); }
	TOgIndexKey(const uchar& _StoreId, const TStr& _KeyNm, const int& _WordVocId, 
		const bool& _TextP, const bool& _AggrP, const TOgIndexKeySortType& _SortType);
	
	TOgIndexKey(TSIn& SIn);
	void Save(TSOut& SOut) const;

	bool IsDef() const { return !KeyNm.Empty(); }
	uchar GetStoreId() const { return StoreId; }
	int GetKeyId() const { return KeyId; }
	TStr GetKeyNm() const { return KeyNm; }
	void PutKeyId(const int& _KeyId) { KeyId = _KeyId; }

	bool IsWordVoc() const { return WordVocId != -1; }
	int GetWordVocId() const { return WordVocId; }
	bool IsText() const { return TextP; }
	bool IsAggr() const { return AggrP; }
	bool IsInternal() const { return InternalP; }

	TOgIndexKeySortType GetSortType() const { return SortType; }
	bool IsSort() const { return SortType != oikstUndef; }
	bool IsSortByStr() const { return SortType == oikstByStr; }
	bool IsSortByFlt() const { return SortType == oikstByFlt; }
	bool IsSortById() const { return SortType == oikstById; }

	void AddField(const int& FieldId) { FieldIdV.Add(FieldId); }
	bool IsFields() const { return !FieldIdV.Empty(); }
	int GetFields() const { return FieldIdV.Len(); }
	int GetFieldId(const int& FieldIdN) const { return FieldIdV[FieldIdN]; }

	const TStr& GetJoinNm() const { return JoinNm; }
};

ClassTPV(TOgIndexWordVoc, POgIndexWordVoc, TOgIndexWordVocV)//{
private:
	TUInt64 Recs; 
	TStrHash<TInt> WordH;

public:
	TOgIndexWordVoc() { }
	static POgIndexWordVoc New() { return new TOgIndexWordVoc; }
	TOgIndexWordVoc(TSIn& SIn): WordH(SIn) { }
	static POgIndexWordVoc Load(TSIn& SIn) { return new TOgIndexWordVoc(SIn); }
	
	void Save(TSOut& SOut) { WordH.Save(SOut); }

	bool IsWordStr(const TStr& WordStr) const { return WordH.IsKey(WordStr); }
	uint64 GetWords() const { return (uint64)WordH.Len(); }
	uint64 GetWordId(const TStr& WordStr) const { return (uint64)WordH.GetKeyId(WordStr); }
	TStr GetWordStr(const uint64& WordId) const { return WordH.GetKey((int)WordId); }
	uint64 GetWordFq(const uint64& WordId) const { return WordH[(int)WordId]; }
	void GetAllWordV(TStrV& WordStrV) const { WordH.GetKeyV(WordStrV); }
	void GetAllWordFqV(TStrIntPrV& WordStrFqV) const { WordH.GetKeyDatPrV(WordStrFqV); }
	void GetAllGreaterById(const uint64& StartWordId, TUInt64V& AllGreaterV);
	void GetAllGreaterByStr(const uint64& StartWordId, TUInt64V& AllGreaterV);
	void GetAllGreaterByFlt(const uint64& StartWordId, TUInt64V& AllGreaterV);
	void GetAllLessById(const uint64& StartWordId, TUInt64V& AllLessV);
	void GetAllLessByStr(const uint64& StartWordId, TUInt64V& AllLessV);
	void GetAllLessByFlt(const uint64& StartWordId, TUInt64V& AllLessV);
	void IncRecs() { Recs++; }
	uint64 AddWordStr(const TStr& WordStr);
};

ClassTP(TOgIndexVoc, POgIndexVoc)//{
private:
	PTokenizer Tokenizer;
    // keys
    THash<TUChStrPr, TOgIndexKey> KeyH;
    // store keys
    THash<TUCh, TIntSet> StoreIdKeyIdSetH;
	// word vocabularies
    TOgIndexWordVocV WordVocV;
	// when returning empty set
	TIntSet EmptySet;

	// access to word vocabulary
	POgIndexWordVoc& GetWordVoc(const int& KeyId);
	const POgIndexWordVoc& GetWordVoc(const int& KeyId) const;

public:
	TOgIndexVoc(): Tokenizer(TTokenizerHtml::New()) { }
    static POgIndexVoc New() { return new TOgIndexVoc; }
    TOgIndexVoc(TSIn& SIn);
    static POgIndexVoc Load(TSIn& SIn) { return new TOgIndexVoc(SIn); }
    void Save(TSOut& SOut) const;

	int GetKeys() const { return KeyH.Len(); }
	bool IsKeyId(const int& KeyId) const;
	bool IsKeyNm(const uchar& StoreId, const TStr& KeyNm) const;
	int GetKeyId(const uchar& StoreId, const TStr& KeyNm) const;
	uchar GetKeyStoreId(const int& KeyId) const;
	TStr GetKeyNm(const int& KeyId) const;
	const TOgIndexKey& GetKey(const int& KeyId) const;
	const TOgIndexKey& GetKey(const uchar& StoreId, const TStr& KeyNm) const;
	int NewWordVoc() { return WordVocV.Add(TOgIndexWordVoc::New()); }
	int AddKey(const uchar& StoreId, const TStr& KeyNm, const int& WordVocId, 
		const bool& TextP = false, const bool& AggrP = false, 
		const TOgIndexKeySortType& SortType = oikstUndef);
	int AddInternalKey(const uchar& StoreId, const TStr& KeyNm, const TStr& JoinNm);
	void AddKeyField(const int& KeyId, const uchar& StoreId, const int& FieldId);
    bool IsStoreKeys(const uchar& StoreId) const;
    const TIntSet& GetStoreKeys(const uchar& StoreId) const;

	bool IsWordVoc(const int& KeyId) const;
	bool IsWordStr(const int& KeyId, const TStr& WordStr) const;
	uint64 GetWords(const int& KeyId) const;
	void GetAllWordStrV(const int& KeyId, TStrV& WordStrV) const;
	void GetAllWordStrFqV(const int& KeyId, TStrIntPrV& WordStrV) const;
	TStr GetWordStr(const int& KeyId, const uint64& WordId) const;
	uint64 GetWordFq(const int& KeyId, const uint64& WordId) const;
    uint64 GetWordId(const int& KeyId, const TStr& WordStr) const;
	void GetWordIdV(const int& KeyId, const TStr& TextStr, TUInt64V& WordIdV) const;
    uint64 AddWordStr(const int& KeyId, const TStr& WordStr);
	void AddWordIdV(const int& KeyId, const TStr& TextStr, TUInt64V& WordIdV);
	PTokenizer GetTokenizer() const { return Tokenizer; }
	void PutTokenizer(const PTokenizer& _Tokenizer) { Tokenizer = _Tokenizer; }
    void GetAllGreaterV(const int& KeyId, const uint64& StartWordId, TOgKeyWordV& AllGreaterV);
    void GetAllLessV(const int& KeyId, const uint64& StartWordId, TOgKeyWordV& AllLessV);

	void SaveTxt(const POgBase& OgBase, const TStr& FNm) const;
};

typedef enum { 
	oqitUndef, 
	oqitLeaf, 
	oqitAnd, 
	oqitOr, 
	oqitNot,
	oqitJoin,
	oqitRecSet
} TOgQueryItemType;

typedef enum { 
	oqctUndef, 
	oqctEqual, 
	oqctGreater, 
	oqctLess, 
	oqctNotEqual 
} TOgQueryCmpType;

ClassTV(TOgQueryItem, TOgQueryItemV)//{
private:
	TOgQueryItemType Type;
    TInt KeyId;
    TUInt64V WordIdV;
    TOgQueryCmpType CmpType;
	TOgQueryItemV ItemV;
	TInt JoinId;
	TInt SampleSize;
	POgRecSet RecSet;
	
	void ParseWordStr(const TStr& WordStr, const POgIndexVoc& IndexVoc);

	POgStore ParseJoins(const POgBase& OgBase, const PJsonVal& JsonVal);
	POgStore ParseJoin(const POgBase& OgBase, const PJsonVal& JsonVal);
	POgStore ParseFrom(const POgBase& OgBase, const PJsonVal& JsonVal);
	void ParseKeys(const POgBase& OgBase, const POgStore& Store, 
		const PJsonVal& JsonVal, const bool& IgnoreOrP);
	TOgQueryItem(const POgBase& OgBase, const PJsonVal& JsonVal);
	TOgQueryItem(const POgBase& OgBase, const POgStore& Store, const PJsonVal& JsonVal);
	TOgQueryItem(const POgBase& OgBase, const POgStore& Store, 
		const TStr& KeyNm, const PJsonVal& KeyVal);

public:
	TOgQueryItem() { };
    TOgQueryItem(const POgBase& OgBase, const int& _KeyId, const uint64& WordId, 
		const TOgQueryCmpType& _CmpType = oqctEqual);
    TOgQueryItem(const POgBase& OgBase, const int& _KeyId, const TStr& WordStr, 
		const TOgQueryCmpType& _CmpType = oqctEqual);
    TOgQueryItem(const POgBase& OgBase, const uchar& StoreId, const TStr& KeyNm, 
		const TStr& WordStr, const TOgQueryCmpType& _CmpType = oqctEqual);
    TOgQueryItem(const POgBase& OgBase, const TStr& StoreNm, const TStr& KeyNm, 
		const TStr& WordStr, const TOgQueryCmpType& _CmpType = oqctEqual);
	TOgQueryItem(const TOgQueryItemType& _Type);
	TOgQueryItem(const TOgQueryItemType& _Type, const TOgQueryItem& Item);
	TOgQueryItem(const TOgQueryItemType& _Type, const TOgQueryItem& Item1, const TOgQueryItem& Item2);
	TOgQueryItem(const TOgQueryItemType& _Type, const TOgQueryItemV& _ItemV);
	TOgQueryItem(const int& _JoinId, const int& _SampleSize, const TOgQueryItem& Item);
	TOgQueryItem(const POgBase& OgBase, const TStr& JoinNm, const int& _SampleSize, 
		const TOgQueryItem& Item);
    TOgQueryItem(const uchar& StoreId, const uint64& RecId);
	TOgQueryItem(const TOgRec& Rec);
	TOgQueryItem(const POgRecSet& _RecSet);

	bool IsDef() const { return Type != oqitUndef; }
	TOgQueryItemType GetType() const { return Type; }
	bool IsLeaf() const { return (Type == oqitLeaf); }
	bool IsAnd() const { return (Type == oqitAnd); }
	bool IsOr() const { return (Type == oqitOr); }
	bool IsNot() const { return (Type == oqitNot); }
	bool IsJoin() const { return (Type == oqitJoin); }
	bool IsRecSet() const { return (Type == oqitRecSet); }

	uchar GetStoreId(const POgBase& OgBase) const;
	bool Empty() const { return !IsItems() && !IsWordIds(); }
	bool IsWgt() const;

	bool IsWordIds() const { return !WordIdV.Empty(); }
	int GetKeyId() const { return KeyId; }
    uint64 GetWordId() const { return WordIdV[0]; }
    void GetKeyWordV(TOgKeyWordV& KeyWordPrV) const;
	TOgQueryCmpType GetCmpType() const { return CmpType; }
	bool IsEqual() const { return (CmpType == oqctEqual); }
    bool IsGreater() const { return (CmpType == oqctGreater); }
    bool IsLess() const { return (CmpType == oqctLess); }
    bool IsNotEqual() const { return (CmpType == oqctNotEqual); }

	bool IsItems() const { return !ItemV.Empty(); }
	int GetItems() const { return ItemV.Len(); }
	const TOgQueryItem& GetItem(const int& ItemN) const { return ItemV[ItemN]; }
	int GetJoinId() const { return JoinId; }
	int GetSampleSize() const { return SampleSize; }
	const POgRecSet& GetRecSet() const { return RecSet; }

	friend class TOgQuery;
};

ClassTP(TOgQuery, POgQuery)//{
private:
	TOgQueryItem QueryItem;

	TInt SortFieldId;
	TBool SortAscP;
	TInt Limit;
	TInt Offset;

	TOgQuery(const POgBase& OgBase, const TOgQueryItem& ItemTree, const int& _SortFieldId, 
		const bool& _SortAscP, const int& _Limit, const int& _Offset);
public:
	static POgQuery New(const POgBase& OgBase, const TOgQueryItem& QueryItem, 
		const int& SortFieldId = -1, const bool& SortAscP = true, const int& Limit = -1, 
		const int& Offset = 0);
	static POgQuery New(const POgBase& OgBase, const PJsonVal& JsonVal);
	static POgQuery New(const POgBase& OgBase, const TStr& TStr);

	bool IsDef() const { return QueryItem.IsDef(); }
	bool Empty() const { return QueryItem.Empty(); }
	bool IsWgt() const { return QueryItem.IsLeaf(); }
	POgStore GetStore(const POgBase& OgBase);
	bool IsSort() const { return SortFieldId != -1; }
	void Sort(const POgBase& OgBase, const POgRecSet& RecSet);
	bool IsLimit() const { return (Limit != -1) || (Offset != 0); }
	POgRecSet GetLimit(const POgRecSet& RecSet);
	
	bool IsOk(const POgBase& OgBase, TStr& MsgStr) const;

	const TOgQueryItem& GetQueryItem() const { return QueryItem; }
};

ClassTP(TOgIndex, POgIndex)//{
public:
	typedef TOgKeyWord TOgGixKey; // (KeyId, WordId)
	typedef TKeyDat<TUInt64, TInt> TOgGixItem; // [RecId, Freq]
	typedef TVec<TOgGixItem> TOgGixItemV;
	typedef TPt<TGixMerger<TOgGixKey, TOgGixItem> > POgGixMerger;
	typedef TPt<TGixKeyStr<TOgGixKey> > POgGixKeyStr;
	typedef TGixItemSet<TOgGixKey, TOgGixItem> TOgGixItemSet;
	typedef TPt<TOgGixItemSet> POgGixItemSet;
	typedef TGix<TOgGixKey, TOgGixItem> TOgGix;
	typedef TPt<TOgGix> POgGix;
	typedef TGixExpItem<TOgGixKey, TOgGixItem> TOgGixExpItem;
	typedef TPt<TOgGixExpItem> POgGixExpItem;

    // mergerer which sums up the frequencies
	class TOgGixDefMerger : public TGixMerger<TOgGixKey, TOgGixItem> {
	public:
		static PGixMerger New() { return new TOgGixDefMerger(); }

		void Union(TOgGixItemV& MainV, const TOgGixItemV& JoinV) const;
		void Intrs(TOgGixItemV& MainV, const TOgGixItemV& JoinV) const;
		void Minus(const TOgGixItemV& MainV, const TOgGixItemV& JoinV, TOgGixItemV& ResV) const;
		void Merge(TOgGixItemV& ItemV) const;
		void Def(const TOgGixKey& Key, TOgGixItemV& MainV) const  { }
	};

	// merger which sums the frequencies but removes the duplicates (e.g. 3+1 = 1+1 = 2)
	class TOgGixRmDupMerger : public TOgGixDefMerger {
	public:
		static PGixMerger New() { return new TOgGixRmDupMerger(); }

		void Union(TOgGixItemV& MainV, const TOgGixItemV& JoinV) const;
		void Intrs(TOgGixItemV& MainV, const TOgGixItemV& JoinV) const;
	};

	// giving pretty names to gix keys
	class TOgGixKeyStr : public TGixKeyStr<TOgGixKey> {
	private:
		POgBase OgBase;
		POgIndexVoc IndexVoc;

		TOgGixKeyStr(const POgBase& _OgBase, const POgIndexVoc& _IndexVoc);
	public:
		static POgGixKeyStr New(const POgBase& OgBase, const POgIndexVoc& IndexVoc) {
			return new TOgGixKeyStr(OgBase, IndexVoc); }

		TStr GetKeyNm(const TOgGixKey& Key) const;
	};
	
private:
    TStr IndexFPath;
    TFAccess Access;
    mutable POgGix Gix;
    POgIndexVoc IndexVoc;
	POgGixMerger DefMerger;

	POgGixExpItem ToExpItem(const TOgQueryItem& QueryItem) const;
    bool DoQuery(const POgGixExpItem& ExpItem, const POgGixMerger& Merger, 
		TOgGixItemV& RecIdFqV) const;

public:
    TOgIndex(const TStr& _IndexFPath, const TFAccess& _Access, 
        const POgIndexVoc& IndexVoc, const int64& CacheSize);
    static POgIndex New(const TStr& IndexFPath, const TFAccess& Access, 
        const POgIndexVoc& IndexVoc, const int64& CacheSize) {
            return new TOgIndex(IndexFPath, Access, IndexVoc, CacheSize); }
	static bool Exists(const TStr& IndexFPath) {
		return TFile::Exists(IndexFPath + "Index.Gix"); }
    
	~TOgIndex();

	TStr GetIndexFPath() const { return IndexFPath; } 
    POgIndexVoc GetIndexVoc() const { return IndexVoc; }
	POgGixMerger GetDefMerger() const { return DefMerger; }

    void Index(const int& KeyId, const TStr& WordStr, const uint64& RecId);
    void Index(const int& KeyId, const TStrV& WordStrV, const uint64& RecId);
	void Index(const int& KeyId, const TStrIntPrV& WordStrFqV, const uint64& RecId);
    void Index(const uchar& StoreId, const TStr& KeyNm, const TStr& WordStr, const uint64& RecId);
    void Index(const uchar& StoreId, const TStr& KeyNm, const TStrV& WordStrV, const uint64& RecId);
	void Index(const uchar& StoreId, const TStr& KeyNm, const TStrIntPrV& WordStrFqV, const uint64& RecId);
	void Index(const uchar& StoreId, const TStrPrV& KeyWordV, const uint64& RecId);
	void IndexText(const int& KeyId, const TStr& TextStr, const uint64& RecId);
	void IndexText(const uchar& StoreId, const TStr& KeyNm, const TStr& TextStr, const uint64& RecId);
	void IndexJoin(const POgStore& Store, const int& JoinId, 
		const uint64& RecId, const uint64& JoinRecId, const int& JoinFq = 1);
	void IndexJoin(const POgStore& Store, const TStr& JoinNm, 
		const uint64& RecId, const uint64& JoinRecId, const int& JoinFq = 1);
    void Index(const int& KeyId, const uint64& WordId, const uint64& RecId, const int& RecFq);

	void Delete(const uchar& StoreId, const TStr& KeyNm, const TStr& WordStr, const uint64& RecId);
	void Delete(const uchar& StoreId, const TStr& KeyNm, const uint64& WordId, const uint64& RecId);
	void Delete(const int& KeyId, const TStr& WordStr, const uint64& RecId);
	void DeleteText(const int& KeyId, const TStr& TextStr, const uint64& RecId);
	void DeleteText(const uchar& StoreId, const TStr& KeyNm, const TStr& TextStr, const uint64& RecId);
	void DeleteJoin(const POgStore& Store, const int& JoinId, 
		const uint64& RecId, const uint64& JoinRecId);
	void DeleteJoin(const POgStore& Store, const TStr& JoinNm, 
		const uint64& RecId, const uint64& JoinRecId);
	void Delete(const int& KeyId, const uint64& WordId, const uint64& RecId);

    bool IsCacheFull() const { return Gix->IsCacheFull(); }
    void MergeIndex(const POgIndex& TmpIndex);

	void SearchAnd(const TIntUInt64PrV& KeyWordV, TUInt64IntKdV& StoreRecIdFqV) const;
	void SearchOr(const TIntUInt64PrV& KeyWordV, TUInt64IntKdV& StoreRecIdFqV) const;
	TPair<TBool, POgRecSet> Search(const POgBase& OgBase, 
		const TOgQueryItem& QueryItem, const POgGixMerger& Merger) const;

	void SaveTxt(const POgBase& OgBase, const TStr& FNm);
};

ClassTP(TOgTempIndex, POgTempIndex)//{
private:
	// for creating temproary indices
	int64 IndexCacheSize;
	TStr TempFPath;
	TStrQ TempIndexFPathQ;
	POgIndex TempIndex;

	UndefDefaultCopyAssign(TOgTempIndex);
	TOgTempIndex(const TStr& _TempFPath, const int64& _IndexCacheSize): 
		 IndexCacheSize(_IndexCacheSize), TempFPath(_TempFPath) { }
public:
	static POgTempIndex New(const TStr& TempFPath, const int64& IndexCacheSize) { 
		return new TOgTempIndex(TempFPath, IndexCacheSize); }

	bool IsIndexFull() const { return TempIndex->IsCacheFull(); }
	const POgIndex& GetIndex() const { return TempIndex; }
	void NewIndex(const POgIndexVoc& IndexVoc);
	void Merge(const POgIndex& Index);
};

ClassTP(TOgOp, POgOp)//{
private:
	TStr OpNm;
protected:
	bool IsFldNm(const TStrKdV& FldNmValPrV, const TStr& FldNm);
	TStr GetFldVal(const TStrKdV& FldNmValPrV, const TStr& FldNm, const TStr& DefFldVal = "");
	void GetFldValV(const TStrKdV& FldNmValPrV, const TStr& FldNm, TStrV& FldValV);
	void GetFldValSet(const TStrKdV& FldNmValPrV, const TStr& FldNm, TStrSet& FldValSet);
	bool IsFldNmVal(const TStrKdV& FldNmValPrV,	const TStr& FldNm, const TStr& FldVal);

	TOgOp(const TStr& _OpNm): OpNm(_OpNm) { }
public:
	virtual ~TOgOp() { }

	TStr GetOpNm() const { return OpNm; }
	virtual void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV) = 0;
	virtual bool IsFunctional() = 0;
	POgRecSet Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV);
};

ClassTP(TOgAggr, POgAggr)//{
public:
	virtual ~TOgAggr() { }

	virtual PXmlTok SaveXml() const = 0;
	virtual PJsonVal SaveJson() const = 0;
};

ClassTP(TOgBase, POgBase)//{
private:
    POgIndexVoc IndexVoc;
    POgIndex Index;
	POgTempIndex TempIndex;
	TVec<POgStore> StoreV;
    THash<TStr, POgStore> StoreH;
	THash<TStr, POgOp> OpH;

private:
    TOgBase();
	~TOgBase() {
		printf("OgBase close\n");
	}

	// searching
	POgRecSet Invert(const POgRecSet& RecSet, const TOgIndex::POgGixMerger& Merger);
	TPair<TBool, POgRecSet> Search(const TOgQueryItem& QueryItem, const TOgIndex::POgGixMerger& Merger);

public:
	static POgBase New() { return new TOgBase; }

    void PutIndex(const POgIndex& NewIndex);
    void AddStore(const POgStore& NewStore);
	void AddOp(const POgOp& NewOp);
	void Init();

    const POgIndexVoc& GetIndexVoc() const { return IndexVoc; }
	const POgIndex& GetIndex() const;
    int GetStores() const { return StoreH.Len(); }
    bool IsStoreN(const uchar& StoreN) const { return 0 <= StoreN && StoreN < StoreH.Len(); }
    const POgStore& GetStoreByStoreN(const int& StoreN) const;
    bool IsStoreId(const uchar& StoreId) const { return !StoreV[(int)StoreId].Empty(); }
    const POgStore& GetStoreByStoreId(const uchar& StoreId) const;
    bool IsStoreNm(const TStr& StoreNm) const { return StoreH.IsKey(StoreNm); }
	const POgStore& GetStoreByStoreNm(const TStr& StoreNm) const;
	int GetOps() const { return OpH.Len(); }
	int GetFirstOpId() const { return OpH.FFirstKeyId(); }
	bool GetNextOpId(int& OpId) const { return OpH.FNextKeyId(OpId); }
	const POgOp& GetOp(const int& OpId) const { return OpH[OpId]; }
	bool IsOp(const TStr& OpNm) const { return OpH.IsKey(OpNm); }
	const POgOp& GetOp(const TStr& OpNm) const { return OpH.GetDat(OpNm); }

	int NewIndexWordVoc() { return IndexVoc->NewWordVoc(); }
	int NewIndexKey(const POgStore& Store, const TStr& KeyNm, const bool& TextP = false, 
		const bool& AggrP = false, const TOgIndexKeySortType& SortType = oikstUndef);
	int NewIndexKey(const POgStore& Store, const TStr& KeyNm, const int& WordVocId, 
		const bool& TextP = false, const bool& AggrP = false, 
		const TOgIndexKeySortType& SortType = oikstUndef);
	int NewFieldIndexKey(const POgStore& Store, const TStr& KeyNm, const int& FieldId,
		const bool& TextP = false, const bool& AggrP = false, 
		const TOgIndexKeySortType& SortType = oikstUndef);
	int NewFieldIndexKey(const POgStore& Store, const int& FieldId, const bool& TextP = false, 
		const bool& AggrP = false, const TOgIndexKeySortType& SortType = oikstUndef);
	int NewFieldIndexKey(const POgStore& Store, const TStr& KeyNm, const int& FieldId, 
		const int& WordVocId, const bool& TextP = false, const bool& AggrP = false, 
		const TOgIndexKeySortType& SortType = oikstUndef);
	int NewFieldIndexKey(const POgStore& Store, const int& FieldId, const int& WordVocId, 
		const bool& TextP = false, const bool& AggrP = false,
		const TOgIndexKeySortType& SortType = oikstUndef);

	POgRecSet Search(const POgQuery& Query);
	POgRecSet Search(const TStr& QueryStr);

  	bool IsTempIndex() const { return !TempIndex.Empty(); }
	void InitTempIndex(const TStr& TempFPath, const uint64& IndexCacheSize);
	void MergeTempIndex() { TempIndex->Merge(Index); TempIndex.Clr(); }
	bool IsTempIndexFull() const { return TempIndex->IsIndexFull(); }
	void NewTempIndex() const { TempIndex->NewIndex(IndexVoc); }
	void CheckTempIndexSize() { if (IsTempIndexFull()) { NewTempIndex(); } }

    void PrintStores(const TStr& FNm);
	void PrintIndexVoc(const TStr& FNm);
	void PrintIndex(const TStr& FNm, const bool& SortP);
};

class TOgTimeIndexKeyId {
public:
	TBool FullP;
	TStr PrefixStr;
	int DateKeyId; // date (e.g. 2012-02-24)
	int YearKeyId; // year (e.g. 2012)
	int MonthKeyId; // month (e.g. feb)
	int DayOfMonthKeyId; // day of month (e.g. 24)
	int DayOfWeekKeyId; // day of week (e.g. fri)
	int TimeOfDayKeyId; // time of day (e.g. morning)
	int HourKeyId; // hour (e.g. 8)

private:
	void InitVoc(const POgIndexVoc& IndexVoc);

public:
	TOgTimeIndexKeyId(const TStr& _PrefixStr = TStr(), const bool& _FullP = false): 
	  PrefixStr(_PrefixStr), FullP(_FullP) { }

	void Init(const POgBase& OgBase, const POgStore& Store);
	void Init(const POgBase& OgBase, const POgStore& Store, const int& FieldId);
	void Load(const POgBase& OgBase, const POgStore& Store);
	
	void ParseSecTm(const TSecTm& SecTm, TStrPrV& KeyWordV);
	static void ParseSecTm(const TStr& PrefixStr, const TSecTm& SecTm, 
		TStrPrV& KeyWordV, const bool& FullP = false);
	void IndexSecTm(const POgBase& OgBase, const TSecTm& SecTm,
		const uchar& StoreId, const uint64& RecId);
};

ClassTPV(TOgFtrExt, POgFtrExt, TOgFtrExtV)//{
private:
	THash<TUCh, TOgJoinSeq> JoinSeqH;
	TUCh FtrStoreId;

protected:
	TOgRec DoSingleJoin(const POgBase& OgBase, const TOgRec& FtrRec) const;

	TOgFtrExt(const POgStore& Store);
	TOgFtrExt(const POgBase& OgBase, const TOgJoinSeq& JoinSeq);
	TOgFtrExt(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV);
public:
	virtual ~TOgFtrExt() { }

	virtual TStr GetNm(const POgBase& OgBase) const { return "[undefined]"; };

	virtual void Clr(const POgBase& OgBase) = 0;

	virtual void Update(const POgBase& OgBase, const TOgRec& FtrRec) { };
	virtual void FinishUpdate(const POgBase& OgBase) { };
	virtual void AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const = 0;
	virtual int GetDim(const POgBase& OgBase) const = 0;
	virtual TStr GetFtr(const POgBase& OgBase, const int& FtrN) const = 0;

	virtual void ExtractStrV(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const;
	virtual void ExtractFltV(const POgBase& OgBase, const TOgRec& FtrRec, TFltV& FltV) const;
	virtual void ExtractTmV(const POgBase& OgBase, const TOgRec& FtrRec, TTmV& TmV) const;

	bool IsStartStore(const uchar& StoreId) const { return JoinSeqH.IsKey(StoreId); }
	bool IsJoin(const uchar& StoreId) const { return !JoinSeqH.GetDat(StoreId).IsJoin(); }
	const TOgJoinSeq& GetJoinSeq(const uchar& StoreId) const { return JoinSeqH.GetDat(StoreId); }
	const TIntPrV& GetJoinIdV(const uchar& StoreId) const { return JoinSeqH.GetDat(StoreId).GetJoinIdV(); }
	uchar GetFtrStoreId() const { return FtrStoreId; }

	static POgFtrExt GetFtrExt(const POgBase& OgBase, const POgStore& Store, const int& FieldId, const bool& SetP);
	static POgFtrExt GetFtrExt(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& FieldId, const bool& SetP);
	static POgFtrExt GetFtrExt(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& FieldId, const bool& SetP);
};

ClassTP(TOgFtrSpace, POgFtrSpace)//{
private:
	TBool UpdateFinishedP;
	TInt Dim;
	TIntV DimV;
	TOgFtrExtV FtrExtV;

public:
	TOgFtrSpace(const POgFtrExt& FtrExt): UpdateFinishedP(false) { FtrExtV.Add(FtrExt); }
	TOgFtrSpace(const TOgFtrExtV& _FtrExtV): UpdateFinishedP(false), FtrExtV(_FtrExtV) { }
	static POgFtrSpace New(const POgFtrExt& FtrExt) { return new TOgFtrSpace(FtrExt); }
	static POgFtrSpace New(const TOgFtrExtV& FtrExtV) { return new TOgFtrSpace(FtrExtV); }

	TStr GetNm(const POgBase& OgBase) const;

	void Clr(const POgBase& OgBase);

	void Update(const POgBase& OgBase, const TOgRec& FtrRec);
	void Update(const POgBase& OgBase, const POgRecSet& FtrRecSet, 
		const PNotify& Notify = TNullNotify::New());
	void FinishUpdate(const POgBase& OgBase);

	void GetSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV) const;
	void GetSpVV(const POgBase& OgBase, const POgRecSet& FtrRecSet, 
		TVec<TIntFltKdV>& SpVV, const PNotify& Notify = TNullNotify::New()) const;
	void GetCentroidSpV(const POgBase& OgBase, const POgRecSet& FtrRecSet, 
		TIntFltKdV& CentroidSpV, const bool& NormalizeP = true) const;
	void GetCentroidV(const POgBase& OgBase, const POgRecSet& FtrRecSet, 
		TFltV& CentroidV, const bool& NormalizeP = true) const;

	int GetDim() const;
	TStr GetFtr(const POgBase& OgBase, const int& FtrN) const;
	PBowDocBs MakeBowDocBs();
};

class TOgFtrExtRnd : public TOgFtrExt {
private:
	mutable TRnd Rnd;

public:
	TOgFtrExtRnd(const POgStore& Store, const int& RndSeed): TOgFtrExt(Store), Rnd(RndSeed) { }
	TOgFtrExtRnd(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& RndSeed): 
	  TOgFtrExt(OgBase, JoinSeq), Rnd(RndSeed) { }
	TOgFtrExtRnd(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& RndSeed): 
	  TOgFtrExt(OgBase, JoinSeqV), Rnd(RndSeed) { }

	static POgFtrExt New(const POgStore& Store, const int& RndSeed) {
		return new TOgFtrExtRnd(Store, RndSeed); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& RndSeed) { 
		return new TOgFtrExtRnd(OgBase, JoinSeq, RndSeed); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& RndSeed) { 
		return new TOgFtrExtRnd(OgBase, JoinSeqV, RndSeed); }

	TStr GetNm(const POgBase& OgBase) const { return "Random"; };

	void Clr(const POgBase& OgBase) { };

	void AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const;
	int GetDim(const POgBase& OgBase) const { return 1; }
	TStr GetFtr(const POgBase& OgBase, const int& FtrN) const { return "Random"; }

	void ExtractFltV(const POgBase& OgBase, const TOgRec& FtrRec, TFltV& FltV) const;
};

class TOgFtrExtNum : public TOgFtrExt {
private:
	TFtrGenNumeric FtrGen;
	TInt FieldId;

	double _GetVal(const POgBase& OgBase, const POgRecSet& RecSet) const; 
	double _GetVal(const POgBase& OgBase, const TOgRec& FtrRec) const; 
	double GetVal(const POgBase& OgBase, const TOgRec& FtrRec) const; 

public:
	TOgFtrExtNum(const POgStore& Store, const int& _FieldId, const bool& NormalizeP = true):
	  TOgFtrExt(Store), FtrGen(NormalizeP), FieldId(_FieldId) { }
	TOgFtrExtNum(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& _FieldId,
	  const bool& NormalizeP = true): TOgFtrExt(OgBase, JoinSeq), FtrGen(NormalizeP),
		FieldId(_FieldId) { }
	TOgFtrExtNum(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& _FieldId,
	  const bool& NormalizeP = true): TOgFtrExt(OgBase, JoinSeqV), FtrGen(NormalizeP),
		FieldId(_FieldId) { }

	static POgFtrExt New(const POgStore& Store, const int& FieldId, const bool& NormalizeP = true) {
		return new TOgFtrExtNum(Store, FieldId, NormalizeP); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& FieldId,
		const bool& NormalizeP = true) { return new TOgFtrExtNum(OgBase, JoinSeq, FieldId, NormalizeP); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& FieldId,
		const bool& NormalizeP = true) { return new TOgFtrExtNum(OgBase, JoinSeqV, FieldId, NormalizeP); }

	TStr GetNm(const POgBase& OgBase) const { 
		return OgBase->GetStoreByStoreId(GetFtrStoreId())->GetFieldNm(FieldId); };

	void Clr(const POgBase& OgBase) { FtrGen = TFtrGenNumeric(); }

	void Update(const POgBase& OgBase, const TOgRec& FtrRec);
	void AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const;
	int GetDim(const POgBase& OgBase) const { return 1; }
	TStr GetFtr(const POgBase& OgBase, const int& FtrN) const { return GetNm(OgBase); }

	void ExtractFltV(const POgBase& OgBase, const TOgRec& FtrRec, TFltV& FltV) const;
};

class TOgFtrExtNom : public TOgFtrExt {
private:
	TFtrGenNominal FtrGen;
	TInt FieldId;

	TStr _GetVal(const POgBase& OgBase, const TOgRec& FtrRec) const; 
	TStr GetVal(const POgBase& OgBase, const TOgRec& FtrRec) const; 

public:
	TOgFtrExtNom(const POgStore& Store, const int& _FieldId): TOgFtrExt(Store), FieldId(_FieldId) { }
	TOgFtrExtNom(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& _FieldId):
		TOgFtrExt(OgBase, JoinSeq), FieldId(_FieldId) { }
	TOgFtrExtNom(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& _FieldId):
		TOgFtrExt(OgBase, JoinSeqV), FieldId(_FieldId) { }

	static POgFtrExt New(const POgStore& Store, const int& FieldId) { return new TOgFtrExtNom(Store, FieldId); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& FieldId) { 
		return new TOgFtrExtNom(OgBase, JoinSeq, FieldId); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& FieldId) { 
		return new TOgFtrExtNom(OgBase, JoinSeqV, FieldId); }

	TStr GetNm(const POgBase& OgBase) const {
		return OgBase->GetStoreByStoreId(GetFtrStoreId())->GetFieldNm(FieldId); };

	void Clr(const POgBase& OgBase) { FtrGen = TFtrGenNominal(); }

	void Update(const POgBase& OgBase, const TOgRec& FtrRec);
	void AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const;
	int GetDim(const POgBase& OgBase) const { return FtrGen.GetVals(); }
	TStr GetFtr(const POgBase& OgBase, const int& FtrN) const { return FtrGen.GetVal(FtrN); }

	void ExtractStrV(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const;
};

class TOgFtrExtMultiNom : public TOgFtrExt {
private:
	TFtrGenMultiNom FtrGen;
	// field Id
	TInt FieldId;

	void ParseDate(const TTm& Tm, TStrV& StrV) const;
	void _GetVal(const POgBase& OgBase, const POgRecSet& RecSet, TStrV& StrV) const; 
	void _GetVal(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const; 
	void GetVal(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const; 

public:
	TOgFtrExtMultiNom(const POgStore& Store, const int& _FieldId): TOgFtrExt(Store), FieldId(_FieldId) { }
	TOgFtrExtMultiNom(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& _FieldId): 
		TOgFtrExt(OgBase, JoinSeq), FieldId(_FieldId) { }
	TOgFtrExtMultiNom(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& _FieldId): 
	  TOgFtrExt(OgBase, JoinSeqV), FieldId(_FieldId) { }

	static POgFtrExt New(const POgStore& Store, const int& FieldId) {
		return new TOgFtrExtMultiNom(Store, FieldId); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& FieldId) { 
		return new TOgFtrExtMultiNom(OgBase, JoinSeq, FieldId); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& FieldId) { 
		return new TOgFtrExtMultiNom(OgBase, JoinSeqV, FieldId); }

	TStr GetNm(const POgBase& OgBase) const {
		return OgBase->GetStoreByStoreId(GetFtrStoreId())->GetFieldNm(FieldId); };

	void Clr(const POgBase& OgBase) { FtrGen = TFtrGenMultiNom(); }

	void Update(const POgBase& OgBase, const TOgRec& FtrRec);
	void AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const;
	int GetDim(const POgBase& OgBase) const { return FtrGen.GetVals(); }
	TStr GetFtr(const POgBase& OgBase, const int& FtrN) const { return FtrGen.GetVal(FtrN); }

	// flat feature extraction
	void ExtractStrV(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const;
	void ExtractTmV(const POgBase& OgBase, const TOgRec& FtrRec, TTmV& TmV) const;
};

typedef enum { ofetmConcat, ofetmCentroid } TOgFtrExtTokenMode;

class TOgFtrExtToken : public TOgFtrExt {
private:
	TFtrGenToken FtrGen;
	TInt FieldId;
	TOgFtrExtTokenMode Mode;

	void _GetVal(const POgBase& OgBase, const POgRecSet& RecSet, TStrV& StrV) const; 
	void _GetVal(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const; 
	void GetVal(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const; 

public:
	TOgFtrExtToken(const POgStore& Store, const int& _FieldId, const TOgFtrExtTokenMode& _Mode,
		const PSwSet& SwSet, const PStemmer& Stemmer): TOgFtrExt(Store), FtrGen(SwSet, Stemmer), 
			FieldId(_FieldId), Mode(_Mode) { }
	TOgFtrExtToken(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& _FieldId,
      const TOgFtrExtTokenMode& _Mode, const PSwSet& SwSet, const PStemmer& Stemmer): 
		TOgFtrExt(OgBase, JoinSeq), FtrGen(SwSet, Stemmer), FieldId(_FieldId), Mode(_Mode) { }
	TOgFtrExtToken(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& _FieldId,
	  const TOgFtrExtTokenMode& _Mode, const PSwSet& SwSet, const PStemmer& Stemmer):
		TOgFtrExt(OgBase, JoinSeqV), FtrGen(SwSet, Stemmer), FieldId(_FieldId), Mode(_Mode) { }

	static POgFtrExt New(const POgStore& Store, const int& FieldId, const TOgFtrExtTokenMode& Mode = ofetmConcat,
	  const PSwSet& SwSet = TSwSet::New(swstEn523), const PStemmer& Stemmer = TStemmer::New(stmtPorter, false)) { 
		  return new TOgFtrExtToken(Store, FieldId, Mode, SwSet, Stemmer); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& FieldId, 
	  const TOgFtrExtTokenMode& Mode = ofetmConcat, const PSwSet& SwSet = TSwSet::New(swstEn523), 
	  const PStemmer& Stemmer = TStemmer::New(stmtPorter, false)) { 
			return new TOgFtrExtToken(OgBase, JoinSeq, FieldId, Mode, SwSet, Stemmer); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& FieldId, 
	  const TOgFtrExtTokenMode& Mode = ofetmConcat, const PSwSet& SwSet = TSwSet::New(swstEn523), 
	  const PStemmer& Stemmer = TStemmer::New(stmtPorter, false)) {
			return new TOgFtrExtToken(OgBase, JoinSeqV, FieldId, Mode, SwSet, Stemmer); }

	TStr GetNm(const POgBase& OgBase) const {
		return OgBase->GetStoreByStoreId(GetFtrStoreId())->GetFieldNm(FieldId); };

	void Clr(const POgBase& OgBase) { FtrGen = TFtrGenToken(FtrGen.GetSwSet(), FtrGen.GetStemmer()); }

	void Update(const POgBase& OgBase, const TOgRec& FtrRec);
	void AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const;
	int GetDim(const POgBase& OgBase) const { return FtrGen.GetVals(); }
	TStr GetFtr(const POgBase& OgBase, const int& FtrN) const { return FtrGen.GetVal(FtrN); }

	void ExtractStrV(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const;
};

class TOgFtrExtJoin : public TOgFtrExt {
private:
	TInt Sample;
	TInt Dim;

	void Def(const POgBase& OgBase);

public:
	TOgFtrExtJoin(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& _Sample): 
		TOgFtrExt(OgBase, JoinSeq), Sample(_Sample) { Def(OgBase); }
	TOgFtrExtJoin(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& _Sample): 
		TOgFtrExt(OgBase, JoinSeqV), Sample(_Sample) { Def(OgBase); }

	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& Sample = 1) { 
		return new TOgFtrExtJoin(OgBase, JoinSeq, Sample); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& Sample = 1) {
		return new TOgFtrExtJoin(OgBase, JoinSeqV, Sample); }

	TStr GetNm(const POgBase& OgBase) const { return ""; }

	void Clr(const POgBase& OgBase) { Def(OgBase); }

	void AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const;
	int GetDim(const POgBase& OgBase) const { return Dim; }
	TStr GetFtr(const POgBase& OgBase, const int& FtrN) const { 
		return OgBase->GetStoreByStoreId(GetFtrStoreId())->GetStoreNm(); }

	// flat feature extraction
	void ExtractStrV(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const;
};

class TOgFtrExtPair : public TOgFtrExt {
private:
	POgFtrExt FtrExt1;
	POgFtrExt FtrExt2;
	TStrHash<TInt> FtrValH;
	TIntPrIntH FtrIdPairH;

	void GetFtrIdV_Update(const POgBase& OgBase, const TOgRec& FtrRec, 
		const POgFtrExt& FtrExt, TIntV& FtrIdV);
	void GetFtrIdV_RdOnly(const POgBase& OgBase, const TOgRec& FtrRec, 
		const POgFtrExt& FtrExt, TIntV& FtrIdV) const;

public:
	TOgFtrExtPair(const POgStore& Store, const POgFtrExt& _FtrExt1,
		const POgFtrExt& _FtrExt2): TOgFtrExt(Store), FtrExt1(_FtrExt1), FtrExt2(_FtrExt2) { }
	TOgFtrExtPair(const POgBase& OgBase, const TOgJoinSeq& JoinSeq,
		const POgFtrExt& _FtrExt1, const POgFtrExt& _FtrExt2): 
			TOgFtrExt(OgBase, JoinSeq), FtrExt1(_FtrExt1), FtrExt2(_FtrExt2) { }
	TOgFtrExtPair(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV,
		const POgFtrExt& _FtrExt1, const POgFtrExt& _FtrExt2):
			TOgFtrExt(OgBase, JoinSeqV), FtrExt1(_FtrExt1), FtrExt2(_FtrExt2) { }

	static POgFtrExt New(const POgStore& Store, const POgFtrExt& FtrExt1,
		const POgFtrExt& FtrExt2) { return new TOgFtrExtPair(Store, FtrExt1, FtrExt2); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeq& JoinSeq,
		const POgFtrExt& FtrExt1, const POgFtrExt& FtrExt2) { 
			return new TOgFtrExtPair(OgBase, JoinSeq, FtrExt1, FtrExt2); }
	static POgFtrExt New(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV,
		const POgFtrExt& FtrExt1, const POgFtrExt& FtrExt2) { 
			return new TOgFtrExtPair(OgBase, JoinSeqV, FtrExt1, FtrExt2); }

	TStr GetNm(const POgBase& OgBase) const {
		return "[" + FtrExt1->GetNm(OgBase) + ", " + FtrExt2->GetNm(OgBase) + "]"; };

	void Clr(const POgBase& OgBase) { FtrValH = TStrHash<TInt>(); FtrIdPairH.Clr(); }

	void Update(const POgBase& OgBase, const TOgRec& FtrRec);
	void AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const;
	int GetDim(const POgBase& OgBase) const { return FtrIdPairH.Len(); }
	TStr GetFtr(const POgBase& OgBase, const int& FtrN) const;

	void ExtractStrV(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const;
};

class TOgOpGetRec : public TOgOp {
private:
public:
	TOgOpGetRec(): TOgOp("store-rec") { }
	static POgOp New() { return new TOgOpGetRec; }
	
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV);
	bool IsFunctional() { return true; }
	POgRecSet Exec(const uchar& StoreId, const uint64& RecId);
};

class TOgOpSearch : public TOgOp {
public:
	TOgOpSearch(): TOgOp("search") { }
	static POgOp New() { return new TOgOpSearch; }
	
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV);
	bool IsFunctional() { return true; }
	POgRecSet Exec(const POgBase& OgBase, const POgQuery& Query);
};

typedef enum { oolstLess, oolstEqual, oolstNotEqual, oolstGreater, oolstIsInRange, oolstIsIn, oolstIsNotIn } TOgOpLinSearchType;
class TOgOpLinSearch : public TOgOp {
private:
	void ParseQuery(const POgBase& OgBase, const uchar& StoreId, const TStr& QueryElt, 
		int& FieldId, TOgOpLinSearchType& LinSearchType, TStr& FieldVal);
public:
	TOgOpLinSearch(): TOgOp("search-lin") { }
	static POgOp New() { return new TOgOpLinSearch(); }
	
	// inherited interface
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV);
	bool IsFunctional() { return true; }
	// actual interface, over full store
	POgRecSet Exec(const POgBase& OgBase, const uchar& StoreId, const int& FieldId, 
		const TOgOpLinSearchType& OpLinSearchType, const int& FieldVal);
	POgRecSet Exec(const POgBase& OgBase, const uchar& StoreId, const int& FieldId, 
		const TOgOpLinSearchType& OpLinSearchType, const TTm& FieldVal);
	POgRecSet Exec(const POgBase& OgBase, const uchar& StoreId, const int& FieldId, 
		const TTm& MnFieldVal, const TTm& MxFieldVal);
	POgRecSet Exec(const POgBase& OgBase, const int& StoreId, const int& FieldId, 
		const uint64& MnFieldVal, const uint64& MxFieldVal);
	POgRecSet Exec(const POgBase& OgBase, const int& StoreId, const int& FieldId, 
		const TOgOpLinSearchType& OpLinSearchType, const TIntV& FieldVals);		
	// actual interface, over given record set
	POgRecSet Exec(const POgBase& OgBase, const POgRecSet& RecSet, const int& FieldId, 
		const TOgOpLinSearchType& OpLinSearchType, const TTm& FieldVal);
	POgRecSet Exec(const POgBase& OgBase, const POgRecSet& RecSet, const int& FieldId, 
		const TTm& MnFieldVal, const TTm& MxFieldVal);
	POgRecSet Exec(const POgBase& OgBase, const POgRecSet& RecSet, 
		const int& FieldId, const bool& FieldVal);
};

class TOgOpSort : public TOgOp {
public:
	TOgOpSort(): TOgOp("sort") { }
	static POgOp New() { return new TOgOpSort; }
	
	// inherited interface
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV);
	bool IsFunctional() { return true; }
	// actual interface
	POgRecSet Exec(const POgBase& OgBase, const POgRecSet& RecSet, 
		const int& FieldId, const bool& Asc);
	POgRecSet Exec(const POgBase& OgBase, const POgRecSet& InRecSet, const bool& Asc);
};

class TOgOpReverse : public TOgOp {
public:
	TOgOpReverse(): TOgOp("reverse") { }
	static POgOp New() { return new TOgOpReverse; }
	
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV);
	bool IsFunctional() { return true; }
	POgRecSet Exec(const POgRecSet& RecSet);	
};

class TOgOpShuffle : public TOgOp {
private:
    TRnd Rnd;

public:
	TOgOpShuffle(): TOgOp("shuffle"), Rnd(1) { }
	static POgOp New() { return new TOgOpShuffle; }
	
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV);
	bool IsFunctional() { return true; }
	POgRecSet Exec(const POgRecSet& InRecSet);	
};

class TOgOpJoin : public TOgOp {
public:
	TOgOpJoin(): TOgOp("join") { }
	static POgOp New() { return new TOgOpJoin; }
	
	// inherited interface
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV);
	bool IsFunctional() { return true; }
	// actual interface
	POgRecSet Exec(const POgBase& OgBase, const POgRecSet& InRecSet, const TOgJoinSeq& JoinSeq);
};

class TOgOpAggrField: public TOgOp {
private:
	void ParseFtrExt(const POgBase& OgBase, const POgStore& StartStore, 
		const TStr& FtrExtStr, TOgJoinSeq& JoinSeq, int& FieldId);
public:
	TOgOpAggrField(): TOgOp("aggr-field") { }
	static POgOp New() { return new TOgOpAggrField; }
	
	// inherited interface
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV);
	bool IsFunctional() { return true; }
	// actual interface
	POgRecSet Exec(const POgBase& OgBase, const POgRecSet& InRecSet,
		const POgStore& StartStore, TOgJoinSeq& JoinSeq, int& AggrFieldId);
};

class TOgOpAggrKey: public TOgOp {
public:
	TOgOpAggrKey(): TOgOp("aggr-key") { }
	static POgOp New() { return new TOgOpAggrKey; }
	
	// inherited interface
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV);
	bool IsFunctional() { return true; }
	// actual interface
	POgRecSet Exec(const POgBase& OgBase, const POgRecSet& InRecSet, const int& KeyId);
};

class TOgOpGroupBy: public TOgOp {
public:
	TOgOpGroupBy(): TOgOp("group-by") { }
	static POgOp New() { return new TOgOpGroupBy; }
	
	// inherited interface
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV);
	bool IsFunctional() { return false; }
	// actual interface
	void Exec(const POgBase& OgBase, const POgRecSet& InRecSet, const uchar& StoreId, 
		const int& FieldId, TOgRecSetV& OutRecSetV);
};

class TOgOpSplitBy: public TOgOp {
public:
	TOgOpSplitBy(): TOgOp("split-by") { }
	static POgOp New() { return new TOgOpSplitBy; }
	
	// inherited interface
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV);
	bool IsFunctional() { return false; }
	// actual interface
	void Exec(const POgBase& OgBase, const POgRecSet& InRecSet, const uchar& StoreId, 
		const int& FieldId, const int& SplitWinSize, TOgRecSetV& OutRecSetV);
};

class TOgOpHrchClust: public TOgOp {
private:
	int GetLevDistVV(const TVec<TIntV>& StepVV, TVec<TIntV>& DistVV);
	int GetLevDist(const TIntV& Vec1, const TIntV& Vec2);
	void MergeClust(TOgRecSetV& OutRecSetV, TVec<TIntV>& StepVV, 
		TVec<TIntV>& DistVV, int& MnVal);
	void UpdateDistVV(const TVec<TIntV>& StepVV, 
	const int& Idx, TVec<TIntV>& DistVV, int& MnVal);
public:
	TOgOpHrchClust(): TOgOp("hrch-clust") { }
	static POgOp New() { return new TOgOpHrchClust; }
	
	// inherited interface
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV);
	bool IsFunctional() { return false; }
	// actual interface
	void Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const uchar& StoreId, const int& FieldId, int& Dist, 
		const int& ClustNo, TOgRecSetV& OutRecSetV);
};

class TOgAggrPiechart : public TOgAggr {
private:
	TStr AggrNm;
	TStr JoinPathStr;
	TInt Count;
	TStrH ValH;

public:
	TOgAggrPiechart(const POgBase& OgBase, const POgRecSet& RecSet, const POgFtrExt& FtrExt);
	static POgAggr New(const POgBase& OgBase, const POgRecSet& RecSet, const POgFtrExt& FtrExt) {
		return new TOgAggrPiechart(OgBase, RecSet, FtrExt); }

	TOgAggrPiechart(const POgBase& OgBase, const POgRecSet& RecSet, const int& KeyId);
	static POgAggr New(const POgBase& OgBase, const POgRecSet& RecSet, const int& KeyId) { 
			return new TOgAggrPiechart(OgBase, RecSet, KeyId); }

	PXmlTok SaveXml() const;
	PJsonVal SaveJson() const;
};

class TOgAggrHistogram : public TOgAggr {
private:
	//meta-data
	TStr AggrNm;
	TStr JoinPathStr;
	// aggregations
	TFlt Sum;
	PMom Mom;
	THist Hist;

public:
	TOgAggrHistogram(const POgBase& OgBase, const POgRecSet& RecSet,
		const POgFtrExt& FtrExt, const int& Buckets);
	static POgAggr New(const POgBase& OgBase, const POgRecSet& RecSet,
		const POgFtrExt& FtrExt, const int& Buckets) {
			return new TOgAggrHistogram(OgBase, RecSet, FtrExt, Buckets); }

	PXmlTok SaveXml() const;
	PJsonVal SaveJson() const;
};

class TOgAggrKeywords : public TOgAggr {
private:
	//meta-data
	TStr AggrNm;
	TStr JoinPathStr;
	PBowKWordSet KWordSet;

public:
	TOgAggrKeywords(const POgBase& OgBase, const POgRecSet& RecSet,
		const POgFtrExt& FtrExt, const int& SampleSize);
	static POgAggr New(const POgBase& OgBase, const POgRecSet& RecSet,
		const POgFtrExt& FtrExt, const int& SampleSize) {
			return new TOgAggrKeywords(OgBase, RecSet, FtrExt, SampleSize); }

	PXmlTok SaveXml() const;
	PJsonVal SaveJson() const;

	static PXmlTok SaveXml(const PBowKWordSet& KWordSet);
};

class TOgAggrTimeLine : public TOgAggr {
private:
	//meta-data
	TStr AggrNm;
	TStr JoinPathStr;
	// aggregations
	TInt Count;
	TStrH AbsDateH;
	TStrH AbsTimeH;
	TStrH MonthH;
	TStrH DayOfWeekH;
	TStrH HourOfDayH;

	PXmlTok	GetXmlList(const TStr& TokNm, const TStrH& StrH) const;

public:
	TOgAggrTimeLine(const POgBase& OgBase, const POgRecSet& RecSet, const POgFtrExt& FtrExt);
	static POgAggr New(const POgBase& OgBase, const POgRecSet& RecSet, const POgFtrExt& FtrExt) {
		return new TOgAggrTimeLine(OgBase, RecSet, FtrExt); }

	PXmlTok SaveXml() const;
	PJsonVal SaveJson() const;
};

#endif
