//////////////////////////////////////
// Simple-App-Server-Function
bool TSAppSrvFun::IsFldNm(const TStrKdV& FldNmValPrV, const TStr& FldNm) {
	const int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
	return (ValN != -1);
}

TStr TSAppSrvFun::GetFldVal(const TStrKdV& FldNmValPrV, 
		const TStr& FldNm, const TStr& DefFldVal) {

	const int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
	return (ValN == -1) ? DefFldVal : FldNmValPrV[ValN].Dat;	
}

int TSAppSrvFun::GetFldInt(const TStrKdV& FldNmValPrV, const TStr& FldNm, const TStr& DefStr) {
	TStr IntStr = GetFldVal(FldNmValPrV, FldNm, DefStr);
	EAssertR(IntStr.IsInt(), "Parameter '" + FldNm + "' not a number");
	return IntStr.GetInt();
}

uint64 TSAppSrvFun::GetFldUInt64(const TStrKdV& FldNmValPrV, const TStr& FldNm, const TStr& DefStr) {
	TStr IntStr = GetFldVal(FldNmValPrV, FldNm, DefStr);
	EAssertR(IntStr.IsUInt64(), "Parameter '" + FldNm + "' not a number");
	return IntStr.GetUInt64();
}

void TSAppSrvFun::GetFldValV(const TStrKdV& FldNmValPrV, const TStr& FldNm, TStrV& FldValV) {
	FldValV.Clr();
	int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
	while (ValN != -1) {
		FldValV.Add(FldNmValPrV[ValN].Dat);
		ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""), ValN + 1);
	}
}

void TSAppSrvFun::GetFldValSet(const TStrKdV& FldNmValPrV, const TStr& FldNm, TStrSet& FldValSet) {
	FldValSet.Clr();
	int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
	while (ValN != -1) {
		FldValSet.AddKey(FldNmValPrV[ValN].Dat);
		ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""), ValN + 1);
	}
}

bool TSAppSrvFun::IsFldNmVal(const TStrKdV& FldNmValPrV, 
		const TStr& FldNm, const TStr& FldVal) {

	int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
	while (ValN != -1) {
		if (FldNmValPrV[ValN].Dat == FldVal) { return true; }
		ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""), ValN + 1);
	}
	return false;
}

TStr TSAppSrvFun::XmlHdStr = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";

PXmlDoc TSAppSrvFun::GetErrorXmlRes(const TStr& ErrorStr) {
	return TXmlDoc::New(TXmlTok::New("error", ErrorStr)); 
}

TStr TSAppSrvFun::GetErrorJsonRes(const TStr& ErrorStr) {
	PJsonVal ErrorVal = TJsonVal::NewObj("error", TJsonVal::NewStr(ErrorStr));
	return TJsonVal::GetStrFromVal(ErrorVal);
}

void TSAppSrvFun::Exec(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
	const PNotify& Notify = RqEnv->GetWebSrv()->GetNotify();
	PHttpResp HttpResp;
	try {
        // log the call
        TStr TimeNow = TTm::GetCurLocTm().GetWebLogDateTimeStr(true);
        Notify->OnStatus(TStr::Fmt("[%s] RequestStart %s", TimeNow.CStr(), FunNm.CStr()));
		TTmStopWatch StopWatch(true);
		// execute the actuall function, according to the type
		PSIn BodySIn; TStr ContTypeVal;
		if (GetFunOutType() == saotXml) {
			PXmlDoc ResXmlDoc = ExecXml(FldNmValPrV, RqEnv);        
			TStr ResXmlStr; ResXmlDoc->SaveStr(ResXmlStr);
			BodySIn = TMIn::New(XmlHdStr + ResXmlStr);
			ContTypeVal = THttp::TextXmlFldVal;
		} else if (GetFunOutType() == saotJSon) {
			TStr ResStr = ExecJSon(FldNmValPrV, RqEnv);
			BodySIn = TMIn::New(ResStr);
			ContTypeVal = THttp::AppJSonFldVal;
		} else {
			BodySIn = ExecSIn(FldNmValPrV, RqEnv, ContTypeVal);
		}
		// log finis of the call
		Notify->OnStatus(TStr::Fmt("[%s] RequestFinish %s [%d ms]", 
			TimeNow.CStr(), FunNm.CStr(), StopWatch.GetMSecInt()));
		// prepare response
		HttpResp = THttpResp::New(THttp::OkStatusCd, 
			ContTypeVal, false, BodySIn);
    } catch (PExcept Except) {
		// known internal error
		PXmlTok TopTok = TXmlTok::New("error");
		TopTok->AddSubTok(TXmlTok::New("message", Except->GetMsgStr()));
		TopTok->AddSubTok(TXmlTok::New("location", Except->GetLocStr()));
		PXmlDoc ErrorXmlDoc = TXmlDoc::New(TopTok); 
        TStr ResXmlStr; ErrorXmlDoc->SaveStr(ResXmlStr);
		// prepare response
		HttpResp = THttpResp::New(THttp::InternalErrStatusCd, 
            THttp::TextHtmlFldVal, false, 
			TMIn::New(XmlHdStr + ResXmlStr));
    } catch (...) {
		// unknown internal error
		PXmlDoc ErrorXmlDoc = TXmlDoc::New(TXmlTok::New("error")); 
        TStr ResXmlStr; ErrorXmlDoc->SaveStr(ResXmlStr);
		// prepare response
        HttpResp = THttpResp::New(THttp::InternalErrStatusCd, 
            THttp::TextHtmlFldVal, false, 
			TMIn::New(XmlHdStr + ResXmlStr));
    }
	// send response
	RqEnv->GetWebSrv()->SendHttpResp(RqEnv->GetSockId(), HttpResp); 
}

