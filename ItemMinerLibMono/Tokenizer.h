#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <base.h>
#include <qminer.h>

// OntoGen-Unicode-Tokenizer
class TContextifyTokenizer : public TTokenizer {
private:
	PSwSet SwSet;
	PStemmer Stemmer;
public:
	TContextifyTokenizer()
	{ 
		SwSet = TSwSet::New();
		Stemmer = TStemmer::New();
		EAssertR(TUnicodeDef::IsDef(), "Unicode not initilaized!"); 
	}

	TContextifyTokenizer(PSwSet _SwSet, PStemmer& _Stemmer):  SwSet(_SwSet), Stemmer(_Stemmer)
	{ 
		EAssertR(TUnicodeDef::IsDef(), "Unicode not initilaized!"); 
	}
	
	static PTokenizer New(PSwSet SwSet = TSwSet::New(), PStemmer Stemmer = TStemmer::New()) 
	{         
		return new TContextifyTokenizer(SwSet, Stemmer); 
	}
	
	static PTokenizer Load(TSIn& SIn) { return new TContextifyTokenizer(SIn); }

	TContextifyTokenizer(TSIn& SIn): SwSet(SIn), Stemmer(SIn)
	{ 
        EAssertR(TUnicodeDef::IsDef(), "Unicode not initilaized!"); 
	}
	
	~TContextifyTokenizer() { }

	void Save(TSOut& SOut) const 
	{ 
		SwSet.Save(SOut);
		Stemmer.Save(SOut);
	}

	PStemmer GetStemmmer() { return Stemmer; }
	PSwSet GetSwSet() { return SwSet; }

	void GetTokens(const PSIn& SIn, TStrV& TokenV) const
	{
		TStr LineStr;
		while (SIn->GetNextLn(LineStr)) {
			TUStr UText(LineStr);
			TStr SimpleText = UText.GetStarterLowerCaseStr();

			// create html lexical
			PSIn HtmlSIn=TStrIn::New(SimpleText);
			THtmlLx HtmlLx(HtmlSIn);

			while (HtmlLx.Sym!=hsyEof) {
				if (HtmlLx.Sym==hsyStr) {
					const TChA& UcChA = HtmlLx.UcChA;
					// delete all non-alpha chars
					TChA WordChA;
					for (int ChN = 0; ChN < UcChA.Len(); ChN++) {
						if (TCh::IsAlpha(UcChA[ChN])) { WordChA += UcChA[ChN]; }
						else if (TCh::IsPunct(UcChA[ChN])) { WordChA += UcChA[ChN]; }
					}
					TStr WordStr = WordChA;
					// check if sw
					if ((SwSet.Empty()) || (!SwSet->IsIn(WordStr))) {
						if (!Stemmer.Empty())
							WordStr=Stemmer->GetStem(WordStr);
						TokenV.Add(WordStr.GetLc());
					}
				}
				// get next symbol
				HtmlLx.GetSym();
			}
		}
	}

	//	// traverse html string symbols
	//	while (HtmlLx.Sym!=hsyEof){
	//		if (HtmlLx.Sym==hsyStr){
	//			const TChA& UcChA = HtmlLx.UcChA;
	//			// delete all non-alpha chars
	//			TStr WordStr = UcChA;		// GREGOR: we don't remove the non-alpha characters. it went from gregor.leban@ijs.si to gregorlebanijssi
	//			// check if sw
	//			if ((SwSet.Empty()) || (!SwSet->IsIn(WordStr))) {
	//				if (!Stemmer.Empty()) { 
	//					WordStr=Stemmer->GetStem(WordStr); }
	//				TokenV.Add(WordStr.GetLc());
	//			}
	//		}
	//		// get next symbol
	//		HtmlLx.GetSym();
	//	}
	//}

	/*
	/// we use this implementation if we want to split on every dot, space, ... (any non alpha char)
	void GetTokens(const TStr& Text, TStrV& TokenV) const
	{
			// create html lexical
		PSIn HtmlSIn=TStrIn::New(Text);
		THtmlLx HtmlLx(HtmlSIn);

		// traverse html string symbols
		while (HtmlLx.Sym!=hsyEof)
		{
			if (HtmlLx.Sym==hsyStr)
			{
				const TChA& UcChA = HtmlLx.UcChA;
				TChA WordChA;
				for (int ChN = 0; ChN < UcChA.Len(); ChN++) 
				{
					if (TCh::IsAlpha(UcChA[ChN])) { WordChA += UcChA[ChN]; }
					else
					{
						AddTokenIfValid(WordChA, TokenV);
						WordChA.Clr();
					}
				}
				AddTokenIfValid(WordChA, TokenV);
			}
			// get next symbol
			HtmlLx.GetSym();
		}
	}

	void AddTokenIfValid(TStr Token, TStrV& TokenV) const
	{
		if (Token.Len() > 0 && (SwSet.Empty() || !SwSet->IsIn(Token)))
		{
			if (!Stemmer.Empty()) { 
				Token=Stemmer->GetStem(Token); }
			TokenV.Add(Token.GetLc());
		}
	}
	*/
};

#endif