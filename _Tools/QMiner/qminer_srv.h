#ifndef QMINERSRV_H
#define QMINERSRV_H

#include <qminer.h>
#include <net.h>

///////////////////////////////////////////
// QMiner-Server-Function
class TOgSrvFun : public TSAppSrvFun {
protected:
	POgBase OgBase;
protected:
	TOgSrvFun(const POgBase& _OgBase, const TStr& FunNm, const TSAppOutType& OutType): 
		 TSAppSrvFun(FunNm, OutType), OgBase(_OgBase) { }

	const POgBase& GetOgBase() const { return OgBase; }

	// helper functions for parsing input parameters
	const POgStore& GetStore(const TStrKdV& FldNmValPrV) const;
	TOgRec GetRec(const TStrKdV& FldNmValPrV, const POgStore& Store) const;

public:
	static void RegDefFunXml(const POgBase& OgBase, TSAppSrvFunV& SrvFunV);
	static void RegDefFunJson(const POgBase& OgBase, TSAppSrvFunV& SrvFunV);
};

///////////////////////////////////////////
// QMiner-Server-Function-Stores
//  lists all stores in the base and their definiton
class TOgSfStores: public TOgSrvFun {
private:
	TOgSfStores(const POgBase& OgBase, const TSAppOutType& OutType):
	   TOgSrvFun(OgBase, "stores", OutType) { }

public:
	static PSAppSrvFun NewXml(const POgBase& OgBase) { return new TOgSfStores(OgBase, saotXml); }
	static PSAppSrvFun NewJson(const POgBase& OgBase) { return new TOgSfStores(OgBase, saotJSon); }
	
private:
	// helper functions for returning XML definitions
	PXmlTok GetStoreFieldsXml(const POgStore& Store);
	PXmlTok GetStoreKeysXml(const POgStore& Store);
	PXmlTok GetStoreJoinsXml(const POgStore& Store);
	PXmlTok GetStoreXml(const POgStore& Store);
public:
	PXmlDoc ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);

private:
	// helper functions for returning JSon definitions
	PJsonVal GetStoreFieldsJson(const POgStore& Store);
	PJsonVal GetStoreKeysJson(const POgStore& Store);
	PJsonVal GetStoreJoinsJson(const POgStore& Store);
	PJsonVal GetStoreJson(const POgStore& Store);
public:
	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////////////////
// QMiner-Server-Function-WordVoc
//  lists all complete vocabulray for given key
class TOgSfWordVoc: public TOgSrvFun {
private:
	void GetWordVoc(const TStrKdV& FldNmValPrV, TStrIntPrV& WordStrFqV); 

	TOgSfWordVoc(const POgBase& OgBase, const TSAppOutType& OutType): 
		TOgSrvFun(OgBase, "word-voc", OutType) { }
public:
	static PSAppSrvFun NewXml(const POgBase& OgBase) { return new TOgSfWordVoc(OgBase, saotXml); }
	static PSAppSrvFun NewJson(const POgBase& OgBase) { return new TOgSfWordVoc(OgBase, saotJSon); }

	PXmlDoc ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////////////////
// QMiner-Server-Function-Record
//  lists all the fields and values from a record
class TOgSfStoreRec: public TOgSrvFun {
private:
	TOgSfStoreRec(const POgBase& OgBase, const TSAppOutType& OutType): 
		TOgSrvFun(OgBase, "record", OutType) { }
public:
	static PSAppSrvFun NewXml(const POgBase& OgBase) { return new TOgSfStoreRec(OgBase, saotXml); }
	static PSAppSrvFun NewJson(const POgBase& OgBase) { return new TOgSfStoreRec(OgBase, saotJSon); }

	PXmlDoc ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////////////////
// QMiner-Server-Function-Operator
//  executes an operator
class TOgSfOp: public TOgSrvFun {
private:
	POgOp Op;

	TOgSfOp(const POgBase& OgBase, const POgOp& _Op, const TSAppOutType& OutType): 
		TOgSrvFun(OgBase, "op-" + _Op->GetOpNm(), OutType), Op(_Op) { }
	static PSAppSrvFun New(const POgBase& OgBase, const POgOp& Op, 
		const TSAppOutType& OutType) { return new TOgSfOp(OgBase, Op, OutType); }
public:
	PXmlDoc ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
	TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);

	friend class TOgSrvFun;
};

#endif
