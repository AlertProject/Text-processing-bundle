#include "qminer_srv.h"

///////////////////////////////////////////
// QMiner-Server-Function
const POgStore& TOgSrvFun::GetStore(const TStrKdV& FldNmValPrV) const {
	TStr StoreIdStr = GetFldVal(FldNmValPrV, "storeid");
	OgAssertR(StoreIdStr.IsInt(), "Missing or invalid store ID");
	const uchar StoreId = (uchar)StoreIdStr.GetInt();
	OgAssertR(OgBase->IsStoreId(StoreId), "No store with such ID");
	return OgBase->GetStoreByStoreId(StoreId);
}

TOgRec TOgSrvFun::GetRec(const TStrKdV& FldNmValPrV, const POgStore& Store) const {
	TStr RecIdStr = GetFldVal(FldNmValPrV, "recid");
	OgAssertR(RecIdStr.IsInt(), "Missing or invalid record ID");
	const uint64 RecId = RecIdStr.GetUInt64();
	OgAssertR(Store->IsRecId(RecId), "No record with such ID");
	return Store->GetRec(RecId);
}

void TOgSrvFun::RegDefFunXml(const POgBase& OgBase, TSAppSrvFunV& SrvFunV) {
	// register basic functions
	SrvFunV.Add(TSASFunExit::New());
	// register qminer functions
	SrvFunV.Add(TOgSfStores::NewXml(OgBase));
	SrvFunV.Add(TOgSfWordVoc::NewXml(OgBase));
	SrvFunV.Add(TOgSfStoreRec::NewXml(OgBase));
	// register operators
	int OpKeyId = OgBase->GetFirstOpId();
	while (OgBase->GetNextOpId(OpKeyId)) {
		SrvFunV.Add(TOgSfOp::New(OgBase, OgBase->GetOp(OpKeyId), saotXml));
	}}

void TOgSrvFun::RegDefFunJson(const POgBase& OgBase, TSAppSrvFunV& SrvFunV) {
	// register basic functions
	SrvFunV.Add(TSASFunExit::New());
	// register qminer functions
	SrvFunV.Add(TOgSfStores::NewJson(OgBase));
	SrvFunV.Add(TOgSfWordVoc::NewJson(OgBase));
	SrvFunV.Add(TOgSfStoreRec::NewJson(OgBase));
	// register operators
	int OpKeyId = OgBase->GetFirstOpId();
	while (OgBase->GetNextOpId(OpKeyId)) {
		SrvFunV.Add(TOgSfOp::New(OgBase, OgBase->GetOp(OpKeyId), saotJSon));
	}
}

///////////////////////////////////////////
// QMiner-Server-Function-Stores
PXmlTok TOgSfStores::GetStoreFieldsXml(const POgStore& Store) {
	PXmlTok FieldsTok = TXmlTok::New("fields");
	for (int FieldN = 0; FieldN < Store->GetFields(); FieldN++) {
		const TOgFieldDesc& FieldDesc = Store->GetFieldDesc(FieldN);
		PXmlTok FieldTok = TXmlTok::New("field");
		FieldTok->AddArg("id", FieldN);
		FieldTok->AddArg("name", FieldDesc.GetFieldNm());
		FieldTok->AddArg("value-type", FieldDesc.GetFieldTypeStr());
		FieldTok->AddArg("feature-type", FieldDesc.GetDefFtrTypeStr());
		FieldTok->AddArg("aggregation-type", FieldDesc.GetAggrTypeStr());
		FieldTok->AddArg("display-type", FieldDesc.GetDisplayTypeStr());
		if (FieldDesc.IsKeys()) {
			PXmlTok KeysTok = TXmlTok::New("keys");
			for (int KeyN = 0; KeyN < FieldDesc.GetKeys(); KeyN++) {
				const int KeyId = FieldDesc.GetKeyId(KeyN);
				PXmlTok KeyTok = TXmlTok::New("key");
				KeyTok->AddArg("id", KeyId);
				KeysTok->AddSubTok(KeyTok);
			}
			FieldTok->AddSubTok(KeysTok);
		}
		FieldsTok->AddSubTok(FieldTok);
	}
	return FieldsTok;
}

