#include "qminer.h"

PNotify TOg::Logger = TStdNotify::New();

TChA TOg::ValidFirstCh = "_";
TChA TOg::ValidCh = "_-";

void TOg::AssertValidNm(const TStr& NmStr) {
	OgAssertR(!NmStr.Empty(), "Name: cannot be empty"); 
	const char FirstCh = NmStr[0];
	if ((('A'<=FirstCh)&&(FirstCh<='Z')) || (('a'<=FirstCh)&&(FirstCh<='z')) || ValidFirstCh.IsChIn(FirstCh)) {
	} else {
		TOgExcept::Throw("Name: invalid first character in '" + NmStr + "'");
	}
	for (int ChN = 1; ChN < NmStr.Len(); ChN++) {
		const char Ch = NmStr[ChN];
		if ((('A'<=Ch)&&(Ch<='Z')) || (('a'<=Ch)&&(Ch<='z')) || (('0'<=Ch)&&(Ch<='9')) || ValidCh.IsChIn(Ch)) {
		} else {
			TOgExcept::Throw(TStr::Fmt("Name: invalid %d character in '%s'", ChN, NmStr.CStr()));
		}
	}
}

TOgJoinDesc::TOgJoinDesc(const TStr& _JoinNm, const uchar& _JoinStoreId, 
		const uchar& StoreId, const POgIndexVoc& IndexVoc): JoinId(-1) { 

	JoinStoreId = _JoinStoreId;
	JoinNm = _JoinNm;
	JoinType = osjtIndex;
	JoinFieldId = -1;
	TStr JoinKeyNm = "Join" + TGuid::GenGuid();
	JoinKeyId = IndexVoc->AddInternalKey(StoreId, JoinKeyNm, JoinNm);
	TOg::AssertValidNm(JoinNm);
}

TOgJoinDesc::TOgJoinDesc(TSIn& SIn): JoinId(SIn), JoinNm(SIn), JoinStoreId(SIn) {
	JoinType = TOgStoreJoinType(TInt(SIn).Val);
	JoinKeyId = TInt(SIn);
	JoinFieldId = TInt(SIn);
}

void TOgJoinDesc::Save(TSOut& SOut) const { 
	JoinId.Save(SOut); JoinNm.Save(SOut); JoinStoreId.Save(SOut);
	TInt(JoinType).Save(SOut);
	JoinKeyId.Save(SOut); JoinFieldId.Save(SOut);
}

POgStore TOgJoinDesc::GetJoinStore(const POgBase& OgBase) const {
	return OgBase->GetStoreByStoreId(GetJoinStoreId());
}

TOgJoinSeq::TOgJoinSeq(const uchar& _StartStoreId, const int& JoinId, const int& Sample):
	StartStoreId(_StartStoreId) { JoinIdV.Add(TIntPr(JoinId, Sample)); }

TOgJoinSeq::TOgJoinSeq(const uchar& _StartStoreId, const TIntPrV& _JoinIdV):
	StartStoreId(_StartStoreId), JoinIdV(_JoinIdV) { }

POgStore TOgJoinSeq::GetStartStore(const POgBase& OgBase) const { 
	return OgBase->GetStoreByStoreId(StartStoreId); 
}

POgStore TOgJoinSeq::GetEndStore(const POgBase& OgBase) const {
	POgStore Store = OgBase->GetStoreByStoreId(StartStoreId);
	for (int JoinIdN = 0; JoinIdN < JoinIdV.Len(); JoinIdN++) {
		const TOgJoinDesc& JoinDesc = Store->GetJoinDesc(JoinIdV[JoinIdN].Val1);
		Store = OgBase->GetStoreByStoreId(JoinDesc.GetJoinStoreId());
	}
	return Store;
}

uchar TOgJoinSeq::GetEndStoreId(const POgBase& OgBase) const {
	return GetEndStore(OgBase)->GetStoreId();
}

TOgFieldDesc::TOgFieldDesc(TSIn& SIn) { 
    FieldId = TInt(SIn);
    FieldNm = TStr(SIn);
	FieldType = TOgFieldType(TInt(SIn).Val);
	DefFtrType = TOgFieldFtrType(TInt(SIn).Val);
	AggrType = TOgFieldAggrType(TInt(SIn).Val);
	DisplayType = TOgFieldDisplayType(TInt(SIn).Val);
	KeyIdV.Load(SIn);
}

void TOgFieldDesc::Save(TSOut& SOut) {
	FieldId.Save(SOut);
	FieldNm.Save(SOut);
	TInt(FieldType).Save(SOut);
	TInt(DefFtrType).Save(SOut);
	TInt(AggrType).Save(SOut);
	TInt(DisplayType).Save(SOut); 
	KeyIdV.Save(SOut);
}

TStr TOgFieldDesc::GetFieldTypeStr() const {
	switch (FieldType) {
        case oftInt : return "INTEGER"; 
        case oftUInt64 : return "UNSIGNED INTEGER 64"; 
        case oftStr : return "STRING"; 
        case oftStrV : return "STRING-VECTOR"; 
        case oftBool : return "BOOLEAN"; 
        case oftFlt : return "DOUBLE"; 
        case oftFltPr : return "DOUBLE-PAIR"; 
        case oftTm : return "DATE-TIME"; 
    }
	Fail; return "";
}

TStr TOgFieldDesc::GetDefFtrTypeStr() const {
    switch (DefFtrType) {
        case offtNone : return "NONE"; 
        case offtNumeric : return "NUMERIC"; 
        case offtNominal : return "NOMINAL"; 
        case offtMultiNom : return "MULTINOMINAL"; 
        case offtToken : return "TOKENIZABLE"; 
        case offtSpNum : return "SPARSE-NUMERIC"; 
        case offtTm : return "DATE-TIME"; 
    }
	Fail; return "";
}

TStr TOgFieldDesc::GetAggrTypeStr() const {
    switch (AggrType) {
        case ofatNone : return "NONE"; 
        case ofatHistogram : return "HISTOGRAM"; 
        case ofatPiechart : return "PIECHART"; 
        case ofatTimeline : return "TIMELINE"; 
        case ofatKeywords : return "KEYWORDS"; 
        case ofatScalar : return "SCALAR"; 
    }
	Fail; return "";
}

TStr TOgFieldDesc::GetDisplayTypeStr() const {
    switch (DisplayType) {
        case ofdtNone : return "NONE"; 
        case ofdtText : return "TEXT"; 
        case ofdtMap : return "MAP"; 
    }
	Fail; return "";
}

TOgStoreIterVec::TOgStoreIterVec(const uint64& _RecIds): 
	FirstP(true), RecId(0), RecIds(_RecIds) { }
	
bool TOgStoreIterVec::Next() {
	if (FirstP) { 
		FirstP = false; 
	} else { 
		RecId++; 
	}
	return (RecId < RecIds); 
}

void TOgStore::LoadOgStore(TSIn& SIn) {	
	StoreId = TUCh(SIn); StoreNm.Load(SIn);
	JoinDescV.Load(SIn); JoinNmToIdH.Load(SIn);
	for (int JoinN = 0; JoinN < JoinDescV.Len(); JoinN++) {
		const TOgJoinDesc& JoinDesc = JoinDescV[JoinN];
		if (JoinDesc.IsIndexJoin()) {
			JoinNmToKeyIdH.AddDat(JoinDesc.GetJoinNm(), JoinDesc.GetJoinKeyId());
		}
	}
	FieldDescV.Load(SIn); FieldNmToIdH.Load(SIn);
}

void TOgStore::SaveOgStore(TSOut& SOut) const { 
    StoreId.Save(SOut); StoreNm.Save(SOut); 
    JoinDescV.Save(SOut); JoinNmToIdH.Save(SOut); 
    FieldDescV.Save(SOut); FieldNmToIdH.Save(SOut); 
}

int TOgStore::AddFieldDesc(const TOgFieldDesc& FieldDesc) { 
	const int FieldId = FieldDescV.Add(FieldDesc);
	FieldDescV[FieldId].PutFieldId(FieldId);
	FieldNmToIdH.AddDat(FieldDesc.GetFieldNm()) = FieldId;
	return FieldId;
}

void TOgStore::FieldError(const int& FieldId, const TStr& TypeStr) const { 
	TOgExcept::Throw(TStr::Fmt("Wrong field-type combination requested: [%d:%s]!", FieldId, TypeStr.CStr())); 
}

void TOgStore::OnAdd(const POgBase& OgBase, const uint64& RecId) {
    for (int TriggerN = 0; TriggerN < TriggerV.Len(); TriggerN++) {
        TriggerV[TriggerN]->OnAdd(OgBase, this, RecId);
    }
}

void TOgStore::OnUpdate(const POgBase& OgBase, const uint64& RecId) {
    for (int TriggerN = 0; TriggerN < TriggerV.Len(); TriggerN++) {
        TriggerV[TriggerN]->OnUpdate(OgBase, this, RecId);
    }
}

void TOgStore::OnDelete(const POgBase& OgBase, const uint64& RecId) {
    for (int TriggerN = 0; TriggerN < TriggerV.Len(); TriggerN++) {
        TriggerV[TriggerN]->OnDelete(OgBase, this, RecId);
    }
}

void TOgStore::StrVToIntV(const TStrV& StrV, TStrHash<TInt, TBigStrPool>& StrH, TIntV& IntV) {
    const int Len = StrV.Len(); IntV.Gen(Len, 0);
    for (int StrN = 0; StrN < Len; StrN++) {
        IntV.Add(StrH.AddKey(StrV[StrN]));
    }
}

void TOgStore::IntVToStrV(const TIntV& IntV, const TStrHash<TInt, TBigStrPool>& StrH, TStrV& StrV) const {
    const int Len = IntV.Len(); StrV.Gen(Len, 0);
    for (int IntN = 0; IntN < Len; IntN++) {
        StrV.Add(StrH.GetKey(IntV[IntN]));
    }
}

void TOgStore::IntVToStrV(const TIntV& IntV, TStrV& StrV) const {
    const int Len = IntV.Len(); StrV.Gen(Len, 0);
    for (int IntN = 0; IntN < Len; IntN++) {
        StrV.Add(TInt::GetStr(IntV[IntN]));
    }
}

int TOgStore::AddJoinDesc(const TOgJoinDesc& JoinDesc) {
	const int JoinId = JoinDescV.Add(JoinDesc);
	JoinDescV[JoinId].PutJoinId(JoinId);
	JoinNmToIdH.AddDat(JoinDesc.GetJoinNm()) = JoinId;
	if (JoinDesc.IsIndexJoin()) { 
		JoinNmToKeyIdH.AddDat(JoinDesc.GetJoinNm(), JoinDesc.GetJoinKeyId()); 
	}
	return JoinId;
}

TIntV TOgStore::GetFieldIdV(const TOgFieldType& Type){
	TIntV FieldIdV;
	for (int i = 0; i < FieldDescV.Len(); i++){
		if (FieldDescV[i].GetFieldType() == Type){
			FieldIdV.Add(i);
		}
	}
	return FieldIdV;
}

void TOgStore::AddTrigger(const POgStoreTrigger& Trigger) { 
	TOg::Logger->OnStatusFmt("Adding trigger to store %s\n", GetStoreNm().CStr());
    Trigger->Init(this);
    TriggerV.Add(Trigger); 
}

TOgRec TOgStore::GetRec(const uint64& RecId) const { 
	return TOgRec(GetStoreId(), RecId); 
}

TOgRec TOgStore::GetRec(const TStr& RecNm) const { 
	return TOgRec(GetStoreId(), GetRecId(RecNm)); 
}

POgRecSet TOgStore::GetAllRecs() const {
	TUInt64V RecIdV;
	POgStoreIter Iter = GetIter();
	while (Iter->Next()) {
		RecIdV.Add(Iter->GetRecId());
	}
	return TOgRecSet::New(GetStoreId(), RecIdV);
}

POgRecSet TOgStore::GetRndRecs(const uint64& SampleSize) const {
	return GetAllRecs()->GetSampleRecSet((int)SampleSize, false);
}

int TOgStore::GetFieldInt(const uint64& RecId, const int& FieldId) const { 
	FieldError(FieldId, "Int"); return -1; 
}

void TOgStore::GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const { 
	FieldError(FieldId, "IntV"); 
}

uint64 TOgStore::GetFieldUInt64(const uint64& RecId, const int& FieldId) const { 
	FieldError(FieldId, "UInt64"); return -1; 
}

TStr TOgStore::GetFieldStr(const uint64& RecId, const int& FieldId) const { 
	FieldError(FieldId, "Str"); return ""; 
}

void TOgStore::GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const { 
	FieldError(FieldId, "StrV"); 
}

bool TOgStore::GetFieldBool(const uint64& RecId, const int& FieldId) const { 
	FieldError(FieldId, "Bool"); return false; 
}

double TOgStore::GetFieldFlt(const uint64& RecId, const int& FieldId) const { 
	FieldError(FieldId, "Flt"); return 0.0; 
}

TFltPr TOgStore::GetFieldFltPr(const uint64& RecId, const int& FieldId) const { 
	FieldError(FieldId, "FltPr"); return TFltPr(); 
}

void TOgStore::GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const { 
	FieldError(FieldId, "FltV"); 
}

void TOgStore::GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const { 
	FieldError(FieldId, "Tm"); 
}

void TOgStore::GetFieldNumSpV(const uint64& RecId, const int& FieldId, TIntFltKdV& SpV) const { 
	FieldError(FieldId, "NumSpV"); 
}

void TOgStore::GetFieldBowSpV(const uint64& RecId, const int& FieldId, PBowSpV& SpV) const { 
	FieldError(FieldId, "BowSpV"); 
}

int TOgStore::GetFieldNmInt(const uint64& RecId, const TStr& FieldNm) const { 
	return GetFieldInt(RecId, GetFieldId(FieldNm)); 
}

void TOgStore::GetFieldNmIntV(const uint64& RecId, const TStr& FieldNm, TIntV& IntV) const { 
	return GetFieldIntV(RecId, GetFieldId(FieldNm), IntV); 
}

uint64 TOgStore::GetFieldNmUInt64(const uint64& RecId, const TStr& FieldNm) const { 
	return GetFieldUInt64(RecId, GetFieldId(FieldNm)); 
}

TStr TOgStore::GetFieldNmStr(const uint64& RecId, const TStr& FieldNm) const { 
	return GetFieldStr(RecId, GetFieldId(FieldNm)); 
}

void TOgStore::GetFieldNmStrV(const uint64& RecId, const TStr& FieldNm, TStrV& StrV) const { 
	return GetFieldStrV(RecId, GetFieldId(FieldNm), StrV); 
}

bool TOgStore::GetFieldNmBool(const uint64& RecId, const TStr& FieldNm) const { 
	return GetFieldBool(RecId, GetFieldId(FieldNm)); 
}

double TOgStore::GetFieldNmFlt(const uint64& RecId, const TStr& FieldNm) const { 
	return GetFieldFlt(RecId, GetFieldId(FieldNm)); 
}

TFltPr TOgStore::GetFieldNmFltPr(const uint64& RecId, const TStr& FieldNm) const { 
	return GetFieldFltPr(RecId, GetFieldId(FieldNm)); 
}

void TOgStore::GetFieldNmFltV(const uint64& RecId, const TStr& FieldNm, TFltV& FltV) const { 
	return GetFieldFltV(RecId, GetFieldId(FieldNm), FltV); 
}

void TOgStore::GetFieldNmTm(const uint64& RecId, const TStr& FieldNm, TTm& Tm) const { 
	return GetFieldTm(RecId, GetFieldId(FieldNm), Tm); 
}

void TOgStore::GetFieldNmNumSpV(const uint64& RecId, const TStr& FieldNm, TIntFltKdV& SpV) const { 
	return GetFieldNumSpV(RecId, GetFieldId(FieldNm), SpV); 
}

void TOgStore::GetFieldNmBowSpV(const uint64& RecId, const TStr& FieldNm, PBowSpV& SpV) const { 
	return GetFieldBowSpV(RecId, GetFieldId(FieldNm), SpV); 
}

TStr TOgStore::GetDisplayText(const uint64& RecId, const int& FieldId) const {
    const TOgFieldDesc& Desc = GetFieldDesc(FieldId);
	if (!Desc.IsDisplayText()) { FieldError(FieldId, "IsDisplayText"); return ""; }	
    if (Desc.IsInt()) {
		return TInt::GetStr(GetFieldInt(RecId, FieldId));
    } else if (Desc.IsIntV()) {
        TIntV FieldIntV; GetFieldIntV(RecId, FieldId, FieldIntV); 
		return TStrUtil::GetStr(FieldIntV);
    } else if (Desc.IsUInt64()) {
		return TUInt64::GetStr(GetFieldUInt64(RecId, FieldId));
	} else if (Desc.IsStr()) {
        return GetFieldStr(RecId, FieldId);
    } else if (Desc.IsStrV()) {
        TStrV FieldStrV; GetFieldStrV(RecId, FieldId, FieldStrV); 
		return TStrUtil::GetStr(FieldStrV);
    } else if (Desc.IsBool()) {
        return GetFieldBool(RecId, FieldId) ? "Yes" : "No";
    } else if (Desc.IsFlt()) {
		return TFlt::GetStr(GetFieldFlt(RecId, FieldId));
    } else if (Desc.IsFltPr()) {
        const TFltPr FieldFltPr = GetFieldFltPr(RecId, FieldId);
        return TStr::Fmt("(%g, %g)", FieldFltPr.Val1.Val, FieldFltPr.Val2.Val);
	} else if (Desc.IsFltV()) {
        TFltV FieldFltV; GetFieldFltV(RecId, FieldId, FieldFltV); 
		return TStrUtil::GetStr(FieldFltV);
    } else if (Desc.IsTm()) {
        TTm FieldTm; GetFieldTm(RecId, FieldId, FieldTm);		
		if (FieldTm.IsDef()) { return FieldTm.GetWebLogDateTimeStr(); } else { return "--";	}
	} else if (Desc.IsNumSpV()) {
		TIntFltKdV FieldIntFltKdV; GetFieldNumSpV(RecId, FieldId, FieldIntFltKdV); 
		return TStrUtil::GetStr(FieldIntFltKdV);
	} else if (Desc.IsBowSpV()) {
		return "[PBowSpV]"; //TODO
    }
	FieldError(FieldId, "GetDisplayText"); return "";	
}

TFltPr TOgStore::GetDisplayMap(const uint64& RecId, const int& FieldId) const {
	const TOgFieldDesc& Desc = GetFieldDesc(FieldId);
	if (!Desc.IsDisplayMap()) { FieldError(FieldId, "IsDisplayMap"); return TFltPr(); }
	if (Desc.IsFltPr()) { return GetFieldFltPr(RecId, FieldId); }
	FieldError(FieldId, "GetDisplayMap"); return TFltPr();
}

TStr TOgStore::GetDisplayText(const uint64& RecId, const TStr& FieldNm) const { 
	return GetDisplayText(RecId, GetFieldId(FieldNm)); 
}

TFltPr TOgStore::GetDisplayMap(const uint64& RecId, const TStr& FieldNm) const { 
	return GetDisplayMap(RecId, GetFieldId(FieldNm)); 
}

void TOgStore::PrintRecSet(const POgBase& OgBase, const POgRecSet& RecSet, TSOut& SOut) const {
	SOut.PutStrFmtLn("Records: %d", RecSet->GetRecs());
    const int Fields = GetFields();
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
		const uint64 RecId = RecSet->GetRecId(RecN);
		const int RecFq = RecSet->GetRecFq(RecN);
        TStr RecNm = GetRecNm(RecId);
        SOut.PutStrFmtLn("[%I64u] %s (fq=%d)", RecId, RecNm.CStr(), RecFq);
        for (int FieldId = 0; FieldId < Fields; FieldId++) {
            const TOgFieldDesc& Desc = GetFieldDesc(FieldId);
            if (Desc.IsStr()) {
                TStr FieldStr = GetFieldStr(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), FieldStr.CStr());
            } else if (Desc.IsStrV()) {
                TStrV FieldStrV; GetFieldStrV(RecId, FieldId, FieldStrV);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), TStr::GetStr(FieldStrV, ", ").CStr());
            } else if (Desc.IsInt()) {
                const int FieldInt = GetFieldInt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %d", Desc.GetFieldNm().CStr(), FieldInt);
		    } else if (Desc.IsUInt64()) {
                const uint64 FieldInt = GetFieldUInt64(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %I64u", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsFlt()) {
                const double FieldFlt = GetFieldFlt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %g", Desc.GetFieldNm().CStr(), FieldFlt);
            } else if (Desc.IsFltPr()) {
                const TFltPr FieldFltPr = GetFieldFltPr(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: (%g, %g)", Desc.GetFieldNm().CStr(), FieldFltPr.Val1.Val, FieldFltPr.Val2.Val);
            } else if (Desc.IsTm()) {
                TTm FieldTm; GetFieldTm(RecId, FieldId, FieldTm);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), FieldTm.GetWebLogDateTimeStr().CStr());
            } else if (Desc.IsBool()) {
                TStr FieldStr = GetFieldBool(RecId, FieldId) ? "T" : "F";
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), FieldStr.CStr());
			}
        }
    }
}

void TOgStore::PrintRecSet(const POgBase& OgBase, const POgRecSet& RecSet, const TStr& FNm) const {
	TFOut FOut(FNm); PrintRecSet(OgBase, RecSet, FOut);
 }

void TOgStore::PrintAll(const POgBase& OgBase, TSOut& SOut) const {
	PrintTypes(OgBase, SOut);
	SOut.PutStrLn("Records:");
    const int Fields = GetFields();
	POgStoreIter Iter = GetIter();
	while (Iter->Next()) {
		const uint64 RecId = Iter->GetRecId();
        TStr RecNm = GetRecNm(RecId);
        SOut.PutStrFmtLn("[%I64u] %s", RecId, RecNm.CStr());
        for (int FieldId = 0; FieldId < Fields; FieldId++) {
            const TOgFieldDesc& Desc = GetFieldDesc(FieldId);
            if (Desc.IsStr()) {
                TStr FieldStr = GetFieldStr(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), FieldStr.CStr());
            } else if (Desc.IsStrV()) {
                TStrV FieldStrV; GetFieldStrV(RecId, FieldId, FieldStrV);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), TStr::GetStr(FieldStrV, ", ").CStr());
            } else if (Desc.IsInt()) {
                const int FieldInt = GetFieldInt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %d", Desc.GetFieldNm().CStr(), FieldInt);
		    } else if (Desc.IsUInt64()) {
                const uint64 FieldInt = GetFieldUInt64(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %I64u", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsFlt()) {
                const double FieldFlt = GetFieldFlt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %g", Desc.GetFieldNm().CStr(), FieldFlt);
            } else if (Desc.IsFltPr()) {
                const TFltPr FieldFltPr = GetFieldFltPr(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: (%g, %g)", Desc.GetFieldNm().CStr(), FieldFltPr.Val1.Val, FieldFltPr.Val2.Val);
            } else if (Desc.IsTm()) {
                TTm FieldTm; GetFieldTm(RecId, FieldId, FieldTm);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), FieldTm.GetWebLogDateTimeStr().CStr());
            } else if (Desc.IsBool()) {
                TStr FieldStr = GetFieldBool(RecId, FieldId) ? "T" : "F";
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), FieldStr.CStr());
            }
        }
    }
}

void TOgStore::PrintAll(const POgBase& OgBase, const TStr& FNm) const {
	TFOut FOut(FNm); PrintAll(OgBase, FOut);
}

void TOgStore::PrintTypes(const POgBase& OgBase, TSOut& SOut) const {
    SOut.PutStrFmtLn("Store Name: %s [%d]", GetStoreNm().CStr(), (int)GetStoreId());
    SOut.PutStrFmtLn("Records: %I64u", GetRecs());
    SOut.PutStrLn("Fields:");
    const int Fields = GetFields();
	const POgIndexVoc& IndexVoc = OgBase->GetIndexVoc();
    for (int FieldId = 0; FieldId < Fields; FieldId++) {
        const TOgFieldDesc& Desc = GetFieldDesc(FieldId);
        TStr Type = Desc.GetFieldTypeStr();
		TStr FtrGenType = Desc.GetDefFtrTypeStr();
		TStr AggrType = Desc.GetAggrTypeStr();
		TStr DisplayType = Desc.GetDisplayTypeStr();
		TChA KeyChA;
		for (int KeyN = 0; KeyN < Desc.GetKeys(); KeyN++) {
			KeyChA += KeyChA.Empty() ? ", IK:" :  ";";
			KeyChA += IndexVoc->GetKeyNm(Desc.GetKeyId(KeyN));
		}
		SOut.PutStrFmtLn("  %s [T:%s, FG:%s, A:%s, D:%s%s]", Desc.GetFieldNm().CStr(),
            Type.CStr(), FtrGenType.CStr(), AggrType.CStr(), DisplayType.CStr(), KeyChA.CStr());
    }
	SOut.PutStrLn(TStr::Fmt("Joins:"));
	const int Joins = GetJoins();
	for (int JoinId = 0; JoinId < Joins; JoinId++) {
		const TOgJoinDesc& Desc = GetJoinDesc(JoinId);
		TStr JoinNm = Desc.GetJoinNm();
		TInt JoinStoreId = (int)Desc.GetJoinStoreId();
		TStr JoinType = Desc.IsFieldJoin() ? "FieldJoin" : "IndexJoin";
		SOut.PutStrFmtLn("  %s [S: %d, T: %s]", JoinNm.CStr(), JoinStoreId.Val, JoinType.CStr());
	}
	SOut.PutStrLn(TStr::Fmt("Keys:"));
	const TIntSet& KeySet = IndexVoc->GetStoreKeys(GetStoreId());
	int KeySetId = KeySet.FFirstKeyId();
	while (KeySet.FNextKeyId(KeySetId)) {
		const int KeyId = KeySet.GetKey(KeySetId);
		const TOgIndexKey& Key = IndexVoc->GetKey(KeyId);
		if (!Key.IsDef()) { continue; }
		if (Key.IsInternal()) { continue; }
		SOut.PutStrFmt("  %s [ID: %d", Key.GetKeyNm().CStr(), KeyId);
		if (Key.IsText()) { SOut.PutStr(" Text"); }
		if (Key.IsAggr()) { SOut.PutStr(" Aggr"); }
		if (Key.IsSort()) { SOut.PutStr(Key.IsSortById() ? "SortByWordId" : "SortByWord"); }
		if (Key.IsWordVoc()) { SOut.PutStrFmt(" WordVoc(#values=%d)", IndexVoc->GetWords(KeyId)); }
		SOut.PutStrLn("]");
	}
}

void TOgStore::PrintTypes(const POgBase& OgBase, const  TStr& FNm) const {
	TFOut FOut(FNm); PrintTypes(OgBase, FOut);
}

void TOgRec::FieldError(const int& FieldId, const TStr& TypeStr) const { 
	TOgExcept::Throw(TStr::Fmt("Wrong field-type combination requested: [%d:%s]!", FieldId, TypeStr.CStr())); 
}

int TOgRec::GetFieldInt(const int& FieldId) const {
	OgAssertR(!IsByRef(), "Cannot retrieve values from records passed by reference!");
    if (FieldIdIntH.IsKey(FieldId)) {
        return FieldIdIntH.GetDat(FieldId);
    }
    FieldError(FieldId, "Int"); return -1; 
}

void TOgRec::GetFieldIntV(const int& FieldId, TIntV& IntV) const { 
	OgAssertR(!IsByRef(), "Cannot retrieve values from records passed by reference!");
    if (FieldIdTmH.IsKey(FieldId)) {
        IntV = FieldIdIntVH.GetDat(FieldId);
    } else {
		FieldError(FieldId, "IntV"); 
	}
}

uint64 TOgRec::GetFieldUInt64(const int& FieldId) const {
	OgAssertR(!IsByRef(), "Cannot retrieve values from records passed by reference!");
    if (FieldIdUInt64H.IsKey(FieldId)) {
        return FieldIdUInt64H.GetDat(FieldId);
    }
	FieldError(FieldId, "UInt64"); return TUInt64::Mx; 
}

