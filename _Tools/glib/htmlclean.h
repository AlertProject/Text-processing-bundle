#ifndef HTMLCLEAN_H
#define HTMLCLEAN_H

#include "base.h"

/////////////////////////////////////////////////
// basic HTML token with score/penalty
ClassTV(THtmlCleanTokSc, THtmlCleanTokScV)//{
public:
	int SymBChX, SymEChX;
	THtmlLxSym Sym;
	TStr Str;
	double Score;

	int first[2];
	double f[2];
	int gapB,gapE;

	THtmlCleanTokSc() { Sym=hsyUndef; Str=""; Score=0; }
	THtmlCleanTokSc(THtmlLx &Lx, double sc) {
		SymBChX=Lx.SymBChX-1; SymEChX=Lx.SymEChX-1;
		Sym=Lx.Sym; Str=TStr(Lx.ChA); 
		Score=sc;
	}
};

/////////////////////////////////////////////////
// Article Extraction
//TODO: TArtExtract -> THtmlClean
class THtmlClean {
private:
	TStrFltH TagProb;

	double GetTagScore(const TStr& tag) const; // extract token score from model
	TStr PatchScripts(TStr html) const; // workaround for handling <script> tags by THtmlLx
public:
    // no probabilities on tokes
	THtmlClean() { }
	// loads model with token probabilities
    THtmlClean(const TStr& TagProbFNm) { Load(TagProbFNm); } 

	// load probabilities
	void Load(const TStr& TagProbFNm) { TFIn FIn(TagProbFNm); TagProb.Load(FIn); }

    // tokenize html (public only for evaluation purposes)
    void GetTokens(const TStr& HtmlStr, THtmlCleanTokScV& TokScV) const;	
    // returns article text with html markup (ArtS, ArtE - starting, ending index of article)
	TStr Extract(const TStr& HtmlStr, int &ArtS, int &ArtE) const;
    // forget the beginning and the end of text
    TStr Extract(const TStr& HtmlStr) const { 
        int ArtS, ArtE; return Extract(HtmlStr, ArtS, ArtE); }
};

#endif
