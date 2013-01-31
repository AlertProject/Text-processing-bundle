#include "qminer_rdf.h"

///////////////////////////////////////////
// QMiner-RDF-Endpoint-Description
TOgRdfEndpointBs::TOgRdfEndpointBs(const POgBase& OgBase, 
		const TStr& _RootUrlStr, const TStr& XmlFNm): RootUrlStr(_RootUrlStr) {

	PXmlDoc XmlDoc = TXmlDoc::LoadTxt(XmlFNm);
	// iterate over all the namespaces in the XML file
	TXmlTokV NmSpaceTokV; XmlDoc->GetTok()->GetTagTokV("namespaces|namespace", NmSpaceTokV);
	for (int NmSpaceTokN = 0; NmSpaceTokN < NmSpaceTokV.Len(); NmSpaceTokN++) {
		PXmlTok NmSpaceTok = NmSpaceTokV[NmSpaceTokN];
		TStr ShortStr = NmSpaceTok->GetStrArgVal("short");
		TStr LongStr = NmSpaceTok->GetStrArgVal("long");
		NmSpaceV.Add(TStrPr(ShortStr, LongStr));
	}
    // iterate over all the stores in the XML file
    TXmlTokV StoreTokV; XmlDoc->GetTok()->GetTagTokV("store", StoreTokV);
    for (int StoreTokN = 0; StoreTokN < StoreTokV.Len(); StoreTokN++) {
        PXmlTok StoreTok = StoreTokV[StoreTokN];
        // get store id
        const uchar StoreId = uchar(StoreTok->GetIntArgVal("id"));
        const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
        // get rdf-endpoint name
        if (!StoreTok->IsSubTag("rdf-endpoint")) { continue; }
        PXmlTok RdfTok = StoreTok->GetTagTok("rdf-endpoint");
        TStr EndpointNm = RdfTok->GetStrArgVal("name");
        EndpointToStoreH.AddDat(EndpointNm, StoreId);
        StoreToEndpointH.AddDat(StoreId, EndpointNm);
        // read record handle type
        TStr RecHandleStr = RdfTok->GetStrArgVal("rec-handle");
        if (RecHandleStr == "ID") { RecIdEndpointH.AddKey(EndpointNm); }
        else if (RecHandleStr == "NAME") { RecNmEndpointH.AddKey(EndpointNm); }
        else { continue; } // unknown handle type
        // get all properties
        TXmlTokV PropertyTokV; RdfTok->GetTagTokV("property", PropertyTokV);
        for (int PropertyTokN = 0; PropertyTokN < PropertyTokV.Len(); PropertyTokN++) {
            PXmlTok PropertyTok = PropertyTokV[PropertyTokN];
            TStr PredStr = PropertyTok->GetStrArgVal("predicate");
            TStr ObjStr = PropertyTok->GetStrArgVal("object");
            StoreToPropertyVH.AddDat(StoreId).Add(TStrPr(PredStr, ObjStr));
        }
        // get all field properties
        TXmlTokV FieldPropertyTokV; RdfTok->GetTagTokV("field-property", FieldPropertyTokV);
        for (int FieldPropertyTokN = 0; FieldPropertyTokN < FieldPropertyTokV.Len(); FieldPropertyTokN++) {
            PXmlTok FieldPropertyTok = FieldPropertyTokV[FieldPropertyTokN];
            TStr PredStr = FieldPropertyTok->GetStrArgVal("predicate");
            // read and parse field id
            TStr FieldNm = FieldPropertyTok->GetStrArgVal("object-field");
            OgAssertR(Store->IsFieldNm(FieldNm), 
                "Wrong field name " + FieldNm + " in " + XmlFNm);
            const int FieldId = Store->GetFieldId(FieldNm);
            // get type and store the property
            TStr TypeStr = FieldPropertyTok->GetStrArgVal("type");
            if (TypeStr == "URI") {
                StoreToFieldUriVH.AddDat(StoreId).Add(TStrIntPr(PredStr, FieldId));
            } else if (TypeStr == "LITERAL") {
                StoreToFieldLiteralVH.AddDat(StoreId).Add(TStrIntPr(PredStr, FieldId));
            }
        }
        // get all join properties
        TXmlTokV JoinPropertyTokV; RdfTok->GetTagTokV("join-property", JoinPropertyTokV);
        for (int JoinPropertyTokN = 0; JoinPropertyTokN < JoinPropertyTokV.Len(); JoinPropertyTokN++) {
            PXmlTok JoinPropertyTok = JoinPropertyTokV[JoinPropertyTokN];
            TStr PredStr = JoinPropertyTok->GetStrArgVal("predicate");
            // read and parse join id
            TStr JoinNm = JoinPropertyTok->GetStrArgVal("object-join");
            OgAssertR(Store->IsJoinNm(JoinNm), 
                "Wrong join name " + JoinNm + " in " + XmlFNm);
            const int JoinId = Store->GetJoinId(JoinNm);
            // store the property
            StoreToJoinVH.AddDat(StoreId).Add(TStrIntPr(PredStr, JoinId));
        }
    }
}

