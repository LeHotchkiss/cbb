#ifndef CBB_UTILS_H
#define CBB_UTILS_H

#if defined(__linux__)
    #define CBB_LINUX
#elif defined(_WIN32) || defined(_WIN64)
    #define CBB_WINDOWS
#endif

#include <stddef.h>

#include "hlib/Array.h"
#include "hlib/String.h"

#include "CML.h"

struct cbb_state_t {
    char* projectFile;
    bool force;
    bool buildAll;
    bool clear;
    bool ignoreErrors;

    cml::table_t<hlib::CConstString, bool> targetsStatus;

    cml::array_t<hlib::CConstString> targetsRequest;
    cml::CObject targets;
};
extern cbb_state_t g_cvars;

bool SetupProject();
bool ClearCache();

void ParseTarget(const char* sName, cml::CObject target);

size_t GetCacheFileName(const char* sFile, char* sBuffer, size_t iBuffSize);

void ResolveFileMask(cml::array_t<cml::string_t>& filesArray, const char* sPath);

bool DoTarget(const char* sName, cml::CObject pData);

#endif
