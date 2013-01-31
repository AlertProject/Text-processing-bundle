/////////////////////////////////////////////////
// Translation-Evaluation-Scores
void TEvalScore::Tokenize(const TStr& Str, TStrV& TokenV) {
    TStr LcStr = GetLc(Str);
    TokenV.Clr(); TChA WordChA; 
    const int Chs = LcStr.Len();
    for (int ChN = 0; ChN < Chs; ChN++) {
        const char Ch = LcStr[ChN];
        if (IsChar(Ch)) { 
            WordChA += Ch; 
        } else if (!WordChA.Empty()) {
            TokenV.Add(WordChA);
            WordChA.Clr();
        }
    }        
}

void TEvalScore::Parse(const TStr& Str, TIntV& WIdV) {
    TStrV TokenV; Tokenize(Str, TokenV); WIdV.Clr();
    for (int WdN = 0; WdN < TokenV.Len(); WdN++) {
        // get the word string
        TStr WdStr = TokenV[WdN];
        // get id of the word
        int WId = WordH.GetKeyId(WdStr);
        // word does not exist yet, add it to the hash table
        if (WId == -1) { WId = WordH.AddKey(WdStr); }
        // add word to the parsed sentence
        WIdV.Add(WId);
    }
}

void TEvalScore::GetNGramH(const TIntV& WIdV, 
        const int& MxNGramLen, TIntH& NGramIdH) {

    const int Wds = WIdV.Len();
    for (int NGramLen = 1; NGramLen <= MxNGramLen; NGramLen++) {
        const int MxNGramStart = Wds - NGramLen + 1;
        for (int NGramStart = 0; NGramStart < MxNGramStart; NGramStart++) {
            // get the ngram
            TIntV NGram; WIdV.GetSubValV(NGramStart, NGramStart+NGramLen-1, NGram);
            IAssert(NGram.Len() == NGramLen);
            // get id of the ngram
            int NGramId = NGramH.GetKeyId(NGram);
            // word does not exist yet, add it to the hash table
            if (NGramId == -1) { NGramId = NGramH.AddKey(NGram); }
            // add ngram to the sentence ngram bag
            NGramIdH.AddDat(NGramId)++;
        }
    }
}

double TEvalScore::EvalSig(const PTransCorpus& FirstTransCorpus, 
        const PTransCorpus& SecondTransCorpus, const int& Shuffles, const int& Seed) {

    // compute defaultscore
    const double DefDiff = TFlt::Abs(Eval(FirstTransCorpus) - Eval(SecondTransCorpus));
    printf("DefDiff: %.5f\n", DefDiff);
    // do shuffels and check the difference
    int RndBetter = 0; TRnd Rnd(Seed);
    for (int ShuffleN = 0; ShuffleN < Shuffles; ShuffleN++) {
        // shuffle
        PTransCorpus OutFirstTransCorpus, OutSecondTransCorpus;
        TTransCorpus::Shuffle(FirstTransCorpus, SecondTransCorpus, 
            Rnd, 0.5, OutFirstTransCorpus, OutSecondTransCorpus);
        // compare scores
        const double FirstTransScore = Eval(OutFirstTransCorpus);
        const double SecondTransScore = Eval(OutSecondTransCorpus);
        const double Diff = TFlt::Abs(FirstTransScore - SecondTransScore);
        printf("EvalSig: %.5f - %.5f - %.5f\n", Diff, FirstTransScore, SecondTransScore);
        // count if neccessary
		if (Diff > DefDiff) { RndBetter++; printf("+"); } else { printf("-"); }
	} printf("\n");
    // compute final significancy
    return double(RndBetter + 1) / double(Shuffles + 1);
}

/////////////////////////////////////////////////
// BLEU-score
double TEvalScoreBleu::Eval(const PTransCorpus& TransCorpus, const TIntV& _SentIdV) {
    // check if the corpus has translations
    IAssert(TransCorpus->IsTrans());

    // ngram counts (cliped and full)
    TIntH ClipCountNGramH, CountNGramH;
    // candidate and effective reference length
    int FullTransLen = 0, FullRefLen = 0;

    // iterate over sentences
    TIntV SentIdV = _SentIdV;
    if (SentIdV.Empty()) { TransCorpus->GetSentIdV(SentIdV); }
    const int Sents = SentIdV.Len();
    for (int SentIdN = 0; SentIdN < Sents; SentIdN++) {
        const int SentId = SentIdV[SentIdN];
        // tokenize translation
        TIntV TransWIdV; Parse(TransCorpus->GetTransStr(SentId), TransWIdV);
        TIntH TransNGramH; GetNGramH(TransWIdV, MxNGramLen, TransNGramH);
        TIntH FreeTransNGramH = TransNGramH; // number of non-matched ngrams
        // counters for getting the closest length of reference sentences
        const int TransLen = TransWIdV.Len(); 
        int BestLen = 0, BestLenDiff = TInt::Mx;
        // go over reference translations and count ngram matches
        TStrV RefTransStrV = TransCorpus->GetRefTransStrV(SentId);
        for (int RefN = 0; RefN < RefTransStrV.Len(); RefN++) {
            // parse reference translation sentence
            TIntV RefWIdV; Parse(RefTransStrV[RefN], RefWIdV);
            TIntH RefNGramH; GetNGramH(RefWIdV, MxNGramLen, RefNGramH);
            // check for matches
            int TransNGramKeyId = TransNGramH.FFirstKeyId();
            while(TransNGramH.FNextKeyId(TransNGramKeyId)) {
                const int NGramId = TransNGramH.GetKey(TransNGramKeyId);
                const int FreeTransNGrams = FreeTransNGramH(NGramId);
                if (RefNGramH.IsKey(NGramId) && (FreeTransNGrams>0)) {
                    // ngram match and still some free ngrams left to clip
                    const int RefNGrams = RefNGramH(NGramId);
                    FreeTransNGramH(NGramId) = TInt::GetMx(0, FreeTransNGrams - RefNGrams);
                }
            }
            // check the length difference
            const int RefLen = RefWIdV.Len();
            const int LenDiff = TInt::Abs(TransLen - RefLen);
            if (LenDiff < BestLenDiff) { 
                BestLen = RefLen; BestLenDiff = LenDiff; 
            }
        }
        // count ngrams
        int TransNGramKeyId = TransNGramH.FFirstKeyId();
        while(TransNGramH.FNextKeyId(TransNGramKeyId)) {
            // get ngram
            const int NGramId = TransNGramH.GetKey(TransNGramKeyId);
            IAssert(NGramId != -1);
            // check if two hash tables are aligned (should be...)
            const int FreeNGramId = FreeTransNGramH.GetKey(TransNGramKeyId);
            IAssert(NGramId == FreeNGramId);
            // get ngram count and clip-count
            const int Count = TransNGramH[TransNGramKeyId];
            const int ClipCount = Count - FreeTransNGramH[TransNGramKeyId];
            // add ngram to the coprus ngram counts
            CountNGramH.AddDat(NGramId) += Count;
            ClipCountNGramH.AddDat(NGramId) += ClipCount;
        }
        // count length
        FullTransLen += TransLen;
        FullRefLen += BestLen;
    }

    // calcualte ngram precisions
    TIntV ClipCountV(MxNGramLen); ClipCountV.PutAll(0);
    int ClipCountKeyId = ClipCountNGramH.FFirstKeyId();
    while (ClipCountNGramH.FNextKeyId(ClipCountKeyId)) {
        const int NGramId = ClipCountNGramH.GetKey(ClipCountKeyId);
        const int NGramLen = GetNGramLen(NGramId);
        IAssert(0 < NGramLen && NGramLen <= MxNGramLen);
        ClipCountV[NGramLen-1] += ClipCountNGramH[ClipCountKeyId];
    }
    TIntV CountV(MxNGramLen); CountV.PutAll(0);
    int CountKeyId = CountNGramH.FFirstKeyId();
    while (CountNGramH.FNextKeyId(CountKeyId)) {
        const int NGramId = CountNGramH.GetKey(CountKeyId);
        const int NGramLen = GetNGramLen(NGramId);
        IAssert(0 < NGramLen && NGramLen <= MxNGramLen);
        CountV[NGramLen-1] += CountNGramH[CountKeyId];
    }
    TFltV PrecV(MxNGramLen, 0);
    for (int NGramLen = 0; NGramLen < MxNGramLen; NGramLen++) {
        const int ClipCount = ClipCountV[NGramLen];
        const int Count = CountV[NGramLen];
        const double Prec = (Count > 0) ? double(ClipCount)/double(Count) : 0.0;
        PrecV.Add(Prec);
        //printf("%d-gram Match:%d Total:%d Prec:%.5f\n", NGramLen+1, ClipCount, Count, Prec);
    }

    // calcualte brevity penalty
    double LogBP = TFlt::GetMn(0.0, 1.0 - double(FullRefLen)/double(FullTransLen));
    double BP = exp(LogBP);

    // calculate full BLEU score
    double BleuScore = BP; 
    const double Wgt = 1.0 / double(MxNGramLen);
    for (int NGramLen = 0; NGramLen < MxNGramLen; NGramLen++) {
        BleuScore *= pow(PrecV[NGramLen], Wgt);
    }    
    printf("BLEU Score: %.5f\n", BleuScore);
    
    // done!
    return BleuScore;
}