bool TOgRdfEndpointBs::IsEndpoint(const TStr& EndpointNm) const { 
    return EndpointToStoreH.IsKey(EndpointNm);
}

uchar TOgRdfEndpointBs::GetEndpointStoreId(const TStr& EndpointNm) const { 
    return EndpointToStoreH.GetDat(EndpointNm).Val; 
}

bool TOgRdfEndpointBs::IsStore(const uchar& StoreId) const { 
    return StoreToEndpointH.IsKey(StoreId);
}

const TStr& TOgRdfEndpointBs::GetStoreEndpointNm(const uchar& StoreId) const { 
    return StoreToEndpointH.GetDat(StoreId); 
}

bool TOgRdfEndpointBs::IsRecIdEndpoint(const TStr& EndpointNm) const {  
    return RecIdEndpointH.IsKey(EndpointNm);
}

bool TOgRdfEndpointBs::IsRecNmEndpoint(const TStr& EndpointNm) const {
    return RecNmEndpointH.IsKey(EndpointNm);
}

bool TOgRdfEndpointBs::IsStoreProperty(const uchar& StoreId) const { 
    return StoreToPropertyVH.IsKey(StoreId);
}

const TStrPrV& TOgRdfEndpointBs::GetStoreProperty(const uchar& StoreId) const { 
    return StoreToPropertyVH.GetDat(StoreId); 
}

bool TOgRdfEndpointBs::IsStoreFieldUri(const uchar& StoreId) const { 
    return StoreToFieldUriVH.IsKey(StoreId); 
}

const TStrIntPrV& TOgRdfEndpointBs::GetStoreFieldUri(const uchar& StoreId) const { 
    return StoreToFieldUriVH.GetDat(StoreId); 
}

bool TOgRdfEndpointBs::IsStoreFieldLiteral(const uchar& StoreId) const { 
    return StoreToFieldLiteralVH.IsKey(StoreId); 
}
const TStrIntPrV& TOgRdfEndpointBs::GetStoreFieldLiteral(const uchar& StoreId) const { 
    return StoreToFieldLiteralVH.GetDat(StoreId); 
}

bool TOgRdfEndpointBs::IsStoreJoin(const uchar& StoreId) const { 
    return StoreToJoinVH.IsKey(StoreId); 
}

const TStrIntPrV& TOgRdfEndpointBs::GetStoreJoin(const uchar& StoreId) const { 
    return StoreToJoinVH.GetDat(StoreId); 
}

TStr TOgRdfEndpointBs::GetRecUri(const POgBase& OgBase, const POgStore& Store, const uint64& RecId) {
    TChA RecUriChA;
    // e.g. http://localhost:8080/rdf/
    RecUriChA += RootUrlStr;
    // e.g. http://localhost:8080/rdf/store/
    const uchar StoreId = Store->GetStoreId();
    const TStr& EndpointNm = GetStoreEndpointNm(StoreId);
    RecUriChA += EndpointNm; RecUriChA += '/';
    // e.g. http://localhost:8080/rdf/store/123
    if (IsRecIdEndpoint(EndpointNm)) { 
		RecUriChA += TUInt64::GetStr(RecId);
    } else {
        RecUriChA += Store->GetRecNm(RecId);
    }
    // done
    return RecUriChA;
}