TStr TOgRec::GetFieldStr(const int& FieldId) const { 
	OgAssertR(!IsByRef(), "Cannot retrieve values from records passed by reference!");
    if (FieldIdStrH.IsKey(FieldId)) {
        return FieldIdStrH.GetDat(FieldId);
    }
    FieldError(FieldId, "Str"); return "";
}

void TOgRec::GetFieldStrV(const int& FieldId, TStrV& StrV) const { 
	OgAssertR(!IsByRef(), "Cannot retrieve values from records passed by reference!");
    if (FieldIdStrVH.IsKey(FieldId)) {
        StrV = FieldIdStrVH.GetDat(FieldId);
    } else {
		FieldError(FieldId, "StrV"); 
	}
}

bool TOgRec::GetFieldBool(const int& FieldId) const { 
	OgAssertR(!IsByRef(), "Cannot retrieve values from records passed by reference!");
    if (FieldIdBoolH.IsKey(FieldId)) {
        return FieldIdBoolH.GetDat(FieldId);
    }
    FieldError(FieldId, "Bool"); return false; 
}

double TOgRec::GetFieldFlt(const int& FieldId) const { 
	OgAssertR(!IsByRef(), "Cannot retrieve values from records passed by reference!");
    if (FieldIdFltH.IsKey(FieldId)) {
        return FieldIdFltH.GetDat(FieldId);
    }
    FieldError(FieldId, "Flt"); return 0.0; 
}

TFltPr TOgRec::GetFieldFltPr(const int& FieldId) const { 
	OgAssertR(!IsByRef(), "Cannot retrieve values from records passed by reference!");
    if (FieldIdFltPrH.IsKey(FieldId)) {
        return FieldIdFltPrH.GetDat(FieldId);
    }
    FieldError(FieldId, "FltPr"); return TFltPr(); 
}

void TOgRec::GetFieldFltV(const int& FieldId, TFltV& FltV) const { 
	OgAssertR(!IsByRef(), "Cannot retrieve values from records passed by reference!");
    if (FieldIdTmH.IsKey(FieldId)) {
        FltV = FieldIdFltVH.GetDat(FieldId);
    } else {
		FieldError(FieldId, "FltV"); 
	}
}

void TOgRec::GetFieldTm(const int& FieldId, TTm& Tm) const { 
	OgAssertR(!IsByRef(), "Cannot retrieve values from records passed by reference!");
    if (FieldIdTmH.IsKey(FieldId)) {
        Tm = FieldIdTmH.GetDat(FieldId);
    } else {
		FieldError(FieldId, "Tm"); 
	}
}

void TOgRec::GetFieldNumSpV(const int& FieldId, TIntFltKdV& NumSpV) const { 
	OgAssertR(!IsByRef(), "Cannot retrieve values from records passed by reference!");
    if (FieldIdTmH.IsKey(FieldId)) {
        NumSpV = FieldIdNumSpVH.GetDat(FieldId);
    } else {
		FieldError(FieldId, "NumSpV"); 
	}
}

void TOgRec::GetFieldBowSpV(const int& FieldId, PBowSpV& BowSpV) const { 
	OgAssertR(!IsByRef(), "Cannot retrieve values from records passed by reference!");
    if (FieldIdTmH.IsKey(FieldId)) {
        BowSpV = FieldIdBowSpVH.GetDat(FieldId);
    } else {
		FieldError(FieldId, "NumSpV"); 
	}
}

TStr TOgRec::GetDisplayText(const POgStore& Store, const int& FieldId) const {
	OgAssertR(!IsByRef(), "Cannot retrieve display text from records passed by reference!");
    const TOgFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	if (!Desc.IsDisplayText()) { FieldError(FieldId, "IsDisplayText"); return ""; }
    if (Desc.IsInt()) {
		return TInt::GetStr(GetFieldInt(FieldId));
    } else if (Desc.IsIntV()) {
		TIntV IntV; GetFieldIntV(FieldId, IntV);
		return TStrUtil::GetStr(IntV);
    } else if (Desc.IsBool()) {
        return GetFieldBool(FieldId) ? "Yes" : "No";
    } else if (Desc.IsUInt64()) {
		return TUInt64::GetStr(GetFieldUInt64(FieldId));
	} else if (Desc.IsStr()) {
        return GetFieldStr(FieldId);
    } else if (Desc.IsStrV()) {
        TStrV FieldStrV; GetFieldStrV(FieldId, FieldStrV);
        return TStr::GetStr(FieldStrV, ", ");
    } else if (Desc.IsFlt()) {
		return TFlt::GetStr(GetFieldFlt(FieldId));
    } else if (Desc.IsFltPr()) {
        const TFltPr FieldFltPr = GetFieldFltPr(FieldId);
        return TStr::Fmt("(%g, %g)", FieldFltPr.Val1.Val, FieldFltPr.Val2.Val);
	} else if (Desc.IsFltV()) {
		TFltV FltV; GetFieldFltV(FieldId, FltV);
		return TStrUtil::GetStr(FltV);
    } else if (Desc.IsTm()) {
        TTm FieldTm; GetFieldTm(FieldId, FieldTm);		
		if (FieldTm.IsDef()) { return FieldTm.GetWebLogDateTimeStr(); } else { return "--"; }
	} else if (Desc.IsNumSpV()) {
		TIntFltKdV IntFltKdV; GetFieldNumSpV(FieldId, IntFltKdV);
		return TStrUtil::GetStr(IntFltKdV);
	} else if (Desc.IsBowSpV()) {
		return "[PBowSpV]"; //TODO
    }
	FieldError(FieldId, "GetDisplayText"); return "";	
}

TFltPr TOgRec::GetDisplayMap(const POgStore& Store, const int& FieldId) const {
	OgAssertR(!IsByRef(), "Cannot retrieve display text from records passed by reference!");
	const TOgFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	if (!Desc.IsDisplayMap()) { FieldError(FieldId, "IsDisplayMap"); return TFltPr(); }
	if (Desc.IsFltPr()) { return GetFieldFltPr(FieldId); }
	FieldError(FieldId, "GetDisplayMap"); return TFltPr();
}

POgRecSet TOgRec::ToRecSet() const {
    OgAssertR(IsByRef(), "Cannot transform record passed by value to a set!");
    return TOgRecSet::New(StoreId, RecId);
}

POgRecSet TOgRec::DoJoin(const POgBase& OgBase, const int& JoinId) const {
    if (IsByRef()) {
		Assert(OgBase->GetStoreByStoreId(GetStoreId())->IsRecId(GetRecId()));
        POgRecSet RecSet = TOgRecSet::New(GetStoreId(), GetRecId());
        return RecSet->DoJoin(OgBase, JoinId, -1, false);
    } else {
 	    const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
	    OgAssertR(Store->IsJoinId(JoinId), "Wrong Join ID");
	    const TOgJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);	
	    TUInt64V JoinRecIdV;
	     if (JoinDesc.IsIndexJoin()) {
            TOgExcept::Throw("Index join on records passed by value not implemented!");
	    } else if (JoinDesc.IsFieldJoin()) {
		    const int JoinFieldId = JoinDesc.GetJoinFieldId();
			const uint64 JoinRecId = GetFieldUInt64(JoinFieldId);
			if (JoinRecId != TUInt64::Mx) { JoinRecIdV.Add(JoinRecId); }
	    } else {
		    TOgExcept::Throw("Unsupported join type for join " + JoinDesc.GetJoinNm() + "!");
	    }
	    return TOgRecSet::New(JoinDesc.GetJoinStoreId(), JoinRecIdV);
    }
}

POgRecSet TOgRec::DoJoin(const POgBase& OgBase, const TStr& JoinNm) const {
 	const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
	if (Store->IsJoinNm(JoinNm)) {
		return DoJoin(OgBase, Store->GetJoinId(JoinNm));
	} else {
		return TOgRecSet::New();
	}
}

POgRecSet TOgRec::DoJoin(const POgBase& OgBase, const TIntPrV& JoinIdV) const {
	POgRecSet RecSet = DoJoin(OgBase, JoinIdV[0].Val1);
	for (int JoinIdN = 1; JoinIdN < JoinIdV.Len(); JoinIdN++) {
		RecSet = RecSet->DoJoin(OgBase, JoinIdV[JoinIdN].Val1, JoinIdV[JoinIdN].Val2, true);
	}
	return RecSet;
}

POgRecSet TOgRec::DoJoin(const POgBase& OgBase, const TOgJoinSeq& JoinSeq) const {
	return DoJoin(OgBase, JoinSeq.GetJoinIdV());
}

TOgRec TOgRec::DoSingleJoin(const POgBase& OgBase, const int& JoinId) const {
	POgRecSet JoinRecSet = DoJoin(OgBase, JoinId);
	return TOgRec(JoinRecSet->GetStoreId(), 
		JoinRecSet->Empty() ? (uint64)TUInt64::Mx : JoinRecSet->GetRecId(0));
}

TOgRec TOgRec::DoSingleJoin(const POgBase& OgBase, const TStr& JoinNm) const {
	POgRecSet JoinRecSet = DoJoin(OgBase, JoinNm);
	return TOgRec(JoinRecSet->GetStoreId(), 
		JoinRecSet->Empty() ? (uint64)TUInt64::Mx : JoinRecSet->GetRecId(0));
}

TOgRec TOgRec::DoSingleJoin(const POgBase& OgBase, const TIntPrV& JoinIdV) const {
	POgRecSet JoinRecSet = DoJoin(OgBase, JoinIdV);
	return TOgRec(JoinRecSet->GetStoreId(), 
		JoinRecSet->Empty() ? (uint64)TUInt64::Mx : JoinRecSet->GetRecId(0));
}

TOgRec TOgRec::DoSingleJoin(const POgBase& OgBase, const TOgJoinSeq& JoinSeq) const {
	POgRecSet JoinRecSet = DoJoin(OgBase, JoinSeq);
	return TOgRec(JoinRecSet->GetStoreId(), 
		JoinRecSet->Empty() ? (uint64)TUInt64::Mx : JoinRecSet->GetRecId(0));
}

PXmlTok TOgRec::GetXmlTok(const POgBase& OgBase, const bool& FieldsP, const bool& StoreInfoP) const {
	POgStore Store = OgBase->GetStoreByStoreId(GetStoreId());
	PXmlTok RecTok = TXmlTok::New("record");
	if (StoreInfoP) {
		RecTok->AddArg("storeid", int(Store->GetStoreId()));
		RecTok->AddArg("storename", Store->GetStoreNm());
	}
	if (ByRefP) {
		RecTok->AddArg("id", RecId);
		RecTok->AddArg("name", Store->GetRecNm(RecId));
	}
	if (!FieldsP) { return RecTok; }
	const int Fields = Store->GetFields();
	for (int FieldN = 0; FieldN < Fields; FieldN++) {
		const TOgFieldDesc& Desc = Store->GetFieldDesc(FieldN);
		if (Desc.HasDisplay()) { 
			PXmlTok FieldTok = TXmlTok::New("field");
			FieldTok->AddArg("id", Desc.GetFieldId());
			FieldTok->AddArg("name", Desc.GetFieldNm());
			const int FieldId = Desc.GetFieldId();
			if (Desc.IsDisplayText()) {
				TStr DisplayText = IsByRef() ? 
					Store->GetDisplayText(RecId, FieldId) : GetDisplayText(Store, FieldId);
				FieldTok->AddArg("text", DisplayText);			
			} else if (Desc.IsDisplayMap()) {
				TFltPr FltPr = IsByRef() ?
					Store->GetDisplayMap(RecId, FieldId) : GetDisplayMap(Store, FieldId);
				FieldTok->AddArg("longitude", FltPr.Val1);
				FieldTok->AddArg("latitude", FltPr.Val2);
			}
			RecTok->AddSubTok(FieldTok);
		} 
	}
	const int Joins = Store->GetJoins();
	for (int JoinId = 0; JoinId < Joins; JoinId++) {
		const TOgJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
		if (JoinDesc.IsFieldJoin()) { 
			const int JoinFieldId = JoinDesc.GetJoinFieldId();
			const uint64 JoinRecId = IsByRef() ?
				Store->GetFieldUInt64(RecId, JoinFieldId) :	GetFieldUInt64(JoinFieldId);
			if (JoinRecId != -1) {
				const POgStore& JoinStore = OgBase->GetStoreByStoreId(JoinDesc.GetJoinStoreId());
				PXmlTok JoinFieldTok = TXmlTok::New("join-field");
				JoinFieldTok->AddArg("id", JoinFieldId);
				JoinFieldTok->AddArg("name", Store->GetFieldNm(JoinFieldId));
				JoinFieldTok->AddArg("recid", JoinRecId);
				JoinFieldTok->AddArg("recnm", JoinStore->GetRecNm(JoinRecId));
				JoinFieldTok->AddArg("storeid", JoinStore->GetStoreId());
				JoinFieldTok->AddArg("storename", JoinStore->GetStoreNm());
				RecTok->AddSubTok(JoinFieldTok);
			}
		}
	}
	return RecTok;
}

PXmlDoc TOgRec::SaveXml(const POgBase& OgBase, const bool& FieldsP, const bool& StoreInfoP) const {
	return TXmlDoc::New(GetXmlTok(OgBase, FieldsP, StoreInfoP));
}

PJsonVal TOgRec::SaveJson(const POgBase& OgBase, const bool& FieldsP, const bool& StoreInfoP) const {
	POgStore Store = OgBase->GetStoreByStoreId(GetStoreId());
	PJsonVal RecVal = TJsonVal::NewObj();
	if (StoreInfoP) {
		RecVal->AddToObj("storeId", int(Store->GetStoreId()));
		RecVal->AddToObj("storeName", Store->GetStoreNm());
	}
	if (ByRefP) {
		RecVal->AddToObj("recId", (int)RecId);
		RecVal->AddToObj("recName", Store->GetRecNm(RecId));
	}
	if (!FieldsP) { return RecVal; }
	const int Fields = Store->GetFields(); TJsonValV FieldValV;
	for (int FieldN = 0; FieldN < Fields; FieldN++) {
		const TOgFieldDesc& Desc = Store->GetFieldDesc(FieldN);
		if (Desc.HasDisplay()) { 
			PJsonVal FieldVal = TJsonVal::NewObj();
			FieldVal->AddToObj("fieldId", Desc.GetFieldId());
			FieldVal->AddToObj("fieldName", Desc.GetFieldNm());
			const int FieldId = Desc.GetFieldId();
			if (Desc.IsDisplayText()) {
				TStr DisplayText = IsByRef() ? 
					Store->GetDisplayText(RecId, FieldId) : GetDisplayText(Store, FieldId);
				FieldVal->AddToObj("text", DisplayText);			
			} else if (Desc.IsDisplayMap()) {
				TFltPr FltPr = IsByRef() ?
					Store->GetDisplayMap(RecId, FieldId) : GetDisplayMap(Store, FieldId);
				FieldVal->AddToObj("longitude", FltPr.Val1);
				FieldVal->AddToObj("latitude", FltPr.Val2);
			}
			FieldValV.Add(FieldVal);
		} 
	}
	RecVal->AddToObj("fields", TJsonVal::NewArr(FieldValV));
	const int Joins = Store->GetJoins(); TJsonValV JoinValV;
	for (int JoinId = 0; JoinId < Joins; JoinId++) {
		const TOgJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
		if (JoinDesc.IsFieldJoin()) { 
			const int JoinFieldId = JoinDesc.GetJoinFieldId();
			const uint64 JoinRecId = IsByRef() ?
				Store->GetFieldUInt64(RecId, JoinFieldId) :	GetFieldUInt64(JoinFieldId);
			if (JoinRecId != -1) {
				const POgStore& JoinStore = OgBase->GetStoreByStoreId(JoinDesc.GetJoinStoreId());
				PJsonVal JoinFieldVal = TJsonVal::NewObj();
				JoinFieldVal->AddToObj("fieldId", JoinFieldId);
				JoinFieldVal->AddToObj("fieldName", Store->GetFieldNm(JoinFieldId));
				JoinFieldVal->AddToObj("recId", (int)JoinRecId);
				JoinFieldVal->AddToObj("recName", JoinStore->GetRecNm(JoinRecId));
				JoinFieldVal->AddToObj("storeId", JoinStore->GetStoreId());
				JoinFieldVal->AddToObj("storeName", JoinStore->GetStoreNm());
				JoinValV.Add(JoinFieldVal);
			}
		}
	}
	RecVal->AddToObj("joinFields", TJsonVal::NewArr(JoinValV));
	return RecVal;
}

void TOgRecSet::GetSampleRecIdV(const int& SampleSize, 
		const bool& SortedP, TUInt64IntKdV& SampleRecIdFqV) const {

	if (SampleSize == -1) {
		SampleRecIdFqV = RecIdFqV;
	} else if (SortedP) { 
		const int SampleRecs = TInt::GetMn(SampleSize, GetRecs());
		SampleRecIdFqV.Gen(SampleRecs, 0);
		for (int RecN = 0; RecN < SampleRecs; RecN++) {
			SampleRecIdFqV.Add(RecIdFqV[RecN]);
		}
	} else {
		for (int RecN = 0; RecN < GetRecs(); RecN++) {
			SampleRecIdFqV.Add(RecIdFqV[RecN]); 
		}
		if (SampleSize < GetRecs()) { 
			TRnd Rnd(1); SampleRecIdFqV.Shuffle(Rnd); 
			SampleRecIdFqV.Trunc(SampleSize); 
		}
	}
}

void TOgRecSet::LimitToSampleRecIdV(const TUInt64IntKdV& SampleRecIdFqV) {
	RecIdFqV = SampleRecIdFqV;
}

TOgRecSet::TOgRecSet(const uchar& _StoreId, const TUInt64V& RecIdV): 
		StoreId(_StoreId), WgtP(false) { 

    RecIdFqV.Gen(RecIdV.Len(), 0);
    for (int RecN = 0; RecN < RecIdV.Len(); RecN++) {
        RecIdFqV.Add(TUInt64IntKd(RecIdV[RecN], 0));
    }
}

TOgRecSet::TOgRecSet(const uchar& _StoreId, const POgStore& OgStore): 
		StoreId(_StoreId), WgtP(false) {

	RecIdFqV.Gen((int)OgStore->GetRecs(), 0);
	POgStoreIter Iter = OgStore->GetIter();
	while(Iter->Next()) {
		RecIdFqV.Add(TUInt64IntKd(Iter->GetRecId(), 0));
	}
}

TOgRecSet::TOgRecSet(TSIn& SIn) {
	StoreId = TUCh(SIn);
	WgtP.Load(SIn);
	RecIdFqV.Load(SIn);	
}

void TOgRecSet::Save(TSOut& SOut) {
	StoreId.Save(SOut);
	WgtP.Save(SOut);
	RecIdFqV.Save(SOut);	
}

void TOgRecSet::GetRecIdV(TUInt64V& RecIdV) const {
    const int Recs = GetRecs();
    RecIdV.Gen(Recs, 0);
    for (int RecN = 0; RecN < Recs; RecN++) {
        RecIdV.Add(GetRecId(RecN));
    }
}

void TOgRecSet::GetRecIdSet(THashSet<TUInt64>& RecIdSet) const {
    const int Recs = GetRecs();
    RecIdSet.Gen(Recs);
    for (int RecN = 0; RecN < Recs; RecN++) {
        RecIdSet.AddKey(GetRecId(RecN));
    }
}

void TOgRecSet::GetRecIdFqH(THash<TUInt64, TInt>& RecIdFqH) const {
    const int Recs = GetRecs();
    RecIdFqH.Gen(Recs);
    for (int RecN = 0; RecN < Recs; RecN++) {
        RecIdFqH.AddDat(GetRecId(RecN), GetRecFq(RecN));
    }
}

void TOgRecSet::PutAllRecFq(const THash<TUInt64, TInt>& RecIdFqH) {
    const int Recs = GetRecs();
    for (int RecN = 0; RecN < Recs; RecN++) {
		const uint64 RecId = GetRecId(RecN);
		if (RecIdFqH.IsKey(RecId)) {
			PutRecFq(RecN, RecIdFqH.GetDat(RecId)); 
		} else {
			PutRecFq(RecN, 0);
		}
	}
}

void TOgRecSet::SortByFq(const bool& Asc) {
	RecIdFqV.SortCmp(TCmpKeyDatByDat<TUInt64, TInt>(Asc));
}

void TOgRecSet::SortByField(const POgBase& OgBase, 
		const bool& Asc, const int& SortFieldId) {

	const int Recs = GetRecs();
	TUInt64IntKdV NewRecIdFqV(Recs, 0);
	const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
	const TOgFieldDesc& Desc = Store->GetFieldDesc(SortFieldId);
	if (Desc.IsInt() || Desc.IsTm()) {
		TVec<TKeyDat<TInt, TInt> > RecWgtIdV(Recs, 0);
		for (int RecN = 0; RecN < Recs; RecN++) {
			const uint64 RecId = GetRecId(RecN);
			int FldVal = 0;
			if (Desc.IsInt()) {
				FldVal = Store->GetFieldInt(RecId, SortFieldId);
			} else if (Desc.IsTm()) {
				TTm Tm; Store->GetFieldTm(RecId, SortFieldId, Tm);
				FldVal = Tm.IsDef() ? TTm::GetDateTimeIntFromTm(Tm) : 0;
			}
			RecWgtIdV.Add(TKeyDat<TInt, TInt>(FldVal, RecN));
		}
		RecWgtIdV.Sort(Asc);
		for (int RecWgtIdN = 0; RecWgtIdN < RecWgtIdV.Len(); RecWgtIdN++) {
			const int RecN = RecWgtIdV[RecWgtIdN].Dat;
			const uint64 RecId = GetRecId(RecN);
			const int RecFq = GetRecFq(RecN);
			NewRecIdFqV.Add(TUInt64IntKd(RecId, RecFq));
		}
	} else if (Desc.IsFlt()) {
		TVec<TKeyDat<TFlt, TInt> > RecWgtIdV(Recs, 0);
		for (int RecN = 0; RecN < Recs; RecN++) {
			const uint64 RecId = GetRecId(RecN);
			if (RecN == -1) { continue; }
			TFlt FldVal = 0;
			if (Desc.IsFlt()) {
				FldVal = Store->GetFieldFlt(RecId, SortFieldId);
			} 
			RecWgtIdV.Add(TKeyDat<TFlt, TInt>(FldVal, RecN));
		}
		RecWgtIdV.Sort(Asc);
		for (int RecWgtIdN = 0; RecWgtIdN < RecWgtIdV.Len(); RecWgtIdN++) {
			const int RecN = RecWgtIdV[RecWgtIdN].Dat;
			const uint64 RecId = GetRecId(RecN);
			const int RecFq = GetRecFq(RecN);
			NewRecIdFqV.Add(TUInt64IntKd(RecId, RecFq));
		}
	} else {
		TOgExcept::Throw("Unsupported sort field type!");
	}
	RecIdFqV = NewRecIdFqV;
}

void TOgRecSet::FilterByRecId(const uint64& MinRecId, const uint64& MaxRecId) {
	const int Recs = GetRecs();
	TUInt64IntKdV NewRecIdFqV(Recs, 0);
	for (int RecN = 0; RecN < Recs; RecN++) {
		const uint64 RecId = GetRecId(RecN);
		if (RecId < MinRecId) continue;
		if (RecId > MaxRecId) continue;
		int RecFq = GetRecFq(RecN);
		NewRecIdFqV.Add(TUInt64IntKd(RecId, RecFq));
	}
	RecIdFqV = NewRecIdFqV;
}

void TOgRecSet::FilterByFq(const int& MinFq, const int& MaxFq) {
	const int Recs = GetRecs();
	TUInt64IntKdV NewRecIdFqV(Recs, 0);
	for (int RecN = 0; RecN < Recs; RecN++) {
		const uint64 RecId = GetRecId(RecN);
		int RecFq = GetRecFq(RecN);
		if (MinFq != -1 && RecFq < MinFq) { continue; }
		if (MaxFq != -1 && RecFq > MaxFq) { continue; }
		NewRecIdFqV.Add(TUInt64IntKd(RecId, RecFq));
	}
	RecIdFqV = NewRecIdFqV;
}

void TOgRecSet::FilterByIntField(const POgBase& OgBase,
		const int& FieldId, const int& MinVal, const int& MaxVal) {

	const int Recs = GetRecs();
	TUInt64IntKdV NewRecIdFqV(Recs, 0);
	const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
	const TOgFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	if (Desc.IsInt()) {
		for (int RecN = 0; RecN < Recs; RecN++) {
			const uint64 RecId = GetRecId(RecN);
			int Val = Store->GetFieldInt(RecId, FieldId);
			if (Val >= MinVal && Val <= MaxVal)
				NewRecIdFqV.Add(TUInt64IntKd(RecId, GetRecFq(RecN)));
		}
	} else {
		TOgExcept::Throw("Unsupported field type!");
	}
	RecIdFqV = NewRecIdFqV;
}

void TOgRecSet::FilterByTmField(const POgBase& OgBase,
		const int& FieldId, const uint64& MinVal, const uint64& MaxVal) {

	const int Recs = GetRecs();
	TUInt64IntKdV NewRecIdFqV(Recs, 0);
	const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
	const TOgFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	if (Desc.IsTm()) {
		for (int RecN = 0; RecN < Recs; RecN++) {
			const uint64 RecId = GetRecId(RecN);
			uint64 Val = Store->GetFieldUInt64(RecId, FieldId);
			if (Val >= MinVal && Val <= MaxVal) {
				NewRecIdFqV.Add(TUInt64IntKd(RecId, GetRecFq(RecN)));
			}
		}
	} else { 
		TOgExcept::Throw("Unsupported field type!"); 
	}
	// overwrite old result vector
	RecIdFqV = NewRecIdFqV;
}

void TOgRecSet::FilterByTmField(const POgBase& OgBase, 
		const int& FieldId, const TTm& MinVal, const TTm& MaxVal) {

	FilterByTmField(OgBase, FieldId,
		MinVal.IsDef() ? TTm::GetMSecsFromTm(MinVal) : (uint64)TUInt64::Mn,
		MaxVal.IsDef() ? TTm::GetMSecsFromTm(MaxVal) : (uint64)TUInt64::Mx);
}

void TOgRecSet::RemoveRecId(const TUInt64& RecId) {
	const int Recs = GetRecs();
	for (int RecN = 0; RecN < Recs; RecN++) {
		if (GetRecId(RecN) == RecId) {
			RecIdFqV.Del(RecN);
			return;
		}
	}
}

void TOgRecSet::RemoveRecIdSet(THashSet<TUInt64>& RemoveItemIdH) {
	const int Recs = GetRecs();
	TUInt64IntKdV NewRecIdFqV(Recs, 0);	
	for (int RecN = 0; RecN < Recs; RecN++) {
		uint64 RecId = GetRecId(RecN);
		if (!RemoveItemIdH.IsKey(RecId)) {
			NewRecIdFqV.Add(TUInt64IntKd(RecId, GetRecFq(RecN)));
		}
	}	
	// overwrite old result vector
	RecIdFqV = NewRecIdFqV;
}

POgRecSet TOgRecSet::Clone() const {
    return TOgRecSet::New(StoreId, RecIdFqV, WgtP);
}

POgRecSet TOgRecSet::GetSampleRecSet(const int& SampleSize, const bool& SortedP) const {
	TUInt64IntKdV SampleRecIdFqV;
	GetSampleRecIdV(SampleSize, SortedP, SampleRecIdFqV);
	return TOgRecSet::New(StoreId, SampleRecIdFqV, WgtP);
}

POgRecSet TOgRecSet::GetLimit(const int& Limit, const int& Offset) const {
	if (Offset >= GetRecs()) {
		return TOgRecSet::New(StoreId);
	} else {
		TUInt64IntKdV LimitRecIdFqV;
		if (Limit == -1) {
			RecIdFqV.GetSubValV(Offset, GetRecs() - 1, LimitRecIdFqV);
		} else {
			const int End = TInt::GetMn(Offset + Limit, GetRecs()) - 1;
			RecIdFqV.GetSubValV(Offset, End, LimitRecIdFqV);
		}
		return TOgRecSet::New(StoreId, LimitRecIdFqV, WgtP);
	}
}

