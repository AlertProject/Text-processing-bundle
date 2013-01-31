#include "htmlclean.h"

double THtmlClean::GetTagScore(const TStr& tag) const {
	if (TagProb.IsKey(tag)) return -3*(1.0-TagProb.GetDat(tag));
	else return -3;
}

TStr THtmlClean::PatchScripts(TStr html) const {
	TStr htmlOrig=html;
	html.ChangeStrAll("<SCRIPT","<script");
	html.ChangeStrAll("</SCRIPT>","</script>");
	int from=0,to;
	while (1) {
		from=html.SearchStr("<script",from);
		if (from==-1) break;
		char quote='#';
		int i=from, escape=0;
		while (1) {
			to=html.SearchStr("</script>",i+1);
			if (to==-1) return htmlOrig;
			for (;i<to;i++) {
				if (escape) { escape=0; continue; }
				if (html[i]=='\\') escape=1;
				if (html[i]=='\'' || html[i]=='"') {
					if (html[i]==quote) quote='#';
					else if (quote=='#') quote=html[i];
				}
				if (html[i]=='\n') quote='#';
			}
			if (quote=='#') break;
		}
		for (int i=from+1;i<to;i++) if (html[i]=='<') html.PutCh(i,'?');
		from=to;
	}
	return html;
}

void THtmlClean::GetTokens(const TStr& HtmlStr, THtmlCleanTokScV& TokScV) const {
	THtmlLx Lx(PSIn(new TStrIn(PatchScripts(HtmlStr))));
	bool inScript=false, inStyle=false;
	while (Lx.GetSym()!=hsyEof) {
		switch (Lx.Sym) {
			case hsyStr:
				if (!inScript && !inStyle) TokScV.Add(THtmlCleanTokSc(Lx,+1.0));
				break;
			case hsyNum:
				if (!inScript && !inStyle) TokScV.Add(THtmlCleanTokSc(Lx,+1.0));
				break;
			case hsyBTag:
				if (!inScript && !inStyle) TokScV.Add(THtmlCleanTokSc(Lx,GetTagScore(Lx.ChA)));
				if (Lx.ChA=="<SCRIPT>") { inScript=true; }
				if (Lx.ChA=="<STYLE>") { inStyle=true; }
				break;
			case hsyETag:
				if (Lx.ChA=="<SCRIPT>") { inScript=false; }
				if (Lx.ChA=="<STYLE>") { inStyle=false; }
				if (!inScript && !inStyle) TokScV.Add(THtmlCleanTokSc(Lx,GetTagScore(Lx.ChA)));
				break;
			default: break;
		}
	}
}

TStr THtmlClean::Extract(const TStr& HtmlStr, int &ArtS, int &ArtE) const {
	THtmlCleanTokScV tok; GetTokens(HtmlStr, tok);
	// Maximum Subsequence Sum with Single Gap
	int gaps=1;
	tok.Ins(0,THtmlCleanTokSc());
	ArtS=0; ArtE=0;
	TVec<TPair<THtmlCleanTokSc, int> > opened;
	double best=1e-10;
	int gapB=-1, gapE=-1;
	for (int t=0;t<=gaps;t++) {
		tok[0].f[t]=0; tok[0].first[t]=0; 
		tok[0].gapB=-1; tok[0].gapE=-1;
	}
	for (int i=1;i<tok.Len();i++) {
		if (tok[i].Sym==hsyBTag) opened.Add(TPair<THtmlCleanTokSc,int>(tok[i],i));
		for (int t=0;t<=gaps;t++) {
			tok[i].f[t]=tok[i].Score; tok[i].first[t]=i; 
			tok[i].gapB=-1; tok[i].gapE=-1;
			if (tok[i-1].f[t]>0) { 
				tok[i].f[t]+=tok[i-1].f[t]; tok[i].first[t]=tok[i-1].first[t]; 
				tok[i].gapB=tok[i-1].gapB; tok[i].gapE=tok[i-1].gapE;
			}
		}
		if (tok[i].Sym==hsyETag) {
			while (opened.Len()>1 && opened.Last().Val1.Str!=tok[i].Str) opened.DelLast();
			if (opened.Len()>1) {
				int id=opened.Last().Val2-1;
				for (int t=1;t<=gaps;t++) {
					if (tok[i].f[t]<tok[id].f[t-1]) { 
						tok[i].f[t]=tok[id].f[t-1]; tok[i].first[t]=tok[id].first[t-1]; 
						tok[i].gapB=tok[id+1].SymBChX; tok[i].gapE=tok[i].SymEChX;
					}
				}
				opened.DelLast();
			}
		}
		if (tok[i].f[gaps]>best) {
			best=tok[i].f[gaps]; ArtS=tok[tok[i].first[gaps]].SymBChX; ArtE=tok[i].SymEChX;
			gapB=tok[i].gapB; gapE=tok[i].gapE;
		}
	}
	tok.Del(0);

	if (gapB==-1) return HtmlStr.GetSubStr(ArtS,ArtE);
	else return HtmlStr.GetSubStr(ArtS,gapB-1) + HtmlStr.GetSubStr(gapE+1,ArtE);
}