/////////////////////////////////////////////////
// NIST-score
double TEvalScoreNist::Eval(const PTransCorpus& TransCorpus, const TIntV& _SentIdV) {
    // check if the corpus has translations
    IAssert(TransCorpus->IsTrans());

    // ngram counts (cliped and full)
    TIntH ClipCountNGramH, CountNGramH;
    // ngram info score
    TIntFltH NGramInfoH;
    // candidate and effective reference length
    double FullTransLen = 0.0, FullRefLen = 0.0;

    // iterate over sentences
    TIntV SentIdV = _SentIdV;
    if (SentIdV.Empty()) { TransCorpus->GetSentIdV(SentIdV); }
    const int Sents = SentIdV.Len();
    for (int SentIdN = 0; SentIdN < Sents; SentIdN++) {
        const int SentId = SentIdV[SentIdN];
        // tokenize translation
        TIntV TransWIdV; Parse(TransCorpus->GetTransStr(SentId), TransWIdV);
        TIntH TransNGramH; GetNGramH(TransWIdV, MxNGramLen, TransNGramH);
        TIntH FreeTransNGramH = TransNGramH; // number of non-matched ngrams
        // counters for getting the closest length of reference sentences
        const int TransLen = TransWIdV.Len(); int RefLenSum = 0;
        // go over reference translations and count ngram matches
        TStrV RefTransStrV = TransCorpus->GetRefTransStrV(SentId);
        // we assume that there is at least one reference translation
        IAssert(!RefTransStrV.Empty());
        for (int RefN = 0; RefN < RefTransStrV.Len(); RefN++) {
            // parse reference translation sentence
            TIntV RefWIdV; Parse(RefTransStrV[RefN], RefWIdV);
            TIntH RefNGramH; GetNGramH(RefWIdV, MxNGramLen, RefNGramH);
            // check for matches
            int TransNGramKeyId = TransNGramH.FFirstKeyId();
            while(TransNGramH.FNextKeyId(TransNGramKeyId)) {
                const int NGramId = TransNGramH.GetKey(TransNGramKeyId);
                const int FreeTransNGrams = FreeTransNGramH(NGramId);
                if (RefNGramH.IsKey(NGramId) && (FreeTransNGrams>0)) {
                    // ngram match and still some free ngrams left to clip
                    const int RefNGrams = RefNGramH(NGramId);
                    FreeTransNGramH(NGramId) = TInt::GetMx(0, FreeTransNGrams - RefNGrams);
                }
            }
            // check the length difference
            const int RefLen = RefWIdV.Len();
            RefLenSum += RefLen;
        }
        // count ngrams
        int TransNGramKeyId = TransNGramH.FFirstKeyId();
        while(TransNGramH.FNextKeyId(TransNGramKeyId)) {
            // get ngram
            const int NGramId = TransNGramH.GetKey(TransNGramKeyId);
            IAssert(NGramId != -1);
            // check if two hash tables are aligned (should be...)
            const int FreeNGramId = FreeTransNGramH.GetKey(TransNGramKeyId);
            IAssert(NGramId == FreeNGramId);
            // get ngram count and clip-count
            const int Count = TransNGramH[TransNGramKeyId];
            const int ClipCount = Count - FreeTransNGramH[TransNGramKeyId];
            // add ngram to the coprus ngram counts
            CountNGramH.AddDat(NGramId) += Count;
            ClipCountNGramH.AddDat(NGramId) += ClipCount;
        }
        // count length
        FullTransLen += double(TransLen);
        FullRefLen += double(RefLenSum) / double(RefTransStrV.Len());
    }

    // calculate ngram info scores
    int CountKeyId = CountNGramH.FFirstKeyId();
    while (CountNGramH.FNextKeyId(CountKeyId)) {
        // get the n-gram
        const int NGramId = CountNGramH.GetKey(CountKeyId);
        TIntV NGram = GetNGram(NGramId);
        // prepare counts
        if (NGram.Len() == 1) {
            // n-gram is a word
            const int WordCount = CountNGramH[CountKeyId];
            const double NGramInfoScore = TMath::Log2(FullTransLen / double(WordCount));
            NGramInfoH.AddDat(NGramId, NGramInfoScore);
        } else {
            // more then one word in the n-gram
            // get a n-gram with removed last element
            TIntV N1Gram = NGram; N1Gram.DelLast();
            const int N1GramId = NGramH.GetKeyId(N1Gram);
            // get the counts
            const int NGramCount = CountNGramH(NGramId);
            const int N1GramCount = CountNGramH(N1GramId);
            // get the score
            const double NGramInfoScore = TMath::Log2(double(N1GramCount) / double(NGramCount));
            NGramInfoH.AddDat(NGramId, NGramInfoScore);
        }
    }

    // calcualte ngram precisions
    TFltV ClipCountV(MxNGramLen); ClipCountV.PutAll(0);
    int ClipCountKeyId = ClipCountNGramH.FFirstKeyId();
    while (ClipCountNGramH.FNextKeyId(ClipCountKeyId)) {
        const int NGramId = ClipCountNGramH.GetKey(ClipCountKeyId);
        const int NGramLen = GetNGramLen(NGramId);
        const double NGramInfo = NGramInfoH(NGramId);
        IAssert(0 < NGramLen && NGramLen <= MxNGramLen);
        const int ClipCountNGram = ClipCountNGramH[ClipCountKeyId];
        ClipCountV[NGramLen-1] += double(ClipCountNGram) * NGramInfo;
    }
    TIntV CountV(MxNGramLen); CountV.PutAll(0);
    CountKeyId = CountNGramH.FFirstKeyId();
    while (CountNGramH.FNextKeyId(CountKeyId)) {
        const int NGramId = CountNGramH.GetKey(CountKeyId);
        const int NGramLen = GetNGramLen(NGramId);
        IAssert(0 < NGramLen && NGramLen <= MxNGramLen);
        CountV[NGramLen-1] += CountNGramH[CountKeyId];
    }
    TFltV PrecV(MxNGramLen, 0);
    for (int NGramLen = 0; NGramLen < MxNGramLen; NGramLen++) {
        const double ClipCount = ClipCountV[NGramLen];
        const int Count = CountV[NGramLen];
        const double Prec = (Count > 0) ? ClipCount / double(Count) : 0.0;
        PrecV.Add(Prec);
    }

    // calcualte brevity penalty
    const double LenFrac = double(FullTransLen)/double(FullRefLen);
    double BP = 0.0;
    if (LenFrac >= 1.0) { BP = 1.0; }
    else if (LenFrac <= 0.0) { BP = 0.0; }
    else {
        // calculate beta
        const double LenFracX = 1.5, BPX = 0.5;
        const double Beta = log(BPX) / TMath::Sqr(log(LenFracX));
        // calculate BP score
        BP = exp(Beta * TMath::Sqr(log(LenFrac)));
    }

    // calculate full NIST score
    double NistScore = 0.0; 
    for (int NGramLen = 0; NGramLen < MxNGramLen; NGramLen++) {
        NistScore += PrecV[NGramLen];
    }    
    NistScore *= BP;
    printf("NIST Score: %.5f\n", NistScore);
    
    // done!
    return NistScore;
}

