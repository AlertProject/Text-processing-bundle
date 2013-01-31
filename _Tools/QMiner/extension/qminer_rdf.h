#ifndef OGRDF_H
#define OGRDF_H

#include <rdf.h>
#include "../qminer.h"
#include "../qminer_srv.h"

///////////////////////////////////////////
// QMiner-RDF-Endpoint-Description
ClassTP(TOgRdfEndpointBs, POgRdfEndpointBs)//{
private:
    // root url string of the enpoints as it's seen from outside (e.g. http://localhost:8080/)
    TStr RootUrlStr;
	// namespaces in RDF (e.g. rdf = http://w3c.org/....)
	TStrPrV NmSpaceV;
    THash<TStr, TUCh> EndpointToStoreH;
    THash<TUCh, TStr> StoreToEndpointH;
    TStrSet RecIdEndpointH;
    TStrSet RecNmEndpointH;	
    THash<TUCh, TStrPrV> StoreToPropertyVH;
    THash<TUCh, TStrIntPrV> StoreToFieldUriVH;
    THash<TUCh, TStrIntPrV> StoreToFieldLiteralVH;
    THash<TUCh, TStrIntPrV> StoreToJoinVH;

    UndefDefaultCopyAssign(TOgRdfEndpointBs);
public:
    TOgRdfEndpointBs(const POgBase& OgBase, const TStr& _RootUrlStr, const TStr& XmlFNm);
	static POgRdfEndpointBs New(const POgBase& OgBase, const TStr& RootUrlStr, const TStr& XmlFNm) {
		return new TOgRdfEndpointBs(OgBase, RootUrlStr, XmlFNm); }

	// namespace
	const TStrPrV& GetNmSpaceV() const { return NmSpaceV; }
    // endpoint
    bool IsEndpoint(const TStr& EndpointNm) const;
    uchar GetEndpointStoreId(const TStr& EndpointNm) const;
    // store
    bool IsStore(const uchar& StoreId) const;
    const TStr& GetStoreEndpointNm(const uchar& StoreId) const;
    // record handle type
    bool IsRecIdEndpoint(const TStr& EndpointNm) const;
    bool IsRecNmEndpoint(const TStr& EndpointNm) const;
    // constant properties
    bool IsStoreProperty(const uchar& StoreId) const;
    const TStrPrV& GetStoreProperty(const uchar& StoreId) const;
    // field uri properties
    bool IsStoreFieldUri(const uchar& StoreId) const;
    const TStrIntPrV& GetStoreFieldUri(const uchar& StoreId) const;
    // field literal properties
    bool IsStoreFieldLiteral(const uchar& StoreId) const;
    const TStrIntPrV& GetStoreFieldLiteral(const uchar& StoreId) const;
    // join properties
    bool IsStoreJoin(const uchar& StoreId) const;
    const TStrIntPrV& GetStoreJoin(const uchar& StoreId) const;

    // creates an URI for a record
    TStr GetRecUri(const POgBase& OgBase, const POgStore& Store, const uint64& RecId);
    // construct a RDF graph around a record
    PRdfGraph MakeRdfGraph(const POgBase& OgBase, const POgStore& Store, const uint64& RecId);
};

///////////////////////////////////////////
// QMiner-Server-Function-RDF-Endpoint
class TOgRdfEndpointSf: public TOgSrvFun {
private:
    // description of how to generate RDF out of stores and records
	TOgRdfEndpointBs RdfEndpointBs;

public:
	TOgRdfEndpointSf(const POgBase& OgBase, const TStr& RootUrlStr, const TStr& RdfXmlFNm);
	static PSAppSrvFun New(const POgBase& OgBase, const TStr& RootUrlStr, const TStr& RdfXmlFNm) { 
        return new TOgRdfEndpointSf(OgBase, RootUrlStr, RdfXmlFNm); }

	PXmlDoc ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

#endif