POgRecSet TOgRecSet::GetMerge(const POgRecSet& RecSet) const {
	POgRecSet CloneRecSet = Clone();
	CloneRecSet->Merge(RecSet);
	return CloneRecSet;
}

void TOgRecSet::Merge(const POgRecSet& RecSet) {
	TUInt64IntKdV MergeRecIdFqV = RecSet->GetRecIdFqV(); 
	if (!MergeRecIdFqV.IsSorted()) { MergeRecIdFqV.Sort(); }
	if (!RecIdFqV.IsSorted()) { RecIdFqV.Sort(); }
	RecIdFqV.Union(MergeRecIdFqV);
}

void TOgRecSet::Merge(const TVec<POgRecSet>& RecSetV) {
	for (int RsIdx = 0; RecSetV.Len(); RsIdx++){
		Merge(RecSetV[RsIdx]);
	}
}
POgRecSet TOgRecSet::GetIntersect(const POgRecSet& RecSet) {
	TUInt64IntKdV TargetRecIdFqV = RecSet->GetRecIdFqV();
	if (!TargetRecIdFqV.IsSorted()) { TargetRecIdFqV.Sort(); }
	if (!RecIdFqV.IsSorted()) { RecIdFqV.Sort(); }
	TUInt64IntKdV ResultRecIdFqV;
	TargetRecIdFqV.Intrs(RecIdFqV, ResultRecIdFqV);
	return TOgRecSet::New(RecSet->GetStoreId(), ResultRecIdFqV, false);
}

POgRecSet TOgRecSet::DoJoin(const POgBase& OgBase, 
		const int& JoinId, const int& SampleSize, const bool& SortedP) const {

	const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
	OgAssertR(Store->IsJoinId(JoinId), "Wrong Join ID");
	const TOgJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);	
	TUInt64IntKdV SampleRecIdKdV;
	GetSampleRecIdV(SampleSize, SortedP, SampleRecIdKdV);
	const int SampleRecs = SampleRecIdKdV.Len();
	TUInt64IntKdV JoinRecIdFqV;
	if (JoinDesc.IsIndexJoin()) {
		const int JoinKeyId = JoinDesc.GetJoinKeyId();
		TIntUInt64PrV JoinQueryV;	
		for (int RecN = 0; RecN < SampleRecs; RecN++) {
			const uint64 RecId = SampleRecIdKdV[RecN].Key;
			JoinQueryV.Add(TIntUInt64Pr(JoinKeyId, RecId));
		}
		const POgStore& Store = OgBase->GetStoreByStoreId(JoinDesc.GetJoinStoreId());
		OgBase->GetIndex()->SearchOr(JoinQueryV, JoinRecIdFqV);
	} else if (JoinDesc.IsFieldJoin()) {
		TUInt64H JoinRecIdFqH;
		const int JoinFieldId = JoinDesc.GetJoinFieldId();
		for (int RecN = 0; RecN < SampleRecs; RecN++) {
			const uint64 RecId = SampleRecIdKdV[RecN].Key;
			const uint64 JoinRecId = Store->GetFieldUInt64(RecId, JoinFieldId);
			if (JoinRecId != TUInt64::Mx) { JoinRecIdFqH.AddDat(JoinRecId)++; }
		}
		JoinRecIdFqH.GetKeyDatKdV(JoinRecIdFqV);
	} else {
		TOgExcept::Throw("Unsupported join type for join " + JoinDesc.GetJoinNm() + "!");
	}
	return TOgRecSet::New(JoinDesc.GetJoinStoreId(), JoinRecIdFqV, true);
}

POgRecSet TOgRecSet::DoJoin(const POgBase& OgBase, const TStr& JoinNm, 
		const int& SampleSize, const bool& SortedP) const {

 	const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
	if (Store->IsJoinNm(JoinNm)) {
		return DoJoin(OgBase, Store->GetJoinId(JoinNm), SampleSize, SortedP);
	} else {
		return TOgRecSet::New();
	}
}

POgRecSet TOgRecSet::DoJoin(const POgBase& OgBase, const TIntPrV& JoinIdV, const bool& SortedP) const {
	POgRecSet RecSet = DoJoin(OgBase, JoinIdV[0].Val1, JoinIdV[0].Val2, SortedP);
	for (int JoinIdN = 1; JoinIdN < JoinIdV.Len(); JoinIdN++) {
		RecSet = RecSet->DoJoin(OgBase, JoinIdV[JoinIdN].Val1, JoinIdV[JoinIdN].Val2, true);
	}
	return RecSet;
}

POgRecSet TOgRecSet::DoJoin(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const bool& SortedP) const {
	return DoJoin(OgBase, JoinSeq.GetJoinIdV(), SortedP);
}

TStr TOgRecSet::GetJoinPathStr(const POgBase& OgBase, const POgStore& StartStore, const TIntPrV& JoinIdV) {
	TStr JoinPathStr;
	uchar LastStoreId = StartStore->GetStoreId();
	for (int JoinIdN = 0; JoinIdN < JoinIdV.Len(); JoinIdN++) {
		POgStore Store = OgBase->GetStoreByStoreId(LastStoreId);
		const TOgJoinDesc& JoinDesc = Store->GetJoinDesc(JoinIdV[JoinIdN].Val1);
		if (!JoinPathStr.Empty()) { JoinPathStr += " -> "; }
		JoinPathStr += JoinDesc.GetJoinNm();
		LastStoreId = JoinDesc.GetJoinStoreId();
	}
	return JoinPathStr;
}

void TOgRecSet::Print(const POgBase& OgBase, TSOut& SOut) {
	OgBase->GetStoreByStoreId(GetStoreId())->PrintRecSet(OgBase, this, SOut);
}

void TOgRecSet::Print(const POgBase& OgBase, TStr& FNm) {
	OgBase->GetStoreByStoreId(GetStoreId())->PrintRecSet(OgBase, this, FNm);
}

PXmlDoc TOgRecSet::SaveXml(const POgBase& OgBase, const int& _MxHits, 
		const int& Offset, const bool& FieldsP) const {

	const int MxHits = (_MxHits == -1) ? GetRecs() : _MxHits;
	POgStore Store = OgBase->GetStoreByStoreId(GetStoreId());
	PXmlTok RecSetTok = TXmlTok::New("record-set");
	RecSetTok->AddArg("storeid", int(Store->GetStoreId()));
	RecSetTok->AddArg("storename", Store->GetStoreNm());
	RecSetTok->AddArg("weighted", IsWgt());
	const int Recs = GetRecs();
	RecSetTok->AddArg("hits", Recs);
	int Hits = 0;
	for (int RecN = Offset; RecN < Recs; RecN++) {
		Hits++; if (Hits > MxHits) { break; }
		PXmlTok RecTok = GetRec(RecN).GetXmlTok(OgBase, FieldsP, false);
		RecSetTok->AddSubTok(RecTok);
	}
	PXmlTok AggrTok = TXmlTok::New("aggergate");
	for (int AggrN = 0; AggrN < AggrV.Len(); AggrN++) {
		AggrTok->AddSubTok(AggrV[AggrN]->SaveXml());
	}
	RecSetTok->AddSubTok(AggrTok);
	return TXmlDoc::New(RecSetTok);
}

PJsonVal TOgRecSet::SaveJson(const POgBase& OgBase, const int& _MxHits, 
		const int& Offset, const bool& FieldsP) const {

	const int MxHits = (_MxHits == -1) ? GetRecs() : _MxHits;
	POgStore Store = OgBase->GetStoreByStoreId(GetStoreId());
	// export result set as XML
	PJsonVal RecSetVal = TJsonVal::NewObj();
	RecSetVal->AddToObj("storeId", int(Store->GetStoreId()));
	RecSetVal->AddToObj("storeName", Store->GetStoreNm());
	RecSetVal->AddToObj("weighted", IsWgt());
	const int Recs = GetRecs();
	RecSetVal->AddToObj("hits", Recs);
	int Hits = 0; TJsonValV RecValV;
	for (int RecN = Offset; RecN < Recs; RecN++) {
		Hits++; if (Hits > MxHits) { break; }
		PJsonVal RecVal = GetRec(RecN).SaveJson(OgBase, FieldsP, false);
		RecValV.Add(RecVal);
	}
	RecSetVal->AddToObj("records", TJsonVal::NewArr(RecValV));
	TJsonValV AggrValV;
	for (int AggrN = 0; AggrN < AggrV.Len(); AggrN++) {
		AggrValV.Add(AggrV[AggrN]->SaveJson());
	}
	RecSetVal->AddToObj("aggergates", TJsonVal::NewArr(AggrValV));
	return RecSetVal;
}

TOgIndexKey::TOgIndexKey(const uchar& _StoreId, const TStr& _KeyNm, const int& _WordVocId, 
	const bool& _TextP, const bool& _AggrP, const TOgIndexKeySortType& _SortType): StoreId(_StoreId), 
		KeyNm(_KeyNm), WordVocId(_WordVocId), TextP(_TextP), AggrP(_AggrP), InternalP(false), 
		SortType(_SortType) { OgAssert(WordVocId >= 0); TOg::AssertValidNm(KeyNm );}
	
TOgIndexKey::TOgIndexKey(TSIn& SIn): StoreId(SIn), KeyId(SIn), KeyNm(SIn), WordVocId(SIn), TextP(SIn), 
	AggrP(SIn), InternalP(SIn), FieldIdV(SIn) { SortType = (TOgIndexKeySortType)TInt(SIn).Val; }

void TOgIndexKey::Save(TSOut& SOut) const {
	StoreId.Save(SOut);
	KeyId.Save(SOut); KeyNm.Save(SOut);
	WordVocId.Save(SOut); TextP.Save(SOut);
	AggrP.Save(SOut); InternalP.Save(SOut);
	FieldIdV.Save(SOut);
	TInt((int)SortType).Save(SOut);
}

uint64 TOgIndexWordVoc::AddWordStr(const TStr& WordStr) {
	const int WordId = WordH.AddKey(WordStr); 
	WordH[WordId]++;
	return (uint64)WordId;
}

void TOgIndexWordVoc::GetAllGreaterById(const uint64& StartWordId, TUInt64V& AllGreaterV) {
    AllGreaterV.Clr();
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        if (WordId > StartWordId) { 
			AllGreaterV.Add((uint64)WordId); 
		}
    }
}

void TOgIndexWordVoc::GetAllGreaterByStr(const uint64& StartWordId, TUInt64V& AllGreaterV) {
    AllGreaterV.Clr();
    TStr StartWordStr = WordH.GetKey((int)StartWordId);
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        TStr WordStr = WordH.GetKey(WordId);
        if (WordStr > StartWordStr) { 
			AllGreaterV.Add((uint64)WordId); 
		}
    }
}

void TOgIndexWordVoc::GetAllGreaterByFlt(const uint64& StartWordId, TUInt64V& AllGreaterV) {
    AllGreaterV.Clr();
    TStr StartWordStr = WordH.GetKey((int)StartWordId);
	TFlt StartWordFlt = StartWordStr.GetFlt();	
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        TStr WordStr = WordH.GetKey(WordId);
		TFlt WordFlt = WordStr.GetFlt();
        if (WordFlt > StartWordFlt) { 
			AllGreaterV.Add((uint64)WordId); 
		}
    }
}


void TOgIndexWordVoc::GetAllLessById(const uint64& StartWordId, TUInt64V& AllLessV) {
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        if (WordId < StartWordId) {
			AllLessV.Add((uint64)WordId); 
		}
    }
}

void TOgIndexWordVoc::GetAllLessByStr(const uint64& StartWordId, TUInt64V& AllLessV) {
    TStr StartWordStr = WordH.GetKey((int)StartWordId);
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        TStr WordStr = WordH.GetKey(WordId);
        if (WordStr < StartWordStr) {
			AllLessV.Add((uint64)WordId);
		}
    }
}

void TOgIndexWordVoc::GetAllLessByFlt(const uint64& StartWordId, TUInt64V& AllLessV) {
    TStr StartWordStr = WordH.GetKey((int)StartWordId);
	TFlt StartWordFlt = StartWordStr.GetFlt();	
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        TStr WordStr = WordH.GetKey(WordId);
		TFlt WordFlt = WordStr.GetFlt();
        if (WordFlt < StartWordFlt) {
			AllLessV.Add((uint64)WordId);
		}
    }
}

POgIndexWordVoc& TOgIndexVoc::GetWordVoc(const int& KeyId) { 
	return WordVocV[KeyH[KeyId].GetWordVocId()];
}

const POgIndexWordVoc& TOgIndexVoc::GetWordVoc(const int& KeyId) const { 
	return WordVocV[KeyH[KeyId].GetWordVocId()];
}

TOgIndexVoc::TOgIndexVoc(TSIn& SIn) {	
	Tokenizer = TTokenizerHtml::New();
    KeyH.Load(SIn);
    StoreIdKeyIdSetH.Load(SIn);
    WordVocV.Load(SIn);
}

void TOgIndexVoc::Save(TSOut& SOut) const {	
    KeyH.Save(SOut);
    StoreIdKeyIdSetH.Save(SOut);
    WordVocV.Save(SOut);
}
	
bool TOgIndexVoc::IsKeyId(const int& KeyId) const { 	
    return KeyH.IsKeyId(KeyId); 
}

bool TOgIndexVoc::IsKeyNm(const uchar& StoreId, const TStr& KeyNm) const { 	
	return KeyH.IsKey(TUChStrPr(StoreId, KeyNm)); 
}

int TOgIndexVoc::GetKeyId(const uchar& StoreId, const TStr& KeyNm) const { 
	OgAssertR(IsKeyNm(StoreId, KeyNm), TStr::Fmt("Unknown key '%s' for store %d", KeyNm.CStr(), (int)StoreId));
    return KeyH.GetKeyId(TUChStrPr(StoreId, KeyNm)); 
}

uchar TOgIndexVoc::GetKeyStoreId(const int& KeyId) const {
	return KeyH.GetKey(KeyId).Val1; 
}

TStr TOgIndexVoc::GetKeyNm(const int& KeyId) const { 
	return KeyH.GetKey(KeyId).Val2; 
}

const TOgIndexKey& TOgIndexVoc::GetKey(const int& KeyId) const {
	return KeyH[KeyId];
}

const TOgIndexKey& TOgIndexVoc::GetKey(const uchar& StoreId, const TStr& KeyNm) const {
	return KeyH.GetDat(TUChStrPr(StoreId, KeyNm));
}

int TOgIndexVoc::AddKey(const uchar& StoreId, const TStr& KeyNm, const int& WordVocId, 
		const bool& TextP, const bool& AggrP, const TOgIndexKeySortType& SortType) {
	
	const int KeyId = KeyH.AddKey(TUChStrPr(StoreId, KeyNm));
	KeyH[KeyId] = TOgIndexKey(StoreId, KeyNm, WordVocId, TextP, AggrP, SortType);
	KeyH[KeyId].PutKeyId(KeyId);
	StoreIdKeyIdSetH.AddDat(StoreId).AddKey(KeyId);
	return KeyId;
}

int TOgIndexVoc::AddInternalKey(const uchar& StoreId, const TStr& KeyNm, const TStr& JoinNm) {
	const int KeyId = KeyH.AddKey(TUChStrPr(StoreId, KeyNm));
	KeyH[KeyId] = TOgIndexKey(StoreId, KeyNm, JoinNm);
	KeyH[KeyId].PutKeyId(KeyId);
	return KeyId;
}

void TOgIndexVoc::AddKeyField(const int& KeyId, const uchar& StoreId, const int& FieldId) {
	OgAssert(StoreId == KeyH[KeyId].GetStoreId());
	KeyH[KeyId].AddField(FieldId);
}

bool TOgIndexVoc::IsStoreKeys(const uchar& StoreId) const {
    return StoreIdKeyIdSetH.IsKey(StoreId);
}

const TIntSet& TOgIndexVoc::GetStoreKeys(const uchar& StoreId) const {
	if (StoreIdKeyIdSetH.IsKey(StoreId)) {
		return StoreIdKeyIdSetH.GetDat(StoreId);
	} 
	return EmptySet;
}

bool TOgIndexVoc::IsWordVoc(const int& KeyId) const {
	return KeyH[KeyId].GetWordVocId() != -1;
}

bool TOgIndexVoc::IsWordStr(const int& KeyId, const TStr& WordStr) const {
    return GetWordVoc(KeyId)->IsWordStr(WordStr);
}

uint64 TOgIndexVoc::GetWords(const int& KeyId) const { 
    return GetWordVoc(KeyId)->GetWords(); 
}

void TOgIndexVoc::GetAllWordStrV(const int& KeyId, TStrV& WordStrV) const { 
	GetWordVoc(KeyId)->GetAllWordV(WordStrV); 
}

void TOgIndexVoc::GetAllWordStrFqV(const int& KeyId, TStrIntPrV& WordStrFqV) const {
	GetWordVoc(KeyId)->GetAllWordFqV(WordStrFqV); 
}

TStr TOgIndexVoc::GetWordStr(const int& KeyId, const uint64& WordId) const { 
	return GetWordVoc(KeyId)->GetWordStr(WordId); 
}

uint64 TOgIndexVoc::GetWordFq(const int& KeyId, const uint64& WordId) const { 
	return GetWordVoc(KeyId)->GetWordFq(WordId); 
}

uint64 TOgIndexVoc::GetWordId(const int& KeyId, const TStr& WordStr) const {
	return GetWordVoc(KeyId)->GetWordId(WordStr);
}

void TOgIndexVoc::GetWordIdV(const int& KeyId, const TStr& TextStr, TUInt64V& WordIdV) const {
	OgAssert(IsWordVoc(KeyId));
	TStrV TokV; Tokenizer->GetTokens(TextStr, TokV);
	WordIdV.Clr(); const POgIndexWordVoc& WordVoc = GetWordVoc(KeyId);
	for(int i = 0; i < TokV.Len(); i++) {
		const TStr& Tok = TokV[i];
		if(IsWordStr(KeyId, Tok)) {
			WordIdV.Add(GetWordId(KeyId, Tok));
		} else {
			WordIdV.Add(TUInt64::Mx);
		}
	}
}

uint64 TOgIndexVoc::AddWordStr(const int& KeyId, const TStr& WordStr) {
	return GetWordVoc(KeyId)->AddWordStr(WordStr);
}

void TOgIndexVoc::AddWordIdV(const int& KeyId, const TStr& TextStr, TUInt64V& WordIdV) {
	OgAssert(IsWordVoc(KeyId));
	TStrV TokV; Tokenizer->GetTokens(TextStr, TokV);
	WordIdV.Clr(); const POgIndexWordVoc& WordVoc = GetWordVoc(KeyId);
	for(int i = 0; i < TokV.Len(); i++) {
		WordIdV.Add(WordVoc->AddWordStr(TokV[i]));
	}
	WordVoc->IncRecs();
}

void TOgIndexVoc::GetAllGreaterV(const int& KeyId, 
        const uint64& StartWordId, TOgKeyWordV& AllGreaterV) {

	TUInt64V WordIdV; 
	if (KeyH[KeyId].IsSortById()) { 
		GetWordVoc(KeyId)->GetAllGreaterById(StartWordId, WordIdV);
    } else if (KeyH[KeyId].IsSortByStr()) {
		GetWordVoc(KeyId)->GetAllGreaterByStr(StartWordId, WordIdV);
    } else if (KeyH[KeyId].IsSortByFlt()) {
		GetWordVoc(KeyId)->GetAllGreaterByFlt(StartWordId, WordIdV);
	}
	AllGreaterV.Gen(WordIdV.Len(), 0);
	for (int WordN = 0; WordN < WordIdV.Len(); WordN++) { 
		AllGreaterV.Add(TOgKeyWord(KeyId, (uint64)WordIdV[WordN]));
	}
}

void TOgIndexVoc::GetAllLessV(const int& KeyId, 
        const uint64& StartWordId, TOgKeyWordV& AllLessV) {

	TUInt64V WordIdV; 
	if (KeyH[KeyId].IsSortById()) { 
		GetWordVoc(KeyId)->GetAllLessById(StartWordId, WordIdV);
    } else if (KeyH[KeyId].IsSortByStr()) {
		GetWordVoc(KeyId)->GetAllLessByStr(StartWordId, WordIdV);
    } else if (KeyH[KeyId].IsSortByFlt()) {
		GetWordVoc(KeyId)->GetAllLessByFlt(StartWordId, WordIdV);
	}
	AllLessV.Gen(WordIdV.Len(), 0);
	for (int WordN = 0; WordN < WordIdV.Len(); WordN++) { 
		AllLessV.Add(TOgKeyWord(KeyId, (uint64)WordIdV[WordN]));
	}
}

void TOgIndexVoc::SaveTxt(const POgBase& OgBase, const TStr& FNm) const {
	TFOut FOut(FNm);
	for (int StoreN = 0; StoreN < OgBase->GetStores(); StoreN++) {
		const POgStore& Store = OgBase->GetStoreByStoreN(StoreN);
		FOut.PutStrFmt("%s[%d]: ", Store->GetStoreNm().CStr(), int(Store->GetStoreId()));
		const TIntSet& KeySet = GetStoreKeys(Store->GetStoreId());
		int KeyId = KeySet.FFirstKeyId();
		while (KeySet.FNextKeyId(KeyId)) {
			TStr KeyNm = GetKeyNm(KeySet.GetKey(KeyId));
			if (KeyId != 0) { FOut.PutStr(", "); }
			FOut.PutStr(KeyNm);
		}
		FOut.PutLn();
	}
	FOut.PutLn();
	int KeyId = KeyH.FFirstKeyId();
	while (KeyH.FNextKeyId(KeyId)) {
		const uchar StoreId = KeyH.GetKey(KeyId).Val1;
		const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
		TStr StoreNm = Store->GetStoreNm();
		TStr KeyNm = KeyH.GetKey(KeyId).Val2;
		const TOgIndexKey& Key = KeyH[KeyId];
		FOut.PutStrFmt("KeyNm: '%s.%s' |Id:%d", StoreNm.CStr(), KeyNm.CStr(), KeyId);
		if (Key.IsText()) { FOut.PutStr("|Text"); }
		if (Key.IsSortByStr()) { FOut.PutStr("|SortByStr"); }
		if (Key.IsSortById()) { FOut.PutStr("|SortById"); }
		if (Key.IsSortByFlt()) { FOut.PutStr("|SortByFlt"); }
		if (Key.IsAggr()) { FOut.PutStr("|Aggr"); }
		if (Key.IsInternal()) { FOut.PutStr("|Internal"); }
		if (Key.IsWordVoc()) { FOut.PutStrFmt("|Words:%d:", GetWords(KeyId)); }
		if (Key.IsFields()) {
			TChA FieldChA; 
			for (int FieldN = 0; FieldN < Key.GetFields(); FieldN++) {
				const TOgFieldDesc& FieldDesc = Store->GetFieldDesc(Key.GetFieldId(FieldN));
				FieldChA += FieldChA.Empty() ? "|" : "; ";
				FieldChA += TStr::Fmt("%s.%s", StoreNm.CStr(), FieldDesc.GetFieldNm().CStr());
			}
			FOut.PutStr(FieldChA);
		}
		FOut.PutStrLn("|");
		if (!Key.IsInternal()) {
			FOut.PutStr("  ");
			int ChsPerLn = 2; const int MxChsPerLn = 100;
			TStrV WordStrV; GetAllWordStrV(KeyId, WordStrV);
			for (int WordN = 0; WordN < WordStrV.Len(); WordN++) {
				if (WordN != 0) { FOut.PutStr(", "); ChsPerLn += 2;}
				const TStr& WordStr = WordStrV[WordN];
				if (ChsPerLn + WordStr.Len() > MxChsPerLn) {
					FOut.PutStr("\n  "); ChsPerLn = 2; }
				FOut.PutStrFmt("'%s'", WordStr.CStr()); ChsPerLn += WordStr.Len() + 2;
			}
			FOut.PutLn();
		}
		FOut.PutLn();
	}
}

void TOgQueryItem::ParseWordStr(const TStr& WordStr, const POgIndexVoc& IndexVoc) {
    if (IndexVoc->GetKey(KeyId).IsText()) {
        if (!IsEqual() && !IsNotEqual()) { 
			TOgExcept::Throw("Wrong sort type for text Key!"); }
        IndexVoc->GetWordIdV(KeyId, WordStr, WordIdV);
    } else {
        if (IndexVoc->IsWordStr(KeyId, WordStr)) { 
		    WordIdV.Add(IndexVoc->GetWordId(KeyId, WordStr));
        }
    }
    if (WordIdV.Empty() && !IndexVoc->GetKey(KeyId).IsText()) {
        TOgExcept::Throw(TStr::Fmt("Unknown query string %d:'%s'!", KeyId.Val, WordStr.CStr()));
    }
}


POgStore TOgQueryItem::ParseJoins(const POgBase& OgBase, const PJsonVal& JsonVal) {
	POgStore JoinStore;
	PJsonVal JoinVal = JsonVal->GetObjKey("$join");	
	if (JoinVal->IsObj()) {
		JoinStore = ParseJoin(OgBase, JoinVal);
	} else if (JoinVal->IsArr()) {
		for (int ValN = 0; ValN < JoinVal->GetArrVals(); ValN++) {
			PJsonVal Val = JoinVal->GetArrVal(ValN);
			OgAssertR(Val->IsObj(), "Query: $join expects object as value");
			if (JoinStore.Empty()) {
				JoinStore = ParseJoin(OgBase, Val);
			} else {
				POgStore _JoinStore = ParseJoin(OgBase, Val);
				OgAssertR(_JoinStore->GetStoreId() == JoinStore->GetStoreId(), "Query: store mismatch");
			}
		}
	} else {
		TOgExcept::Throw("Query: bad join parameter: '" + TJsonVal::GetStrFromVal(JsonVal) + "'");
	}
	return JoinStore;
}

POgStore TOgQueryItem::ParseJoin(const POgBase& OgBase, const PJsonVal& JoinVal) {
	OgAssertR(JoinVal->IsObjKey("$query"), "Query: $join expects object as value");
	TOgQueryItem SubQuery(OgBase, JoinVal->GetObjKey("$query"));
	const POgStore& Store = OgBase->GetStoreByStoreId(SubQuery.GetStoreId(OgBase));
	OgAssertR(JoinVal->IsObjKey("$name") && JoinVal->GetObjKey("$name")->IsStr(), 
		"Query: $join expects $name parameter with string as value");
	const int _JoinId = Store->GetJoinId(JoinVal->GetObjKey("$name")->GetStr());
	if (JoinVal->IsObjKey("$sample")) {	
		OgAssertR(JoinVal->GetObjKey("$sample")->IsNum(), "Query: $sample expects number as value"); }
	const int _SampleSize = JoinVal->IsObjKey("$sample") ? TFlt::Round(JoinVal->GetObjKey("$sample")->GetNum()) : -1;
	ItemV.Add(TOgQueryItem(_JoinId, _SampleSize, SubQuery));
	const uchar JoinStoreId = Store->GetJoinDesc(_JoinId).GetJoinStoreId();
	return OgBase->GetStoreByStoreId(JoinStoreId);
}

POgStore TOgQueryItem::ParseFrom(const POgBase& OgBase, const PJsonVal& JsonVal) {
	OgAssertR(JsonVal->GetObjKey("$from")->IsStr(), "Query: $from expects string as value");
	TStr StoreNm = JsonVal->GetObjKey("$from")->GetStr();
	return OgBase->GetStoreByStoreNm(StoreNm);
}

