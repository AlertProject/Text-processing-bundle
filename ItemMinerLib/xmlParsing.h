#ifndef XMLPARSING_H
#define XMLPARSING_H

#include <base.h>

TIntV GetEmailIds(const PXmlTok& QueryXml, const TStr& TagPath);
void GetKeywords(const PXmlTok& QueryXml, const TStr& TagPath, TStrV& KeywordsV, TStrV& IgnoreKeywordsV);

TInt GetIntArg(const PXmlTok& QueryXml, TStr TagPath, const TStr& ArgNm, int DfVal);
TStr GetStrArg(const PXmlTok& QueryXml, TStr TagPath, const TStr& ArgNm, const TStr& DfVal);

TInt GetIntArg(const PXmlTok& QueryXml, const TStr& ArgNm, int DfVal);
TStr GetStrArg(const PXmlTok& QueryXml, const TStr& ArgNm, const TStr& DfVal);
bool GetBoolArg(const PXmlTok& QueryXml, const TStr& ArgNm, const bool DfVal);
#endif