//////////////////////////////////////////////////////////////////////////
// Simple-App-Server
TSAppSrv::TSAppSrv(const int& PortN, const TSAppSrvFunV& SrvFunV, const PNotify& Notify, 
		const bool& _ShowParamP, const bool& _ListFunP): TWebSrv(PortN, true, Notify) {

    ShowParamP = _ShowParamP;
    ListFunP = _ListFunP;
    // initiaize hash-table with mappings
    for (int SrvFunN = 0; SrvFunN < SrvFunV.Len(); SrvFunN++) {
        PSAppSrvFun SrvFun =  SrvFunV[SrvFunN];
        FunNmToFunH.AddDat(SrvFun->GetFunNm(), SrvFun);
    }
}

void TSAppSrv::OnHttpRq(const int& SockId, const PHttpRq& HttpRq) {
	// last appropriate error code, start with bad request
	int ErrStatusCd = THttp::BadRqStatusCd;
    try {
        // check http-request correctness - return if error
        EAssertR(HttpRq->IsOk(), "Bad HTTP request!");
        // check url correctness - return if error
        PUrl RqUrl = HttpRq->GetUrl();
        EAssertR(RqUrl->IsOk(), "Bad request URL!");
        // extract function name
        PUrl HttpRqUrl = HttpRq->GetUrl();
        TStr FunNm = HttpRqUrl->GetPathSeg(0);
		// check if we have the function registered
		if (!FunNm.Empty() && !FunNmToFunH.IsKey(FunNm)) { 
			ErrStatusCd = THttp::ErrNotFoundStatusCd;
			TExcept::Throw("Unknown function '" + FunNm + "'!");
		}
        // extract parameters
        TStrKdV FldNmValPrV;
        PUrlEnv HttpRqUrlEnv = HttpRq->GetUrlEnv();
        const int Keys = HttpRqUrlEnv->GetKeys();
        for (int KeyN = 0; KeyN < Keys; KeyN++) {
            TStr KeyNm = HttpRqUrlEnv->GetKeyNm(KeyN);
            const int Vals = HttpRqUrlEnv->GetVals(KeyN);
            for (int ValN = 0; ValN < Vals; ValN++) {
                TStr Val = HttpRqUrlEnv->GetVal(KeyN, ValN);
                FldNmValPrV.Add(TStrKd(KeyNm, Val));
            }
        }
		// report call
		if (ShowParamP) {  GetNotify()->OnStatus(" " + HttpRq->GetUrl()->GetUrlStr()); }
		// request parsed well, from now on it's internal error
		ErrStatusCd = THttp::InternalErrStatusCd;
		// processed requested function
		if (!FunNm.Empty()) {
			// prepare request environment
			PSAppSrvRqEnv RqEnv = TSAppSrvRqEnv::New(this, SockId, HttpRq);
			// retrieve function
			PSAppSrvFun SrvFun = FunNmToFunH.GetDat(FunNm);
			// call function
			SrvFun->Exec(FldNmValPrV, RqEnv);
		} else {
			// internal SAppSrv call
			if (!ListFunP) {
				// we are not allowed to list functions
				ErrStatusCd = THttp::ErrNotFoundStatusCd;
				TExcept::Throw("Unknow page");
			}
			// prpeare a list of registered functions
			PXmlTok TopTok = TXmlTok::New("registered-functions");
			int KeyId = FunNmToFunH.FFirstKeyId();
			while (FunNmToFunH.FNextKeyId(KeyId)) {
				PXmlTok FunTok = TXmlTok::New("function");
				FunTok->AddArg("name", FunNmToFunH.GetKey(KeyId));
				TopTok->AddSubTok(FunTok);
			}
			TStr ResXmlStr; TXmlDoc::New(TopTok)->SaveStr(ResXmlStr);
			PSIn BodySIn = TMIn::New(TSAppSrvFun::XmlHdStr + ResXmlStr);
			// prepare response
			PHttpResp HttpResp = THttpResp::New(THttp::OkStatusCd, 
				THttp::TextXmlFldVal, false, BodySIn);
			// send response
			SendHttpResp(SockId, HttpResp); 
		}
    } catch (PExcept Except) {
		// known internal error
		PXmlTok TopTok = TXmlTok::New("error");
		TopTok->AddSubTok(TXmlTok::New("message", Except->GetMsgStr()));
		TopTok->AddSubTok(TXmlTok::New("location", Except->GetLocStr()));
		PXmlDoc ErrorXmlDoc = TXmlDoc::New(TopTok); 
        TStr ResXmlStr; ErrorXmlDoc->SaveStr(ResXmlStr);
        // prepare response
		PHttpResp HttpResp = THttpResp::New(ErrStatusCd, 
            THttp::TextHtmlFldVal, false, 
			TMIn::New(TSAppSrvFun::XmlHdStr + ResXmlStr));
        // send response
	    SendHttpResp(SockId, HttpResp); 
    } catch (...) {
		// unknown internal error
		PXmlDoc ErrorXmlDoc = TXmlDoc::New(TXmlTok::New("error")); 
        TStr ResXmlStr; ErrorXmlDoc->SaveStr(ResXmlStr);
        // prepare response
        PHttpResp HttpResp = THttpResp::New(ErrStatusCd, 
            THttp::TextHtmlFldVal, false, 
			TMIn::New(TSAppSrvFun::XmlHdStr + ResXmlStr));
        // send response
	    SendHttpResp(SockId, HttpResp); 
    }
}