void TOgQueryItem::ParseKeys(const POgBase& OgBase, const POgStore& Store, 
		const PJsonVal& JsonVal, const bool& IgnoreOrP) {

	for (int KeyN = 0; KeyN < JsonVal->GetObjKeys(); KeyN++) {
		TStr KeyNm; PJsonVal KeyVal;
		JsonVal->GetObjKeyVal(KeyN, KeyNm, KeyVal);
		if (KeyNm.IsPrefix("$")) {
			if (KeyNm == "$or") {
				if (!IgnoreOrP) {
					OgAssertR(KeyVal->IsArr(), "Query: $or expects array as value");
					ItemV.Add(TOgQueryItem(OgBase, Store, KeyVal));
				}
			} else if (KeyNm == "$not") {
				ItemV.Add(TOgQueryItem(oqitNot, TOgQueryItem(OgBase, Store, KeyVal)));
			} else if (KeyNm == "$join") {
			} else if (KeyNm == "$from") {
				OgAssertR(KeyVal->IsStr() && Store->GetStoreNm() == KeyVal->GetStr(), "Query: store mismatch");
			} else {
				TOgExcept::Throw("Query: unknown parameter " + KeyNm);
			}
		} else {
			ItemV.Add(TOgQueryItem(OgBase, Store, KeyNm, KeyVal));
		}
	}
}

TOgQueryItem::TOgQueryItem(const POgBase& OgBase, const PJsonVal& JsonVal) {
	if (JsonVal->IsObj()) {
		Type = oqitAnd;	POgStore Store;
		if (JsonVal->IsObjKey("$join")) {
			Store = ParseJoins(OgBase, JsonVal);
		} 
		if (JsonVal->IsObjKey("$from")) {
			if (Store.Empty()) {
				Store = ParseFrom(OgBase, JsonVal);
			} else {
				POgStore FromStore = ParseFrom(OgBase, JsonVal);
				OgAssertR(FromStore->GetStoreId() == Store->GetStoreId(), "Query: store mismatch");
			}
		}
		bool IgnoreOrP = false;
		if (JsonVal->IsObjKey("$or") && Store.Empty()) {
			IgnoreOrP = true; // so we don't pares it again in ParseKeys below
			TOgQueryItemV OrItemV;
			PJsonVal OrVal = JsonVal->GetObjKey("$or");
			OgAssertR(OrVal->IsArr(), "Query: $or expects array as value");
			for (int ValN = 0; ValN < OrVal->GetArrVals(); ValN++) {
				PJsonVal Val = OrVal->GetArrVal(ValN);
				if (Val->IsObj()) {
					OrItemV.Add(TOgQueryItem(OgBase, Val));
					const uchar OrStoreId = OrItemV.Last().GetStoreId(OgBase);
					if (Store.Empty()) { Store = OgBase->GetStoreByStoreId(OrStoreId); }
					else { OgAssertR(OrStoreId == Store->GetStoreId(), "Query: store mismatch"); }
				} else {
					TOgExcept::Throw("Query: OR query expects objets: '" + 
						TJsonVal::GetStrFromVal(JsonVal) + "'");
				}
			}
			ItemV.Add(TOgQueryItem(oqitOr, OrItemV));
		} 
		if (!Store.Empty()) {
			ParseKeys(OgBase, Store, JsonVal, IgnoreOrP);
		} else {
			TOgExcept::Throw("Query: underspecified query");
		}
	} else {
		TOgExcept::Throw("Query: expected an object: '" + TJsonVal::GetStrFromVal(JsonVal) + "'");
	}
}

TOgQueryItem::TOgQueryItem(const POgBase& OgBase, const POgStore& Store, const PJsonVal& JsonVal) {
	if (JsonVal->IsObj()) {
		Type = oqitAnd;
		if (JsonVal->IsObjKey("$join")) {
			POgStore JoinStore = ParseJoins(OgBase, JsonVal);
			OgAssertR(JoinStore->GetStoreId() == Store->GetStoreId(), "Query: store mismatch");			
		} 
		if (JsonVal->IsObjKey("$from")) {
			POgStore FromStore = ParseFrom(OgBase, JsonVal);
			OgAssertR(FromStore->GetStoreId() == Store->GetStoreId(), "Query: store mismatch");
		}
		ParseKeys(OgBase, Store, JsonVal, false);
	} else if (JsonVal->IsArr()) {	
		Type = oqitOr;
		for (int ValN = 0; ValN < JsonVal->GetArrVals(); ValN++) {
			PJsonVal Val = JsonVal->GetArrVal(ValN);
			if (Val->IsObj()) {
				ItemV.Add(TOgQueryItem(OgBase, Store, Val));
			} else {
				TOgExcept::Throw("Query: OR query expects objets: '" + TJsonVal::GetStrFromVal(JsonVal) + "'");
			}
		}
	}
}

TOgQueryItem::TOgQueryItem(const POgBase& OgBase, const POgStore& Store, const TStr& KeyNm, const PJsonVal& KeyVal) {
	const POgIndexVoc& IndexVoc = OgBase->GetIndexVoc();
	OgAssertR(IndexVoc->IsKeyNm(Store->GetStoreId(), KeyNm), "Query: unknown key " + KeyNm);
	if (KeyVal->IsStr()) {
		KeyId = IndexVoc->GetKeyId(Store->GetStoreId(), KeyNm);
		Type = oqitLeaf;
		CmpType = oqctEqual;
		ParseWordStr(KeyVal->GetStr(), IndexVoc);
	} else if (KeyVal->IsObj()) {
		KeyId = IndexVoc->GetKeyId(Store->GetStoreId(), KeyNm);
		Type = oqitLeaf;
		if (KeyVal->IsObjKey("$ne")) {
			OgAssertR(KeyVal->GetObjKey("$ne")->IsStr(), "Query: $ne value must be string");
			CmpType = oqctNotEqual;
			ParseWordStr(KeyVal->GetObjKey("$ne")->GetStr(), IndexVoc);
		} else if (KeyVal->IsObjKey("$gt")) {
			OgAssertR(KeyVal->GetObjKey("$gt")->IsStr(), "Query: $gt value must be string");
			CmpType = oqctGreater;
			ParseWordStr(KeyVal->GetObjKey("$gt")->GetStr(), IndexVoc);
		} else if (KeyVal->IsObjKey("$lt")) {
			OgAssertR(KeyVal->GetObjKey("$lt")->IsStr(), "Query: $lt value must be string");
			CmpType = oqctLess;
			ParseWordStr(KeyVal->GetObjKey("$lt")->GetStr(), IndexVoc);
		} else {
			TOgExcept::Throw("Query: Invalid operator: '" + TJsonVal::GetStrFromVal(KeyVal) + "'");
		}
	} else if (KeyVal->IsArr()) {
		Type = oqitAnd;
		for (int ValN = 0; ValN < KeyVal->GetArrVals(); ValN++) {
			PJsonVal Val = KeyVal->GetArrVal(ValN);
			OgAssertR(Val->IsStr() || Val->IsObj(), 
				"Query: Multiple conditions for a key must be string or object");
			ItemV.Add(TOgQueryItem(OgBase, Store, KeyNm, Val));
		}
	} else {
		TOgExcept::Throw("Query: Invalid key definition: '" + TJsonVal::GetStrFromVal(KeyVal) + "'");
	}
}

TOgQueryItem::TOgQueryItem(const POgBase& OgBase, const int& _KeyId, 
	const uint64& WordId, const TOgQueryCmpType& _CmpType): Type(oqitLeaf), 
		KeyId(_KeyId), CmpType(_CmpType) { WordIdV.Add(WordId); }

TOgQueryItem::TOgQueryItem(const POgBase& OgBase, const int& _KeyId,
		const TStr& WordStr, const TOgQueryCmpType& _CmpType): Type(oqitLeaf) {

    KeyId = _KeyId;
    OgAssertR(OgBase->GetIndexVoc()->IsKeyId(KeyId), "Unknown Key ID: " + KeyId.GetStr());
    CmpType = _CmpType;
	ParseWordStr(WordStr, OgBase->GetIndexVoc());
}

TOgQueryItem::TOgQueryItem(const POgBase& OgBase, const uchar& StoreId, const TStr& KeyNm,
		const TStr& WordStr, const TOgQueryCmpType& _CmpType): Type(oqitLeaf) {

    OgAssertR(OgBase->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
	KeyId = OgBase->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
	CmpType = _CmpType;
	ParseWordStr(WordStr, OgBase->GetIndexVoc());
}

TOgQueryItem::TOgQueryItem(const POgBase& OgBase, const TStr& StoreNm, const TStr& KeyNm,
		const TStr& WordStr, const TOgQueryCmpType& _CmpType): Type(oqitLeaf) {

	const uchar StoreId = OgBase->GetStoreByStoreNm(StoreNm)->GetStoreId();
    OgAssertR(OgBase->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
	KeyId = OgBase->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
	CmpType = _CmpType;
	ParseWordStr(WordStr, OgBase->GetIndexVoc());
}

TOgQueryItem::TOgQueryItem(const TOgQueryItemType& _Type): Type(_Type) { 
	OgAssert(Type == oqitAnd || Type == oqitOr); }

TOgQueryItem::TOgQueryItem(const TOgQueryItemType& _Type, const TOgQueryItem& Item): 
	Type(_Type), ItemV(1, 0) { ItemV.Add(Item); 
		OgAssert(Type == oqitAnd || Type == oqitOr || Type == oqitNot);}
	
TOgQueryItem::TOgQueryItem(const TOgQueryItemType& _Type, const TOgQueryItem& Item1, 
	const TOgQueryItem& Item2): Type(_Type), ItemV(2, 0) { ItemV.Add(Item1); ItemV.Add(Item2); 
		OgAssert(Type == oqitAnd || Type == oqitOr);}
	
TOgQueryItem::TOgQueryItem(const TOgQueryItemType& _Type, const TOgQueryItemV& _ItemV): 
	Type(_Type), ItemV(_ItemV) { OgAssert(Type == oqitAnd || Type == oqitOr);}

TOgQueryItem::TOgQueryItem(const int& _JoinId, const int& _SampleSize, const TOgQueryItem& Item): 
	Type(oqitJoin), ItemV(1, 0), JoinId(_JoinId), SampleSize(_SampleSize) { ItemV.Add(Item); }

TOgQueryItem::TOgQueryItem(const POgBase& OgBase, const TStr& JoinNm, const int& _SampleSize,
		const TOgQueryItem& Item): Type(oqitJoin), ItemV(1, 0), SampleSize(_SampleSize) { 
			
	ItemV.Add(Item); 
	const uchar StoreId = Item.GetStoreId(OgBase);
	const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
	JoinId = Store->GetJoinId(JoinNm);
}

TOgQueryItem::TOgQueryItem(const uchar& StoreId, const uint64& RecId):
	Type(oqitRecSet), RecSet(TOgRecSet::New(StoreId, RecId)) { }

TOgQueryItem::TOgQueryItem(const TOgRec& Rec):
	Type(oqitRecSet), RecSet(TOgRecSet::New(Rec.GetStoreId(), Rec.GetRecId())) { }

TOgQueryItem::TOgQueryItem(const POgRecSet& _RecSet):
	Type(oqitRecSet), RecSet(_RecSet) { }

uchar TOgQueryItem::GetStoreId(const POgBase& OgBase) const {
	if (Type == oqitLeaf) { 
		return OgBase->GetIndexVoc()->GetKeyStoreId(KeyId); 
	} else if (Type == oqitJoin) {
		OgAssertR(ItemV.Len() == 1, "QueryItem: Join node must have exactly one child");
		const uchar StoreId = ItemV[0].GetStoreId(OgBase);
		const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
		return Store->GetJoinDesc(JoinId).GetJoinStoreId();
	} else {
		OgAssertR(!ItemV.Empty(), "QueryItem: Non-leaf node without children"); 
		uchar StoreId;
		for (int ItemN = 0; ItemN < ItemV.Len(); ItemN++) {
			if (ItemN == 0) {
				StoreId = ItemV[ItemN].GetStoreId(OgBase);
			} else {
				if (ItemV[ItemN].GetStoreId(OgBase) != StoreId) {
					TOgExcept::Throw("QueryItem: clidren nodes return records from different stores");
				}
			}
		}
		return StoreId;
	}
}

bool TOgQueryItem::IsWgt() const { 
	if (IsLeaf()) {
		return true;
	} else if (IsOr()) {
		bool WgtP = true;
		for (int ItemN = 0; ItemN < ItemV.Len(); ItemN++) {
			WgtP = WgtP && ItemV[ItemN].IsWgt();
		}
		return WgtP;
	} else if (IsJoin()) {
		return true;
	}
	return false; 
}

void TOgQueryItem::GetKeyWordV(TOgKeyWordV& KeyWordPrV) const {
    KeyWordPrV.Clr();
    for (int WordIdN = 0; WordIdN < WordIdV.Len(); WordIdN++) {
        KeyWordPrV.Add(TOgKeyWord(KeyId, WordIdV[WordIdN]));
    }
}

TOgQuery::TOgQuery(const POgBase& OgBase, const TOgQueryItem& _QueryItem,
	const int& _SortFieldId, const bool& _SortAscP, const int& _Limit, 
	const int& _Offset): QueryItem(_QueryItem), SortFieldId(_SortFieldId),
		SortAscP(_SortAscP), Limit(_Limit), Offset(_Offset) { }

POgQuery TOgQuery::New(const POgBase& OgBase, const TOgQueryItem& QueryItem, 
		const int& SortFieldId, const bool& SortAscP, const int& Limit, const int& Offset) {

	return new TOgQuery(OgBase, QueryItem, SortFieldId, SortAscP, Limit, Offset); 
}

POgQuery TOgQuery::New(const POgBase& OgBase, const PJsonVal& JsonVal) {
	POgQuery Query = New(OgBase, TOgQueryItem(OgBase, JsonVal));
	if (JsonVal->IsObjKey("$sort")) {
		PJsonVal SortVal = JsonVal->GetObjKey("$sort");
		OgAssert(SortVal->IsObj() && SortVal->GetObjKeys()==1);
		TStr FieldNm; PJsonVal AscVal; SortVal->GetObjKeyVal(0, FieldNm, AscVal);
		POgStore Store = Query->GetStore(OgBase);
		OgAssert(Store->IsFieldNm(FieldNm));
		Query->SortFieldId = Store->GetFieldId(FieldNm);
		OgAssert(AscVal->IsNum());
		Query->SortAscP = (AscVal->GetNum() > 0.0);
	}
	if (JsonVal->IsObjKey("$limit")) {
		PJsonVal LimitVal = JsonVal->GetObjKey("$limit");
		OgAssert(LimitVal->IsNum());
		Query->Limit = TFlt::Round(LimitVal->GetNum());
	}
	if (JsonVal->IsObjKey("$offset")) {
		PJsonVal OffsetVal = JsonVal->GetObjKey("$offset");
		OgAssert(OffsetVal->IsNum());
		Query->Offset = TFlt::Round(OffsetVal->GetNum());
	}
	return Query;
}
	
POgQuery TOgQuery::New(const POgBase& OgBase, const TStr& TStr) {
	return New(OgBase, TJsonVal::GetValFromStr(TStr));
}
	
POgStore TOgQuery::GetStore(const POgBase& OgBase) {
	const uchar StoreId = QueryItem.GetStoreId(OgBase);
	return OgBase->GetStoreByStoreId(StoreId);
}

void TOgQuery::Sort(const POgBase& OgBase, const POgRecSet& RecSet) {
	RecSet->SortByField(OgBase, SortAscP, SortFieldId);
}

POgRecSet TOgQuery::GetLimit(const POgRecSet& RecSet) {
	return RecSet->GetLimit(Limit, Offset);
}

bool TOgQuery::IsOk(const POgBase& OgBase, TStr& MsgStr) const {
	try {
		const uchar StoreId = QueryItem.GetStoreId(OgBase);
		MsgStr.Clr(); return true;
	} catch (PExcept Except) {
		MsgStr = Except->GetMsgStr();
		return false;
	}
}

void TOgIndex::TOgGixDefMerger::Union(
		TOgGixItemV& MainV, const TOgGixItemV& JoinV) const {

    TOgGixItemV ResV; int ValN1 = 0; int ValN2 = 0;
    while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
        const TOgGixItem& Val1 = MainV.GetVal(ValN1);
        const TOgGixItem& Val2 = JoinV.GetVal(ValN2);
        if (Val1 < Val2) { ResV.Add(Val1); ValN1++; }
        else if (Val1 > Val2) { ResV.Add(Val2); ValN2++; }
		else { ResV.Add(TOgGixItem(Val1.Key, Val1.Dat + Val2.Dat)); ValN1++; ValN2++; }
    }
    for (int RestValN1 = ValN1; RestValN1 < MainV.Len(); RestValN1++){
        ResV.Add(MainV.GetVal(RestValN1));}
    for (int RestValN2 = ValN2; RestValN2 < JoinV.Len(); RestValN2++){
        ResV.Add(JoinV.GetVal(RestValN2));}    
    MainV = ResV;
}

void TOgIndex::TOgGixDefMerger::Intrs(
		TOgGixItemV& MainV, const TOgGixItemV& JoinV) const {

    TOgGixItemV ResV; int ValN1 = 0; int ValN2 = 0;
    while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
        const TOgGixItem& Val1 = MainV.GetVal(ValN1);
        const TOgGixItem& Val2 = JoinV.GetVal(ValN2);
        if (Val1 < Val2) { ValN1++; }
        else if (Val1 > Val2) { ValN2++; }
		else { ResV.Add(TOgGixItem(Val1.Key, Val1.Dat + Val2.Dat)); ValN1++; ValN2++; }
    }
    MainV = ResV;
}

void TOgIndex::TOgGixDefMerger::Minus(const TOgGixItemV& MainV, 
		const TOgGixItemV& JoinV, TOgGixItemV& ResV) const {

	MainV.Minus(JoinV, ResV);
}

void TOgIndex::TOgGixDefMerger::Merge(TOgGixItemV& ItemV) const {
	if (ItemV.Empty()) { return; } // nothing to do in this case
    ItemV.Sort(); int LastItemN = 0;
    for (int ItemN = 1; ItemN < ItemV.Len(); ItemN++) {
        if (ItemV[ItemN] != ItemV[ItemN-1])  {
            LastItemN++;
            ItemV[LastItemN] = ItemV[ItemN];
        } else {
            ItemV[LastItemN].Dat += ItemV[ItemN].Dat;
        }
    }
    ItemV.Reserve(ItemV.Reserved(), LastItemN+1);
}

void TOgIndex::TOgGixRmDupMerger::Union(TOgGixItemV& MainV, const TOgGixItemV& JoinV) const
{
	TOgGixItemV ResV; int ValN1 = 0; int ValN2 = 0;
    while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
        const TOgGixItem& Val1 = MainV.GetVal(ValN1);
        const TOgGixItem& Val2 = JoinV.GetVal(ValN2);
        if (Val1 < Val2) { ResV.Add(TOgGixItem(Val1.Key, 1)); ValN1++; }
        else if (Val1 > Val2) { ResV.Add(TOgGixItem(Val2.Key, 1)); ValN2++; }
		else { 
			int fq1 = TInt::GetMn(1, Val1.Dat);
			int fq2 = TInt::GetMn(1, Val2.Dat);			
			ResV.Add(TOgGixItem(Val1.Key, fq1 + fq2)); ValN1++; ValN2++; 
		}
    }
    for (int RestValN1 = ValN1; RestValN1 < MainV.Len(); RestValN1++)
	{
		TOgGixItem Item = MainV.GetVal(RestValN1);	
		Item.Dat = TInt::GetMn(1, Item.Dat);
        ResV.Add(Item);
	}
    for (int RestValN2 = ValN2; RestValN2 < JoinV.Len(); RestValN2++)
	{
		TOgGixItem Item = JoinV.GetVal(RestValN2);		
		Item.Dat = TInt::GetMn(1, Item.Dat);
        ResV.Add(Item);
	}    
    MainV = ResV;
}

void TOgIndex::TOgGixRmDupMerger::Intrs(TOgGixItemV& MainV, const TOgGixItemV& JoinV) const
{
	TOgGixItemV ResV; int ValN1 = 0; int ValN2 = 0;
    while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
        const TOgGixItem& Val1 = MainV.GetVal(ValN1);
        const TOgGixItem& Val2 = JoinV.GetVal(ValN2);
        if (Val1 < Val2) { ValN1++; }
        else if (Val1 > Val2) { ValN2++; }
		else 
		{ 
			int fq1 = TInt::GetMn(1, Val1.Dat);
			int fq2 = TInt::GetMn(1, Val2.Dat);
			ResV.Add(TOgGixItem(Val1.Key, fq1 + fq2)); ValN1++; ValN2++; 
		}
    }
    MainV = ResV;
}

TOgIndex::TOgGixKeyStr::TOgGixKeyStr(const POgBase& _OgBase, 
	const POgIndexVoc& _IndexVoc): OgBase(_OgBase), IndexVoc(_IndexVoc) { }

TStr TOgIndex::TOgGixKeyStr::GetKeyNm(const TOgGixKey& Key) const {
	const int KeyId = Key.Val1;
	const uchar StoreId = IndexVoc->GetKeyStoreId(KeyId);
	const uint64 WordId = Key.Val2;
	const TOgIndexKey& IndexKey = IndexVoc->GetKey(KeyId);
	const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
	TChA KeyChA = Store->GetStoreNm();
	if (IndexKey.IsInternal()) {
		KeyChA += "->"; KeyChA += IndexKey.GetJoinNm();	
		KeyChA += "->"; KeyChA += Store->GetRecNm(WordId);
	} else {
		KeyChA += '.'; KeyChA += IndexKey.GetKeyNm();
		if (IndexKey.IsWordVoc()) {
			KeyChA += '['; KeyChA += IndexVoc->GetWordStr(KeyId, WordId); KeyChA += ']';
		}
	}
	return KeyChA;
}

TOgIndex::POgGixExpItem TOgIndex::ToExpItem(const TOgQueryItem& QueryItem) const {
	if (QueryItem.IsLeaf()) {
		if (QueryItem.IsEqual()) {
			// ==
			TOgKeyWordV AllKeyV; QueryItem.GetKeyWordV(AllKeyV);
			return TOgGixExpItem::NewAndV(AllKeyV);
		} else if (QueryItem.IsGreater()) {
			// >=
			TOgKeyWordV AllGreaterV;
			IndexVoc->GetAllGreaterV(QueryItem.GetKeyId(), 
				QueryItem.GetWordId(), AllGreaterV);
			return TOgGixExpItem::NewOrV(AllGreaterV);
		} else if (QueryItem.IsLess()) {
			// <=
			TOgKeyWordV AllLessV;
			IndexVoc->GetAllLessV(QueryItem.GetKeyId(), 
				QueryItem.GetWordId(), AllLessV);
			return TOgGixExpItem::NewOrV(AllLessV);
		} else if (QueryItem.IsNotEqual()) {
			// !=
			TOgKeyWordV AllKeyV; QueryItem.GetKeyWordV(AllKeyV);
			return TOgGixExpItem::NewNot(TOgGixExpItem::NewAndV(AllKeyV));
		} else {
			TOgExcept::Throw("Index: Unknown query item operator");
		}
	} else if (QueryItem.IsAnd()) {
		TVec<POgGixExpItem> ExpItemV(QueryItem.GetItems(), 0);
		for (int ItemN = 0; ItemN < QueryItem.GetItems(); ItemN++) {
			ExpItemV.Add(ToExpItem(QueryItem.GetItem(ItemN)));
		}
		return TOgGixExpItem::NewAndV(ExpItemV);
	} else if (QueryItem.IsOr()) {
		TVec<POgGixExpItem> ExpItemV(QueryItem.GetItems(), 0);
		for (int ItemN = 0; ItemN < QueryItem.GetItems(); ItemN++) {
			ExpItemV.Add(ToExpItem(QueryItem.GetItem(ItemN)));
		}
		return TOgGixExpItem::NewOrV(ExpItemV);
	} else if (QueryItem.IsNot()) {
		OgAssert(QueryItem.GetItems() == 1);
		return TOgGixExpItem::NewNot(ToExpItem(QueryItem.GetItem(0)));
	}  else if (QueryItem.IsJoin()) {
		TOgExcept::Throw("Index: QueryItem of type Join must be handled outside TOgIndex");
	} else {
		TOgExcept::Throw("Index: Unknown query item type");
	}
	return TOgGixExpItem::NewEmpty();
}

bool TOgIndex::DoQuery(const TOgIndex::POgGixExpItem& ExpItem, 
        const POgGixMerger& Merger, TOgGixItemV& ResIdFqV) const {

	ResIdFqV.Clr(); 
    return ExpItem->Eval(Gix, ResIdFqV, Merger);
}

TOgIndex::TOgIndex(const TStr& _IndexFPath, const TFAccess& _Access, 
        const POgIndexVoc& _IndexVoc, const int64& CacheSize) {

	IndexFPath = _IndexFPath;
    Access = _Access;
	DefMerger = TOgGixDefMerger::New();
    Gix = TOgGix::New("Index", IndexFPath, Access, CacheSize, DefMerger);
    IndexVoc = _IndexVoc;
}

TOgIndex::~TOgIndex() {
	TOg::Logger->OnStatus("Start saving and closing index");
    if (Access != faRdOnly) { Gix.Clr(); }
	TOg::Logger->OnStatus("Index closed");
}

void TOgIndex::Index(const int& KeyId, const TStr& WordStr, const uint64& RecId) {
    const uint64 WordId = IndexVoc->AddWordStr(KeyId, WordStr);
    Index(KeyId, WordId, RecId, 1);
}

void TOgIndex::Index(const int& KeyId, const TStrV& WordStrV, const uint64& RecId) {
	TUInt64H WordIdH;
	for (int WordN = 0; WordN < WordStrV.Len(); WordN++) {
		const TStr WordStr = WordStrV[WordN].GetLc();
		WordIdH.AddDat(IndexVoc->AddWordStr(KeyId, WordStr))++;
	}
    int WordKeyId = WordIdH.FFirstKeyId();
    while (WordIdH.FNextKeyId(WordKeyId)) {
        const uint64 WordId = WordIdH.GetKey(WordKeyId);
		const int WordFq = WordIdH[WordKeyId];
        Index(KeyId, WordId, RecId, WordFq);
    }
}

void TOgIndex::Index(const int& KeyId, const TStrIntPrV& WordStrFqV, const uint64& RecId) {
    TIntH WordIdH;
	for (int WordN = 0; WordN < WordStrFqV.Len(); WordN++) {
		const TStr WordStr = WordStrFqV[WordN].Val1.GetLc();
        const uint64 WordId = IndexVoc->AddWordStr(KeyId, WordStr);
		const int WordFq = WordStrFqV[WordN].Val2;
        Index(KeyId, WordId, RecId, WordFq);	
	}
}

void TOgIndex::Index(const uchar& StoreId, const TStr& KeyNm, 
		const TStr& WordStr, const uint64& RecId) {

	Index(IndexVoc->GetKeyId(StoreId, KeyNm), WordStr, RecId);
}

void TOgIndex::Index(const uchar& StoreId, const TStr& KeyNm, 
		const TStrV& WordStrV, const uint64& RecId) {

	Index(IndexVoc->GetKeyId(StoreId, KeyNm), WordStrV, RecId);
}

void TOgIndex::Index(const uchar& StoreId, const TStr& KeyNm, 
		const TStrIntPrV& WordStrFqV, const uint64& StoreRecId) {

	Index(IndexVoc->GetKeyId(StoreId, KeyNm), WordStrFqV, StoreRecId);
}

void TOgIndex::Index(const uchar& StoreId, const TStrPrV& KeyWordV, const uint64& RecId) {
	for (int KeyWordN = 0; KeyWordN < KeyWordV.Len(); KeyWordN++) {
		const TStrPr& KeyWord = KeyWordV[KeyWordN];
		const int KeyId = IndexVoc->GetKeyId(StoreId, KeyWord.Val1);
		const uint64 WordId = IndexVoc->AddWordStr(KeyId, KeyWord.Val2);
		Index(KeyId, WordId, RecId, 1);
	}
}

