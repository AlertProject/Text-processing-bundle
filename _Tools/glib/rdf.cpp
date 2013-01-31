#include "stdafx.h"
#include "rdf.h"

/////////////////////////////////////////////////
// RDF-URIs
TStrPr TRdfUri::RdfNmSpace = TStrPr("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
TStr TRdfUri::RdfResource = "rdf:resource";
TStr TRdfUri::RdfAbout = "rdf:about";

TStr TRdfUri::GetUriStr(const TStr& Str) {
	// transform string into unicode string
	TUStr UStr(Str);
	// simplify the string
	TStr SimpleStr = UStr.GetStarterLowerCaseStr();
	// remove new lines
	SimpleStr.DelChAll('\n'); SimpleStr.DelChAll('\r');
	// shorten spaces
    SimpleStr.ChangeStrAll("  ", " ", true);
	// replace spaces with underscore
	SimpleStr.ChangeChAll(' ', '_');
	// done :-)
	return SimpleStr;
}

/////////////////////////////////////////////////
// RDF-Node
TStr TRdfNode::GetNodeType() const {
	if (IsUri()) { return "URI reference"; }
	else if (IsBlank()) { return "Blank Node"; }
	else if (IsLiteral()) { return "Literal"; }
    return "";
}

TStr TRdfNode::GetFullUri(const TStrPrV& NmSpaceV) const { 
	EAssert(IsUri());
	for (int NmSpaceN = 0; NmSpaceN < NmSpaceV.Len(); NmSpaceN++) {
		if (UriStr.IsPrefix(NmSpaceV[NmSpaceN].Val1)) {
			TStr FullUriStr = UriStr;
			FullUriStr.ChangeStr(
				NmSpaceV[NmSpaceN].Val1 + ":", 
				NmSpaceV[NmSpaceN].Val2);
			return FullUriStr;
		}
	}
	return UriStr;
}

TStr TRdfNode::GetShortUri(const TStrPrV& NmSpaceV) const { 
	EAssert(IsUri());
	for (int NmSpaceN = 0; NmSpaceN < NmSpaceV.Len(); NmSpaceN++) {
		if (UriStr.IsPrefix(NmSpaceV[NmSpaceN].Val2)) {
			TStr ShortUriStr = UriStr;
			ShortUriStr.ChangeStr(
				NmSpaceV[NmSpaceN].Val2,
				NmSpaceV[NmSpaceN].Val1 + ":");
			return ShortUriStr;
		}
	}
	return UriStr;
}

/////////////////////////////////////////////////
// RDF-Triple
TRdfTriple::TRdfTriple(const int& _TripleId, const PRdfNode& _Subject, const PRdfNode& _Predicate, 
		const PRdfNode& _Object): TripleId(_TripleId), Subject(_Subject), Predicate(_Predicate), Object(_Object) {

	EAssertR(Subject->IsUri() || Subject->IsBlank(), "Wrong Subject type " + Subject->GetNodeType());
	EAssertR(Predicate->IsUri(), "Wrong Predicate type " + Predicate->GetNodeType());
	EAssertR(Object->IsUri() || Object->IsBlank() || Object->IsLiteral(), "Wrong Object type " + Object->GetNodeType());
}

/////////////////////////////////////////////////
// RDF-Graph
PRdfNode TRdfGraph::AddUri(const TStr& UriStr) {
	// return existing one if there already
	if (UriNodeH.IsKey(UriStr)) { 
		return NodeV[UriNodeH.GetDat(UriStr)]; 
	} else {
		// create new of otherwise
		const int UriNodeId = NodeV.Len();
		PRdfNode UriNode = new TRdfNode(UriNodeId, UriStr);
		EAssert(NodeV.Add(UriNode) == UriNodeId);
		UriNodeH.AddDat(UriNode->GetUri(), UriNodeId);
		return UriNode;
	}
}

PRdfNode TRdfGraph::AddBlank(const TStr& NodeIdStr) {
	const int BlankNodeId = NodeV.Len();
	PRdfNode BlankNode = new TRdfNode(TRdfNode::rntBlank, BlankNodeId, NodeIdStr);
	EAssert(NodeV.Add(BlankNode) == BlankNodeId);
	return BlankNode;
}

PRdfNode TRdfGraph::AddLiteralStr(const TStr& LiteralStr, const TStr& LangStr)  {
	const int LiteralNodeId = NodeV.Len();
	PRdfNode LiteralNode = new TRdfNode(LiteralNodeId, LiteralStr, LangStr);
	EAssert(NodeV.Add(LiteralNode) == LiteralNodeId);
	return LiteralNode;
}

PRdfNode TRdfGraph::AddLiteralInt(const int& LiteralInt) {
	const int LiteralNodeId = NodeV.Len();
	PRdfNode LiteralNode = new TRdfNode(LiteralNodeId, LiteralInt);
	EAssert(NodeV.Add(LiteralNode) == LiteralNodeId);
	return LiteralNode;
}

PRdfNode TRdfGraph::AddLiteralFlt(const double& LiterlFlt) {
	const int LiteralNodeId = NodeV.Len();
	PRdfNode LiteralNode = new TRdfNode(LiteralNodeId, LiterlFlt);
	EAssert(NodeV.Add(LiteralNode) == LiteralNodeId);
	return LiteralNode;
}

PRdfNode TRdfGraph::AddLiteralDate(const TTm& LiterlTm) {
	const int LiteralNodeId = NodeV.Len();
	PRdfNode LiteralNode = new TRdfNode(LiteralNodeId, LiterlTm);
	EAssert(NodeV.Add(LiteralNode) == LiteralNodeId);
	return LiteralNode;
}

PRdfTriple TRdfGraph::GetTriple(const int& TripleId) const { 
	Assert(IsTripleId(TripleId)); 
	const TIntTr& Triple = TripleV[TripleId];
	return PRdfTriple(new TRdfTriple(TripleId, GetNode(Triple.Val1), 
		GetNode(Triple.Val2), GetNode(Triple.Val3)));
}

PRdfTriple TRdfGraph::AddTriple(const PRdfNode& Subject, const PRdfNode& Predicate, const PRdfNode& Object) {
	// generarte Id
	const int TripleId = TripleV.Len();
	// store as integer triple 
	TIntTr Triple(Subject->GetNodeId(), Predicate->GetNodeId(), Object->GetNodeId());
	EAssert(TripleV.Add(Triple) == TripleId);
	// index node apperances
	SubNodeTripleVH.AddDat(Subject->GetNodeId()).Add(TripleId);
	PredNodeTripleVH.AddDat(Predicate->GetNodeId()).Add(TripleId);
	ObjNodeTripleVH.AddDat(Object->GetNodeId()).Add(TripleId);
	// return proper triple object
	return  PRdfTriple(new TRdfTriple(TripleId, Subject, Predicate, Object));
}

//<?xml version="1.0"?>
//<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:cd="http://www.recshop.fake/cd#">
//	<rdf:Description rdf:about="http://www.recshop.fake/cd/Empire Burlesque">
//		<cd:artist rdf:resource="http://www.recshop.fake/cd/dylan" />
//		...
//		...
//	</rdf:Description>
//</rdf:RDF>

PXmlDoc TRdfGraph::GetRdfXml(const PRdfNode& Node) const {
	PXmlTok RdfTok = TXmlTok::New("rdf:RDF");
	// load name spaces
	for (int NmSpaceN = 0; NmSpaceN < NmSpaceV.Len(); NmSpaceN++) {
		const TStrPr& NmSpace = NmSpaceV[NmSpaceN];
		RdfTok->AddArg("xmlns:" + NmSpace.Val1, NmSpace.Val2);
	}
	
	if (SubNodeTripleVH.IsKey(Node->GetNodeId())) { 
		RdfTok->AddSubTok(GetNodeXmlTok(Node)); }
	return TXmlDoc::New(RdfTok);
}

PXmlDoc TRdfGraph::GetRdfXml() const {
	PXmlTok RdfTok = TXmlTok::New("rdf:RDF");

	TStrSet UsedNamespaces;
	for (int i = 0; i < NodeV.Len(); i++){
		const PRdfNode& Node = NodeV[i];
		if (Node->IsUri()) {
			const TStr& Uri = Node->GetUri();
			int Colon = Uri.SearchCh(':');
			if (Colon != -1) {
				const TStr NsPrefix = Uri.GetSubStr(0, Colon - 1);
				UsedNamespaces.AddKey(NsPrefix);
			}
		}
	}

	int NodeId = SubNodeTripleVH.FFirstKeyId();
	while (SubNodeTripleVH.FNextKeyId(NodeId)) {
		const PRdfNode& Node = NodeV[SubNodeTripleVH.GetKey(NodeId)];
		RdfTok->AddSubTok(GetNodeXmlTok(Node));
	}

	// load name spaces
	for (int NmSpaceN = 0; NmSpaceN < NmSpaceV.Len(); NmSpaceN++) {
		const TStrPr& NmSpace = NmSpaceV[NmSpaceN];
		if (UsedNamespaces.IsKey(NmSpace.Val1)) {
			RdfTok->AddArg("xmlns:" + NmSpace.Val1, NmSpace.Val2);
		}
	}

	return TXmlDoc::New(RdfTok);
}

PXmlTok TRdfGraph::GetNodeXmlTok(const PRdfNode& Node) const {
	// make XML node for the node
	PXmlTok NodeTok;
	
	// root concept description
	NodeTok = TXmlTok::New("rdf:Description");
	if (Node->IsUri()) {
		NodeTok->AddArg("rdf:about", Node->GetFullUri(NmSpaceV));		
	} else if (Node->IsBlank()) {
		NodeTok->AddArg("rdf:nodeID", Node->GetBlankNodeId());		
	} else {
		TExcept::Throw("Unknown object type!");
	}
	bool HasType = false;
	// get all triples with the given node as subject		
	const TIntV& TripleV = SubNodeTripleVH.GetDat(Node->GetNodeId());
	// serialize all the properties
	for (int TripleN = 0; TripleN < TripleV.Len(); TripleN++) {
		PRdfTriple Triple = GetTriple(TripleV[TripleN]);
		PXmlTok PropTok;
		if (Triple->GetObject()->IsLiteralStr()) {
			// literal is string
			PropTok = TXmlTok::New(Triple->GetPredicate()->GetShortUri(NmSpaceV), 
				Triple->GetObject()->GetLiteralStr());
		} else if (Triple->GetObject()->IsLiteralInt()) {
			// literal is integer
			PropTok = TXmlTok::New(Triple->GetPredicate()->GetShortUri(NmSpaceV), 
				TInt::GetStr(Triple->GetObject()->GetLiteralInt()));
		} else if (Triple->GetObject()->IsLiteralFlt()) {
			// literal is double
			PropTok = TXmlTok::New(Triple->GetPredicate()->GetShortUri(NmSpaceV), 
				TFlt::GetStr(Triple->GetObject()->GetLiteralFlt()));
		} else if (Triple->GetObject()->IsLiteralTm()) {
			// literal is time
			PropTok = TXmlTok::New(Triple->GetPredicate()->GetShortUri(NmSpaceV), 
				Triple->GetObject()->GetLiteralTm().GetWebLogDateTimeStr());
		} else if (Triple->GetObject()->IsUri()) {
			// object is a resource		
			const TStr& ShortUri = Triple->GetPredicate()->GetShortUri(NmSpaceV);
			if (!HasType && ShortUri == "rdf:type") {
				HasType = true;
				NodeTok->PutStr(Triple->GetObject()->GetShortUri(NmSpaceV));
			} else {
				PropTok = TXmlTok::New(ShortUri);
				PropTok->AddArg("rdf:resource", Triple->GetObject()->GetFullUri(NmSpaceV));					
			}
		} else if (Triple->GetObject()->IsBlank()) {
			// object is a resource					
			PropTok = TXmlTok::New(Triple->GetPredicate()->GetShortUri(NmSpaceV));
			PropTok->AddArg("rdf:nodeID", Triple->GetObject()->GetBlankNodeId());					
		} else {
			TExcept::Throw("Unknown object type!");
		}
		if (!PropTok.Empty()) {
			NodeTok->AddSubTok(PropTok);
		}		
	}			
	// done
	return NodeTok;
}