/////////////////////////////////////////////////
// Translation-Corpus
void TTransCorpus::CleanTmx(const TStr& InTmxFNm, const TStr& OutXmlFNm) {
    TFIn FIn(InTmxFNm); TFOut FOut(OutXmlFNm);
    TStr ChStr; bool InTagP = false;
    while (!FIn.Eof()) {
        ChStr = TStr::GetChStr(FIn.GetCh());
        if (ChStr == "<") { InTagP = true; FOut.PutStr(ChStr); }
        else if (ChStr == ">") { InTagP = false; FOut.PutStr(ChStr); }
        else if (!InTagP && ChStr != "\n" && ChStr != "\r") { 
            FOut.PutStr(TXmlDoc::GetXmlStr(ChStr)); 
        } else { FOut.PutStr(ChStr); }
    }
}

TStr TTransCorpus::CleanRtf(const TStr& InStr) {
    TChA OutChA; int Chs = InStr.Len(); bool RtfTagP = false;
    for (int ChN = 0; ChN < Chs; ChN++) {
        const char Ch = InStr[ChN];
        if (Ch == '{' || Ch == '}') { /* skip */ }
        else if (Ch == '\\') { RtfTagP = true; }
        else if (Ch == ' ' && RtfTagP) { RtfTagP = false; }
        else if (!RtfTagP) { OutChA += Ch; }
    }
    return OutChA;
}

int TTransCorpus::CountWords(const TIntStrH& StrH) {
    int Words = 0, KeyId = StrH.FFirstKeyId();
    while (StrH.FNextKeyId(KeyId)) {
        const TStr& Str = StrH[KeyId];
        TStrV WordV; Str.SplitOnWs(WordV);
        Words += WordV.Len();
    }
    return Words;
}

int TTransCorpus::CountWords(const TIntStrVH& StrVH, const bool& AllSents) {
    int Words = 0, KeyId = StrVH.FFirstKeyId();
    while (StrVH.FNextKeyId(KeyId)) {
        const TStrV& StrV = StrVH[KeyId];
        if (AllSents) { 
            for (int StrN = 0; StrN < StrV.Len(); StrN++) {                
                TStrV WordV; StrV[StrN].SplitOnWs(WordV);
                Words += WordV.Len();
            }
        } else {
            TStrV WordV; StrV[0].SplitOnWs(WordV);
            Words += WordV.Len();
        }
    }
    return Words;
}

inline void TTransCorpus::AddSentenceNoTrans(const int& SentId, 
        const TStr& OrgStr, const TStr& RefTransStr) {

    AddSentenceNoTrans(SentId, OrgStr, TStrV::GetV(RefTransStr));
}

void TTransCorpus::AddSentenceNoTrans(const int& SentId, 
        const TStr& OrgStr, const TStrV& RefTransStrV) {

    // check if any empty
    if (OrgStr.Empty()) { return; }
    if (RefTransStrV[0].Empty()) { return; }
    // we shouldn't have this key yet
    IAssert(!OrgStrH.IsKey(SentId));
    // add new sentence
    OrgStrH.AddDat(SentId, OrgStr); 
    RefTransStrVH.AddDat(SentId, RefTransStrV);
    // check that key ids match
    const int OrgKeyId = OrgStrH.GetKeyId(SentId);
    const int RefTransKeyId = RefTransStrVH.GetKeyId(SentId);
    IAssert(OrgKeyId == RefTransKeyId);
}