void TOgIndex::IndexText(const int& KeyId, const TStr& TextStr, const uint64& RecId) {
	TUInt64V WordIdV; IndexVoc->AddWordIdV(KeyId, TextStr, WordIdV);
	TUInt64H WordIdFqH;
	for (int WordIdN = 0; WordIdN < WordIdV.Len(); WordIdN++) {
		WordIdFqH.AddDat(WordIdV[WordIdN])++; 
	}
	int WordKeyId = WordIdFqH.FFirstKeyId();
	while (WordIdFqH.FNextKeyId(WordKeyId)) {
		Index(KeyId, WordIdFqH.GetKey(WordKeyId), RecId, WordIdFqH[WordKeyId]);
	}
}

void TOgIndex::IndexText(const uchar& StoreId, const TStr& KeyNm, 
		const TStr& TextStr, const uint64& RecId) {

	IndexText(IndexVoc->GetKeyId(StoreId, KeyNm), TextStr, RecId);
}

void TOgIndex::IndexJoin(const POgStore& Store, const int& JoinId,
		const uint64& RecId, const uint64& JoinRecId, const int& JoinFq) {

	Index(Store->GetJoinKeyId(JoinId), RecId, JoinRecId, JoinFq);
}

void TOgIndex::IndexJoin(const POgStore& Store, const TStr& JoinNm,
		const uint64& RecId, const uint64& JoinRecId, const int& JoinFq) {

	Index(Store->GetJoinKeyId(JoinNm), RecId, JoinRecId, JoinFq);
}

void TOgIndex::Index(const int& KeyId, const uint64& WordId, const uint64& RecId, const int& RecFq) {
	Assert(KeyId != -1);
	OgAssertR(!Gix->IsReadOnly(), "Cannot edit read-only index!");
    Gix->AddItem(TOgKeyWord(KeyId, WordId), TOgGixItem(RecId, RecFq));
}

void TOgIndex::Delete(const uchar& StoreId, const TStr& KeyNm, const TStr& WordStr, const uint64& RecId) {
	const int KeyId = IndexVoc->GetKeyId(StoreId, KeyNm);
	const uint64 WordId = IndexVoc->AddWordStr(KeyId, WordStr);
	Delete(KeyId, WordId, RecId);
}

void TOgIndex::Delete(const uchar& StoreId, const TStr& KeyNm, const uint64& WordId, const uint64& RecId) {
	Delete(IndexVoc->GetKeyId(StoreId, KeyNm), WordId, RecId);
}

void TOgIndex::Delete(const int& KeyId, const TStr& WordStr, const uint64& RecId) {
	const uint64 WordId = IndexVoc->AddWordStr(KeyId, WordStr);
	Delete(KeyId, WordId, RecId);
}

void TOgIndex::DeleteText(const int& KeyId, const TStr& TextStr, const uint64& RecId) {
	TUInt64V WordIdV; IndexVoc->AddWordIdV(KeyId, TextStr, WordIdV);
	TUInt64Set WordIdSet;
	for (int WordIdN = 0; WordIdN < WordIdV.Len(); WordIdN++) {
		WordIdSet.AddKey(WordIdV[WordIdN]); 
	}
	int WordKeyId = WordIdSet.FFirstKeyId();
	while (WordIdSet.FNextKeyId(WordKeyId)) {
		Delete(KeyId, WordIdSet.GetKey(WordKeyId), RecId);
	}
}

void TOgIndex::DeleteText(const uchar& StoreId, const TStr& KeyNm, 
		const TStr& TextStr, const uint64& RecId) {

	DeleteText(IndexVoc->GetKeyId(StoreId, KeyNm), TextStr, RecId);
}

void TOgIndex::DeleteJoin(const POgStore& Store, const int& JoinId, 
		const uint64& RecId, const uint64& JoinRecId) {

	Delete(Store->GetJoinKeyId(JoinId), RecId, JoinRecId);	
}

void TOgIndex::DeleteJoin(const POgStore& Store, const TStr& JoinNm, 
		const uint64& RecId, const uint64& JoinRecId) {

	Delete(Store->GetJoinKeyId(JoinNm), RecId, JoinRecId);	
}

void TOgIndex::Delete(const int& KeyId, const uint64& WordId,  const uint64& RecId) {
	Assert(KeyId != -1);
	OgAssertR(!Gix->IsReadOnly(), "Cannot edit read-only index!");
	Gix->DelItem(TOgKeyWord(KeyId, WordId), TOgGixItem(RecId, 0));
}

void TOgIndex::MergeIndex(const POgIndex& TmpIndex) {
    Gix->MergeIndex(TmpIndex->Gix);
}

void TOgIndex::SearchAnd(const TIntUInt64PrV& KeyWordV, TUInt64IntKdV& StoreRecIdFqV) const {
	TVec<POgGixExpItem> ExpItemV(KeyWordV.Len(), 0);
	for (int ItemN = 0; ItemN < KeyWordV.Len(); ItemN++) {
		ExpItemV.Add(TOgGixExpItem::NewItem(KeyWordV[ItemN]));
	}
	POgGixExpItem ExpItem =TOgGixExpItem::NewAndV(ExpItemV);	
    DoQuery(ExpItem, DefMerger, StoreRecIdFqV);
}

void TOgIndex::SearchOr(const TIntUInt64PrV& KeyWordV, TUInt64IntKdV& StoreRecIdFqV) const {
	TVec<POgGixExpItem> ExpItemV(KeyWordV.Len(), 0);
	for (int ItemN = 0; ItemN < KeyWordV.Len(); ItemN++) {
		ExpItemV.Add(TOgGixExpItem::NewItem(KeyWordV[ItemN]));
	}
	POgGixExpItem ExpItem =TOgGixExpItem::NewOrV(ExpItemV);	
    DoQuery(ExpItem, DefMerger, StoreRecIdFqV);
}

TPair<TBool, POgRecSet> TOgIndex::Search(const POgBase& OgBase,
		const TOgQueryItem& QueryItem, const POgGixMerger& Merger) const {

	const uchar StoreId = QueryItem.GetStoreId(OgBase);
	if (QueryItem.Empty()) { return TPair<TBool, POgRecSet>(false, TOgRecSet::New(StoreId)); }
	POgGixExpItem ExpItem = ToExpItem(QueryItem);
	TUInt64IntKdV StoreRecIdFqV;
	const bool NotP = DoQuery(ExpItem, Merger, StoreRecIdFqV);
	POgRecSet RecSet = TOgRecSet::New(StoreId, StoreRecIdFqV, QueryItem.IsWgt());
	return TPair<TBool, POgRecSet>(NotP, RecSet);
}

void TOgIndex::SaveTxt(const POgBase& OgBase, const TStr& FNm) {
	Gix->SaveTxt(FNm, TOgGixKeyStr::New(OgBase, IndexVoc));
}

void TOgTempIndex::NewIndex(const POgIndexVoc& IndexVoc) {
    TUInt64 NowTmMSec = TTm::GetMSecsFromTm(TTm::GetCurUniTm());
    TStr TempIndexFPath = TempFPath + NowTmMSec.GetStr() + "/";
    EAssertR(TDir::GenDir(TempIndexFPath), "Unable to create directory '" + TempIndexFPath + "'");
    TempIndexFPathQ.Push(TempIndexFPath);
    TOg::Logger->OnStatus(TStr::Fmt("Creating a temporary index in %s ...", TempIndexFPath.CStr()));
	TempIndex = TOgIndex::New(TempIndexFPath, faCreate, IndexVoc, IndexCacheSize);
}

void TOgTempIndex::Merge(const POgIndex& Index) {
	TempIndex.Clr();
	while (!TempIndexFPathQ.Empty()) {
        TStr TempIndexFPath = TempIndexFPathQ.Top();
		TempIndexFPathQ.Pop();
        TOg::Logger->OnStatus(TStr::Fmt("Merging a temporary index from %s ...", TempIndexFPath.CStr()));
		POgIndex NewIndex = TOgIndex::New(TempIndexFPath,
            faRdOnly, Index->GetIndexVoc(), int64(10*TInt::Mega));
        Index->MergeIndex(NewIndex);
        TOg::Logger->OnStatus("Closing temporary index Start"); 
		NewIndex.Clr();
		TOg::Logger->OnStatus("Closing temporary index Done");
        TFFile TempFile(TempIndexFPath, ""); TStr DelFNm;
        while (TempFile.Next(DelFNm)) { TFile::Del(DelFNm); }
        if (!TDir::DelDir(TempIndexFPath)) {
            TOg::Logger->OnStatus(
				TStr::Fmt("Unable to delete directory '%s'", TempIndexFPath.CStr())); 
		}
    }
}

bool TOgOp::IsFldNm(const TStrKdV& FldNmValPrV, const TStr& FldNm) {
	const int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
	return (ValN != -1);
}

TStr TOgOp::GetFldVal(const TStrKdV& FldNmValPrV, 
		const TStr& FldNm, const TStr& DefFldVal) {

	const int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
	return (ValN == -1) ? DefFldVal : FldNmValPrV[ValN].Dat;	
}

void TOgOp::GetFldValV(const TStrKdV& FldNmValPrV, const TStr& FldNm, TStrV& FldValV) {
	FldValV.Clr();
	int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
	while (ValN != -1) {
		FldValV.Add(FldNmValPrV[ValN].Dat);
		ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""), ValN + 1);
	}
}

void TOgOp::GetFldValSet(const TStrKdV& FldNmValPrV, const TStr& FldNm, TStrSet& FldValSet) {
	FldValSet.Clr();
	int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
	while (ValN != -1) {
		FldValSet.AddKey(FldNmValPrV[ValN].Dat);
		ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""), ValN + 1);
	}
}

bool TOgOp::IsFldNmVal(const TStrKdV& FldNmValPrV, 
		const TStr& FldNm, const TStr& FldVal) {

	int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
	while (ValN != -1) {
		if (FldNmValPrV[ValN].Dat == FldVal) { return true; }
		ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""), ValN + 1);
	}
	return false;
}

POgRecSet TOgOp::Exec(const POgBase& OgBase, const TOgRecSetV& InOgRecSetV, 
	const TStrKdV& FldNmValPrV){

	OgAssertR(IsFunctional(), "Non-functional operator called as functional!");
	TOgRecSetV OutRSetV; Exec(OgBase, InOgRecSetV, FldNmValPrV, OutRSetV);
	OgAssertR(OutRSetV.Len() == 1, "Non-functional return for functional operator!");
	return OutRSetV[0];
}

POgRecSet TOgBase::Invert(const POgRecSet& RecSet, const TOgIndex::POgGixMerger& Merger) {
	TOgIndex::TOgGixItemV AllResIdV;
	const uchar StoreId = RecSet->GetStoreId();
	const POgStore& Store = GetStoreByStoreId(StoreId);
	POgStoreIter Iter = Store->GetIter();
	while (Iter->Next()) { 
		AllResIdV.Add(TOgIndex::TOgGixItem(Iter->GetRecId(), 1)); 
	}
	if (!AllResIdV.IsSorted()) { AllResIdV.Sort(); }
	TOgIndex::TOgGixItemV ResIdFqV;
	Merger->Minus(AllResIdV, RecSet->GetRecIdFqV(), ResIdFqV);
	return TOgRecSet::New(StoreId, ResIdFqV, false);
}

TPair<TBool, POgRecSet> TOgBase::Search(const TOgQueryItem& QueryItem, const TOgIndex::POgGixMerger& Merger) {
	if (QueryItem.IsLeaf()) {
		return TPair<TBool, POgRecSet>(false, NULL);
	} else if (QueryItem.IsJoin()) {
		TPair<TBool, POgRecSet> NotRecSet = Search(QueryItem.GetItem(0), Merger);
		if (NotRecSet.Val2.Empty()) { NotRecSet = Index->Search(this, QueryItem.GetItem(0), Merger); } 
		// in case it's negated, we must invert it
		if (NotRecSet.Val1) { NotRecSet.Val2 = Invert(NotRecSet.Val2, Merger); }
		POgRecSet JoinRecSet = NotRecSet.Val2->DoJoin(this, QueryItem.GetJoinId(),
			QueryItem.GetSampleSize(), NotRecSet.Val2->IsWgt());
		return TPair<TBool, POgRecSet>(false, JoinRecSet);
	} else if (QueryItem.IsRecSet()) {
		return TPair<TBool, POgRecSet>(false, QueryItem.GetRecSet());
	} else {
		TOgQueryItemType Type = QueryItem.GetType();
		OgAssert(Type == oqitAnd || Type == oqitOr || Type == oqitNot);
		// do all subsequents and keep track if any needs handling
		TBoolV NotV; TOgRecSetV RecSetV; bool EmptyP = true;
		for (int ItemN = 0; ItemN < QueryItem.GetItems(); ItemN++) {
			TPair<TBool, POgRecSet> NotRecSet = Search(QueryItem.GetItem(ItemN), Merger);
			NotV.Add(NotRecSet.Val1); RecSetV.Add(NotRecSet.Val2);
			EmptyP = EmptyP && RecSetV.Last().Empty();
		}
		if (EmptyP) {
			return TPair<TBool, POgRecSet>(false, NULL);
		} else {
			// yup, let's do it!
			if (QueryItem.IsAnd() || QueryItem.IsOr()) {
				// first gather all the subordinate items, that can be handled by index
				TOgQueryItemV IndexQueryItemV;
				for (int ItemN = 0; ItemN < RecSetV.Len(); ItemN++) {
					const POgRecSet& RecSet = RecSetV[ItemN];
					if (RecSet.Empty()) {
						IndexQueryItemV.Add(QueryItem.GetItem(ItemN));
					}
				}
				POgRecSet RecSet; int ItemN = 0; bool NotP = false;
				if (IndexQueryItemV.Empty()) {
					RecSet = RecSetV[0]; ItemN++;
				} else { 
					TOgQueryItem IndexQueryItem(QueryItem.GetType(), IndexQueryItemV);
					TPair<TBool, POgRecSet> NotRecSet = Index->Search(this, IndexQueryItem, Merger);
					NotP = NotRecSet.Val1; RecSet = NotRecSet.Val2;	
				}
				TUInt64IntKdV ResRecIdFqV = RecSet->GetRecIdFqV();
				OgAssert(ResRecIdFqV.IsSorted());
				if (QueryItem.IsAnd()) {
					for (; ItemN < RecSetV.Len(); ItemN++) {
						if (RecSetV[ItemN].Empty()) { continue; }
						const TUInt64IntKdV& RecIdFqV = RecSetV[ItemN]->GetRecIdFqV();
						if (!NotP && !NotV[ItemN]) {
							Merger->Intrs(ResRecIdFqV, RecIdFqV);
						} else if (NotP && NotV[ItemN]) {
							Merger->Union(ResRecIdFqV, RecIdFqV);
						} else if (NotP && !NotV[ItemN]) {
							TUInt64IntKdV _ResRecIdFqV;
							Merger->Minus(RecIdFqV, ResRecIdFqV, _ResRecIdFqV);
							ResRecIdFqV = _ResRecIdFqV;
							NotP = false;
						} else if (!NotP && NotV[ItemN]) {
							TUInt64IntKdV _ResRecIdFqV;
							Merger->Minus(ResRecIdFqV, RecIdFqV, _ResRecIdFqV);
							ResRecIdFqV = _ResRecIdFqV;	
							NotP = false;
						}
					}
				} else if (QueryItem.IsOr()) {
					for (; ItemN < RecSetV.Len(); ItemN++) {
						if (RecSetV[ItemN].Empty()) { continue; }
						const TUInt64IntKdV& RecIdFqV = RecSetV[ItemN]->GetRecIdFqV();
						if (!NotP && !NotV[ItemN]) {
							Merger->Union(ResRecIdFqV, RecIdFqV);
						} else if (NotP && NotV[ItemN]) {
							Merger->Intrs(ResRecIdFqV, RecIdFqV);
						} else if (NotP && !NotV[ItemN]) {
							TUInt64IntKdV _ResRecIdFqV;
							Merger->Minus(ResRecIdFqV, RecIdFqV, _ResRecIdFqV);
							ResRecIdFqV = _ResRecIdFqV;	
							NotP = true;
						} else if (!NotP && NotV[ItemN]) {
							TUInt64IntKdV _ResRecIdFqV;
							Merger->Minus(RecIdFqV, ResRecIdFqV, _ResRecIdFqV);
							ResRecIdFqV = _ResRecIdFqV;
							NotP = true;
						}
					}
				}
				RecSet = TOgRecSet::New(RecSet->GetStoreId(), ResRecIdFqV, QueryItem.IsWgt());
				return TPair<TBool, POgRecSet>(NotP, RecSet);
			} else if (QueryItem.IsNot()) {
				OgAssert(RecSetV.Len() == 1);
				return TPair<TBool, POgRecSet>(!NotV[0], RecSetV[0]);
			}
		}
	}
	TOgExcept::Throw("Unsupported query item type");
	return TPair<TBool, POgRecSet>(false, NULL);
}

TOgBase::TOgBase() { 
	StoreV.Gen(255); StoreV.PutAll(NULL);
}

void TOgBase::PutIndex(const POgIndex& NewIndex) {
    IndexVoc = NewIndex->GetIndexVoc();
    Index = NewIndex;
	TempIndex.Clr();
}

void TOgBase::AddStore(const POgStore& NewStore) {
	StoreV[(int)NewStore->GetStoreId()] = NewStore;
    StoreH.AddDat(NewStore->GetStoreNm(), NewStore);
}

void TOgBase::AddOp(const POgOp& NewOp) {
	OpH.AddDat(NewOp->GetOpNm(), NewOp);
}

void TOgBase::Init() {
	AddOp(TOgOpGetRec::New());
	AddOp(TOgOpSearch::New());
	AddOp(TOgOpLinSearch::New());
	AddOp(TOgOpSort::New());
	AddOp(TOgOpReverse::New());
	AddOp(TOgOpShuffle::New());
	AddOp(TOgOpJoin::New());
	AddOp(TOgOpAggrField::New());
	AddOp(TOgOpAggrKey::New());
	AddOp(TOgOpSplitBy::New());
}

const POgStore& TOgBase::GetStoreByStoreN(const int& StoreN) const {
	OgAssert(IsStoreN(StoreN));
	return StoreH[StoreN]; 
}

const POgStore& TOgBase::GetStoreByStoreId(const uchar& StoreId) const { 
	OgAssert(IsStoreId(StoreId));
	return StoreV[(int)StoreId]; 
}

const POgStore& TOgBase::GetStoreByStoreNm(const TStr& StoreNm) const { 
	OgAssertR(IsStoreNm(StoreNm), "Unknown store name " + StoreNm);
	return StoreH.GetDat(StoreNm); 
}

const POgIndex& TOgBase::GetIndex() const { 
	return TempIndex.Empty() ? Index : TempIndex->GetIndex(); 
}

int TOgBase::NewIndexKey(const POgStore& Store, const TStr& KeyNm, const bool& TextP, 
		const bool& AggrP, const TOgIndexKeySortType& SortType) {

	return NewIndexKey(Store, KeyNm, NewIndexWordVoc(), TextP, AggrP, SortType);
}

int TOgBase::NewIndexKey(const POgStore& Store, const TStr& KeyNm, const int& WordVocId,
		const bool& TextP, const bool& AggrP, const TOgIndexKeySortType& SortType) {

	return IndexVoc->AddKey(Store->GetStoreId(), KeyNm, WordVocId, TextP, AggrP, SortType);
}

int TOgBase::NewFieldIndexKey(const POgStore& Store, const TStr& KeyNm, const int& FieldId,
		const bool& TextP, const bool& AggrP, const TOgIndexKeySortType& SortType) {

	return NewFieldIndexKey(Store, KeyNm, FieldId, NewIndexWordVoc(), TextP, AggrP, SortType);
}

int TOgBase::NewFieldIndexKey(const POgStore& Store, const int& FieldId,
		const bool& TextP, const bool& AggrP, const TOgIndexKeySortType& SortType) {

	return NewFieldIndexKey(Store, Store->GetFieldNm(FieldId), FieldId, NewIndexWordVoc(), TextP, AggrP, SortType);
}

int TOgBase::NewFieldIndexKey(const POgStore& Store, const TStr& KeyNm, const int& FieldId, 
		const int& WordVocId, const bool& TextP, const bool& AggrP, const TOgIndexKeySortType& SortType) {

	const int KeyId = IndexVoc->AddKey(Store->GetStoreId(), KeyNm, WordVocId, TextP, AggrP, SortType);
	IndexVoc->AddKeyField(KeyId, Store->GetStoreId(), FieldId);
	Store->AddFieldKey(FieldId, KeyId);
	return KeyId;
}

int TOgBase::NewFieldIndexKey(const POgStore& Store, const int& FieldId, const int& WordVocId, 
		const bool& TextP, const bool& AggrP, const TOgIndexKeySortType& SortType) {

	return NewFieldIndexKey(Store, Store->GetFieldNm(FieldId), FieldId, WordVocId, TextP, AggrP, SortType);
}

POgRecSet TOgBase::Search(const POgQuery& Query) {
	TOgIndex::POgGixMerger Merger = Index->GetDefMerger();
	TPair<TBool, POgRecSet> NotRecSet = Search(Query->GetQueryItem(), Merger);
	if (NotRecSet.Val2.Empty()) { 
		NotRecSet = Index->Search(this, Query->GetQueryItem(), Merger); 
	}
	POgRecSet RecSet = NotRecSet.Val2;
	if (NotRecSet.Val1) { RecSet = Invert(NotRecSet.Val2, Merger); }
	if (Query->IsSort()) { Query->Sort(this, RecSet); }
	if (Query->IsLimit()) { RecSet = Query->GetLimit(RecSet); }
	return RecSet;
}

POgRecSet TOgBase::Search(const TStr& QueryStr) {
	return Search(TOgQuery::New(this, QueryStr));
}

void TOgBase::InitTempIndex(const TStr& TempFPath, const uint64& IndexCacheSize) { 
	TempIndex = TOgTempIndex::New(TempFPath, IndexCacheSize); 
	TempIndex->NewIndex(IndexVoc);
}

void TOgBase::PrintStores(const TStr& FNm) {
	TFOut FOut(FNm);
	for (int StoreN = 0; StoreN < GetStores(); StoreN++) {
		POgStore Store = GetStoreByStoreN(StoreN);
		FOut.PutStrLn("--------------------------------------------------------------");
		Store->PrintTypes(this, FOut);
	}
	FOut.PutStrLn("--------------------------------------------------------------");
}

void TOgBase::PrintIndexVoc(const TStr& FNm) {
	IndexVoc->SaveTxt(this, FNm);
}

void TOgBase::PrintIndex(const TStr& FNm, const bool& SortP) {
	Index->SaveTxt(this, FNm);
	if (SortP) {
		TIntKdV SizeIdV;
		PBigStrPool StrPool = TBigStrPool::New();
		{TFIn FIn(FNm); TStr LnStr; 
		while (FIn.GetNextLn(LnStr)) {
			if (LnStr.IsWs()) { continue; }
			TStr SizeStr = LnStr.RightOfLast('\t');
			int Size;
			if (SizeStr.IsInt(Size)) {
				SizeIdV.Add(TIntKd(Size, StrPool->AddStr(LnStr)));
			}
		}}
		SizeIdV.Sort(false); TFOut FOut(FNm);
		for (int SizeIdN = 0; SizeIdN < SizeIdV.Len(); SizeIdN++) {
			const int StrId = SizeIdV[SizeIdN].Dat;
			FOut.PutStrLn(StrPool->GetStr(StrId));
		}
	}
}

void TOgTimeIndexKeyId::InitVoc(const POgIndexVoc& IndexVoc) {
	for (int MonthN = 1; MonthN <= 12; MonthN++) { 
		IndexVoc->AddWordStr(MonthKeyId, TTmInfo::GetMonthNm(MonthN)); 
	}
	for (int DayOfMonthN = 1; DayOfMonthN <= 32; DayOfMonthN++) {
		IndexVoc->AddWordStr(DayOfMonthKeyId, TInt::GetStr(DayOfMonthN)); 
	}
	for (int DayN = 1; DayN <= 7; DayN++) {
		IndexVoc->AddWordStr(DayOfWeekKeyId, TTmInfo::GetDayOfWeekNm(DayN)); 
	}
	IndexVoc->AddWordStr(TimeOfDayKeyId, "Morning");
	IndexVoc->AddWordStr(TimeOfDayKeyId, "Afternoon");
	IndexVoc->AddWordStr(TimeOfDayKeyId, "Evening");
	IndexVoc->AddWordStr(TimeOfDayKeyId, "Night");
	for (int HourN = 0; HourN <= 24; HourN++) {
		IndexVoc->AddWordStr(HourKeyId, TInt::GetStr(HourN)); 
	}
}

void TOgTimeIndexKeyId::Init(const POgBase& OgBase, const POgStore& Store) {
	DateKeyId = OgBase->NewIndexKey(Store, PrefixStr + "Date", false, true, oikstByStr);
	if (!FullP) { return; }
	YearKeyId = OgBase->NewIndexKey(Store, PrefixStr + "Year", false, true, oikstByStr);
	MonthKeyId = OgBase->NewIndexKey(Store, PrefixStr + "Month", false, true, oikstById);
	DayOfMonthKeyId = OgBase->NewIndexKey(Store, PrefixStr + "DayOfMonth", false, true, oikstById);
	DayOfWeekKeyId = OgBase->NewIndexKey(Store, PrefixStr + "DayOfWeek", false, true, oikstById);
	TimeOfDayKeyId = OgBase->NewIndexKey(Store, PrefixStr + "TimeOfDay", false, true, oikstByStr);
	HourKeyId = OgBase->NewIndexKey(Store, PrefixStr + "Hour", false, true, oikstById);
	InitVoc(OgBase->GetIndexVoc());
}

void TOgTimeIndexKeyId::Init(const POgBase& OgBase, const POgStore& Store, const int& FieldId) {
	DateKeyId = OgBase->NewFieldIndexKey(Store, PrefixStr + "Date", FieldId, false, true, oikstByStr);
	if (!FullP) { return; }
	YearKeyId = OgBase->NewFieldIndexKey(Store, PrefixStr + "Year", FieldId, false, true, oikstByStr);
	MonthKeyId = OgBase->NewFieldIndexKey(Store, PrefixStr + "Month", FieldId, false, true, oikstById);
	DayOfMonthKeyId = OgBase->NewFieldIndexKey(Store, PrefixStr + "DayOfMonth", FieldId, false, true, oikstById);
	DayOfWeekKeyId = OgBase->NewFieldIndexKey(Store, PrefixStr + "DayOfWeek", FieldId, false, true, oikstById);
	TimeOfDayKeyId = OgBase->NewFieldIndexKey(Store, PrefixStr + "TimeOfDay", FieldId, false, true, oikstByStr);
	HourKeyId = OgBase->NewFieldIndexKey(Store, PrefixStr + "Hour", FieldId, false, true, oikstById);
	InitVoc(OgBase->GetIndexVoc());
}

void TOgTimeIndexKeyId::Load(const POgBase& OgBase, const POgStore& Store) {
	const POgIndexVoc& IndexVoc =  OgBase->GetIndexVoc();
	DateKeyId = IndexVoc->GetKeyId(Store->GetStoreId(), PrefixStr + "Date");
	if (!FullP) { return; }
	YearKeyId = IndexVoc->GetKeyId(Store->GetStoreId(), PrefixStr + "Year");
	MonthKeyId = IndexVoc->GetKeyId(Store->GetStoreId(), PrefixStr + "Month");
	DayOfMonthKeyId = IndexVoc->GetKeyId(Store->GetStoreId(), PrefixStr + "DayOfMonth");
	DayOfWeekKeyId = IndexVoc->GetKeyId(Store->GetStoreId(), PrefixStr + "DayOfWeek");
	TimeOfDayKeyId = IndexVoc->GetKeyId(Store->GetStoreId(), PrefixStr + "TimeOfDay");
	HourKeyId = IndexVoc->GetKeyId(Store->GetStoreId(), PrefixStr + "Hour");
}

void TOgTimeIndexKeyId::ParseSecTm(const TSecTm& SecTm, TStrPrV& KeyWordV) {
	ParseSecTm(PrefixStr, SecTm, KeyWordV, FullP);
}