PXmlTok TOgSfStores::GetStoreKeysXml(const POgStore& Store) {
	PXmlTok KeysTok = TXmlTok::New("keys");
	const POgIndexVoc& IndexVoc = OgBase->GetIndexVoc();
	const TIntSet& KeySet = IndexVoc->GetStoreKeys(Store->GetStoreId());
	int KeySetId = KeySet.FFirstKeyId();
	while (KeySet.FNextKeyId(KeySetId)) {
		const int KeyId = KeySet.GetKey(KeySetId);
		const TOgIndexKey& Key = IndexVoc->GetKey(KeyId);
		if (!Key.IsDef()) { continue; }
		if (Key.IsInternal()) { continue; }
		PXmlTok KeyTok = TXmlTok::New("key");
		KeyTok->AddArg("id", KeyId);
		KeyTok->AddArg("name", Key.GetKeyNm());
		KeyTok->AddArg("text", Key.IsText());
		KeyTok->AddArg("aggregation", Key.IsAggr());
		if (Key.IsSort()) {	KeyTok->AddArg("sort-by", TStr(Key.IsSortById() ? "word-id" : "word-str")); }
		PXmlTok WordVocTok = TXmlTok::New("word-voc");
		WordVocTok->AddArg("id", Key.GetWordVocId());
		WordVocTok->AddArg("values", IndexVoc->GetWords(KeyId));
		KeyTok->AddSubTok(WordVocTok);
		if (Key.IsFields()) {
			PXmlTok FieldsTok = TXmlTok::New("fields");
			for (int FieldN = 0; FieldN < Key.GetFields(); FieldN++) {
				const int FieldId = Key.GetFieldId(FieldN);
				PXmlTok FieldTok = TXmlTok::New("field");
				FieldTok->AddArg("id", FieldId);
				FieldsTok->AddSubTok(FieldTok);
			}
			KeyTok->AddSubTok(FieldsTok);
		}
		KeysTok->AddSubTok(KeyTok);
	}
	return KeysTok;
}

PXmlTok TOgSfStores::GetStoreJoinsXml(const POgStore& Store) {
	// output the results 
	PXmlTok JoinsTok = TXmlTok::New("joins");
	for (int JoinId = 0; JoinId < Store->GetJoins(); JoinId++) {
		// get join description
		const TOgJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
		const int JoinStoreId = JoinDesc.GetJoinStoreId();
		TStr JoinStoreNm = OgBase->GetStoreByStoreId(JoinStoreId)->GetStoreNm();
		//serialize it to XML
		PXmlTok JoinTok = TXmlTok::New("join");
		JoinTok->AddArg("id", JoinId);
		JoinTok->AddArg("name", JoinDesc.GetJoinNm());
		JoinTok->AddArg("store-id", JoinStoreId);
		JoinTok->AddArg("store-name", JoinStoreNm);
		if (JoinDesc.IsFieldJoin()) {
			JoinTok->AddArg("type", TStr("field"));
		} else if (JoinDesc.IsIndexJoin()) {
			JoinTok->AddArg("type", TStr("index"));
		}
		JoinsTok->AddSubTok(JoinTok);
	}
	return JoinsTok;
}

PXmlTok TOgSfStores::GetStoreXml(const POgStore& Store) {
	// get basic properties
	PXmlTok StoreTok = TXmlTok::New("store");
	StoreTok->AddArg("id", int(Store->GetStoreId()));
	StoreTok->AddArg("name", Store->GetStoreNm());
	StoreTok->AddArg("records", Store->GetRecs());
	StoreTok->AddSubTok(GetStoreFieldsXml(Store));
	StoreTok->AddSubTok(GetStoreKeysXml(Store));
	StoreTok->AddSubTok(GetStoreJoinsXml(Store));
	return StoreTok;
}