inline void TTransCorpus::AddSentenceNoOrg(const int& SentId, 
        const TStr& TransStr, const TStr& RefTransStr) {

    AddSentenceNoOrg(SentId, TransStr, TStrV::GetV(RefTransStr));
}

void TTransCorpus::AddSentenceNoOrg(const int& SentId, 
        const TStr& TransStr, const TStrV& RefTransStrV) {

    IAssert(!TransStrH.IsKey(SentId));
    // add new sentence
    TransStrH.AddDat(SentId, TransStr); 
    RefTransStrVH.AddDat(SentId, RefTransStrV);
    // check that key ids match
    const int TransKeyId = TransStrH.GetKeyId(SentId);
    const int RefTransKeyId = RefTransStrVH.GetKeyId(SentId);
    IAssert(TransKeyId == RefTransKeyId);
}

inline void TTransCorpus::AddSentence(const int& SentId, const TStr& OrgStr, 
        const TStr& TransStr, const TStr& RefTransStr) {

    AddSentence(SentId, OrgStr, TransStr, TStrV::GetV(RefTransStr));
}

void TTransCorpus::AddSentence(const int& SentId, const TStr& OrgStr, 
        const TStr& TransStr, const TStrV& RefTransStrV) {

    IAssert(!OrgStrH.IsKey(SentId));
    // add new sentence
    OrgStrH.AddDat(SentId, OrgStr); 
    TransStrH.AddDat(SentId, TransStr); 
    RefTransStrVH.AddDat(SentId, RefTransStrV);
    // check that key ids match
    const int OrgKeyId = OrgStrH.GetKeyId(SentId);
    const int TransKeyId = TransStrH.GetKeyId(SentId);
    const int RefTransKeyId = RefTransStrVH.GetKeyId(SentId);
    IAssert(OrgKeyId == TransKeyId);
    IAssert(OrgKeyId == RefTransKeyId);
}

void TTransCorpus::GetSentIdV(TIntV& SentIdV) const {
    if (IsOrg()) { OrgStrH.GetKeyV(SentIdV); }
    else if (IsTrans()) { TransStrH.GetKeyV(SentIdV); }
    else if (IsRefTrans()) { RefTransStrVH.GetKeyV(SentIdV); }
    else { Fail; }
}

void TTransCorpus::Shuffle(const PTransCorpus& InFirstTransCorpus, 
        const PTransCorpus& InSecondTransCorpus, TRnd& Rnd, const double& SwapProb,
        PTransCorpus& OutFirstTransCorpus, PTransCorpus& OutSecondTransCorpus) {

    // prepare new corpuses
    OutFirstTransCorpus = TTransCorpus::New();
    OutSecondTransCorpus = TTransCorpus::New();
    // swap sentences
    TIntV FirstSentIdV, SecondSentIdV;
    InFirstTransCorpus->GetSentIdV(FirstSentIdV);
    InSecondTransCorpus->GetSentIdV(SecondSentIdV);
    for (int SentIdN = 0; SentIdN < FirstSentIdV.Len(); SentIdN++) {
        // get sentence id
        const int SentId = FirstSentIdV[SentIdN];
        // check if id same in both cases
        IAssert(SecondSentIdV[SentIdN] == SentId);
        // read sentences
        TStr OrgStr1 = InFirstTransCorpus->GetOrgStr(SentId);
        TStr OrgStr2 = InSecondTransCorpus->GetOrgStr(SentId);
        IAssert(OrgStr1 == OrgStr2 );
        TStr RefTransStr1 = InFirstTransCorpus->GetRefTransStrV(SentId)[0];
        TStr RefTransStr2 = InSecondTransCorpus->GetRefTransStrV(SentId)[0];
        IAssert(RefTransStr1 == RefTransStr2);
        TStr FirstTransStr = InFirstTransCorpus->GetTransStr(SentId);
        TStr SecondTransStr = InSecondTransCorpus->GetTransStr(SentId);
        // swap sentences
        if (Rnd.GetUniDev() < SwapProb) { 
            // we swap
            OutFirstTransCorpus->AddSentence(SentId, OrgStr1, SecondTransStr, RefTransStr1);
            OutSecondTransCorpus->AddSentence(SentId, OrgStr1, FirstTransStr, RefTransStr1);
        } else {
            // no swap
            OutFirstTransCorpus->AddSentence(SentId, OrgStr1, FirstTransStr, RefTransStr1);
            OutSecondTransCorpus->AddSentence(SentId, OrgStr1, SecondTransStr, RefTransStr1);
        }
    }
}


PTransCorpus TTransCorpus::LoadAC(const TStr& InXmlFNm, const int& MxSents) {
    // prepare prset structures
    PTransCorpus TransCorpus = TTransCorpus::New();
    // we load xml by skiping first tags
    PSIn XmlSIn=TFIn::New(InXmlFNm); 
    TXmlDoc::SkipTopTag(XmlSIn); // ignore TEI
    printf("Ignoring: %s\n", TXmlDoc::LoadTxt(XmlSIn)->GetTok()->GetTagNm().CStr()); // ignore teiHeader
    TXmlDoc::SkipTopTag(XmlSIn); // ignore text
    TXmlDoc::SkipTopTag(XmlSIn); // ignore body
    PXmlDoc XmlDoc; int XmlDocs = 0, SentId = 0;;
    forever{
        // load xml tree
        XmlDocs++; printf("%7d Sentences \r", SentId);
        XmlDoc=TXmlDoc::LoadTxt(XmlSIn);
        // stop if at the last tag
        if (!XmlDoc->IsOk()) { /*printf("Error: %s\n", XmlDoc->GetMsgStr().CStr());*/ break; }
        // extract documents from xml-trees
        PXmlTok TopTok=XmlDoc->GetTok();
        if (TopTok->IsTag("div")){
            // extract document Id
            TStr DocNm = TopTok->GetArgVal("n");
            // and paragraphs
            TXmlTokV LinkTokV; TopTok->GetTagTokV("linkGrp|link", LinkTokV);
            for (int LinkTokN = 0; LinkTokN < LinkTokV.Len(); LinkTokN++) {
                PXmlTok LinkTok = LinkTokV[LinkTokN];
                TStr LinkType = LinkTok->GetArgVal("type");
                // skip if paragraph for one language is empty
                if (LinkType == "1:1") {
                    TXmlTokV S1TokV; LinkTok->GetTagTokV("s1", S1TokV); 
                    TXmlTokV S2TokV; LinkTok->GetTagTokV("s2", S2TokV);
                    IAssert(S1TokV.Len() == 1); IAssert(S2TokV.Len() == 1);
                    TStr ParaStr1 = S1TokV[0]->GetTagTokStr("");
                    TStr ParaStr2 = S2TokV[0]->GetTagTokStr("");
                    TransCorpus->AddSentenceNoTrans(SentId, ParaStr1, ParaStr2); SentId++;
                }
            }
        } else {
            printf("Unknow tag: %s\n", TopTok->GetTagNm().CStr());
        }
        if ((MxSents != -1) && (TransCorpus->GetSentences() > MxSents)) { break; }
    }
    printf("\n");
    // finish
    return TransCorpus;
}