void TOgTimeIndexKeyId::ParseSecTm(const TStr& PrefixStr, 
		const TSecTm& SecTm, TStrPrV& KeyWordV, const bool& FullP) {

	if (!SecTm.IsDef()) { return; }
	KeyWordV.Add(TStrPr(PrefixStr + "Date", SecTm.GetDtYmdStr()));
	if (!FullP) { return; }
	KeyWordV.Add(TStrPr(PrefixStr + "Year", TInt::GetStr(SecTm.GetYearN())));
	KeyWordV.Add(TStrPr(PrefixStr + "Month", SecTm.GetMonthNm()));
	KeyWordV.Add(TStrPr(PrefixStr + "DayOfMonth", TInt::GetStr(SecTm.GetDayN())));
	KeyWordV.Add(TStrPr(PrefixStr + "DayOfWeek", SecTm.GetDayOfWeekNm()));
	KeyWordV.Add(TStrPr(PrefixStr + "TimeOfDay", SecTm.GetDayPart()));
	KeyWordV.Add(TStrPr(PrefixStr + "Hour", TInt::GetStr(SecTm.GetHourN())));
}

void TOgTimeIndexKeyId::IndexSecTm(const POgBase& OgBase, 
		const TSecTm& SecTm, const uchar& StoreId, const uint64& RecId) {

	const POgIndex& Index =  OgBase->GetIndex();
	Index->Index(DateKeyId, SecTm.GetDtYmdStr(), RecId);
	if (!FullP) { return; }
	Index->Index(YearKeyId, TInt::GetStr(SecTm.GetYearN()), RecId);
	Index->Index(MonthKeyId, SecTm.GetMonthNm(), RecId);
	Index->Index(DayOfMonthKeyId, TInt::GetStr(SecTm.GetDayN()), RecId);
	Index->Index(DayOfWeekKeyId, SecTm.GetDayOfWeekNm(), RecId);
	Index->Index(TimeOfDayKeyId, SecTm.GetDayPart(), RecId);
	Index->Index(HourKeyId, TInt::GetStr(SecTm.GetHourN()), RecId);
}

TOgRec TOgFtrExt::DoSingleJoin(const POgBase& OgBase, const TOgRec& FtrRec) const {
	OgAssertR(IsStartStore(FtrRec.GetStoreId()), "Start store not supported!");
	return FtrRec.DoSingleJoin(OgBase, GetJoinIdV(FtrRec.GetStoreId()));
}

TOgFtrExt::TOgFtrExt(const POgStore& Store): FtrStoreId(Store->GetStoreId()) { 
	JoinSeqH.AddDat(Store->GetStoreId(), TOgJoinSeq(Store->GetStoreId()));
}

TOgFtrExt::TOgFtrExt(const POgBase& OgBase, const TOgJoinSeq& JoinSeq) {
	JoinSeqH.AddDat(JoinSeq.GetStartStoreId(), JoinSeq);
	FtrStoreId = JoinSeqH[0].GetEndStoreId(OgBase);
}
	
TOgFtrExt::TOgFtrExt(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV) {
	OgAssertR(!JoinSeqV.Empty(), "At least one join sequence must be supplied!");
	for (int JoinSeqN = 0; JoinSeqN < JoinSeqV.Len(); JoinSeqN++) {
		const TOgJoinSeq& JoinSeq = JoinSeqV[JoinSeqN];
		JoinSeqH.AddDat(JoinSeq.GetStartStoreId(), JoinSeq);
	}
	FtrStoreId = JoinSeqH[0].GetEndStoreId(OgBase);
}

void TOgFtrExt::ExtractStrV(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const { 
	TOgExcept::Throw("ExtractStrV not implemented!"); 
}

void TOgFtrExt::ExtractFltV(const POgBase& OgBase, const TOgRec& FtrRec, TFltV& FltV) const {
	TOgExcept::Throw("ExtractFltV not implemented!"); 
}

void TOgFtrExt::ExtractTmV(const POgBase& OgBase, const TOgRec& FtrRec, TTmV& TmV) const {
	TOgExcept::Throw("ExtractTmV not implemented!"); 
}

POgFtrExt TOgFtrExt::GetFtrExt(const POgBase& OgBase, const POgStore& Store, const int& FieldId, const bool& SetP) {
	return GetFtrExt(OgBase, TOgJoinSeqV::GetV(TOgJoinSeq(Store->GetStoreId())), FieldId, SetP);
}

POgFtrExt TOgFtrExt::GetFtrExt(const POgBase& OgBase, const TOgJoinSeq& JoinSeq, const int& FieldId, const bool& SetP) {
	return GetFtrExt(OgBase, TOgJoinSeqV::GetV(JoinSeq), FieldId, SetP);
}

POgFtrExt TOgFtrExt::GetFtrExt(const POgBase& OgBase, const TOgJoinSeqV& JoinSeqV, const int& FieldId, const bool& SetP) {
	OgAssertR(!JoinSeqV.Empty(), "At least one join sequence must be supplied!");
	const TOgFieldDesc& FieldDesc = JoinSeqV[0].GetEndStore(OgBase)->GetFieldDesc(FieldId);
	if (FieldDesc.IsDefFtrNumeric()) {
		return TOgFtrExtNum::New(OgBase, JoinSeqV, FieldId);
	} else if (FieldDesc.IsDefFtrNominal()) {
		if (SetP) { return TOgFtrExtMultiNom::New(OgBase, JoinSeqV, FieldId); }
		else { return TOgFtrExtNom::New(OgBase, JoinSeqV, FieldId); }
	} else if (FieldDesc.IsDefFtrMultiNom()) {
		return TOgFtrExtMultiNom::New(OgBase, JoinSeqV, FieldId);
	} else if (FieldDesc.IsDefFtrToken()) {
		return TOgFtrExtToken::New(OgBase, JoinSeqV, FieldId);
	} else if (FieldDesc.IsDefFtrSpNum()) {		
		TOgExcept::Throw("Feature not yet implemented"); return NULL; //TODO
	} else if (FieldDesc.IsDefFtrTm()) {
		return TOgFtrExtMultiNom::New(OgBase, JoinSeqV, FieldId);
	}
	TOgExcept::Throw("Unknown default feature type " + FieldDesc.GetDefFtrTypeStr());
	return NULL;
}

TStr TOgFtrSpace::GetNm(const POgBase& OgBase) const {
	TChA NmChA = "Space: [";
	for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
		if (FtrExtN > 0) { NmChA += ", "; }
		NmChA += FtrExtV[FtrExtN]->GetNm(OgBase);
	}
	NmChA += "]";
	return NmChA;
}

void TOgFtrSpace::Clr(const POgBase& OgBase) {
	UpdateFinishedP = false;
	for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
		FtrExtV[FtrExtN]->Clr(OgBase);
	}
}

void TOgFtrSpace::Update(const POgBase& OgBase, const TOgRec& FtrRec) {
	UpdateFinishedP = false;
	for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
		FtrExtV[FtrExtN]->Update(OgBase, FtrRec);
	}
}

void TOgFtrSpace::Update(const POgBase& OgBase, const POgRecSet& FtrRecSet, const PNotify& Notify) {
	UpdateFinishedP = false;
	for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
		if (RecN % 1000 == 0) { Notify->OnStatusFmt("%d\r", RecN); }
		Update(OgBase, FtrRecSet->GetRec(RecN));
	}
}

void TOgFtrSpace::FinishUpdate(const POgBase& OgBase) {
	DimV.Gen(FtrExtV.Len(), 0); Dim = 0;
	for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
		FtrExtV[FtrExtN]->FinishUpdate(OgBase);
		const int FtrExtDim = FtrExtV[FtrExtN]->GetDim(OgBase);
		DimV.Add(FtrExtDim);
		Dim += FtrExtDim;
	}
	UpdateFinishedP = true;
}

void TOgFtrSpace::GetSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV) const {
	OgAssertR(UpdateFinishedP, "Feature space not fully defined (no call to FinishUpdate())!");
	int Offset = 0;
	for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
		FtrExtV[FtrExtN]->AddSpV(OgBase, FtrRec, SpV, Offset);
	}
}

void TOgFtrSpace::GetSpVV(const POgBase& OgBase, const POgRecSet& FtrRecSet, 
		TVec<TIntFltKdV>& SpVV, const PNotify& Notify) const {

	for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
		if (RecN % 1000 == 0) { Notify->OnStatusFmt("%d\r", RecN); }
		SpVV.Add(TIntFltKdV()); GetSpV(OgBase, FtrRecSet->GetRec(RecN), SpVV.Last());
	}
}
	
void TOgFtrSpace::GetCentroidSpV(const POgBase& OgBase, const POgRecSet& FtrRecSet, 
		TIntFltKdV& CentroidSpV, const bool& NormalizeP) const {

	for (int RecStrN = 0; RecStrN < FtrRecSet->GetRecs(); RecStrN++) { 
		TIntFltKdV RecSpV; GetSpV(OgBase, FtrRecSet->GetRec(RecStrN), RecSpV);
		TIntFltKdV SumSpV;
		TLinAlg::AddVec(RecSpV, CentroidSpV, SumSpV);
		CentroidSpV = SumSpV;
	}
	if (NormalizeP) { TLinAlg::Normalize(CentroidSpV); }
}

void TOgFtrSpace::GetCentroidV(const POgBase& OgBase, const POgRecSet& FtrRecSet,
		TFltV& CentroidV, const bool& NormalizeP) const {

	CentroidV.Gen(GetDim()); CentroidV.PutAll(0.0);
	for (int RecStrN = 0; RecStrN < FtrRecSet->GetRecs(); RecStrN++) { 
		TIntFltKdV RecSpV; GetSpV(OgBase, FtrRecSet->GetRec(RecStrN), RecSpV);
		TLinAlg::AddVec(1.0, RecSpV, CentroidV, CentroidV);
	}
	if (NormalizeP) { TLinAlg::Normalize(CentroidV); }
}

int TOgFtrSpace::GetDim() const {
	OgAssertR(UpdateFinishedP, "Feature space not fully defined (no call to FinishUpdate())!");
	return Dim;
}

TStr TOgFtrSpace::GetFtr(const POgBase& OgBase, const int& FtrN) const {
	OgAssertR(UpdateFinishedP, "Feature space not fully defined (no call to FinishUpdate())!");
	int SumDim = 0;
	for (int DimN = 0; DimN < DimV.Len(); DimN++) {
		SumDim += DimV[DimN];
		if (SumDim > FtrN) { 
			const int LocalFtrN = FtrN - (SumDim - DimV[DimN]);
			return FtrExtV[DimN]->GetFtr(OgBase, LocalFtrN);
		}
	}
	TOgExcept::Throw("Feature number out of bounds!");
	return TStr();
}

void TOgFtrExtRnd::AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {
	SpV.Add(TIntFltKd(Offset, Rnd.GetUniDev())); Offset++;
}

void TOgFtrExtRnd::ExtractFltV(const POgBase& OgBase, const TOgRec& FtrRec, TFltV& FltV) const {
	FltV.Add(Rnd.GetUniDev());
}

double TOgFtrExtNum::_GetVal(const POgBase& OgBase, const POgRecSet& RecSet) const {
	const POgStore& FtrStore = OgBase->GetStoreByStoreId(GetFtrStoreId());
	Assert(RecSet->GetStoreId() == FtrStore->GetStoreId());
	const TOgFieldDesc& FieldDesc = FtrStore->GetFieldDesc(FieldId);
	double FieldVal = 0.0;
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
		const uint64 RecId = RecSet->GetRecId(RecN);
		if (FieldDesc.IsInt()) {
			FieldVal += (double)FtrStore->GetFieldInt(RecId, FieldId);
		} else if (FieldDesc.IsFlt()) {
			FieldVal += FtrStore->GetFieldFlt(RecId, FieldId);
		} else if (FieldDesc.IsUInt64()) {
			FieldVal += (double)FtrStore->GetFieldUInt64(RecId, FieldId);
		} else if (FieldDesc.IsBool()) {
			FieldVal +=  FtrStore->GetFieldBool(RecId, FieldId) ? 1.0 : 0.0;
		} else {
			TOgExcept::Throw("Field feature type " + FieldDesc.GetDefFtrTypeStr() + 
				"not supported by Feature-Numeric!");
		}
	}
	return FieldVal;
}

double TOgFtrExtNum::_GetVal(const POgBase& OgBase, const TOgRec& FtrRec) const {
	const POgStore& FtrStore = OgBase->GetStoreByStoreId(GetFtrStoreId());
	Assert(FtrRec.GetStoreId() == FtrStore->GetStoreId());
	const TOgFieldDesc& FieldDesc = FtrStore->GetFieldDesc(FieldId);
	if (FieldDesc.IsInt()) {
		return (double)FtrRec.GetFieldInt(FieldId);
	} else if (FieldDesc.IsFlt()) {
		return FtrRec.GetFieldFlt(FieldId);
	} else if (FieldDesc.IsUInt64()) {
		return (double)FtrRec.GetFieldUInt64(FieldId);
	} else if (FieldDesc.IsBool()) {
		return FtrRec.GetFieldBool(FieldId) ? 1.0 : 0.0;
	}
	TOgExcept::Throw("Field feature type " + FieldDesc.GetDefFtrTypeStr() + 
		"not supported by Feature-Numeric!");
	return 0.0;
}

double TOgFtrExtNum::GetVal(const POgBase& OgBase, const TOgRec& FtrRec) const {
	Assert(IsStartStore(FtrRec.GetStoreId()));
	if (!IsJoin(FtrRec.GetStoreId())) {
		if (FtrRec.IsByRef()) {
			return _GetVal(OgBase, FtrRec.ToRecSet());
		} else {
			return _GetVal(OgBase, FtrRec);
		}
	} else {
		POgRecSet RecSet = FtrRec.DoJoin(OgBase, GetJoinIdV(FtrRec.GetStoreId()));
		return _GetVal(OgBase, RecSet);
	}
}

void TOgFtrExtNum::Update(const POgBase& OgBase, const TOgRec& FtrRec) {
	FtrGen.Update(GetVal(OgBase, FtrRec));
}

void TOgFtrExtNum::AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {
	FtrGen.AddFtr(GetVal(OgBase, FtrRec), SpV, Offset);
}

void TOgFtrExtNum::ExtractFltV(const POgBase& OgBase, const TOgRec& FtrRec, TFltV& FltV) const {
	FltV.Add(GetVal(OgBase, FtrRec));
}

TStr TOgFtrExtNom::_GetVal(const POgBase& OgBase, const TOgRec& FtrRec) const {
	const POgStore& FtrStore = OgBase->GetStoreByStoreId(GetFtrStoreId());
	Assert(FtrRec.GetStoreId() == FtrStore->GetStoreId());
	const TOgFieldDesc& FieldDesc = FtrStore->GetFieldDesc(FieldId);
	if (FieldDesc.IsStr()) {
		return FtrRec.IsByVal() ? FtrRec.GetFieldStr(FieldId) :
			FtrStore->GetFieldStr(FtrRec.GetRecId(), FieldId);
	} else if (FieldDesc.IsInt()) {
		const int FieldVal = FtrRec.IsByVal() ? 
			FtrRec.GetFieldInt(FieldId) :
			FtrStore->GetFieldInt(FtrRec.GetRecId(), FieldId);
		return TInt::GetStr(FieldVal);
	} else if (FieldDesc.IsUInt64()) {
		const uint64 FieldVal = FtrRec.IsByVal() ? 
			FtrRec.GetFieldUInt64(FieldId) :
			FtrStore->GetFieldUInt64(FtrRec.GetRecId(), FieldId);
		return TUInt64::GetStr(FieldVal);
	} else if (FieldDesc.IsBool()) {
		const bool FieldVal = FtrRec.IsByVal() ? 
			FtrRec.GetFieldBool(FieldId) :
			FtrStore->GetFieldBool(FtrRec.GetRecId(), FieldId);
		return FieldVal ? "Yes" : "No";
	}
	TOgExcept::Throw("Field feature type " + FieldDesc.GetDefFtrTypeStr() + 
		"not supported by Feature-Extractor-Nominal!");
	return 0;
}

TStr TOgFtrExtNom::GetVal(const POgBase& OgBase, const TOgRec& FtrRec) const {
	Assert(IsStartStore(FtrRec.GetStoreId()));
	if (!IsJoin(FtrRec.GetStoreId())) {
		return _GetVal(OgBase, FtrRec);
	} else {
		TOgRec JoinRec = DoSingleJoin(OgBase, FtrRec);
		return _GetVal(OgBase, JoinRec);
	}
}

void TOgFtrExtNom::Update(const POgBase& OgBase, const TOgRec& FtrRec) {
	FtrGen.Update(GetVal(OgBase, FtrRec));
}

void TOgFtrExtNom::AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {
	FtrGen.AddFtr(GetVal(OgBase, FtrRec), SpV, Offset);
}

void TOgFtrExtNom::ExtractStrV(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const {
	StrV.Add(GetVal(OgBase, FtrRec));
}

void TOgFtrExtMultiNom::ParseDate(const TTm& Tm, TStrV& StrV) const {
	TSecTm SecTm = Tm.GetSecTm();
	StrV.Add(SecTm.GetDtYmdStr());
	StrV.Add(SecTm.GetMonthNm());
	StrV.Add(TInt::GetStr(SecTm.GetDayN()));
	StrV.Add(SecTm.GetDayOfWeekNm());
	StrV.Add(SecTm.GetDayPart());
}

void TOgFtrExtMultiNom::_GetVal(const POgBase& OgBase, const POgRecSet& RecSet, TStrV& StrV) const {
	const POgStore& FtrStore = OgBase->GetStoreByStoreId(GetFtrStoreId());
	Assert(RecSet->GetStoreId() == FtrStore->GetStoreId());
	const TOgFieldDesc& FieldDesc = FtrStore->GetFieldDesc(FieldId);
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
		const uint64 RecId = RecSet->GetRecId(RecN);
		if (FieldDesc.IsStr()) {
			StrV.Add(FtrStore->GetFieldStr(RecId, FieldId));
		} else if (FieldDesc.IsStrV()) {
			TStrV RecStrV; FtrStore->GetFieldStrV(RecId, FieldId, RecStrV);
			StrV.AddV(RecStrV);
		} else if (FieldDesc.IsInt()) {
			StrV.Add(TInt::GetStr(FtrStore->GetFieldInt(RecId, FieldId)));
		} else if (FieldDesc.IsIntV()) {
			TIntV RecIntV; FtrStore->GetFieldIntV(RecId, FieldId, RecIntV);
			for (int RecIntN = 0; RecIntN < RecIntV.Len(); RecIntN++) {
				StrV.Add(RecIntV[RecIntN].GetStr());
			}
		} else if (FieldDesc.IsUInt64()) {
			StrV.Add(TUInt64::GetStr(FtrStore->GetFieldUInt64(RecId, FieldId)));
		} else if (FieldDesc.IsBool()) {
			StrV.Add(FtrStore->GetFieldBool(RecId, FieldId) ? "Yes" : "No");
		} else if (FieldDesc.IsTm()) {
			TTm FieldTm; FtrStore->GetFieldTm(RecId, FieldId, FieldTm);
			ParseDate(FieldTm, StrV);
		} else {
			TOgExcept::Throw("Field feature type " + FieldDesc.GetDefFtrTypeStr() + 
				"not supported by Feature-Multi-Nominal!");
		}
	}
}

void TOgFtrExtMultiNom::_GetVal(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const {
	const POgStore& FtrStore = OgBase->GetStoreByStoreId(GetFtrStoreId());
	Assert(FtrRec.GetStoreId() == FtrStore->GetStoreId());
	const TOgFieldDesc& FieldDesc = FtrStore->GetFieldDesc(FieldId);
	if (FieldDesc.IsStr()) {
		StrV.Add(FtrRec.GetFieldStr(FieldId));
	} else if (FieldDesc.IsStrV()) {
		TStrV RecStrV; FtrRec.GetFieldStrV(FieldId, RecStrV);
		StrV.AddV(RecStrV);
	} else if (FieldDesc.IsInt()) {
		StrV.Add(TInt::GetStr(FtrRec.GetFieldInt(FieldId)));
	} else if (FieldDesc.IsIntV()) {
		TIntV RecIntV; FtrRec.GetFieldIntV(FieldId, RecIntV);
		for (int RecIntN = 0; RecIntN < RecIntV.Len(); RecIntN++) {
			StrV.Add(RecIntV[RecIntN].GetStr());
		}
	} else if (FieldDesc.IsUInt64()) {
		StrV.Add(TUInt64::GetStr(FtrRec.GetFieldUInt64(FieldId)));
	} else if (FieldDesc.IsBool()) {
		StrV.Add(FtrRec.GetFieldBool(FieldId) ? "Yes" : "No");
	} else if (FieldDesc.IsTm()) {
		TTm FieldTm; FtrRec.GetFieldTm(FieldId, FieldTm);
		ParseDate(FieldTm, StrV);
	}
	TOgExcept::Throw("Field feature type " + FieldDesc.GetDefFtrTypeStr() + 
		"not supported by Feature-Multi-Nominal!");
}

void TOgFtrExtMultiNom::GetVal(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const {
	Assert(IsStartStore(FtrRec.GetStoreId()));
	if (!IsJoin(FtrRec.GetStoreId())) {
		if (FtrRec.IsByRef()) {
			_GetVal(OgBase, FtrRec.ToRecSet(), StrV);
		} else {
			_GetVal(OgBase, FtrRec, StrV);
		}
	} else {
		POgRecSet RecSet = FtrRec.DoJoin(OgBase, GetJoinIdV(FtrRec.GetStoreId()));
		_GetVal(OgBase, RecSet, StrV);
	}
}

void TOgFtrExtMultiNom::Update(const POgBase& OgBase, const TOgRec& FtrRec) {
	TStrV StrV; GetVal(OgBase, FtrRec, StrV);
	FtrGen.Update(StrV);
}

void TOgFtrExtMultiNom::AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {

	TStrV StrV; GetVal(OgBase, FtrRec, StrV);
	FtrGen.AddFtr(StrV, SpV, Offset);
}

void TOgFtrExtMultiNom::ExtractStrV(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const {
	GetVal(OgBase, FtrRec, StrV);
}

void TOgFtrExtMultiNom::ExtractTmV(const POgBase& OgBase, const TOgRec& FtrRec, TTmV& TmV) const {
	const POgStore& FtrStore = OgBase->GetStoreByStoreId(GetFtrStoreId());
	Assert(FtrRec.GetStoreId() == FtrStore->GetStoreId());
	const TOgFieldDesc& FieldDesc = FtrStore->GetFieldDesc(FieldId);
	Assert(IsStartStore(FtrRec.GetStoreId()));
	if (FieldDesc.IsTm()) {
		if (!IsJoin(FtrRec.GetStoreId())) {
			TTm FieldTm;
			if (FtrRec.IsByRef()) {
				FtrStore->GetFieldTm(FtrRec.GetRecId(), FieldId, FieldTm);
			} else {
				FtrRec.GetFieldTm(FieldId, FieldTm);
			}
			TmV.Add(FieldTm);
		} else {
			POgRecSet RecSet = FtrRec.DoJoin(OgBase, GetJoinIdV(FtrRec.GetStoreId()));
			for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
				TTm FieldTm; 
				FtrStore->GetFieldTm(FtrRec.GetRecId(), FieldId, FieldTm);				
				TmV.Add(FieldTm);
			}
		}
	} else {
		TOgExcept::Throw("Expected TTm type, but found " + FieldDesc.GetDefFtrTypeStr());
	}
}

void TOgFtrExtToken::_GetVal(const POgBase& OgBase, const POgRecSet& RecSet, TStrV& StrV) const {
	const POgStore& FtrStore = OgBase->GetStoreByStoreId(GetFtrStoreId());
	Assert(RecSet->GetStoreId() == FtrStore->GetStoreId());
	const TOgFieldDesc& FieldDesc = FtrStore->GetFieldDesc(FieldId);
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
		const uint64 RecId = RecSet->GetRecId(RecN);
		if (FieldDesc.IsStr()) {
			StrV.Add(FtrStore->GetFieldStr(RecId, FieldId));
		} else if (FieldDesc.IsStrV()) {
			TStrV RecStrV; FtrStore->GetFieldStrV(RecId, FieldId, RecStrV);
			StrV.AddV(RecStrV);
		} else {
			TOgExcept::Throw("Field feature type " + FieldDesc.GetDefFtrTypeStr() + 
				"not supported by Feature-Tokenizer!");
		}
	}
}

void TOgFtrExtToken::_GetVal(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const {
	const POgStore& FtrStore = OgBase->GetStoreByStoreId(GetFtrStoreId());
	Assert(FtrRec.GetStoreId() == FtrStore->GetStoreId());
	const TOgFieldDesc& FieldDesc = FtrStore->GetFieldDesc(FieldId);
	if (FieldDesc.IsStr()) {
		StrV.Add(FtrRec.GetFieldStr(FieldId));
	} else if (FieldDesc.IsStrV()) {
		TStrV RecStrV; FtrRec.GetFieldStrV(FieldId, RecStrV);
		StrV.AddV(RecStrV);
	}
	TOgExcept::Throw("Field feature type " + FieldDesc.GetDefFtrTypeStr() + 
		"not supported by Feature-Tokenizer!");
}

void TOgFtrExtToken::GetVal(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const {
	Assert(IsStartStore(FtrRec.GetStoreId()));
	if (!IsJoin(FtrRec.GetStoreId())) {
		if (FtrRec.IsByRef()) {
			_GetVal(OgBase, FtrRec.ToRecSet(), StrV);
		} else {
			_GetVal(OgBase, FtrRec, StrV);
		}
	} else {
		POgRecSet RecSet = FtrRec.DoJoin(OgBase, GetJoinIdV(FtrRec.GetStoreId()));
		_GetVal(OgBase, RecSet, StrV);
	}
}

void TOgFtrExtToken::Update(const POgBase& OgBase, const TOgRec& FtrRec) {
	TStrV RecStrV; GetVal(OgBase, FtrRec, RecStrV);
	if (Mode == ofetmConcat) {
		FtrGen.Update(TStr::GetStr(RecStrV, "\n"));
	} else if (Mode == ofetmCentroid) {
		for (int RecStrN = 0; RecStrN < RecStrV.Len(); RecStrN++) { 
			FtrGen.Update(RecStrV[RecStrN]); }
	} else {
		TOgExcept::Throw("Unknown tokenizer mode for handling multiple instances");
	}
}

void TOgFtrExtToken::AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {
	TStrV RecStrV; GetVal(OgBase, FtrRec, RecStrV);
	if (Mode == ofetmConcat) {
		FtrGen.AddFtr(TStr::GetStr(RecStrV, "\n"), SpV, Offset);
	} else if (Mode == ofetmCentroid) {
		TIntFltKdV CentroidSpV;
		for (int RecStrN = 0; RecStrN < RecStrV.Len(); RecStrN++) { 
			TIntFltKdV RecSpV; FtrGen.AddFtr(RecStrV[RecStrN], RecSpV);
			TIntFltKdV SumSpV;
			TLinAlg::AddVec(RecSpV, CentroidSpV, SumSpV);
			CentroidSpV = SumSpV;
		}
		TLinAlg::Normalize(CentroidSpV);
		for (int SpN = 0; SpN < CentroidSpV.Len(); SpN++) {
			const int Id = CentroidSpV[SpN].Key;
			const double Wgt = CentroidSpV[SpN].Dat;
			SpV.Add(TIntFltKd(Offset + Id, Wgt));
		}
		Offset += FtrGen.GetVals();
	} else {
		TOgExcept::Throw("Unknown tokenizer mode for handling multiple instances");
	}

}

void TOgFtrExtToken::ExtractStrV(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const {
	TStrV RecStrV; GetVal(OgBase, FtrRec, RecStrV);
	for (int RecStrN = 0; RecStrN < RecStrV.Len(); RecStrN++) { 
		FtrGen.GetTokenV(RecStrV[RecStrN], StrV);
	}
}

void TOgFtrExtJoin::Def(const POgBase& OgBase) {
	OgAssertR(Sample > 0, "Sample parameter must be > 0!");
	uint64 MxRecId = 0; Dim = 0;
	const POgStore& FtrStore = OgBase->GetStoreByStoreId(GetFtrStoreId());
	POgStoreIter Iter = FtrStore->GetIter();
	while (Iter->Next()) {
		const uint64 RecId = Iter->GetRecId() / Sample;
		if (RecId > MxRecId) { MxRecId = RecId; }
	}
	if (MxRecId > 0) { Dim = (int)(MxRecId + 1); }
}

void TOgFtrExtJoin::AddSpV(const POgBase& OgBase, const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {
	Assert(IsStartStore(FtrRec.GetStoreId()));
	POgRecSet RecSet = FtrRec.DoJoin(OgBase, GetJoinIdV(FtrRec.GetStoreId())); 
	RecSet->SortById(true);
	const int FirstSpN = SpV.Len(); 
	int SampleId = 0; double NormSq = 0.0, SampleFq = 0.0; 
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
		const int Id = (int)RecSet->GetRecId(RecN) / Sample;
		const double Fq = (double)RecSet->GetRecFq(RecN);
		if (Id < Dim) { 
			if (Id == SampleId) {
				SampleFq += Fq;
			} else {
				NormSq += TMath::Sqr(SampleFq);
				SpV.Add(TIntFltKd(Offset + SampleId, SampleFq)); 
				SampleId = Id; SampleFq = Fq;
			}
		} else {
			break; // we are out of defined feature space
		}
	}
	// add last element if nonzero
	if (SampleFq > 0.0) {
		NormSq += TMath::Sqr(SampleFq);
		SpV.Add(TIntFltKd(Offset + SampleId, SampleFq)); 
	}
	if (NormSq > 0.0) { 
		const double InvNorm = 1.0 / TMath::Sqrt(NormSq);
		for (int SpN = FirstSpN; SpN < SpV.Len(); SpN++) {
			SpV[SpN].Dat *= InvNorm;
		}
	}
	Offset += Dim;
}

void TOgFtrExtJoin::ExtractStrV(const POgBase& OgBase, const TOgRec& FtrRec, TStrV& StrV) const {
	Assert(IsStartStore(FtrRec.GetStoreId()));
	POgRecSet RecSet = FtrRec.DoJoin(OgBase, GetJoinIdV(FtrRec.GetStoreId())); 
	const POgStore& FtrStore = OgBase->GetStoreByStoreId(GetFtrStoreId());
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
		StrV.Add(FtrStore->GetRecNm(RecSet->GetRecId(RecN)));
	}	
}

void TOgFtrExtPair::GetFtrIdV_Update(const POgBase& OgBase,
		const TOgRec& _FtrRec, const POgFtrExt& FtrExt, TIntV& FtrIdV) {

	Assert(IsStartStore(_FtrRec.GetStoreId()));
	POgRecSet FtrRecSet = _FtrRec.DoJoin(OgBase, GetJoinSeq(_FtrRec.GetStoreId())); 
	TIntSet FtrIdSet;
	for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
		const TOgRec& FtrRec = FtrRecSet->GetRec(RecN);
		TStrV FtrValV; FtrExt->ExtractStrV(OgBase, FtrRec, FtrValV);
		if (FtrValV.Empty()) { FtrValV.Add("[empty]"); }
		for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
			const int FtrId = FtrValH.AddKey(FtrValV[FtrValN]);
			FtrValH[FtrId]++;
			FtrIdSet.AddKey(FtrId);
		}
	}
	FtrIdSet.GetKeyV(FtrIdV);
}

void TOgFtrExtPair::GetFtrIdV_RdOnly(const POgBase& OgBase,
		const TOgRec& _FtrRec, const POgFtrExt& FtrExt, TIntV& FtrIdV) const {

	Assert(IsStartStore(_FtrRec.GetStoreId()));
	POgRecSet FtrRecSet = _FtrRec.DoJoin(OgBase, GetJoinSeq(_FtrRec.GetStoreId())); 
	TIntSet FtrIdSet;
	for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
		const TOgRec& FtrRec = FtrRecSet->GetRec(RecN);
		TStrV FtrValV; FtrExt->ExtractStrV(OgBase, FtrRec, FtrValV);
		if (FtrValV.Empty()) { FtrValV.Add("[empty]"); }
		for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
			const TStr& FtrVal = FtrValV[FtrValN];
			const int FtrId = FtrValH.GetKeyId(FtrVal);
			if (FtrId != -1) { FtrIdSet.AddKey(FtrId); }
		}
	}
	FtrIdSet.GetKeyV(FtrIdV);
}