//////////////////////////////////////
// File-Download-Function
PSIn TSASFunFPath::ExecSIn(const TStrKdV& FldNmValPrV, 
        const PSAppSrvRqEnv& RqEnv, TStr& ContTypeStr) {

	// construct file name
    TStr FNm = FPath;
    PUrl Url = RqEnv->GetHttpRq()->GetUrl();
    const int PathSegs = Url->GetPathSegs();
    if (PathSegs  == 1) { 
        // nothing specified, do the default
        TStr PathSeg = Url->GetPathSeg(0);
        if (PathSeg.LastCh() != '/') { FNm += "/"; }
        FNm += "index.html"; 
    } else {
        // extract file name
        for (int PathSegN = 1; PathSegN < PathSegs; PathSegN++) {
            FNm += "/"; FNm += Url->GetPathSeg(PathSegN);
        }
    }

    // get mime-type
	TStr FExt = FNm.GetFExt();
	if (FExt == ".htm") { ContTypeStr = THttp::TextHtmlFldVal; }
	else if (FExt == ".html") { ContTypeStr = THttp::TextHtmlFldVal; }
	else if (FExt == ".js") { ContTypeStr = THttp::TextJavaScriptFldVal; }
	else if (FExt == ".css") { ContTypeStr = THttp::TextCssFldVal; }
	else if (FExt == ".png") { ContTypeStr = THttp::ImagePngFldVal; }
	else if (FExt == ".jpg") { ContTypeStr = THttp::ImageJpgFldVal; }
	else if (FExt == ".jpeg") { ContTypeStr = THttp::ImageJpgFldVal; }
	else if (FExt == ".gif") { ContTypeStr = THttp::ImageGifFldVal; }
	else {
		printf("Unknown MIME type for extension '%s' for file '%s'", FExt.CStr(), FNm.CStr());
		ContTypeStr = THttp::AppOctetFldVal; 
	}

    // return stream to the file
    return TFIn::New(FNm);
}