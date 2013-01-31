#include "base.h"

#ifdef WIN32
#define DLLExport extern "C" __declspec(dllexport)
#endif 

#ifdef UNIX
#define DLLExport extern "C"
void __attribute__ ((constructor)) my_load(void);
void __attribute__ ((destructor)) my_unload(void);
#endif

typedef int hnd;

typedef TGix<TInt, TInt> TIntIntGix;
typedef TPt<TIntIntGix> PIntIntGix;

/////////////////////////////////////////////////
// Global
DLLExport void DelCStr(char* CStr);

DLLExport void SetVerbosity(int VerbosityLev);
DLLExport int GetVerbosity();

// Testing
DLLExport bool DllValid();

/////////////////////////////////////////////////
// Strings
DLLExport int CStr_GetStrs();

DLLExport uint64 GetTmFromStr(int Year, int Month, int Day, int Hour, int Minute, int Seconds);

// profiles
DLLExport int ProfileNew(char* ProfilePath, char* UnicodeDefFile, int MxNGramLen, int MxCachedNGrams, int IndexCacheSizeMB, int ItemCacheSizeMB);
DLLExport int ProfileLoad(char* ProfilePath, char* UnicodeDefFile, int FAccess, int IndexCacheSizeMB, int ItemCacheSizeMB);
DLLExport void ProfileClose(int ProfileHnd);

// query results
DLLExport void ClearResults(int ProfileHnd);

// items
DLLExport char* AddItem(int ProfileHnd, char* ItemInfo, char* ItemContent);
DLLExport char* UpdateItem(int ProfileHnd, char* ItemInfo, char* ItemContent);
DLLExport void RemoveItem(int ProfileHnd, int ItemId);
DLLExport void RemoveItems(int ProfileHnd, char* RemoveContent);

// tags
DLLExport void SetTag(int ProfileHnd, int ItemId, char* TagId);
DLLExport void RemoveTag(int ProfileHnd, int ItemId, char* TagId);

// commands
//DLLExport char* GetTopWords(int ProfileHnd, int KeywordCount, bool GroupByThreads, int MaxNGramLen, int MinNGramFq);
DLLExport char* ExecuteCommand(int ProfileHnd, char* Command);
DLLExport char* TokenizeText(int ProfileHnd, char* Text);

// bag-of-words
DLLExport hnd GetBowDocBsHnd(int ProfileHnd, int MxNGramLen, int MnNGramFq);
DLLExport hnd GetBowDocWgtBsHnd(int ProfileHnd, int BowDocBsHnd);

// query
DLLExport char* Query(int ProfileHnd, char* QueryInfo);

DLLExport bool SetTagData(int ProfileHnd, char* TagData);
DLLExport char* GetTagData(int ProfileHnd, char* TagData);

DLLExport char* GetLastInformation(int ProfileHnd);
DLLExport char* GetStatus(int ProfileHnd);
DLLExport void UpdateSettings(int ProfileHnd, char* Settings);
//DLLExport char* GetSimilarityBetweenThreads(int ProfileHnd);