PRdfGraph TOgRdfEndpointBs::MakeRdfGraph(const POgBase& OgBase, const POgStore& Store, const uint64& RecId) {
    const uchar StoreId = Store->GetStoreId();
	PRdfGraph RdfGraph = TRdfGraph::New(GetNmSpaceV());
    // create record resource
    PRdfNode RecNode = RdfGraph->AddUri(GetRecUri(OgBase, Store, RecId));
    // add fixed properties
    if (IsStoreProperty(StoreId)) {
        const TStrPrV& PropertyV = GetStoreProperty(StoreId);
        for (int PropertyN = 0; PropertyN < PropertyV.Len(); PropertyN ++) {
            const TStrPr& Property = PropertyV[PropertyN];
            PRdfNode PredNode = RdfGraph->AddUri(Property.Val1);
            PRdfNode ObjNode = RdfGraph->AddUri(Property.Val2);
            RdfGraph->AddTriple(RecNode, PredNode, ObjNode);
        }
    }
    // add field URIs
    if (IsStoreFieldUri(StoreId)) { 
        const TStrIntPrV& FieldUriV = GetStoreFieldUri(StoreId);
        for (int FieldUriN = 0; FieldUriN < FieldUriV.Len(); FieldUriN ++) {
            const TStrIntPr& FieldUri = FieldUriV[FieldUriN];
            PRdfNode PredNode = RdfGraph->AddUri(FieldUri.Val1);
            PRdfNode ObjNode = RdfGraph->AddUri(Store->GetFieldStr(RecId, FieldUri.Val2));
            RdfGraph->AddTriple(RecNode, PredNode, ObjNode);
        }
    }
    // add field Literals
    if (IsStoreFieldLiteral(StoreId)) {
        const TStrIntPrV& FieldLiteralV = GetStoreFieldLiteral(StoreId);
        for (int FieldLiteralN = 0; FieldLiteralN < FieldLiteralV.Len(); FieldLiteralN ++) {
            const TStrIntPr& FieldLiteral = FieldLiteralV[FieldLiteralN];
            PRdfNode PredNode = RdfGraph->AddUri(FieldLiteral.Val1);
            // prepare literal node depanding on the field type
            PRdfNode ObjNode; const int FieldId = FieldLiteral.Val2;
            const TOgFieldDesc& FieldDesc = Store->GetFieldDesc(FieldId);
            if (FieldDesc.IsInt()) {
                ObjNode = RdfGraph->AddLiteralInt(Store->GetFieldInt(RecId, FieldId));
            } else if (FieldDesc.IsStr()) {
                ObjNode = RdfGraph->AddLiteralStr(Store->GetFieldStr(RecId, FieldId));
            } else if (FieldDesc.IsStr()) {
                ObjNode = RdfGraph->AddLiteralFlt(Store->GetFieldFlt(RecId, FieldId));
            } else if (FieldDesc.IsStr()) {
                TTm FieldTm; Store->GetFieldTm(RecId, FieldId, FieldTm);
                ObjNode = RdfGraph->AddLiteralDate(FieldTm);
            } else {
                TOgExcept::Throw("Cannot convert field to RDF Literal node!");
            }
            RdfGraph->AddTriple(RecNode, PredNode, ObjNode); 
        }
    }
    // add joins
    if (IsStoreJoin(StoreId)) {
        const TStrIntPrV& JoinV = GetStoreJoin(StoreId);
        for (int JoinN = 0; JoinN < JoinV.Len(); JoinN ++) {
            const TStrIntPr& Join = JoinV[JoinN];
            PRdfNode PredNode = RdfGraph->AddUri(Join.Val1);
            // do join
            const int& JoinId = Join.Val2;
            POgRecSet RecSet = TOgRecSet::New(StoreId, TUInt64V::GetV(RecId));
            POgRecSet JoinRecSet = RecSet->DoJoin(OgBase, JoinId, 100, false);
            // prepare uris of the joined records
            const uchar JoinStoreId = JoinRecSet->GetStoreId();
            const POgStore& JoinStore = OgBase->GetStoreByStoreId(JoinStoreId);
            const int JoinRecs = JoinRecSet->GetRecs();
            for (int JoinRecN = 0; JoinRecN < JoinRecs; JoinRecN++) {
                const uint64 JoinRecId = JoinRecSet->GetRecId(JoinRecN);
                PRdfNode ObjNode = RdfGraph->AddUri(GetRecUri(OgBase, JoinStore, JoinRecId));
                RdfGraph->AddTriple(RecNode, PredNode, ObjNode);
            }
        }
    }
    // return generated graph
    return RdfGraph;
}

///////////////////////////////////////////
// QMiner-Server-Function-RDF-Endpoint
TOgRdfEndpointSf::TOgRdfEndpointSf(const POgBase& OgBase, const TStr& RootUrlStr, const TStr& RdfXmlFNm):
	TOgSrvFun(OgBase, "rdf", saotXml), RdfEndpointBs(OgBase, RootUrlStr, RdfXmlFNm) { }
		
PXmlDoc TOgRdfEndpointSf::ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
    PUrl Url = RqEnv->GetHttpRq()->GetUrl();
    // get the store
    if (Url->GetPathSegs() >= 2) {    
        TStr EndpointNm = Url->GetPathSeg(1);
        OgAssertR(RdfEndpointBs.IsEndpoint(EndpointNm), "Unkown endpoint " + EndpointNm);
        const uchar StoreId = RdfEndpointBs.GetEndpointStoreId(EndpointNm);
        const POgStore& Store = OgBase->GetStoreByStoreId(StoreId);
        // get the record id
        if (Url->GetPathSegs() >= 3) {
            uint64 RecId = 0; TStr RecHndStr = Url->GetPathSeg(2);
            if (RdfEndpointBs.IsRecIdEndpoint(EndpointNm)) {
                OgAssertR(RecHndStr.IsInt(), "Record ID not a number: " + RecHndStr);
				RecId = RecHndStr.GetUInt64(); 
                OgAssertR(Store->IsRecId(RecId), "Record with such ID does not exist: " + RecHndStr);
            } else if (RdfEndpointBs.IsRecNmEndpoint(EndpointNm)) {
                OgAssertR(Store->IsRecNm(RecHndStr), "Record with such name does not exist: " + RecHndStr);
                RecId = Store->GetRecId(RecHndStr);
            }
            // generate RDF graph
            PRdfGraph RdfGraph = RdfEndpointBs.MakeRdfGraph(OgBase, Store, RecId);
			if (RdfGraph->IsNodeId(0)) {
		        // return serializated rdf graph
			    return RdfGraph->GetRdfXml(RdfGraph->GetNode(0));
			} else {
				return GetErrorXmlRes("Empty graph!");
			}
        } else {
            return GetErrorXmlRes("No record endpoint name specified in the URL!");
        }
    } else {
	    return GetErrorXmlRes("No store endpoint name specified in the URL!");
    }
}