PTransCorpus TTransCorpus::LoadEP(const TStr& InOrgFPath, const TStr& InTransFPath) {
    // prepare prset structures
    PTransCorpus TransCorpus = TTransCorpus::New();
    // iterate over all the files
    TStr NrmInTransFPath = TStr::GetNrAbsFPath(InTransFPath);
    TFFile OrgFNms(InOrgFPath, "txt", false); TStr OrgFNm;
    int SentId = 0;
    while (OrgFNms.Next(OrgFNm)) {
        // get name of the file with aligned sentences
        TStr TransFNm = NrmInTransFPath + OrgFNm.GetFBase();
        IAssertR(TFile::Exists(TransFNm), TransFNm);
        // load file
        printf("Loading %s and %s ...\r", OrgFNm.CStr(), TransFNm.CStr());
        TLnRet OrgLnRet(TFIn::New(OrgFNm));
        TLnRet TransLnRet(TFIn::New(TransFNm));
        TStr OrgLn, TransLn; int LnN = 1; bool EmptyLnP = false;
        while (OrgLnRet.NextLn(OrgLn)) {
            if (!TransLnRet.NextLn(TransLn)) {
                printf("\nEarly stop in line (%s:%s)[%d]\n", OrgLn.CStr(), TransLn.CStr(), LnN);
                break; // first file finished, let's stop
            }
            if (OrgLn.Empty() || TransFNm.Empty()) {
                // skip empty line and skip till 
                EmptyLnP = true;
            } else if (OrgLn[0] == '<' || TransLn[0] == '<') {
                if (TransLn[0] != OrgLn[0]) {
                    printf("\nError in line (%s:%s)[%d]\n", OrgLn.CStr(), TransLn.CStr(), LnN);
                    break; // we stop, lines not aligned anymore ...
                }
                // reset the empty count
                EmptyLnP = false;
                // skip XML tags
            } else if (!EmptyLnP) {
                // aligned sentence!
                TransCorpus->AddSentenceNoTrans(SentId, 
                    OrgLn.ToTrunc(), TransLn.ToTrunc()); 
                SentId++;
            }
            LnN++;
        }         
    }
    printf("\nDone!\n");
    // finish
    return TransCorpus;
}

PTransCorpus TTransCorpus::LoadTMX(const TStr& InTmxFPath,
        const TStr& OrgLang, const TStr& RefTransLang) {

    // prepare prset structures
    PTransCorpus TransCorpus = TTransCorpus::New();
    // iterate over all the TMX files
    TFFile TmxFNms(InTmxFPath, "tmx", false); TStr TmxFNm; int SentId = 0;
    while (TmxFNms.Next(TmxFNm)) {
        printf("Loading %s ...\n", TmxFNm.CStr());
        // we load xml by skiping first tag...
        TStr CleanTmxFNm = TmxFNm + ".xml";
        CleanTmx(TmxFNm, CleanTmxFNm);
        PSIn XmlSIn = TFIn::New(CleanTmxFNm);
        PXmlDoc XmlDoc = TXmlDoc::LoadTxt(XmlSIn);
        // stop if at the last tag
        if (!XmlDoc->IsOk()) { 
            printf(" error: %s\n", XmlDoc->GetMsgStr().CStr());
            continue; 
        }
        // extract sentences from xml-trees
        TXmlTokV TuTokV; XmlDoc->GetTagTokV("tmx|body|tu", TuTokV);
        const int TuToks = TuTokV.Len();
        for (int TuTokN = 0; TuTokN < TuToks; TuTokN++) {
            if (TuTokN % 100 == 0) { printf(" %d / %d\r", TuTokN, TuToks); }
            TXmlTokV TuvTokV; TuTokV[TuTokN]->GetTagTokV("tuv", TuvTokV);
            IAssert(TuvTokV.Len() == 2);
            TStr OrgSent, RefTransSent;
            for (int TuvTokN = 0; TuvTokN < TuvTokV.Len(); TuvTokN++) {
                TStr Lang = TuvTokV[TuvTokN]->GetStrArgVal("xml:lang", "");
                TStr Sent = CleanRtf(TuvTokV[TuvTokN]->GetTagTok("seg")->GetTokStr(false));
                if (Lang == OrgLang) { OrgSent = Sent; } 
                else if (Lang == RefTransLang) { RefTransSent = Sent; }
            }
            TransCorpus->AddSentenceNoTrans(SentId, OrgSent, RefTransSent); SentId++;
        }
        printf(" %d / %d\n", TuToks, TuToks);
    }    
    // finish
    return TransCorpus;
}

