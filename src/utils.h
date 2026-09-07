#ifndef CBB_UTILS_H
#define CBB_UTILS_H

#define CBB_DEFAULT_PROJECT     ("default.cml")
#define CBB_VERSION             ("1.0.0")

#if defined(__linux__)
    #define CBB_LINUX
#elif defined(_WIN32) || defined(_WIN64)
    #define CBB_WINDOWS
#endif

#include <stddef.h>

#include "hlib/Array.h"
#include "hlib/String.h"

#include "CML.h"

extern struct cbb_state_t {
    char* projectFile   = (char*)CBB_DEFAULT_PROJECT;
    bool force          = false;
    bool buildAll       = false;
    bool clear          = false;
    bool ignoreErrors   = false;
    int addTabs         = 0;
    int verbosity       = 1;

    cml::string_t gccPrefix;
    cml::table_t<hlib::CConstString, bool> targetsStatus;
    cml::array_t<hlib::CConstString> targetsRequest;
    cml::CObject targets;
} g_cvars;

bool SetupProject();
bool ClearCache();

void ParseTarget(const char* sName, cml::CObject target);

size_t GetCacheFileName(const char* sFile, char* sBuffer, size_t iBuffSize);
size_t GetFileModifTime(const char* sFile);
size_t GetFileCachedTime(const char* sFile);
void CacheFile(const char* sFile);

size_t GetObjectFileName(const char* sFile, char* sBuffer, size_t iBuffSize);

void ResolveFileMask(cml::array_t<cml::string_t>& filesArray, const char* sPath);

bool DoTarget(const char* sName, cml::CObject pData);

int SetPWD(const char* sPath);

#endif
