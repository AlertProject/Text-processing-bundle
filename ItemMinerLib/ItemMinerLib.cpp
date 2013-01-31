#include "ItemMinerLib.h"
#include "base.h"
#include "xmlParsing.h"

#include "Stores.h"
#include "Profile.h"
#include "base/gix.h"
//#include <ontogensrv.h>

class TMailMinerState
{
private:
	THash<TUInt64, char*> AddrToCStrH;
	TInt LastHnd;
	
	TInt LastProfileHnd;
	THash<TInt, PProfile> HndToProfileH;
		
	TInt LastBowDocBsHnd;
	THash<TInt, PBowDocBs> HndToBowDocBsH;
	
	TInt LastBowDocWgtBsHnd;
	THash<TInt, PBowDocWgtBs> HndToBowDocWgtBsH;

	TInt LastGixHnd;
	THash<TInt, PIntIntGix> HndToGixH;

	TInt VerbosityLev;
	UndefCopyAssign(TMailMinerState);

public:

	TMailMinerState()	
	{ 
		//printf("*** Open TTntLibState\n");
	}
	~TMailMinerState()	
	{ 
		// printf("*** Close TTntLibState\n");
	}

	// strings
	char* AddCStr(char* CStr)	
	{
		AddrToCStrH.AddDat(TUInt64(CStr), CStr); return CStr;
	}

	void DelCStr(char* CStr)	
	{
		AddrToCStrH.DelKey(TUInt64(CStr));
	}

	int GetCStrs()	
	{
		return AddrToCStrH.Len();
	}

	void GetCStrV(TStrV& StrV)
	{
		StrV.Gen(GetCStrs(), 0);
		int CStrP=AddrToCStrH.FFirstKeyId();
		while (AddrToCStrH.FNextKeyId(CStrP))
		{
			StrV.Add(AddrToCStrH[CStrP]);
		}
	}

	// profile
	int AddProfile(const PProfile& profile)
	{
		LastProfileHnd = LastHnd++;
		HndToProfileH.AddDat(LastProfileHnd, profile);
		return LastProfileHnd;
	}

	PProfile GetProfile(const int& profileHnd)	
	{
		return HndToProfileH.GetDat(profileHnd);
	}
	
	void DelProfile(const int& profileHnd)	
	{
		HndToProfileH.DelKey(profileHnd);
	}

	// verbosity
	void SetVerbosity(int _VerbosityLev)
	{VerbosityLev=_VerbosityLev;}
	
	int GetVerbosity()
	{return VerbosityLev;}
};

static TMailMinerState* State;

#ifdef WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fwdreason, LPVOID lpvReserved)
{
	switch (fwdreason)
	{
		case DLL_PROCESS_ATTACH:
			//printf("*** MailMinerLib DLL-Process-Attach\n");
			State=new TMailMinerState(); break;
		case DLL_THREAD_ATTACH:
			//printf("*** MailMinerLib DLL-Thread-Attach\n"); 
			break;
		case DLL_THREAD_DETACH:
			//printf("*** MailMinerLib DLL-Thread-Detach\n"); 
			break;
		case DLL_PROCESS_DETACH:
			printf("Destructing ItemMinerLib\n");
			delete State; 
			break;
		default: 
			printf("*** MailMinerLib DLL-Unknown-Entry\n"); 
			break;
	}
	return true;
}
#endif

#ifdef UNIX

// Called when the library is loaded and before dlopen() returns
void my_load(void)
{
    // Add initialization code…
	State=new TMailMinerState();
}

// Called when the library is unloaded and before dlclose()
// returns
void my_unload(void)
{
	printf("Destructing ItemMinerLib\n");
    // Add clean-up code…N
}
#endif

char* CopyStrToCStr(const TStr& Str)
{
	int StrLen=Str.Len();
	char* CStr=new char[StrLen+1];
	strcpy(CStr, Str.CStr());
	return State->AddCStr(CStr);
}

char* CopyStrToCStr(const TChA& ChA)
{
	char* CStr=new char[ChA.Len()+1];
	strcpy(CStr, ChA.CStr());
	return State->AddCStr(CStr);
}

char* CopyCStrToCStr(const char* SrcCStr)
{
	int CStrLen=int(strlen(SrcCStr));
	char* DstCStr=new char[CStrLen+1];
	strcpy(DstCStr, SrcCStr);
	return State->AddCStr(DstCStr);
}

/////////////////////////////////////////////////
// Global
void DelCStr(char* CStr)
{
	if (CStr!=NULL)
	{
		State->DelCStr(CStr);
		delete[] CStr;
	}
}

void SetVerbosity(int VerbosityLev)
{
	State->SetVerbosity(VerbosityLev);
}

int GetVerbosity()
{
	return State->GetVerbosity();
}