PTransCorpus TTransCorpus::LoadTxt(const TStr& InOrgFNm, 
        const TStr& InTransFNm, const TStr& InRefTransFNm) {

    // open files
    PSIn OrgSIn = !InOrgFNm.Empty() ? TFIn::New(InOrgFNm) : PSIn();
    PSIn TransSIn = !InTransFNm.Empty() ? TFIn::New(InTransFNm) : PSIn();
    PSIn RefTransSIn = !InRefTransFNm.Empty() ? TFIn::New(InRefTransFNm) : PSIn();
    // check which are given
    const bool IsOrgP = !OrgSIn.Empty();
    const bool IsTransP = !TransSIn.Empty();
    const bool IsRefTransP = !RefTransSIn.Empty();
    // print warnings
    if (!IsOrgP) { printf("No original sentences!\n"); }
    if (!IsTransP) { printf("No machine translation sentences!\n"); }
    if (!IsRefTransP) { printf("No reference translation sentences!\n"); }
    // traverse the files and add sentences
    PTransCorpus TransCorpus = TTransCorpus::New();
    TLnRet OrgLnRet(OrgSIn), TransLnRet(TransSIn), RefTransLnRet(RefTransSIn);
    TStr OrgLnStr, TransLnStr, RefTransLnStr; int LnN = 1;
    forever {
        // try to read next line, otherwise break
        if (IsOrgP && !OrgLnRet.NextLn(OrgLnStr)) { break; }
        if (IsTransP && !TransLnRet.NextLn(TransLnStr)) { break; }
        if (IsRefTransP && !RefTransLnRet.NextLn(RefTransLnStr)) { break; }
        // print progress
        if (LnN % 100 == 0) { printf("  %7d Sentences\r", LnN); }
        // add sentence and translation(s) to the corpus
        if (!IsOrgP) { 
            TransCorpus->AddSentenceNoOrg(LnN, TransLnStr, RefTransLnStr);
        } else if (!IsTransP) { 
            TransCorpus->AddSentenceNoTrans(LnN, OrgLnStr, RefTransLnStr);
        } else { IAssert(IsRefTransP);
            TransCorpus->AddSentence(LnN, OrgLnStr, TransLnStr, RefTransLnStr);
        }
        // next sentence :-)
        LnN++;
    } printf("\n");
    // finish
    return TransCorpus;
}

void TTransCorpus::SaveTxt(const TStr& OutFBase, const TStr& OutOrgFNm, 
        const TStr& OutTransFNm, const TStr& OutRefTransFNm, TStrV& OrgFNmV, 
        TStrV& TransFNmV, TStrV& RefTransFNmV, const int& LinesPerFile) {
    
    // prepare filenames
    OrgFNmV.Clr(); TransFNmV.Clr(); RefTransFNmV.Clr();
    if (!OutOrgFNm.Empty()) { OrgFNmV.Add(GetOutFNm(OutFBase, 0, LinesPerFile, OutOrgFNm)); }
    if (!OutTransFNm.Empty()) { TransFNmV.Add(GetOutFNm(OutFBase, 0, LinesPerFile, OutTransFNm)); }
    if (!OutRefTransFNm.Empty()) { RefTransFNmV.Add(GetOutFNm(OutFBase, 0, LinesPerFile, OutRefTransFNm)); }
    // open files
    PSOut OrgSOut = !OutOrgFNm.Empty() ? TFOut::New(OrgFNmV.Last()) : PSOut();
    PSOut TransSOut = !OutTransFNm.Empty() ? TFOut::New(TransFNmV.Last()) : PSOut();
    PSOut RefTransSOut = !OutRefTransFNm.Empty() ? TFOut::New(RefTransFNmV.Last()) : PSOut();
    // check which are given
    const bool IsOrgP = !OrgSOut.Empty() && IsOrg();
    const bool IsTransP = !TransSOut.Empty() && IsTrans();
    const bool IsRefTransP = !RefTransSOut.Empty() && IsRefTrans();
    // print warnings
    if (!IsOrgP) { printf("No original sentences!\n"); }
    if (!IsTransP) { printf("No machine translation sentences!\n"); }
    if (!IsRefTransP) { printf("No reference translation sentences!\n"); }
    // go over all the sentences and store them in the file
    TIntV SentIdV; GetSentIdV(SentIdV);
    for (int SentIdN = 0; SentIdN < SentIdV.Len(); SentIdN++) {
        const int SentId = SentIdV[SentIdN];
        if (IsOrgP) { OrgSOut->PutStrLn(GetOrgStr(SentId)); }
        if (IsTransP) { TransSOut->PutStrLn(GetTransStr(SentId)); }
        if (IsRefTransP) { RefTransSOut->PutStrLn(GetRefTransStrV(SentId)[0]); }
        // should we break and go to next file?
        if ((LinesPerFile!=-1) && (SentIdN>0) && (SentIdN % LinesPerFile == 0)) {
            // prepare filenames
            if (!OutOrgFNm.Empty()) { OrgFNmV.Add(GetOutFNm(OutFBase, SentIdN, LinesPerFile, OutOrgFNm)); }
            if (!OutTransFNm.Empty()) { TransFNmV.Add(GetOutFNm(OutFBase, SentIdN, LinesPerFile, OutTransFNm)); }
            if (!OutRefTransFNm.Empty()) { RefTransFNmV.Add(GetOutFNm(OutFBase, SentIdN, LinesPerFile, OutRefTransFNm)); }
            // open next files files
            OrgSOut = !OutOrgFNm.Empty() ? TFOut::New(OrgFNmV.Last()) : PSOut();
            TransSOut = !OutTransFNm.Empty() ? TFOut::New(TransFNmV.Last()) : PSOut();
            RefTransSOut = !OutRefTransFNm.Empty() ? TFOut::New(RefTransFNmV.Last()) : PSOut();
        }
    }
}

void TTransCorpus::SaveSgm(const TStr& OutOrgFNm, 
        const TStr& OutTransFNm, const TStr& OutRefTransFNm) {

    // open files
    PSOut OrgSOut = !OutOrgFNm.Empty() ? TFOut::New(OutOrgFNm) : NULL;
    PSOut TransSOut = !OutTransFNm.Empty() ? TFOut::New(OutTransFNm) : NULL;
    PSOut RefTransSOut = !OutRefTransFNm.Empty() ? TFOut::New(OutRefTransFNm) : NULL;
    // check which are given
    const bool IsOrgP = !OrgSOut.Empty() && IsOrg();
    const bool IsTransP = !TransSOut.Empty() && IsTrans();
    const bool IsRefTransP = !RefTransSOut.Empty() && IsRefTrans();
    // prepare headers
    if (IsOrgP) {
        OrgSOut->PutStrLn("<srcset setid=\"tmp\" srclang=\"source\" trglang=\"target\">");
        OrgSOut->PutStrLn("<DOC docid=\"tmpdoc\">");
    }
    if (IsTransP) {
        TransSOut->PutStrLn("<tstset setid=\"tmp\" srclang=\"source\" trglang=\"target\">");
        TransSOut->PutStrLn("<DOC docid=\"tmpdoc\" sysid=\"trans\">");
    }
    if (IsRefTransP) {
        RefTransSOut->PutStrLn("<refset setid=\"tmp\" srclang=\"source\" trglang=\"target\">");
        RefTransSOut->PutStrLn("<DOC docid=\"tmpdoc\" sysid=\"ref\">");
    }
    // output sentences
    TIntV SentIdV; GetSentIdV(SentIdV);
    for (int SentIdN = 0; SentIdN < SentIdV.Len(); SentIdN++) {
        const int SentId = SentIdV[SentIdN];
        if (IsOrgP) { 
            OrgSOut->PutStrLn(TStr::Fmt(
                "<p><seg id=\"%d\">%s</seg></p>", 
                SentId, GetOrgStr(SentId).CStr())); 
        }
        if (IsTransP) { 
            TransSOut->PutStrLn(TStr::Fmt(
                "<p><seg id=\"%d\">%s</seg></p>", 
                SentId, GetTransStr(SentId).CStr()));
        }
        if (IsRefTransP) { 
            RefTransSOut->PutStrLn(TStr::Fmt(
                "<p><seg id=\"%d\">%s</seg></p>", 
                SentId, GetRefTransStrV(SentId)[0].CStr()));
        }
    }
    // prepare footers
    if (IsOrgP) {
        OrgSOut->PutStrLn("</DOC>");
        OrgSOut->PutStrLn("</srcset>");
    }
    if (IsTransP) {
        TransSOut->PutStrLn("</DOC>");
        TransSOut->PutStrLn("</tstset>");
    }
    if (IsRefTransP) {
        RefTransSOut->PutStrLn("</DOC>");
        RefTransSOut->PutStrLn("</refset>");
    }
}

