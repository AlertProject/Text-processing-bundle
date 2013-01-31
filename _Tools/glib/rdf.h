#ifndef RDF_H
#define RDF_H

#include "base.h"

/////////////////////////////////////////////////
// forward
ClassHdTP(TRdfGraph, PRdfGraph);

/////////////////////////////////////////////////
// RDF-URIs
class TRdfUri {
public:
	static TStrPr RdfNmSpace;
	static TStr RdfResource;
	static TStr RdfAbout;

	// clean string and make it URI-friendy
	// e.g. Blaž Fortuna => Blaz_Fortuna
	static TStr GetUriStr(const TStr& Str);
};

/////////////////////////////////////////////////
// RDF-Node
ClassTP(TRdfNode, PRdfNode)//{
private:
	typedef enum { rntUri, rntBlank, rntLiteralStr, rntLiteralInt, rntLiteralFlt, rntLiteralTm } TRdfNodeType;
private:
	// for all nodes
	TRdfNodeType NodeType;
	TInt NodeId;

	// URI or a blank node
	TStr UriStr;
	TRdfNode(const int &_NodeId, const TStr& _UriStr): NodeType(rntUri), NodeId(_NodeId), UriStr(_UriStr) { }
	TRdfNode(const TRdfNodeType& _NodeType, const int &_NodeId, 
		const TStr& _UriStr): NodeType(_NodeType), NodeId(_NodeId), UriStr(_UriStr) { }

	// Literal String
	TStr LiteralStr;
	TStr LangStr;
	TRdfNode(const int& _NodeId, const TStr& _LiteralStr, const TStr& _LangStr):
		NodeType(rntLiteralStr), NodeId(_NodeId), LiteralStr(_LiteralStr), LangStr(_LangStr) { }
	
	// Literal Integer
	TInt LiteralInt;
	TRdfNode(const int& _NodeId, const int& _LiteralInt):
		NodeType(rntLiteralInt), NodeId(_NodeId), LiteralInt(_LiteralInt) { }

	// Literal Floating point
	TFlt LiteralFlt;
	TRdfNode(const int& _NodeId, const double& _LiteralFlt):
		NodeType(rntLiteralFlt), NodeId(_NodeId), LiteralFlt(_LiteralFlt) { }

	// Literal Time and Date
	TTm LiteralTm;
	TRdfNode(const int& _NodeId, const TTm& _LiteralTm):
		NodeType(rntLiteralTm), NodeId(_NodeId), LiteralTm(_LiteralTm) { }

public:
	TRdfNode() { }

	// for all
	TStr GetNodeType() const;
	int GetNodeId() const { return NodeId; }

	// URI
	bool IsUri() const { return (NodeType == rntUri); }
	const TStr& GetUri() const { EAssert(IsUri()); return UriStr; }
	TStr GetFullUri(const TStrPrV& NmSpaceV) const;
	TStr GetShortUri(const TStrPrV& NmSpaceV) const;
	// Blank Node
	bool IsBlank() const { return (NodeType == rntBlank); }
	const TStr& GetBlankNodeId() const { return UriStr; }
	// Literal
	bool IsLiteral() const { return (NodeType > rntBlank); }
	// Literal String
	bool IsLiteralStr() const { return (NodeType == rntLiteralStr); }
	const TStr& GetLiteralStr() const { EAssert(IsLiteralStr()); return LiteralStr; }
	const TStr& GetLangStr() const { EAssert(IsLiteralStr()); return LangStr; }
	// Literal Integer
	bool IsLiteralInt() const { return (NodeType == rntLiteralInt); }
	int GetLiteralInt() const { EAssert(IsLiteralInt()); return LiteralInt; }
	// Literal Floating point
	bool IsLiteralFlt() const { return (NodeType == rntLiteralFlt); }
	double GetLiteralFlt() const { EAssert(IsLiteralFlt()); return LiteralFlt; }
	// Literal Time and Date
	bool IsLiteralTm() const { return (NodeType == rntLiteralTm); }
	const TTm& GetLiteralTm() const { EAssert(IsLiteralTm()); return LiteralTm; }

	friend class TRdfGraph;
};
typedef TVec<PRdfNode> TRdfNodeV;

/////////////////////////////////////////////////
// RDF-Triple
ClassTP(TRdfTriple, PRdfTriple)//{
private:
	TInt TripleId;
	PRdfNode Subject;
	PRdfNode Predicate;
	PRdfNode Object;

	TRdfTriple(const int& _TripleId, const PRdfNode& _Subject, const PRdfNode& _Predicate, const PRdfNode& _Object);
	TRdfTriple() { }
public:
	const PRdfNode& GetSubject() const { return Subject; }
	const PRdfNode& GetPredicate() const { return Predicate; }
	const PRdfNode& GetObject() const { return Object; }

	friend class TRdfGraph;
};
typedef TVec<PRdfTriple> TRdfTripleV;

/////////////////////////////////////////////////
// RDF-Graph
ClassTP(TRdfGraph, PRdfGraph)//{
private:
	TStrPrV NmSpaceV;
	// vector of URIs
	TRdfNodeV NodeV;
	// vector of triples
	TIntTrV TripleV;
	// index of URIs -> URI nodes
	TStrHash<TInt> UriNodeH;
	// index of subject Nodes -> triples
	THash<TInt, TIntV> SubNodeTripleVH;
	// index of predicate Nodes -> triples
	THash<TInt, TIntV> PredNodeTripleVH;
	// index of object Nodes -> triples
	THash<TInt, TIntV> ObjNodeTripleVH;

public:
	TRdfGraph(const TStrPrV& _NmSpaceV): NmSpaceV(_NmSpaceV) { NmSpaceV.AddUnique(TRdfUri::RdfNmSpace); }
    static PRdfGraph New(const TStrPrV& NmSpaceV) { return new TRdfGraph(NmSpaceV); }

	// RDF nodes
	bool IsNodeId(const int& NodeId) const { return (0 <= NodeId) && (NodeId < NodeV.Len()); }
	PRdfNode GetNode(const int& NodeId) const { Assert(IsNodeId(NodeId)); return NodeV[NodeId]; }
	// creation of RDF Nodes
	PRdfNode AddUri(const TStr& UriStr);
	PRdfNode AddBlank();
	PRdfNode AddBlank(const TStr& NodeIdStr);
	PRdfNode AddLiteralStr(const TStr& LiteralStr, const TStr& LangStr = TStr());
	PRdfNode AddLiteralInt(const int& LiteralInt);
	PRdfNode AddLiteralFlt(const double& LiterlFlt);
	PRdfNode AddLiteralDate(const TTm& LiterlTm);

	// triples
	bool IsTripleId(const int& TripleId) const { return (0 <= TripleId) && (TripleId < TripleV.Len()); }
	PRdfTriple GetTriple(const int& TripleId) const;
	// creation of Triples
	PRdfTriple AddTriple(const PRdfNode& Subject, const PRdfNode& Predicate, const PRdfNode& Object);

    // serialization
    PXmlDoc GetRdfXml(const PRdfNode& Node) const;
	PXmlDoc GetRdfXml() const;
private:
	PXmlTok GetNodeXmlTok(const PRdfNode& Node) const;
};

#endif
