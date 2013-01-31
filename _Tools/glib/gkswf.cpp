/////////////////////////////////////////////////
// Includes
#include "gkswf.h"

int TWfBmpGks::GetEncoderClsid(const TStr& EncoderType, CLSID* pClsid) {
    // convert EncoderType to WCHAR*
    const int StrLen = EncoderType.Len() + 1;
    WCHAR* format = new WCHAR[StrLen];
    const int Res = MultiByteToWideChar(CP_ACP, 0, 
        EncoderType.CStr(), StrLen, format, StrLen);

    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) { return -1; } // Failure

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == NULL) { return -1; } // Failure

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for(UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }    
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

void TWfBmpGks::SaveBmpGks(const TStr& EncoderType, const TStr& FNm) {
    WCHAR* FNmWChar = new WCHAR[FNm.Len() + 1];
    const int Res = MultiByteToWideChar(CP_ACP, 0, 
        FNm.CStr(), FNm.Len() + 1, FNmWChar, FNm.Len() + 1);
    CLSID clsid; GetEncoderClsid(EncoderType, &clsid);
	Gks->EndPaint(); g->ReleaseHDC(HdcHandle);
    Bmp->Save(FNmWChar, &clsid, NULL);
    HdcHandle = g->GetHDC(); Gks->BeginPaint(HdcHandle);
    delete FNmWChar;
}

TWfBmpGks::TWfBmpGks(const int& Width, const int& Height) {
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    Bmp = new Gdiplus::Bitmap(Width, Height);
    g = Gdiplus::Graphics::FromImage(Bmp);
    Gks = TWfGks::New();
    HdcHandle = g->GetHDC(); 
	Gks->BeginPaint(HdcHandle);
}

TWfBmpGks::~TWfBmpGks() {
    Gks->EndPaint(); Gks.Clr();
	g->ReleaseHDC(HdcHandle);
    delete Bmp; delete g;
	Gdiplus::GdiplusShutdown(gdiplusToken);
}