void TOgFtrExtPair::Update(const POgBase& OgBase, const TOgRec& FtrRec) {
	TIntV FtrIdV1; GetFtrIdV_Update(OgBase, FtrRec, FtrExt1, FtrIdV1);
	TIntV FtrIdV2; GetFtrIdV_Update(OgBase, FtrRec, FtrExt2, FtrIdV2);
	for (int FtrIdN1 = 0; FtrIdN1 < FtrIdV1.Len(); FtrIdN1++) {
		const int FtrId1 = FtrIdV1[FtrIdN1];
		for (int FtrIdN2 = 0; FtrIdN2 < FtrIdV2.Len(); FtrIdN2++) {
			const int FtrId2 = FtrIdV2[FtrIdN2];
			FtrIdPairH.AddDat(TIntPr(FtrId1, FtrId2))++;
		}
	}
}

void TOgFtrExtPair::AddSpV(const POgBase& OgBase,
	    const TOgRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {

	TIntV FtrIdV1; GetFtrIdV_RdOnly(OgBase, FtrRec, FtrExt1, FtrIdV1);
	TIntV FtrIdV2; GetFtrIdV_RdOnly(OgBase, FtrRec, FtrExt2, FtrIdV2);
	TIntFltH PairIdWgtH;
	for (int FtrIdN1 = 0; FtrIdN1 < FtrIdV1.Len(); FtrIdN1++) {
		const int FtrId1 = FtrIdV1[FtrIdN1];
		for (int FtrIdN2 = 0; FtrIdN2 < FtrIdV2.Len(); FtrIdN2++) {
			const int FtrId2 = FtrIdV2[FtrIdN2];
			const int PairId = FtrIdPairH.GetKeyId(TIntPr(FtrId1, FtrId2));
			if (PairId != -1) { PairIdWgtH.AddDat(PairId) += 1.0; }
		}
	}
	TIntFltKdV PairSpV; PairIdWgtH.GetKeyDatKdV(PairSpV); PairSpV.Sort();
	TLinAlg::Normalize(PairSpV);
	for (int PairSpN = 0; PairSpN < PairSpV.Len(); PairSpN++) {
		const TIntFltKd PairSp = PairSpV[PairSpN];
		SpV.Add(TIntFltKd(PairSp.Key + Offset, PairSp.Dat));
	}
    Offset += GetDim(OgBase);
}

TStr TOgFtrExtPair::GetFtr(const POgBase& OgBase, const int& FtrN) const {
	if (FtrIdPairH.IsKeyId(FtrN)) {
		const TIntPr& Pair = FtrIdPairH.GetKey(FtrN);
		TStr FtrVal1 = FtrValH.GetKey(Pair.Val1);
		TStr FtrVal2 = FtrValH.GetKey(Pair.Val2);
		return "[" + FtrVal1 + ", " + FtrVal2 + "]";
	}
	return "";
}

void TOgFtrExtPair::ExtractStrV(const POgBase& OgBase, const TOgRec& _FtrRec, TStrV& StrV) const {
	Assert(IsStartStore(_FtrRec.GetStoreId()));
	POgRecSet FtrRecSet = _FtrRec.DoJoin(OgBase, GetJoinSeq(_FtrRec.GetStoreId())); 
	for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
		const TOgRec& FtrRec = FtrRecSet->GetRec(RecN);
		TStrV FtrValV1; FtrExt1->ExtractStrV(OgBase, FtrRec, FtrValV1);
		TStrV FtrValV2; FtrExt2->ExtractStrV(OgBase, FtrRec, FtrValV2);
		if (FtrValV1.Empty()) { FtrValV1.Add("[empty]"); }
		if (FtrValV2.Empty()) { FtrValV2.Add("[empty]"); }
		for (int FtrValN1 = 0; FtrValN1 < FtrValV1.Len(); FtrValN1++) {
			const TStr& FtrVal1 = FtrValV1[FtrValN1];
			for (int FtrValN2 = 0; FtrValN2 < FtrValV2.Len(); FtrValN2++) {
				const TStr& FtrVal2 = FtrValV2[FtrValN2];
				StrV.Add("[" + FtrVal1 + ", " + FtrVal2 + "]");
			}
		}
	}
}

void TOgOpGetRec::Exec(const POgBase& OgBase, const TOgRecSetV& InOgRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutOgRecSetV) {

	TStr StoreIdStr = GetFldVal(FldNmValPrV, "storeid");
	if (StoreIdStr.IsInt()) {
		const uchar StoreId = uchar(StoreIdStr.GetInt());		
		if (OgBase->IsStoreId(StoreId)) {
			const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
			POgRecSet RecSet;
			if (IsFldNm(FldNmValPrV, "recnm")){
				TStr RecNm = GetFldVal(FldNmValPrV, "recnm");
				OgAssertR(Store->IsRecNm(RecNm), "Unknown record name '" + RecNm + "'!");
				RecSet = Exec(Store->GetStoreId(), Store->GetRecId(RecNm));
			} else if (IsFldNm(FldNmValPrV, "recid"))	{
				TStr RecIdStr = GetFldVal(FldNmValPrV, "recid");
				if (RecIdStr.IsUInt64()){
					uint64 RecId = RecIdStr.GetUInt64();
					OgAssertR(Store->IsRecId(RecId), "Unknown record id '" + RecIdStr + "'!");
					RecSet = Exec(Store->GetStoreId(), RecId);
				} else { 
					TOgExcept::Throw("Wrong recid='" + RecIdStr + "': not a number!");
				}
			} else {
				TOgExcept::Throw("Record name or ID not specified!");
			}
			OutOgRecSetV.Add(RecSet);
		} else {
			TOgExcept::Throw("Wrong storeid='" + StoreIdStr + "': unknown ID!");
		}
	} else {
		TOgExcept::Throw("Wrong storeid='" + StoreIdStr + "': not a number!");
	}
	OutOgRecSetV.Add(TOgRecSet::New());
}

POgRecSet TOgOpGetRec::Exec(const uchar& StoreId, const uint64& RecId) {
	
	return TOgRecSet::New(StoreId, RecId);
}

void TOgOpSearch::Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV) {

	OgAssertR(IsFldNm(FldNmValPrV, "q"), "No query ('q=') provided");
	TStr QueryStr = GetFldVal(FldNmValPrV, "q");
	OutRecSetV.Add(OgBase->Search(QueryStr));
}

POgRecSet TOgOpSearch::Exec(const POgBase& OgBase, const POgQuery& Query) {
	POgRecSet RecSet = OgBase->Search(Query);
	return RecSet;
}

void TOgOpLinSearch::ParseQuery(const POgBase& OgBase, const uchar& StoreId, 
		const TStr& QueryElt, int& FieldId, TOgOpLinSearchType& LinSearchType, TStr& FieldVal) {

	POgStore Store = OgBase->GetStoreByStoreId(StoreId);			
	OgAssertR(QueryElt.IsChIn(':'), "Error in query '" + QueryElt + "'");
	TStr FieldStr, ValStr; QueryElt.SplitOnCh(FieldStr, ':', ValStr);
	OgAssertR(!ValStr.Empty(), "Error in query '" + QueryElt + "'");
	OgAssertR(FieldStr.IsInt(), "Field not a number '" + FieldStr + "'");
	FieldId = FieldStr.GetInt();
	OgAssertR(Store->IsFieldId(FieldId), "Invalid field " + FieldStr + " for store " + Store->GetStoreNm());
	char Range = ValStr[0]; 
	FieldVal = ValStr.GetSubStr(1);		
	switch (Range) {
		case ':' : LinSearchType = oolstEqual; break;
		case '<' : LinSearchType = oolstLess; break;
		case '>' : LinSearchType = oolstGreater; break;
		case '!' : LinSearchType = oolstNotEqual; break;
		default : TOgExcept::Throw("Unknown range operator in '" + QueryElt + "'");
	}
}