/////////////////////////////////////////////////
// Strings
int CStr_GetStrs()
{
	return State->GetCStrs();
}

uint64 GetTmFromStr(int Year, int Month, int Day, int Hour, int Minute, int Seconds)
{
	const int year = Year, month = Month, day = Day, hour = Hour, minute = Minute, seconds = Seconds;
	TTm Tm(year, month, day, -1, hour, minute, seconds);
	return TTm::GetMSecsFromTm(Tm);
}

bool DllValid()
{
	return true;
}

#define MB 1024*1024

int ProfileNew(char* ProfilePath, char* UnicodeDefFile, int MxNGramLen, int MxCachedNGrams, int IndexCacheSizeMB, int ItemCacheSizeMB)
{
	try
	{
		PProfile Profile = TProfile::New(ProfilePath, UnicodeDefFile, MxNGramLen, MxCachedNGrams, IndexCacheSizeMB * MB, ItemCacheSizeMB * MB);
		int ProfileHnd = State->AddProfile(Profile);
		return ProfileHnd;
	}
	catch (PExcept E)
	{
		printf("%s\n", E->GetMsgStr().CStr());
	}
	return -1;
}

int ProfileLoad(char* ProfilePath, char* UnicodeDefFile, int FAccess, int IndexCacheSizeMB, int ItemCacheSizeMB)
{
	try
	{
		PProfile Profile = TProfile::Load(ProfilePath, UnicodeDefFile, TFAccess(FAccess), IndexCacheSizeMB * MB, ItemCacheSizeMB * MB);
		int ProfileHnd = State->AddProfile(Profile);
		return ProfileHnd;
	}
	catch (PExcept E)
	{
		TStr LastInformation = E->GetMsgStr();
	}
	return -1;
}


void ProfileClose(int ProfileHnd)
{
	if (ProfileHnd < 0) 
		return;
	Try;
	State->DelProfile(ProfileHnd);
	Catch;
}

void ClearResults(int ProfileHnd)
{
	if (ProfileHnd < 0) 
		return;
	PProfile Profile = State->GetProfile(ProfileHnd);
	try
	{
		Profile->ClearResults();
	}
	catch (PExcept E)
	{
		Profile->LastInformation = E->GetMsgStr();
	}
}

char* GetLastInformation(int ProfileHnd)
{
	if (ProfileHnd < 0) 
		return CopyStrToCStr(TStr("Invalid Profile Handle"));
	PProfile Profile = State->GetProfile(ProfileHnd);
	return CopyStrToCStr(Profile->LastInformation);
}

char* GetStatus(int ProfileHnd)
{
	if (ProfileHnd < 0) 
		return CopyStrToCStr(TStr("Invalid Profile Handle"));
	PProfile Profile = State->GetProfile(ProfileHnd);
	TChA StatusChA;
	Profile->PrintStatus(StatusChA);
	return CopyStrToCStr(StatusChA);
}


char* AddItem(int ProfileHnd, char* ItemInfo, char* ItemContent)
{
	if (ProfileHnd < 0) 
		return CopyStrToCStr(TStr("Invalid Profile Handle"));
	PProfile Profile = State->GetProfile(ProfileHnd);
	try
	{
		TStr result = Profile->AddItem(ItemInfo, ItemContent);
		return CopyStrToCStr(result);
	}
	catch (PExcept E)
	{
		TStr ret = TProfile::BuildErrorInfo(E->GetMsgStr(), E->GetLocStr());
		Profile->LastInformation = E->GetMsgStr() + "; " + E->GetLocStr();
		return CopyStrToCStr(ret);
	}
}

char* UpdateItem(int ProfileHnd, char* ItemInfo, char* ItemContent)
{
	if (ProfileHnd < 0) 
		return CopyStrToCStr(TStr("Invalid Profile Handle"));
	PProfile Profile = State->GetProfile(ProfileHnd);
	try
	{
		TStr result = Profile->UpdateItem(ItemInfo, ItemContent);
		return CopyStrToCStr(result);
	}
	catch (PExcept E)
	{
		TStr ret = TProfile::BuildErrorInfo(E->GetMsgStr(), E->GetLocStr());
		Profile->LastInformation = E->GetMsgStr() + "; " + E->GetLocStr();
		return CopyStrToCStr(ret);
	}
}


void RemoveItem(int ProfileHnd, int ItemId)
{
	if (ProfileHnd < 0)
		return;
	PProfile Profile = State->GetProfile(ProfileHnd);
	try
	{
		Profile->RemoveItem(ItemId);
	}
	catch (PExcept E)
	{
		Profile->LastInformation = E->GetMsgStr();
	}
}