void TTransCorpus::SaveTransScript(const TStr& OutFBase, const TStr& OrgLang,
        const TStr& TransLang, const TStrV& OrgFNmV, const TStrV& RefTransFNmV) {

    const int FNms = OrgFNmV.Len();
    IAssert(OrgFNmV.Len() == RefTransFNmV.Len());

    // make tokenize
    PSOut TokSOut = TFOut::New(OutFBase + "_tokenize.sh");
    for (int FNmN = 0; FNmN < FNms; FNmN++) {
        TokSOut->PutStrLn("tokenize.pl -lang=" + OrgLang + " " + 
            OrgFNmV[FNmN].GetFBase() + " " + 
            TStr::PutFExt(OrgFNmV[FNmN], ".tok").GetFBase());
        TokSOut->PutStrLn("tokenize.pl -lang=" + TransLang + " " + 
            RefTransFNmV[FNmN].GetFBase() + " " + 
            TStr::PutFExt(RefTransFNmV[FNmN], ".tok").GetFBase());           
    }
    TokSOut->Flush();

    // make align
    PSOut AlignSOut = TFOut::New(OutFBase + "_align.sh");
    for (int FNmN = 0; FNmN < FNms; FNmN++) {
        AlignSOut->PutStrLn("ssal -f " + 
            TStr::PutFExt(OrgFNmV[FNmN], ".tok").GetFBase() + " " + 
            TStr::PutFExt(RefTransFNmV[FNmN], ".tok").GetFBase());
        AlignSOut->PutStrLn("mv " + 
            TStr::PutFExt(OrgFNmV[FNmN], ".tok.al").GetFBase() + " " + 
            TStr::PutFExt(OrgFNmV[FNmN], ".al").GetFBase());
        AlignSOut->PutStrLn("mv " + 
            TStr::PutFExt(RefTransFNmV[FNmN], ".tok.al").GetFBase() + " " + 
            TStr::PutFExt(RefTransFNmV[FNmN], ".al").GetFBase());
    }
    AlignSOut->Flush();

    // make lowercase
    PSOut LcSOut = TFOut::New(OutFBase + "_lowercase.sh");
    for (int FNmN = 0; FNmN < FNms; FNmN++) {
        LcSOut->PutStrLn("lc-latin.pl " + 
            TStr::PutFExt(OrgFNmV[FNmN], ".al").GetFBase() + " " + 
            TStr::PutFExt(OrgFNmV[FNmN], ".lc").GetFBase());
        LcSOut->PutStrLn("lc-latin.pl " + 
            TStr::PutFExt(RefTransFNmV[FNmN], ".al").GetFBase() + " " + 
            TStr::PutFExt(RefTransFNmV[FNmN], ".lc").GetFBase());
    }
    LcSOut->Flush();

    // rest -- script
}

/////////////////////////////////////////////////
// Aligned-Pair
TAlignPair::TAlignPair(const TStr& _Lang1, const TStr& _Lang2): Lang1(_Lang1), Lang2(_Lang2) { 
    BowDocBs1 = TBowDocBs::New(TSwSet::New(swstNone), TStemmer::New(stmtNone), NULL);
    BowDocBs2 = TBowDocBs::New(TSwSet::New(swstNone), TStemmer::New(stmtNone), NULL);
}

TAlignPair::TAlignPair(TSIn& SIn) { 
    Lang1.Load(SIn); Lang2.Load(SIn);
    BowDocBs1 = TBowDocBs::Load(SIn); BowDocBs1->SaveTxtStat(Lang1 + Lang2 + ".txt");
    BowDocBs2 = TBowDocBs::Load(SIn); BowDocBs2->SaveTxtStat(Lang1 + Lang2 + ".txt");
    Def();
}

void TAlignPair::Save(TSOut& SOut) const {
    Lang1.Save(SOut); Lang2.Save(SOut); 
    BowDocBs1->Save(SOut); BowDocBs2->Save(SOut);
}

void TAlignPair::AddSent(const TStr& Sent1, const TStr& Sent2) {
    TStr DocNm = TInt::GetStr(BowDocBs1->GetDocs());
    BowDocBs1->AddHtmlDoc(DocNm, TStrV(), Sent1, false);
    BowDocBs2->AddHtmlDoc(DocNm, TStrV(), Sent2, false);
    IAssert(BowDocBs1->GetDocs() == BowDocBs2->GetDocs());
}

void TAlignPair::Def() {
    // calcualte weights for words
    BowDocWgtBs1 = TBowDocWgtBs::New(BowDocBs1, bwwtNrmTFIDF);
    BowDocWgtBs2 = TBowDocWgtBs::New(BowDocBs2, bwwtNrmTFIDF);
    // make a matrix out of weighted sparse vectors
    TVec<PBowSpV> SpVV1, SpVV2;
    const int Docs = BowDocWgtBs1->GetDocs();
    for (int DocN = 0; DocN < Docs; DocN++) {
        PBowSpV SpV1 = BowDocWgtBs1->GetSpV(DocN);
        PBowSpV SpV2 = BowDocWgtBs2->GetSpV(DocN);
        if (SpV1->Len() < 3) { continue; }
        if (SpV2->Len() < 3) { continue; }
        SpVV1.Add(SpV1);
        SpVV2.Add(SpV2);
    }
    Matrix1 = TBowMatrix(SpVV1);
    Matrix2 = TBowMatrix(SpVV2);
}