PXmlDoc TOgSfStores::ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
	PXmlTok TopTok = TXmlTok::New("stores");
	const POgBase& OgBase = GetOgBase();
	const int Stores = OgBase->GetStores();
	for (int StoreN = 0; StoreN < Stores; StoreN++) {
		const POgStore& Store = OgBase->GetStoreByStoreN(StoreN);
		TopTok->AddSubTok(GetStoreXml(Store));
	}
	return TXmlDoc::New(TopTok);
}

PJsonVal TOgSfStores::GetStoreFieldsJson(const POgStore& Store) {
	TJsonValV FieldValV;
	for (int FieldN = 0; FieldN < Store->GetFields(); FieldN++) {
		const TOgFieldDesc& FieldDesc = Store->GetFieldDesc(FieldN);
		PJsonVal FieldVal = TJsonVal::NewObj();
		FieldVal->AddToObj("fieldId", FieldN);
		FieldVal->AddToObj("fieldName", FieldDesc.GetFieldNm());
		FieldVal->AddToObj("valueType", FieldDesc.GetFieldTypeStr());
		FieldVal->AddToObj("featureType", FieldDesc.GetDefFtrTypeStr());
		FieldVal->AddToObj("aggregationType", FieldDesc.GetAggrTypeStr());
		FieldVal->AddToObj("displayType", FieldDesc.GetDisplayTypeStr());
		if (FieldDesc.IsKeys()) {
			TJsonValV KeyValV;
			for (int KeyN = 0; KeyN < FieldDesc.GetKeys(); KeyN++) {
				const int KeyId = FieldDesc.GetKeyId(KeyN);
				KeyValV.Add(TJsonVal::NewObj("keyId", KeyId));
			}
			FieldVal->AddToObj("keys", TJsonVal::NewArr(KeyValV));
		}
		FieldValV.Add(FieldVal);
	}
	return TJsonVal::NewArr(FieldValV);
}

PJsonVal TOgSfStores::GetStoreKeysJson(const POgStore& Store) {
	TJsonValV KeyValV;
	const POgIndexVoc& IndexVoc = OgBase->GetIndexVoc();
	const TIntSet& KeySet = IndexVoc->GetStoreKeys(Store->GetStoreId());
	int KeySetId = KeySet.FFirstKeyId();
	while (KeySet.FNextKeyId(KeySetId)) {
		const int KeyId = KeySet.GetKey(KeySetId);
		const TOgIndexKey& Key = IndexVoc->GetKey(KeyId);
		if (!Key.IsDef()) { continue; }
		if (Key.IsInternal()) { continue; }
		PJsonVal KeyVal = TJsonVal::NewObj();
		KeyVal->AddToObj("keyId", KeyId);
		KeyVal->AddToObj("keyName", Key.GetKeyNm());
		KeyVal->AddToObj("keyText", Key.IsText());
		KeyVal->AddToObj("aggregation", Key.IsAggr());
		if (Key.IsSort()) {	KeyVal->AddToObj("sortBy", TStr(Key.IsSortById() ? "word-id" : "word-str")); }
		PJsonVal WordVocVal = TJsonVal::NewObj();
		WordVocVal->AddToObj("wordVocId", Key.GetWordVocId());
		WordVocVal->AddToObj("values", (int)IndexVoc->GetWords(KeyId));
		KeyVal->AddToObj("wordVoc", WordVocVal);
		if (Key.IsFields()) {
			TJsonValV FieldValV;
			for (int FieldN = 0; FieldN < Key.GetFields(); FieldN++) {
				const int FieldId = Key.GetFieldId(FieldN);
				FieldValV.Add(TJsonVal::NewObj("fieldId", FieldId));
			}
			KeyVal->AddToObj("fields", TJsonVal::NewArr(FieldValV));
		}
		KeyValV.Add(KeyVal);
	}
	return TJsonVal::NewArr(KeyValV);
}