void TOgOpLinSearch::Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV){

	OgAssertR(IsFldNm(FldNmValPrV, "storeid"), "No StoreID provided");
	TStr StoreIdStr = GetFldVal(FldNmValPrV, "storeid");
	OgAssertR(StoreIdStr.IsInt(), "StoreID not a number");
	const uchar StoreId = (uchar)StoreIdStr.GetInt();
	OgAssertR(OgBase->IsStoreId(StoreId), "Invalid StoreID");
	OgAssertR(IsFldNm(FldNmValPrV, "q"), "No specified query");
	TStr QueryElt = GetFldVal(FldNmValPrV, "q");
	int FieldId; TOgOpLinSearchType LinSearchType; TStr FieldVal; 
	ParseQuery(OgBase, StoreId, QueryElt, FieldId, LinSearchType, FieldVal);
	POgRecSet RecSet;
	OgAssertR(OgBase->GetStoreByStoreId(StoreId)->IsFieldId(FieldId), 
		"FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	const TOgFieldDesc& FieldDesc = OgBase->GetStoreByStoreId(StoreId)->GetFieldDesc(FieldId);
	if (FieldDesc.IsInt()){
		if (FieldVal.IsInt()){
			RecSet = Exec(OgBase, StoreId, FieldId, LinSearchType, FieldVal.GetInt());
			OutRecSetV.Add(RecSet);
		} else {
			TOgExcept::Throw("Field value not an int " + FieldVal);
		}
	} else {
		TOgExcept::Throw("Field type not supported " + FieldDesc.GetFieldTypeStr());
	}
}
	
POgRecSet TOgOpLinSearch::Exec(const POgBase& OgBase, const uchar& StoreId, const int& FieldId, 
		const TOgOpLinSearchType& OpLinSearchType, const int& FieldVal){

	POgStore OgStore = OgBase->GetStoreByStoreId(StoreId);	
	OgAssertR(OgStore->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	TUInt64V RecsV; POgStoreIter Iter = OgStore->GetIter();
	while (Iter->Next()) {
		const uint64 RecId = Iter->GetRecId();
		if (OpLinSearchType == oolstEqual) { 
			if (OgStore->GetFieldInt(RecId, FieldId) == FieldVal) { RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstLess) { 
			if (OgStore->GetFieldInt(Iter->GetRecId(), FieldId) < FieldVal) { RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstGreater) { 
			if (OgStore->GetFieldInt(RecId, FieldId) > FieldVal) { RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstNotEqual) { 
			if (OgStore->GetFieldInt(RecId, FieldId) != FieldVal) { RecsV.Add(RecId); }
		} else { 
			TOgExcept::Throw("Unknown range operator.");
		}
	}	
	return TOgRecSet::New(StoreId, RecsV);
}

POgRecSet TOgOpLinSearch::Exec(const POgBase& OgBase, const uchar& StoreId, const int& FieldId, 
		const TOgOpLinSearchType& OpLinSearchType, const TTm& FieldVal) {
		
	POgStore OgStore = OgBase->GetStoreByStoreId(StoreId);
	OgAssertR(OgStore->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	OgAssertR(FieldVal.IsDef(), "Invalid FieldVal '" + FieldVal.GetStr() + "' is not valid time.");	
	TUInt64V RecsV; POgStoreIter Iter = OgStore->GetIter();
	while (Iter->Next()) {
		const uint64 RecId = Iter->GetRecId();
		TTm RecFieldVal; OgStore->GetFieldTm(RecId, FieldId, RecFieldVal);
		if (OpLinSearchType == oolstEqual) { 
			if (RecFieldVal == FieldVal) { RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstLess) { 
			if (RecFieldVal < FieldVal) { RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstGreater) { 
			if (RecFieldVal > FieldVal) { RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstNotEqual) { 
			if (RecFieldVal != FieldVal) { RecsV.Add(RecId); }
		} else { 
			TOgExcept::Throw("Unknown range operator.");
		}
	}	
    return TOgRecSet::New(StoreId, RecsV);
}

POgRecSet TOgOpLinSearch::Exec(const POgBase& OgBase, const uchar& StoreId,
		const int& FieldId, const TTm& MnFieldVal, const TTm& MxFieldVal) {

	POgStore OgStore = OgBase->GetStoreByStoreId(StoreId);
	OgAssertR(OgStore->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	OgAssertR(MnFieldVal.IsDef(), "Invalid FieldVal '" + MnFieldVal.GetStr() + "' is not valid time.");	
	OgAssertR(MxFieldVal.IsDef(), "Invalid FieldVal '" + MxFieldVal.GetStr() + "' is not valid time.");	
	TUInt64V RecsV; POgStoreIter Iter = OgStore->GetIter();
	while (Iter->Next()) {
		const uint64 RecId = Iter->GetRecId();
		TTm RecFieldVal; OgStore->GetFieldTm(RecId, FieldId, RecFieldVal);
		if (MnFieldVal < RecFieldVal && RecFieldVal < MxFieldVal) { RecsV.Add(RecId); }
	}
    return TOgRecSet::New(StoreId, RecsV);
}

POgRecSet TOgOpLinSearch::Exec(const POgBase& OgBase, const int& StoreId,
		const int& FieldId, const uint64& MinFieldVal, const uint64& MaxFieldVal) {
			
	POgStore OgStore = OgBase->GetStoreByStoreId(StoreId);
	OgAssertR(OgStore->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	TUInt64V RecsV; POgStoreIter Iter = OgStore->GetIter();
	while (Iter->Next()) {
		const uint64 RecId = Iter->GetRecId();
		const uint64 RecFieldVal = OgStore->GetFieldUInt64(RecId, FieldId);
		if (MinFieldVal <= RecFieldVal && RecFieldVal <= MaxFieldVal) { RecsV.Add(RecId); }
	}	
    return TOgRecSet::New(StoreId, RecsV);
}

POgRecSet TOgOpLinSearch::Exec(const POgBase& OgBase, const int& StoreId,
		const int& FieldId, const TOgOpLinSearchType& OpLinSearchType, const TIntV& FieldVals) {
	
	POgStore OgStore = OgBase->GetStoreByStoreId(StoreId);
	OgAssertR(OgStore->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");

	THashSet<TInt> FieldValsH;
	for (int i=0; i < FieldVals.Len(); i++)
		FieldValsH.AddKey(FieldVals[i]);

	TUInt64V RecsV; POgStoreIter Iter = OgStore->GetIter();
	if (OpLinSearchType == oolstIsIn) {
		while (Iter->Next()) {
			const uint64 RecId = Iter->GetRecId();
			const int RecFieldVal = OgStore->GetFieldInt(RecId, FieldId);
			if (FieldValsH.IsKey(RecFieldVal)) 
				RecsV.Add(RecId);
		}	
	} else if (OpLinSearchType == oolstIsNotIn) {
		while (Iter->Next()) {
			const uint64 RecId = Iter->GetRecId();
			const int RecFieldVal = OgStore->GetFieldInt(RecId, FieldId);
			if (!FieldValsH.IsKey(RecFieldVal)) 
				RecsV.Add(RecId);
		}	
	} else { 
		throw TOgExcept::New("Unknown range operator."); 
	}
	return TOgRecSet::New(StoreId, RecsV);	
}

POgRecSet TOgOpLinSearch::Exec(const POgBase& OgBase, const POgRecSet& RecSet, const int& FieldId, 
		const TOgOpLinSearchType& OpLinSearchType, const TTm& FieldVal) {
		
	POgStore OgStore = OgBase->GetStoreByStoreId(RecSet->GetStoreId());
	OgAssertR(OgStore->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	OgAssertR(FieldVal.IsDef(), "Invalid FieldVal '" + FieldVal.GetStr() + "' is not valid time.");	
	TUInt64V RecsV; 
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++){
		const uint64 RecId = RecSet->GetRecId(RecN);	
		TTm RecFieldVal; OgStore->GetFieldTm(RecId, FieldId, RecFieldVal);
		if (OpLinSearchType == oolstEqual) { 
			if (RecFieldVal == FieldVal){ RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstLess) { 
			if (RecFieldVal < FieldVal){ RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstGreater) { 
			if (RecFieldVal > FieldVal){ RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstNotEqual) { 
			if (RecFieldVal != FieldVal){ RecsV.Add(RecId); }
		} else { 
			TOgExcept::Throw("Unknown range operator.");
		}
	}
	return TOgRecSet::New(RecSet->GetStoreId(), RecsV);	
}

POgRecSet TOgOpLinSearch::Exec(const POgBase& OgBase, const POgRecSet& RecSet,
		const int& FieldId, const TTm& MnFieldVal, const TTm& MxFieldVal) {

	POgStore OgStore = OgBase->GetStoreByStoreId(RecSet->GetStoreId());
	OgAssertR(OgStore->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	OgAssertR(MnFieldVal.IsDef(), "Invalid FieldVal '" + MnFieldVal.GetStr() + "' is not valid time.");	
	OgAssertR(MxFieldVal.IsDef(), "Invalid FieldVal '" + MxFieldVal.GetStr() + "' is not valid time.");	
	TUInt64V RecsV; 
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++){
		const uint64 RecId = RecSet->GetRecId(RecN);	
		TTm RecFieldVal; OgStore->GetFieldTm(RecId, FieldId, RecFieldVal);
		if (MnFieldVal < RecFieldVal && RecFieldVal < MxFieldVal) { RecsV.Add(RecId); }
	}
    return TOgRecSet::New(RecSet->GetStoreId(), RecsV);
}

POgRecSet TOgOpLinSearch::Exec(const POgBase& OgBase, const POgRecSet& RecSet, 
		const int& FieldId, const bool& FieldVal) {
		
	POgStore OgStore = OgBase->GetStoreByStoreId(RecSet->GetStoreId());
	OgAssertR(OgStore->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	TUInt64V RecsV; 
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++){
		const uint64 RecId = RecSet->GetRecId(RecN);	
		const bool RecFieldVal = OgStore->GetFieldBool(RecId, FieldId);
		if (RecFieldVal == FieldVal) { RecsV.Add(RecId); }
	}
	return TOgRecSet::New(RecSet->GetStoreId(), RecsV);	
}

void TOgOpSort::Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV) {
	
	TStr AscStr = GetFldVal(FldNmValPrV, "sort", "asc"); bool Asc;
	if (AscStr.ToLc() == "asc") { Asc = true; } 
	else if (AscStr.ToLc() == "desc") { Asc = false; } 
	else { TOgExcept::Throw("No 'sort' parameter specifying the type of sort has been found!"); }
	for (int RecN = 0; RecN < InRecSetV.Len(); RecN++) {
		POgRecSet RecSet = InRecSetV[RecN];
		// Field id/nm parameter
		if (IsFldNm(FldNmValPrV, "fid")){
			TStr FieldIdStr = GetFldVal(FldNmValPrV, "fid");
			OgAssertR(FieldIdStr.IsInt(), "Field ID not a number");
			const int FieldId = FieldIdStr.GetInt();		
			OutRecSetV.Add(Exec(OgBase, RecSet, FieldId, Asc));
		} else{ 
			OutRecSetV.Add(Exec(OgBase, RecSet, Asc));
		}	
	}
}

POgRecSet TOgOpSort::Exec(const POgBase& OgBase, 
		const POgRecSet& RecSet, const int& FieldId, const bool& Asc) {

	RecSet->SortByField(OgBase, Asc, FieldId);	
	return RecSet;
}

POgRecSet TOgOpSort::Exec(const POgBase& OgBase, 
		const POgRecSet& RecSet, const bool& Asc) {

	RecSet->SortByFq(Asc);	
	return RecSet;
}

void TOgOpReverse::Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV) {
	
	for (int RecN = 0; RecN < InRecSetV.Len(); RecN++) {
		POgRecSet RecSet = InRecSetV[RecN];
		OutRecSetV.Add(Exec(RecSet));	
	}
}

POgRecSet TOgOpReverse::Exec(const POgRecSet& RecSet) {
	RecSet->Reverse();	
	return RecSet;
}

////////////////////////////////////////////////
// Shuffle Operator
void TOgOpShuffle::Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
	const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV) {
	
	for (int RecN = 0; RecN < InRecSetV.Len(); RecN++) {
		POgRecSet RecSet = InRecSetV[RecN];
		OutRecSetV.Add(Exec(RecSet));	
	}
}

POgRecSet TOgOpShuffle::Exec(const POgRecSet& RecSet) {
    RecSet->Shuffle(Rnd);	
	return RecSet;
}

void TOgOpJoin::Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
	const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV) {

	TStr SampleSizeStr = GetFldVal(FldNmValPrV, "sample", "-1");
	OgAssertR(SampleSizeStr.IsInt(), "Join ID not a number");
	int SampleSize = SampleSizeStr.GetInt();
	TStrV JoinIdStrV; TIntPrV JoinIdV;
	GetFldValV(FldNmValPrV, "joinid", JoinIdStrV);	
	for (int JoinIdN = 0; JoinIdN < JoinIdStrV.Len(); JoinIdN++) {
		TStr JoinIdStr = JoinIdStrV[JoinIdN];
		OgAssertR(JoinIdStr.IsInt(), "Join ID not a number");
		const int JoinId = JoinIdStr.GetInt();
		JoinIdV.Add(TIntPr(JoinId, SampleSize));
	}
	for (int RecN = 0; RecN < InRecSetV.Len(); RecN++) {
		POgRecSet RecSet = InRecSetV[RecN];
		TOgJoinSeq JoinSeq(RecSet->GetStoreId(), JoinIdV);
		RecSet = Exec(OgBase, RecSet, JoinSeq);
		OutRecSetV.Add(RecSet);
	}
}

POgRecSet TOgOpJoin::Exec(const POgBase& OgBase, const POgRecSet& RecSet, const TOgJoinSeq& JoinSeq) {
	return RecSet->DoJoin(OgBase, JoinSeq, RecSet->IsWgt());	
}

void TOgOpAggrField::ParseFtrExt(const POgBase& OgBase, const POgStore& StartStore, 
		const TStr& FtrExtStr, TOgJoinSeq& JoinSeq, int& FieldId) {

	TStr FieldIdStr;
	POgStore Store = StartStore; TIntPrV JoinIdV;
	if (FtrExtStr.IsChIn(';')) {
		TStr JoinStr; FtrExtStr.SplitOnCh(JoinStr, ';', FieldIdStr);
		TStrV JoinIdStrV; JoinStr.SplitOnAllCh(':', JoinIdStrV, false);
		for (int JoinIdStrN = 0; JoinIdStrN < JoinIdStrV.Len(); JoinIdStrN++) {
			const TStr& JoinIdStr = JoinIdStrV[JoinIdStrN];
			OgAssert(JoinIdStr.IsInt());
			const int JoinId = JoinIdStr.GetInt();
			JoinIdV.Add(TIntPr(JoinId, 1000));
			OgAssertR(Store->IsJoinId(JoinId), "Bad join id!");
			const uchar JoinStoreId = Store->GetJoinDesc(JoinId).GetJoinStoreId();
			Store = OgBase->GetStoreByStoreId(JoinStoreId);
		}
		JoinSeq = TOgJoinSeq(StartStore->GetStoreId(), JoinIdV);
	} else {
		JoinSeq = TOgJoinSeq(StartStore->GetStoreId());
		FieldIdStr = FtrExtStr;
	}
	OgAssertR(FieldIdStr.IsInt(), "Bad Field Id: '" + FieldIdStr  + "'!");
	FieldId = FieldIdStr.GetInt();
}

void TOgOpAggrField::Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
	const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV) {

	OgAssertR(IsFldNm(FldNmValPrV, "storeid"), "No StoreID provided");
	TStr StoreIdStr = GetFldVal(FldNmValPrV, "storeid");
	OgAssertR(StoreIdStr.IsInt(), "StoreID not a number");
	const uchar StoreId = (uchar)StoreIdStr.GetInt();
	OgAssertR(OgBase->IsStoreId(StoreId), "Invalid StoreID");
	for (int RecN = 0; RecN < InRecSetV.Len(); RecN++) {
		POgRecSet RecSet = InRecSetV[RecN];
		const POgStore& Store = OgBase->GetStoreByStoreId(RecSet->GetStoreId());
		TStrV AggrStrV; GetFldValV(FldNmValPrV, "aggrfid", AggrStrV);
		for (int AggrStrN = 0; AggrStrN < AggrStrV.Len(); AggrStrN++) {
			const TStr& AggrStr = AggrStrV[AggrStrN];
			int AggrFieldId; TOgJoinSeq JoinSeq;
			ParseFtrExt(OgBase, Store, AggrStr, JoinSeq, AggrFieldId);
			RecSet = Exec(OgBase, RecSet, Store, JoinSeq, AggrFieldId);
		}
		OutRecSetV.Add(RecSet);
	}
}

POgRecSet TOgOpAggrField::Exec(const POgBase& OgBase, const POgRecSet& RecSet, 
		const POgStore& StartStore, TOgJoinSeq& JoinSeq, int& AggrFieldId) {

	POgStore AggrStore = JoinSeq.GetEndStore(OgBase);
	const TOgFieldDesc& FieldDesc = AggrStore->GetFieldDesc(AggrFieldId);
	OgAssertR(FieldDesc.HasDefAggr(), TStr::Fmt(
		"Aggregation for %d not supported!", AggrFieldId));
	if (FieldDesc.IsAggrPiechart()) {
		POgFtrExt FtrExt = TOgFtrExtMultiNom::New(OgBase, JoinSeq, AggrFieldId);
		RecSet->AddAggr(TOgAggrPiechart::New(OgBase, RecSet, FtrExt));
	} else if (FieldDesc.IsAggrHistogram()) {
		POgFtrExt FtrExt = TOgFtrExtNum::New(OgBase, JoinSeq, AggrFieldId);
		RecSet->AddAggr(TOgAggrHistogram::New(OgBase, RecSet, FtrExt, 20));
	} else if (FieldDesc.IsAggrScalar()) {
		TOgExcept::Throw("Feature not impelemented!");
	} else if (FieldDesc.IsAggrTimeline()) {
		POgFtrExt FtrExt = TOgFtrExtMultiNom::New(OgBase, JoinSeq, AggrFieldId);
		RecSet->AddAggr(TOgAggrTimeLine::New(OgBase, RecSet, FtrExt));
	} else if (FieldDesc.IsAggrKeywords()) {
		POgFtrExt FtrExt = FieldDesc.IsDefFtrToken() ? 
			TOgFtrExtToken::New(OgBase, JoinSeq, AggrFieldId) :
			TOgFtrExtMultiNom::New(OgBase, JoinSeq, AggrFieldId);
		RecSet->AddAggr(TOgAggrKeywords::New(OgBase, RecSet, FtrExt, 1000));
	}
	return RecSet;
}

void TOgOpAggrKey::Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
	const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV) {

	for (int RecN = 0; RecN < InRecSetV.Len(); RecN++) {
		POgRecSet RecSet = InRecSetV[RecN];
		TStrV AggrStrV; GetFldValV(FldNmValPrV, "aggrkid", AggrStrV);
		for (int AggrStrN = 0; AggrStrN < AggrStrV.Len(); AggrStrN++) {
			const TStr& KeyIdStr = AggrStrV[AggrStrN];
			OgAssertR(KeyIdStr.IsInt(),
				"Key id for aggregation not a number!");
			const int KeyId = KeyIdStr.GetInt();
			RecSet = Exec(OgBase, RecSet, KeyId);
		}
		OutRecSetV.Add(RecSet);
	}
}

POgRecSet TOgOpAggrKey::Exec(const POgBase& OgBase,
		const POgRecSet& RecSet, const int& KeyId) {

	RecSet->AddAggr(TOgAggrPiechart::New(OgBase, RecSet, KeyId));
	return RecSet;
}

void TOgOpGroupBy::Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV) {

	for (int InRecN = 0; InRecN < InRecSetV.Len(); InRecN++) {
		uchar StoreId = InRecSetV[InRecN]->GetStoreId();
		TStr GroupByFldIdStr; GetFldVal(FldNmValPrV, "fid", GroupByFldIdStr);
		OgAssertR(GroupByFldIdStr.IsInt(), "Field id for grouping not a number!");
		int FieldId = GroupByFldIdStr.GetInt();
		Exec(OgBase, InRecSetV[InRecN], StoreId, FieldId, OutRecSetV);
	}
}

void TOgOpGroupBy::Exec(const POgBase& OgBase, const POgRecSet& InRecSet, 
		const uchar& StoreId, const int& FieldId, TOgRecSetV& OutRecSetV) {

	POgStore OgStore = OgBase->GetStoreByStoreId(StoreId);
	OgAssertR(OgStore->IsFieldId(FieldId), 
			"FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	const int Recs = InRecSet->GetRecs();
	TUInt64V RecV; int CrtVal = -1, PrevVal = -1;
	for (int RecN = 0; RecN < Recs; RecN++) {	
		const uint64 RecId = InRecSet->GetRecId(RecN);
		CrtVal = OgStore->GetFieldInt(RecId, FieldId);
		if (CrtVal == PrevVal) { 
			RecV.Add(RecId);
		} else { 
			OutRecSetV.Add(TOgRecSet::New(OgStore->GetStoreId(), RecV)); 
			RecV.Clr(true); RecV.Add(RecId);
			PrevVal = CrtVal;
		}			
	}
}

void TOgOpSplitBy::Exec(const POgBase& OgBase, const TOgRecSetV& InRecSetV, 
		const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV) {

	for (int InRecN = 0; InRecN < InRecSetV.Len(); InRecN++) {
		int StoreId = InRecSetV[InRecN]->GetStoreId();
		TStr SplitByFldIdStr; GetFldVal(FldNmValPrV, "spfid", SplitByFldIdStr);
		OgAssertR(SplitByFldIdStr.IsInt(), "Field id for splitting not a number!");
		int FieldId = SplitByFldIdStr.GetInt();
		TStr SplitWinStr; GetFldVal(FldNmValPrV, "spwin", SplitWinStr);
		OgAssertR(SplitWinStr.IsInt(), "Window for splitting not a number!");
		int SplitWin = SplitWinStr.GetInt();
		Exec(OgBase, InRecSetV[InRecN], StoreId, FieldId, SplitWin, OutRecSetV);
	}
}

void TOgOpSplitBy::Exec(const POgBase& OgBase, const POgRecSet& InRecSet, 
		const uchar& StoreId, const int& FieldId, const int& SplitWinSize, 
		TOgRecSetV& OutRecSetV) {
	
	OgAssertR(OgBase->GetStoreByStoreId(StoreId)->IsFieldId(FieldId), 
			"FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	TUInt64V RecsV; TFlt CrtVal = -1.0, StartVal = -1.0;
	POgStore OgStore = OgBase->GetStoreByStoreId(StoreId);
	for (int RecN = 0; RecN < InRecSet->GetRecs(); RecN++) {	
		if (OgStore->GetFieldDesc(FieldId).GetFieldType() == oftInt) {
			CrtVal = TFlt(OgStore->GetFieldInt(InRecSet->GetRecId(RecN), FieldId));
		} else if (OgStore->GetFieldDesc(FieldId).GetFieldType() == oftFlt) {
			CrtVal = OgStore->GetFieldFlt(InRecSet->GetRecId(RecN), FieldId);
		} else if (OgStore->GetFieldDesc(FieldId).GetFieldType() == oftTm) {
			TTm Tm; OgStore->GetFieldTm(InRecSet->GetRecId(RecN), FieldId, Tm);
			CrtVal = double(TTm::GetMSecsFromTm(Tm)) / double(TTmInfo::GetHourMSecs());
		} else {
			TOgExcept::Throw("Sequence FieldType is not Int or Tm!");
		}
		if ((CrtVal <= (StartVal + SplitWinSize)) || (RecN == 0)){ 
			RecsV.Add(InRecSet->GetRecId(RecN));
			if (RecN == 0) {StartVal = CrtVal;}
		} else { 
			OutRecSetV.Add(TOgRecSet::New(OgStore->GetStoreId(), RecsV)); 
			RecsV.Clr(true); RecsV.Add(InRecSet->GetRecId(RecN));
			StartVal = CrtVal;
		}			
	}
	OutRecSetV.Add(TOgRecSet::New(OgStore->GetStoreId(), RecsV)); 
}

void TOgOpHrchClust::Exec(const POgBase& OgBase, const TOgRecSetV& InOgRecSetV, 
	const TStrKdV& FldNmValPrV, TOgRecSetV& OutRecSetV) {

		//TO DO
}

void TOgOpHrchClust::Exec(const POgBase& OgBase, const TOgRecSetV& InRecSet, 
		const uchar& StoreId, const int& FieldId, int& Dist, const int& ClustNo,
		TOgRecSetV& OutRecSetV) {

	OgAssertR(OgBase->GetStoreByStoreId(StoreId)->IsFieldId(FieldId), 
			"FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	POgStore OgStore = OgBase->GetStoreByStoreId(StoreId);

	TStrH RecFieldIdH; THash<TIntV, TInt> SeqH; TVec<TIntV> StepVV; 
	for (int InRSetN = 0; InRSetN < InRecSet.Len(); InRSetN++){
		TIntV TempV; TStr RecFieldVal;
		for (int InRSetRecN = 0; InRSetRecN < InRecSet[InRSetN]->GetRecs(); InRSetRecN++){
			if (OgStore->GetFieldDesc(FieldId).GetFieldType() == oftInt) {
				RecFieldVal = TInt(OgStore->GetFieldInt(InRecSet[InRSetN]->GetRecId(InRSetRecN), FieldId)).GetStr();
			} else if (OgStore->GetFieldDesc(FieldId).GetFieldType() == oftStr) {
				RecFieldVal = OgStore->GetFieldStr(InRecSet[InRSetN]->GetRecId(InRSetRecN), FieldId);
			} else {
				TOgExcept::Throw("Sequence FieldType is not Int of String!");
			}
			RecFieldIdH.AddDat(RecFieldVal);			
			TempV.Add(RecFieldIdH.GetKeyId(RecFieldVal));
		}
		if (TempV.Len() > 0){
			if (SeqH.IsKey(TempV)){
				int OutRSetN = SeqH.GetDat(TempV); 
				OutRecSetV[OutRSetN]->Merge(InRecSet[InRSetN]);
			} else {
				StepVV.Add(TempV);			
				OutRecSetV.Add(InRecSet[InRSetN]);
				int SeqKeyId = SeqH.AddDat(TempV, OutRecSetV.Len() - 1);
			}
		}
	}

	//
	int MnVal = 0; int Th = 1; 	
	while (OutRecSetV.Len() > ClustNo){
		while (Th < Dist){		
			while (MnVal < Th){	
				TVec<TVec<TInt> > DistVV;
				MnVal = GetLevDistVV(StepVV, DistVV);
				MergeClust(OutRecSetV, StepVV, DistVV, MnVal);		
			}
			Th++;	
			if (OutRecSetV.Len() <= ClustNo) {break;}
		}
		Dist++;
	}
}

int TOgOpHrchClust::GetLevDistVV(const TVec<TIntV>& StepVV, TVec<TIntV>& DistVV) {
	DistVV.Gen(StepVV.Len());
	for (int i = 0; i < StepVV.Len(); i++) {
		DistVV[i].Gen(StepVV.Len());
	}
	int MnVal = 100;
	for (int StepV1N = 0; StepV1N < StepVV.Len(); StepV1N++){
		for (int StepV2N = StepV1N + 1; StepV2N < StepVV.Len(); StepV2N++){
			DistVV[StepV1N][StepV2N] = GetLevDist(StepVV[StepV1N], StepVV[StepV2N]);
			if (MnVal > DistVV[StepV1N][StepV2N]) { MnVal = DistVV[StepV1N][StepV2N]; }
		}
	}			
	return MnVal;
}

int TOgOpHrchClust::GetLevDist(const TIntV& Vec1, const TIntV& Vec2) {
	TVec<TIntV> DistVV;  
	DistVV.Gen(Vec1.Len() + 1);
	for (int i = 0; i < Vec1.Len() + 1; i++) {
		DistVV[i].Gen(Vec2.Len() + 1);
	}

	for (int i = 0; i <= Vec1.Len(); i++){DistVV[i][0] = i;}
	for (int j = 0; j <= Vec2.Len(); j++){DistVV[0][j] = j;}
  
	for (int j = 1; j <= Vec2.Len(); j++) {
		for (int i = 1; i <= Vec1.Len(); i++){
			if (Vec1[i - 1] == Vec2[j - 1]) { 
				DistVV[i][j] = DistVV[i-1][j-1];
			} else { 
				TVec<TInt> DV; 
				DV.Add(DistVV[i-1][j] + 1); DV.Add(DistVV[i][j-1] + 1); DV.Add(DistVV[i-1][j-1] + 1);
				DV.Sort();
				DistVV[i][j] = DV[0];
			}
		}
	}	
   return DistVV[Vec1.Len()][Vec2.Len()];
}

void TOgOpHrchClust::MergeClust(TOgRecSetV& OutOgRecSetV, 
		TVec<TIntV>& StepVV, TVec<TIntV>& DistVV, int& MnVal) {

	bool NotMerged = true;
	while (NotMerged){
		NotMerged = false;
		for (int i = 0; i < OutOgRecSetV.Len(); i++){
			for (int j = i + 1; j < OutOgRecSetV.Len(); j++) {
				if (DistVV[i][j] == MnVal){		

					OutOgRecSetV[i]->Merge(OutOgRecSetV[j]);
					StepVV[i].Union(StepVV[j]);

					for (int k = 0; k < OutOgRecSetV.Len(); k++) { DistVV[k].Del(j); }
					DistVV.Del(j);
					OutOgRecSetV.Del(j);
					StepVV.Del(j); 					
					NotMerged = true; break;
				}
			}					
			if (NotMerged){ 
				UpdateDistVV(StepVV, i, DistVV, MnVal);	 
				break;
			}
		}
	}	
}

void TOgOpHrchClust::UpdateDistVV(const TVec<TIntV>& StepVV, 
		const int& Idx, TVec<TIntV>& DistVV, int& MnVal) {

	for (int Seq1Idx = 0; Seq1Idx < StepVV.Len(); Seq1Idx++){
		for (int Seq2Idx = Seq1Idx + 1; Seq2Idx < StepVV.Len(); Seq2Idx++){
			if (Seq1Idx == Idx || Seq2Idx == Idx) {
				DistVV[Seq1Idx][Seq2Idx] = GetLevDist(StepVV[Seq1Idx], StepVV[Seq2Idx]);
				if (MnVal > DistVV[Seq1Idx][Seq2Idx]) { MnVal = DistVV[Seq1Idx][Seq2Idx]; }
			}
		}
	}			
}

TOgAggrPiechart::TOgAggrPiechart(const POgBase& OgBase,
		const POgRecSet& RecSet, const POgFtrExt& FtrExt) {

	JoinPathStr = TOgRecSet::GetJoinPathStr(OgBase, 
		OgBase->GetStoreByStoreId(RecSet->GetStoreId()), 
		FtrExt->GetJoinIdV(RecSet->GetStoreId()));
	// prepare field name
	AggrNm = FtrExt->GetNm(OgBase);
	// prepare piechart
	TStrV FtrValV; const int Recs = RecSet->GetRecs();
	for (int RecN = 0; RecN < Recs; RecN++) {
		FtrExt->ExtractStrV(OgBase, RecSet->GetRec(RecN), FtrValV);
		for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
			ValH.AddDat(FtrValV[FtrValN])++; Count++;
		}
	}
	ValH.SortByDat(false);
}

TOgAggrPiechart::TOgAggrPiechart(const POgBase& OgBase, 
		const POgRecSet& RecSet, const int& KeyId) {

	// prepare key name
	AggrNm = OgBase->GetIndexVoc()->GetKeyNm(KeyId);
	// prepare piechart
	TUInt64IntKdV ResV = RecSet->GetRecIdFqV();
	if (!ResV.IsSorted()) { ResV.Sort(); }
	const uchar StoreId = RecSet->GetStoreId();
	const uint64 Words = OgBase->GetIndexVoc()->GetWords(KeyId);
	for (uint64 WordId = 0; WordId < Words; WordId++) {
		TIntUInt64PrV FilterQueryItemV;
		FilterQueryItemV.Add(TIntUInt64Pr(KeyId, WordId));
		const POgStore& Store = OgBase->GetStoreByStoreId(RecSet->GetStoreId());
		TUInt64IntKdV FilterV; OgBase->GetIndex()->SearchAnd(FilterQueryItemV, FilterV);
		FilterV.Intrs(ResV); const int WordFq = FilterV.Len();
		TStr WordStr = OgBase->GetIndexVoc()->GetWordStr(KeyId, WordId);
		ValH.AddDat(WordStr) = WordFq; Count += WordFq;
	}
	ValH.SortByDat(false);
}

PXmlTok TOgAggrPiechart::SaveXml() const { 
	PXmlTok TopTok = TXmlTok::New("piechart");
	TopTok->AddArg("name", AggrNm);
	if (!JoinPathStr.Empty()) { TopTok->AddArg("join", JoinPathStr); }

	const double FltCount = (Count > 0) ? double(Count) : 1;
	int ValKeyId = ValH.FFirstKeyId();
	while (ValH.FNextKeyId(ValKeyId)) {
		PXmlTok ValTok = TXmlTok::New("value", ValH.GetKey(ValKeyId));
		const int ValFq = ValH[ValKeyId];
		const double Percent = 100.0 * (double(ValFq) / FltCount);
		ValTok->AddArg("fq", ValFq);
		ValTok->AddArg("perc", TFlt::GetStr(Percent, "%.2f"));
		TopTok->AddSubTok(ValTok);
	}

	return TopTok;
}

PJsonVal TOgAggrPiechart::SaveJson() const { 
	return TJsonVal::NewNull(); //TODO
}

///////////////////////////////
// QMiner-Aggregator-Histogram
TOgAggrHistogram::TOgAggrHistogram(const POgBase& OgBase,
		const POgRecSet& RecSet, const POgFtrExt& FtrExt, const int& Buckets) {

	JoinPathStr = TOgRecSet::GetJoinPathStr(OgBase, 
		OgBase->GetStoreByStoreId(RecSet->GetStoreId()), 
		FtrExt->GetJoinIdV(RecSet->GetStoreId()));
	AggrNm = FtrExt->GetNm(OgBase);
	if (RecSet->Empty()) { return; }
	double MnVal = TFlt::Mx, MxVal = TFlt::Mn;
	const int Recs = RecSet->GetRecs(); TFltV FtrValV; 
	for (int RecN = 0; RecN < Recs; RecN++) {
		FtrExt->ExtractFltV(OgBase, RecSet->GetRec(RecN), FtrValV);
		for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
			const double Val = FtrValV[FtrValN];
			MnVal = TFlt::GetMn(MnVal, Val);
			MxVal = TFlt::GetMx(MxVal, Val);
		}
	}
	Mom = TMom::New(); Sum = 0.0;
	Hist = THist(MnVal, MxVal, Buckets);
	for (int RecN = 0; RecN < Recs; RecN++) {
		FtrExt->ExtractFltV(OgBase, RecSet->GetRec(RecN), FtrValV);
		for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
			const double FtrVal = FtrValV[FtrValN].Val;
			Mom->Add(FtrVal); Sum += FtrVal;
			Hist.Add(FtrValV[FtrValN], true); 
		}
	}
	Mom->Def();
}

PXmlTok TOgAggrHistogram::SaveXml() const { 
	PXmlTok TopTok = TXmlTok::New("histogram");
	TopTok->AddArg("name", AggrNm);
	if (!JoinPathStr.Empty()) { TopTok->AddArg("join", JoinPathStr); }
	TopTok->AddArg("count", Mom->GetVals());
	if (TFlt::Abs(Sum) > 100) { 
		TopTok->AddArg("sum", TFlt::GetStr(Sum.Val, "%.0f"));
	} else {
		TopTok->AddArg("sum", TFlt::GetStr(Sum.Val, "%.2f"));
	}
	TopTok->AddArg("min", TFlt::GetStr(Mom->GetMn(), "%.2f"));
	TopTok->AddArg("max", TFlt::GetStr(Mom->GetMx(), "%.2f"));
	TopTok->AddArg("mean", TFlt::GetStr(Mom->GetMean(), "%.2f"));
	TopTok->AddArg("sdev", TFlt::GetStr(Mom->GetSDev(), "%.2f"));
	TopTok->AddArg("median", TFlt::GetStr(Mom->GetMedian(), "%.2f"));

	double PercentSum = 0.0;
	for (int BucketN = 0; BucketN < Hist.GetBuckets(); BucketN++) {
		PXmlTok ValTok = TXmlTok::New("value");
		ValTok->AddArg("min", Hist.GetBucketMn(BucketN));
		ValTok->AddArg("max", Hist.GetBucketMx(BucketN));
		ValTok->AddArg("fq", Hist.GetBucketVal(BucketN));
		const double Percent = 100.0 * Hist.GetBucketValPerc(BucketN);
		PercentSum += Percent;
		ValTok->AddArg("perc", TFlt::GetStr(Percent, "%.2f"));
		ValTok->AddArg("perc-sum", TFlt::GetStr(PercentSum, "%.2f"));
		TopTok->AddSubTok(ValTok);
	}

	return TopTok;
}

PJsonVal TOgAggrHistogram::SaveJson() const { 
	return TJsonVal::NewNull(); //TODO
}

TOgAggrKeywords::TOgAggrKeywords(const POgBase& OgBase, const POgRecSet& _RecSet, 
		const POgFtrExt& FtrExt, const int& SampleSize) {

	POgRecSet RecSet = _RecSet;
	if (SampleSize != -1 && RecSet->GetRecs() > SampleSize) {
		RecSet = _RecSet->GetSampleRecSet(SampleSize, false);
	}
	JoinPathStr = TOgRecSet::GetJoinPathStr(OgBase, 
		OgBase->GetStoreByStoreId(RecSet->GetStoreId()), 
		FtrExt->GetJoinIdV(RecSet->GetStoreId()));
	AggrNm = FtrExt->GetNm(OgBase);
	PBowDocBs BowDocBs = TBowDocBs::New();
	POgStore RecStore = OgBase->GetStoreByStoreId(RecSet->GetStoreId());
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {		
		TStr DocNm = RecStore->GetRecNm(RecSet->GetRecId(RecN));
		TStrV WdStrV; FtrExt->ExtractStrV(OgBase, RecSet->GetRec(RecN), WdStrV);
		BowDocBs->AddDoc(DocNm, TStrV(), WdStrV);
	}
	PBowDocWgtBs BowDocWgtBs = TBowDocWgtBs::New(BowDocBs, bwwtLogDFNrmTFIDF);
	TIntV AllDIdV; BowDocBs->GetAllDIdV(AllDIdV);
	PBowSpV BowSpV = TBowClust::GetConceptSpV(BowDocWgtBs, TBowSim::New(bstCos), AllDIdV);
	KWordSet = BowSpV->GetKWordSet(BowDocBs);
	KWordSet->SortByWgt();
}

PXmlTok TOgAggrKeywords::SaveXml() const { 
	PXmlTok TopTok = SaveXml(KWordSet);
	TopTok->AddArg("name", AggrNm);
	if (!JoinPathStr.Empty()) { TopTok->AddArg("join", JoinPathStr); }
	return TopTok;
}

PXmlTok TOgAggrKeywords::SaveXml(const PBowKWordSet& KWordSet) {
	PXmlTok TopTok = TXmlTok::New("keywords");
	const int KWords = TInt::GetMn(KWordSet->GetKWords(), 100);
	for (int KWordN = 0; KWordN < KWords; KWordN++) {
		PXmlTok ValTok = TXmlTok::New("keyword");
		ValTok->AddArg("str", KWordSet->GetKWordStr(KWordN));
		ValTok->AddArg("wgt", KWordSet->GetKWordWgt(KWordN));
		TopTok->AddSubTok(ValTok);
	}
	return TopTok;
}

PJsonVal TOgAggrKeywords::SaveJson() const { 
	return TJsonVal::NewNull(); //TODO
}

PXmlTok TOgAggrTimeLine::GetXmlList(const TStr& TokNm, const TStrH& StrH) const {
	const double FltCount = (Count > 0) ? double(Count) : 1;
	PXmlTok TopTok = TXmlTok::New(TokNm);
	int KeyId = StrH.FFirstKeyId();
	while (StrH.FNextKeyId(KeyId)) {
		PXmlTok XmlTok = TXmlTok::New(TokNm, StrH.GetKey(KeyId));
		const int ValFq = StrH[KeyId];
		const double Percent = 100.0 * (double(ValFq) / FltCount);
		XmlTok->AddArg("fq", ValFq);
		XmlTok->AddArg("perc", TFlt::GetStr(Percent, "%.2f"));
		TopTok->AddSubTok(XmlTok);
	}
	return TopTok;
}

TOgAggrTimeLine::TOgAggrTimeLine(const POgBase& OgBase, const POgRecSet& RecSet, const POgFtrExt& FtrExt) {
	JoinPathStr = TOgRecSet::GetJoinPathStr(OgBase, 
		OgBase->GetStoreByStoreId(RecSet->GetStoreId()), 
		FtrExt->GetJoinIdV(RecSet->GetStoreId()));
	AggrNm = FtrExt->GetNm(OgBase);
	for (int MonthN = 0; MonthN < 12; MonthN++) {
		MonthH.AddKey(TTmInfo::GetMonthNm(MonthN+1)); }
	for (int DayOfWeekN = 0; DayOfWeekN < 7; DayOfWeekN++) {
		DayOfWeekH.AddKey(TTmInfo::GetDayOfWeekNm(DayOfWeekN+1)); }
	for (int HourOfDayN = 0; HourOfDayN < 24; HourOfDayN++) {
		HourOfDayH.AddKey(TInt::GetStr(HourOfDayN)); }
	TTmV FtrValV; const int Recs = RecSet->GetRecs();
	for (int RecN = 0; RecN < Recs; RecN++) {
		FtrExt->ExtractTmV(OgBase, RecSet->GetRec(RecN), FtrValV);
		for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
			const TTm& Tm = FtrValV[FtrValN]; 
			if (Tm.IsDef()) {
				TSecTm SecTm(Tm); Count++;
				TStr DateStr = Tm.GetWebLogDateStr();
				TStr TimeStr = TStr::Fmt("%02d:00" , Tm.GetHour());
				AbsDateH.AddDat(DateStr)++;
				AbsTimeH.AddDat(DateStr + " " + TimeStr)++;
				MonthH.AddDat(SecTm.GetMonthNm())++;
				DayOfWeekH.AddDat(SecTm.GetDayOfWeekNm())++;
				if (0 <= Tm.GetHour() && Tm.GetHour() < 24) { 
					HourOfDayH[Tm.GetHour()]++; }
			}
		}
	}
	AbsDateH.SortByKey(true);
	AbsTimeH.SortByKey(true);
}

PXmlTok TOgAggrTimeLine::SaveXml() const { 
	PXmlTok TopTok = TXmlTok::New("timeline");
	TopTok->AddArg("name", AggrNm);
	if (!JoinPathStr.Empty()) { TopTok->AddArg("join", JoinPathStr); }

	TopTok->AddSubTok(GetXmlList("month", MonthH));
	TopTok->AddSubTok(GetXmlList("day-of-week", DayOfWeekH));
	TopTok->AddSubTok(GetXmlList("hour-of-day", HourOfDayH));
	TopTok->AddSubTok(GetXmlList("date", AbsDateH));
	TopTok->AddSubTok(GetXmlList("time", AbsTimeH));

	return TopTok;
}

PJsonVal TOgAggrTimeLine::SaveJson() const { 
	return TJsonVal::NewNull(); //TODO
}
