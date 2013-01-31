#include "xmlParsing.h"
#include "base.h"

TIntV GetEmailIds(const PXmlTok& QueryXml, const TStr& TagPath)
{
	TIntV EmailIdV;
	TXmlTokV Ids;
	QueryXml->GetTagTokV(TagPath, Ids);
	for (int EmlInd = 0; EmlInd < Ids.Len(); EmlInd++) 
	{
		TInt EmailIdInt = Ids[EmlInd]->GetIntArgVal("id", -1);
		if (EmailIdInt != -1)
			EmailIdV.AddUnique(EmailIdInt);
	}
	return EmailIdV;
}

void GetKeywords(const PXmlTok& QueryXml, const TStr& TagPath, TStrV& KeywordsV, TStrV& IgnoreKeywordsV)
{
	TXmlTokV KwsXmlV;
	QueryXml->GetTagTokV(TagPath, KwsXmlV);
	for (int KwInd = 0; KwInd < KwsXmlV.Len(); KwInd++) 
	{
		TStr Kw = KwsXmlV[KwInd]->GetTokStr(false);
		int hide = KwsXmlV[KwInd]->GetIntArgVal("hide", 0);
		if (hide)
			IgnoreKeywordsV.Add(Kw);
		else
			KeywordsV.Add(Kw);
	}
}

TInt GetIntArg(const PXmlTok& QueryXml, const TStr& TagPath, const TStr& ArgNm, int DfVal)
{
	PXmlTok XmlTok = QueryXml->GetTagTok(TagPath);
	if (XmlTok.Empty()) return DfVal;
	return XmlTok->GetIntArgVal(ArgNm, DfVal);
}

TStr GetStrArg(const PXmlTok& QueryXml, const TStr& TagPath, const TStr& ArgNm, const TStr& DfVal)
{
	PXmlTok XmlTok = QueryXml->GetTagTok(TagPath);
	if (XmlTok.Empty()) return DfVal;
	return XmlTok->GetArgVal(ArgNm, DfVal);
}

TInt GetIntArg(const PXmlTok& QueryXml, const TStr& ArgNm, int DfVal)
{
	if (QueryXml.Empty()) return DfVal;
	return QueryXml->GetIntArgVal(ArgNm, DfVal);
}

TStr GetStrArg(const PXmlTok& QueryXml, const TStr& ArgNm, const TStr& DfVal)
{
	if (QueryXml.Empty()) return DfVal;
	return QueryXml->GetArgVal(ArgNm, DfVal);
}

bool GetBoolArg(const PXmlTok& QueryXml, const TStr& ArgNm, const bool DfVal)
{
	if (QueryXml.Empty()) return DfVal;
	TStr val = QueryXml->GetArgVal(ArgNm, "");
	if (val == "") return DfVal;
	val = val.GetLc();
	if (val == "true" || val == "1")
		return true;
	return false;
}