PJsonVal TOgSfStores::GetStoreJoinsJson(const POgStore& Store) {
	// output the results 
	TJsonValV JoinValV;
	for (int JoinId = 0; JoinId < Store->GetJoins(); JoinId++) {
		// get join description
		const TOgJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
		const int JoinStoreId = (int)JoinDesc.GetJoinStoreId();
		TStr JoinStoreNm = OgBase->GetStoreByStoreId(JoinStoreId)->GetStoreNm();
		//serialize it to XML
		PJsonVal JoinVal = TJsonVal::NewObj();
		JoinVal->AddToObj("joinId", JoinId);
		JoinVal->AddToObj("joinName", JoinDesc.GetJoinNm());
		JoinVal->AddToObj("joinStoreId", JoinStoreId);
		JoinVal->AddToObj("joinStoreName", JoinStoreNm);
		if (JoinDesc.IsFieldJoin()) {
			JoinVal->AddToObj("joinType", TStr("field"));
		} else if (JoinDesc.IsIndexJoin()) {
			JoinVal->AddToObj("joinType", TStr("index"));
		}
		JoinValV.Add(JoinVal);
	}
	return TJsonVal::NewArr(JoinValV);
}

PJsonVal TOgSfStores::GetStoreJson(const POgStore& Store) {
	// get basic properties
	PJsonVal StoreVal = TJsonVal::NewObj();
	StoreVal->AddToObj("storeId", int(Store->GetStoreId()));
	StoreVal->AddToObj("storeName", Store->GetStoreNm());
	StoreVal->AddToObj("storeRecords", int(Store->GetRecs()));
	StoreVal->AddToObj("fields", GetStoreFieldsJson(Store));
	StoreVal->AddToObj("keys", GetStoreKeysJson(Store));
	StoreVal->AddToObj("joins", GetStoreJoinsJson(Store));
	return StoreVal;
}

TStr TOgSfStores::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
	const POgBase& OgBase = GetOgBase();
	const int Stores = OgBase->GetStores();
	TJsonValV StoreValV;
	for (int StoreN = 0; StoreN < Stores; StoreN++) {
		const POgStore& Store = OgBase->GetStoreByStoreN(StoreN);
		StoreValV.Add(GetStoreJson(Store));
	}
	PJsonVal JsonVal = TJsonVal::NewArr(StoreValV);
	return TJsonVal::GetStrFromVal(JsonVal);
}


///////////////////////////////////////////
// QMiner-Server-Function-WordVoc
void TOgSfWordVoc::GetWordVoc(const TStrKdV& FldNmValPrV, TStrIntPrV& WordStrFqV) {
	POgIndexVoc IndexVoc = OgBase->GetIndexVoc();
	// read key id
	int KeyId = -1;
	if (IsFldNm(FldNmValPrV, "keyid")) {
		KeyId = GetFldInt(FldNmValPrV, "keyid");
		if (!IndexVoc->IsKeyId(KeyId) || !IndexVoc->IsWordVoc(KeyId)) {
			throw TOgExcept::New(TStr::Fmt("Wrong keyid='%d': unknown ID or no vocabular!", KeyId));
		}
	} else if (IsFldNm(FldNmValPrV, "store") && IsFldNm(FldNmValPrV, "key")) {
		// parse store
		TStr StoreNm = GetFldVal(FldNmValPrV, "store");
		if (!OgBase->IsStoreNm(StoreNm)) { throw TOgExcept::New("Unknown store " + StoreNm); }
		const uchar StoreId = OgBase->GetStoreByStoreNm(StoreNm)->GetStoreId();
		// parse key
		TStr KeyNm = GetFldVal(FldNmValPrV, "key");
		if (!IndexVoc->IsKeyNm(StoreId, KeyNm)) { throw TOgExcept::New("Unknown key " + StoreNm + "." + KeyNm); }
		KeyId = IndexVoc->GetKeyId(StoreId, KeyNm);
	} else {
		throw TOgExcept::New("No specified key (either keyid or [store,key] names)");
	}
	// filter down by prefix
	TStr PrefixStr = GetFldVal(FldNmValPrV, "prefix");
	// get all the words
	IndexVoc->GetAllWordStrFqV(KeyId, WordStrFqV);
	// find all the matching words
	TIntStrPrV WordFqStrV;
	const int Words = WordStrFqV.Len();
	for (int WordN = 0; WordN < Words; WordN++) {
		const TStr& WordStr = WordStrFqV[WordN].Val1;
		if (PrefixStr.Empty() || WordStr.IsPrefix(PrefixStr)) {
			const int WordFq = WordStrFqV[WordN].Val2;
			WordFqStrV.Add(TIntStrPr(WordFq, WordStr));
		}
	}
	// sort limit
	WordFqStrV.Sort(false);
	// limit if necessary
	const int MxWords = GetFldInt(FldNmValPrV, "limit", "-1");
	if (MxWords != -1) { WordFqStrV.Trunc(MxWords); }
	// output the results 
	GetSwitchedPrV<TInt, TStr>(WordFqStrV, WordStrFqV);
}