void RemoveItems(int ProfileHnd, char* RemoveContent)
{
	if (ProfileHnd < 0) 
		return;
	PProfile Profile = State->GetProfile(ProfileHnd);
	try
	{
		PXmlDoc ItemDataXml = TXmlDoc::LoadStr(RemoveContent);
		if (!ItemDataXml->IsOk())
		{
			Profile->LastInformation = "Invalid XML data";
			return;
		}

		TStr ItemIdsStr = ItemDataXml->GetTagTok("removeItems")->GetTokStr(false);
		TStrV ItemIdsStrV;
		ItemIdsStr.SplitOnAllCh(',', ItemIdsStrV);
		for (int i=0; i < ItemIdsStrV.Len(); i++)
			Profile->RemoveItem(ItemIdsStrV[i].GetInt(-1));
	}
	catch (PExcept E)
	{
		Profile->LastInformation = E->GetMsgStr();
	}
}

void SetTag(int ProfileHnd, int ItemId, char* TagId)
{
	if (ProfileHnd < 0) 
		return;
	PProfile Profile = State->GetProfile(ProfileHnd);
	try
	{
		Profile->SetTag(ItemId, TagId);
	}
	catch (PExcept E)
	{
		Profile->LastInformation = E->GetMsgStr();
	}
}

void RemoveTag(int ProfileHnd, int ItemId, char* TagId)
{
	if (ProfileHnd < 0) 
		return;
	PProfile Profile = State->GetProfile(ProfileHnd);
	try
	{
		Profile->RemoveTag(ItemId, TagId);
	}
	catch (PExcept E)
	{
		Profile->LastInformation = E->GetMsgStr();
	}
}

char* Query(int ProfileHnd, char* QueryInfo)
{
	if (ProfileHnd < 0) 
		return CopyStrToCStr(TStr("Invalid Profile Handle"));
	PProfile Profile = State->GetProfile(ProfileHnd);
	try
	{
		PXmlDoc QueryXml = TXmlDoc::LoadStr(QueryInfo);
		if (QueryXml->IsOk())
		{
			PXmlTok QueryTok = QueryXml->GetTagTok("query");
			TStr QueryType = QueryTok->GetStrArgVal("type", "");
			if (QueryType == "generalQuery")
			{
				TStr result = Profile->GeneralQuery(QueryXml);
				return CopyStrToCStr(result);
			}
			else if (QueryType == "customQuery")
			{
				TStr result = Profile->CustomQuery(QueryXml);
				return CopyStrToCStr(result);
			}
		}
		else
			Profile->LastInformation = "Invalid XML. " + QueryXml->GetMsgStr();
	}
	catch (PExcept E)
	{
		Profile->LastInformation = E->GetMsgStr();
	}
	catch (...)
	{
		Profile->LastInformation = "Undefined exception while processing a query";
	}

	TStr err("");
	return CopyStrToCStr(err);
}

bool SetTagData(int ProfileHnd, char *TagData)
{
	if (ProfileHnd < 0) 
		return false;
	PProfile Profile = State->GetProfile(ProfileHnd);
	try
	{
		return Profile->SetTagData(TagData);
	}
	catch (PExcept E)
	{
		Profile->LastInformation = E->GetMsgStr();
		return false;
	}
}


char* GetTagData(int ProfileHnd, char* TagData)
{
	if (ProfileHnd < 0) 
		return CopyStrToCStr(TStr("Invalid Profile Handle"));
	PProfile Profile = State->GetProfile(ProfileHnd);
	try
	{
		TStr result = Profile->GetTagData(TagData);
		return CopyStrToCStr(result);
	}
	catch (PExcept E)
	{
		Profile->LastInformation = E->GetMsgStr();
		return CopyStrToCStr(TStr(""));
	}
}


char* ExecuteCommand(int ProfileHnd, char* Command)
{
	if (ProfileHnd < 0) 
		return CopyStrToCStr(TStr("Invalid Profile Handle"));
	PProfile Profile = State->GetProfile(ProfileHnd);
	try
	{
		TStr result = Profile->ExecuteCommand(Command);
		return CopyStrToCStr(result);
	}
	catch (PExcept E)
	{
		Profile->LastInformation = E->GetMsgStr();
		return CopyStrToCStr(TStr(""));
	}
}

char* TokenizeText(int ProfileHnd, char* Text)
{
	if (ProfileHnd < 0) 
		return CopyStrToCStr(TStr("Invalid Profile Handle"));
	PProfile Profile = State->GetProfile(ProfileHnd);
	try
	{
		TStr result = Profile->TokenizeText(Text);
		return CopyStrToCStr(result);
	}
	catch (PExcept E)
	{
		Profile->LastInformation = E->GetMsgStr();
	}
	return false;
}

void UpdateSettings(int ProfileHnd, char* Settings)
{
	if (ProfileHnd < 0) 
		return;
	PProfile Profile = State->GetProfile(ProfileHnd);
	Profile->UpdateSettings(Settings);
}