const TMatrix& TAlignPair::GetMatrix(const TStr& Lang) const {
    if (Lang == Lang1) { return Matrix1; }
    else if (Lang == Lang2) { return Matrix2; }
    TExcept::Throw("Wrong language " + Lang + "!");
    Fail; return *(new TBowMatrix); // this should never happen
}

void TAlignPair::GetSpV(const TStr& Str, const TStr& Lang, TIntFltKdV& SpV) const {
    EAssertR((Lang == Lang1) || (Lang == Lang2), "Wrong language " + Lang + "!");
    // get bow sparse vector
    PBowSpV BowSpV = (Lang == Lang1) ? 
        BowDocBs1->GetSpVFromHtmlStr(Str, BowDocWgtBs1) :
        BowDocBs2->GetSpVFromHtmlStr(Str, BowDocWgtBs2);
    // make it universal
    BowSpV->GetIntFltKdV(SpV);
}

TStr TAlignPair::GetSpVStr(const TIntFltKdV& SpV, const TStr& Lang, 
        const int& MxWords, const double& MxWgtPrc) const {

    EAssertR((Lang == Lang1) || (Lang == Lang2), "Wrong language " + Lang + "!");
    // get right bow doc bs
    PBowDocBs BowDocBs = (Lang == Lang1) ? BowDocBs1 : BowDocBs2;
    PBowSpV BowSpV = TBowSpV::New(-1, SpV);
    return BowSpV->GetStr(BowDocBs, MxWords, MxWgtPrc, " ", false, false);
}

PAlignPair TAlignPair::LoadAcXml(const TStr& FNm, const int& MxSents) {
    printf("Loading %s ...\n", FNm.CStr());
    // get the lanugagne names
    TStr BaseNm = FNm.GetFMid();
    TStrV PartV; BaseNm.SplitOnAllCh('-', PartV);
    IAssertR(PartV.Len() == 3, "Bad file name: " + BaseNm);
    // prepare aligne pair
    PAlignPair AlignPair = TAlignPair::New(PartV[1], PartV[2]);
    // parse the XML
    PTransCorpus TransCorpus = TTransCorpus::LoadAC(FNm, MxSents * 4);
    // select subset of sentences which will go into aligned corpus
    const int AllSents = TransCorpus->GetSentences();
    TIntV SentIdV(AllSents, 0);
    for (int SentId = 0; SentId < AllSents; SentId++) {
        SentIdV.Add(SentId); 
    }
    if (MxSents != -1 && AllSents > MxSents) {
    	TRnd Rnd(1);
        SentIdV.Shuffle(Rnd);
        SentIdV.Trunc(MxSents);
    }
    // add the sentences to the bow
    const int Sents = SentIdV.Len();
    for (int SentIdN = 0; SentIdN < Sents; SentIdN++) {
        const int SentId = SentIdV[SentIdN];
        const TStr& Sent1 = TransCorpus->GetOrgStr(SentId);
        const TStr& Sent2 = TransCorpus->GetRefTransStrV(SentId)[0];
        AlignPair->AddSent(Sent1, Sent2);
    }
    // finish the alignment pair
    AlignPair->Def();
    return AlignPair;
}

/////////////////////////////////////////////////
// Aligned-Pair-Base
void TAlignPairBs::AddAlignPair(const PAlignPair& AlignPair) {
    // remember the languages
    const int LangId1 = LangH.AddKey(AlignPair->GetLang1());
    const int LangId2 = LangH.AddKey(AlignPair->GetLang2());
    // remember the align pair
    AlignPairV.Add(AlignPair);
    // prepare pointers from languages to alignments
    LangToAlignPairVH.AddDat(LangId1).Add(AlignPair);
    LangToAlignPairVH.AddDat(LangId2).Add(AlignPair);
    TIntPr LangIdPr(TInt::GetMn(LangId1, LangId2), TInt::GetMx(LangId1, LangId2));
    LangPrToAlignPairH.AddDat(LangIdPr, AlignPair);
}

PAlignPair TAlignPairBs::GetAlignPair(const int& LangId1, const int& LangId2) {
    TIntPr LangIdPr(TInt::GetMn(LangId1, LangId2), TInt::GetMx(LangId1, LangId2));
    return LangPrToAlignPairH.GetDat(LangIdPr);
}

TAlignPairBs::TAlignPairBs(TSIn& SIn) {
    // load languages
    LangH.Load(SIn);
    // load aligned pairs and add them to index
    TAlignPairV TmpAlignPairV(SIn);
    for (int TmpAlignPairN = 0; TmpAlignPairN < TmpAlignPairV.Len(); TmpAlignPairN++) {
        AddAlignPair(TmpAlignPairV[TmpAlignPairN]);
    }
}

void TAlignPairBs::Save(TSOut& SOut) const { 
    LangH.Save(SOut);
    AlignPairV.Save(SOut);
}

TStr TAlignPairBs::MapQuery(const TAlignPairMap& Map, const TStr& QueryStr, 
        const int& QueryLangId, const int& TargetLangId, const int& TransQueryMtpy,
        const double& MxWgtPrc) {

    // get alignment corpus
    PAlignPair AlignPair = GetAlignPair(QueryLangId, TargetLangId);
    AlignPair->Def();
    // get languages
    const TStr& QueryLang = LangH.GetKey(QueryLangId);
    const TStr& TargetLang = LangH.GetKey(TargetLangId);
    // get sparse vector from the query
    TIntFltKdV InSpV; AlignPair->GetSpV(QueryStr, QueryLang, InSpV);
    // get sparse matrices with aligned columns
    const TMatrix& QueryMatrix = AlignPair->GetMatrix(QueryLang);
    const TMatrix& TargetMatrix = AlignPair->GetMatrix(TargetLang);
    // map the query
    TIntFltKdV OutSpV; Map(InSpV, QueryMatrix, TargetMatrix, OutSpV);
    // make query back to string
    return AlignPair->GetSpVStr(OutSpV, TargetLang, 
        InSpV.Len() * TransQueryMtpy, MxWgtPrc);
}

PAlignPairBs TAlignPairBs::LoadAcXml(const TStr& FPath, const int& MxSents) {
    // prepare base
    PAlignPairBs AlignPairBs = TAlignPairBs::New();
    // iterate over files and load language to language alignments
    TFFile FFile(FPath, ".xml", false); TStr FNm;
    while (FFile.Next(FNm)) {
        PAlignPair AlignPair = TAlignPair::LoadAcXml(FNm, MxSents);
        AlignPairBs->AddAlignPair(AlignPair);
    }
    // finish
    return AlignPairBs;
}
