#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hlib/String.h"

#if defined(CBB_LINUX)
    #include <glob.h>
    #include <sys/stat.h>
    #include <unistd.h>
#elif defined(CBB_WINDOWS)
    #include <windows.h>
#endif

bool SetupProject() {
    int code;

    #if defined(CBB_LINUX)
        code =  system( "mkdir -p ./.cbb > /dev/null 2>&1" );
        code += system( "mkdir -p ./.cbb/obj > /dev/null 2>&1" );
    #elif defined(CBB_WINDOWS)
        code =  system( "if not exist \".cbb\" mkdir \".cbb\"" );
        code += system( "if not exist \".cbb\\obj\" mkdir \".cbb\\obj\"" );
    #endif

    return code == 0;
}

bool ClearCache() {
    int code;

    #if defined(CBB_LINUX)
        code = system( "find ./.cbb -name \"*.cache\" -delete" );
    #elif defined(CBB_WINDOWS)
        code = system( "del /q \".cbb/*\" & for /d %I in (\".cbb/*\") do rd /s /q \"%I\"" );
    #endif

    return code == 0;
}

size_t GetCacheFileName(const char* sFile, char* sBuffer, size_t iBuffSize) {
    char buff[128];
    char nameBuff[256];
    
    strncpy(buff, g_cvars.projectFile, sizeof(buff));
    strncpy(nameBuff, sFile, sizeof(nameBuff));

    for(int i = 0; i < sizeof(buff) - 1; i++) {
        if(buff[i] == '\0') {
            break;
        }

        if( buff[i] == '.' || buff[i] == '/' ) {
            buff[i] = '_';
        }
    }

    for(int i = 0; i < sizeof(nameBuff) - 1; i++) {
        if(nameBuff[i] == '\0') {
            break;
        }

        if( nameBuff[i] == '.' || nameBuff[i] == '/' ) {
            nameBuff[i] = '_';
        }
    }

    return snprintf(sBuffer, iBuffSize, ".cbb/%s_%s.cache", buff, nameBuff);
}

void ResolveFileMask(cml::array_t<cml::string_t>& filesArray, const char* sPath) {
    #if defined(CBB_LINUX)
        glob_t data;
        int globFlags = GLOB_NOSORT;

        int code = glob(sPath, globFlags, NULL, &data);

        if(code != 0) {
            return;
        }

        for(size_t i = 0; i < data.gl_pathc; i++) {
            filesArray.PushBack( data.gl_pathv[i] );
        }

        globfree(&data);

    #elif defined(CBB_WINDOWS)

    #endif
}

size_t GetFileModifTime(const char* sFile) {
    #if defined(CBB_LINUX)
        struct stat fileInfo;
        int code = stat(sFile, &fileInfo);

        if(code < 0) {
            return 0;
        }

        return fileInfo.st_mtim.tv_sec;
    #elif defined(CBB_WINDOWS)

    #endif
}

size_t GetFileCachedTime(const char* sFile) {
    char buff[256];

    GetCacheFileName(sFile, buff, sizeof(buff));

    FILE* cache = fopen(buff, "rb");

    if(cache == NULL) {
        return (size_t)(-1);
    }

    fgets(buff, sizeof(buff), cache);

    fclose(cache);

    return strtol(buff, NULL, 10);
}

void CacheFile(const char* sFile) {
    char buff[256];
    GetCacheFileName(sFile, buff, sizeof(buff));

    time_t now = GetFileModifTime(sFile);

    FILE* cache = fopen(buff, "wb");
    if(cache == NULL) { return; }

    fprintf(cache, "%zd\n", now);
    fclose(cache);
}

size_t GetObjectFileName(const char* sFile, char* sBuffer, size_t iBuffSize) {
    char buff[256];
    char nameBuff[128];

    strncpy(buff, sFile, sizeof(buff));
    strncpy(nameBuff, g_cvars.projectFile, sizeof(nameBuff));

    for(size_t i = 0; i < sizeof(buff); i++) {
        if(buff[i] == '\0') {
            break;
        }

        if(buff[i] == '.' || buff[i] == '/') {
            buff[i] = '_';
        }
    }

    for(size_t i = 0; i < sizeof(nameBuff); i++) {
        if(nameBuff[i] == '\0') {
            break;
        }

        if(nameBuff[i] == '.' || nameBuff[i] == '/') {
            nameBuff[i] = '_';
        }
    }

    return snprintf(sBuffer, iBuffSize, ".cbb/obj/%s_%s.o", nameBuff, buff);
}

int SetPWD(const char* sPath) {
    int code;
    char buff[512];
    #if defined(CBB_LINUX)
        snprintf(buff, sizeof(buff), "./%s", sPath);
        code = chdir(buff);
    #elif defined(CBB_WINDOWS)
        snprintf(buff, sizeof(buff), "cd \"%s\"", sPath);
    #endif

    return code;
}