PXmlDoc TOgSfWordVoc::ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
	try {
		// get matching words
		TStrIntPrV WordStrFqV; GetWordVoc(FldNmValPrV, WordStrFqV);
		// output the results 
		PXmlTok TopTok = TXmlTok::New("word-voc");
		for (int WordN = 0; WordN < WordStrFqV.Len(); WordN++) {
			PXmlTok WordTok = TXmlTok::New("word");
			WordTok->AddArg("str", WordStrFqV[WordN].Val1);
			WordTok->AddArg("fq", WordStrFqV[WordN].Val2);
			TopTok->AddSubTok(WordTok);
		}
		return TXmlDoc::New(TopTok);
	} catch (PExcept Except) {
		return GetErrorXmlRes(Except->GetMsgStr());
	}
}

TStr TOgSfWordVoc::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
	try {
		// get matching words
		TStrIntPrV WordStrFqV; GetWordVoc(FldNmValPrV, WordStrFqV);
		// output the results 
		TJsonValV WordValV;		
		for (int WordN = 0; WordN < WordStrFqV.Len(); WordN++) {
			PJsonVal WordVal = TJsonVal::NewObj();
			WordVal->AddToObj("str", WordStrFqV[WordN].Val1);
			WordVal->AddToObj("fq", WordStrFqV[WordN].Val2);
			WordValV.Add(WordVal);
		}
		PJsonVal JsonVal = TJsonVal::NewArr(WordValV);
		return TJsonVal::GetStrFromVal(JsonVal);
	} catch (PExcept Except) {
		return GetErrorJsonRes(Except->GetMsgStr());
	}
}

///////////////////////////////////////////
// QMiner-Server-Function-Record
PXmlDoc TOgSfStoreRec::ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
	const POgStore& Store = GetStore(FldNmValPrV);
	TOgRec Rec = GetRec(FldNmValPrV, Store);
	return Rec.SaveXml(OgBase);
}

TStr TOgSfStoreRec::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
	const POgStore& Store = GetStore(FldNmValPrV);
	TOgRec Rec = GetRec(FldNmValPrV, Store);
	return TJsonVal::GetStrFromVal(Rec.SaveJson(OgBase));
}

///////////////////////////////////////////
// QMiner-Server-Function-Operator
PXmlDoc TOgSfOp::ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
	// executed operator
	TStrKdV OpFldNmValPrV = FldNmValPrV;
	POgRecSet RecSet = Op->Exec(OgBase, TOgRecSetV(), OpFldNmValPrV);
	// read offset paramaters
	const int Offset = GetFldInt(FldNmValPrV, "offset", "0");
	const int MxHits = GetFldInt(FldNmValPrV, "hits", "100");
	// prepare XML serialization
	return RecSet->SaveXml(OgBase, MxHits, Offset, true);
}

TStr TOgSfOp::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
	// executed operator
	TStrKdV OpFldNmValPrV = FldNmValPrV;
	POgRecSet RecSet = Op->Exec(OgBase, TOgRecSetV(), OpFldNmValPrV);
	// read offset paramaters
	const int Offset = GetFldInt(FldNmValPrV, "offset", "0");
	const int MxHits = GetFldInt(FldNmValPrV, "hits", "100");
	// prepare XML serialization
	PJsonVal RecSetVal = RecSet->SaveJson(OgBase, MxHits, Offset, true);
	return TJsonVal::GetStrFromVal(RecSetVal);
}
