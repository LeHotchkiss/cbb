#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

#include "hlib/String.h"

#if defined(CBB_LINUX)
    #include <glob.h>
#elif defined(CBB_WINDOWS)

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
    static char buff[128];
    strncpy(buff, g_cvars.projectFile, sizeof(buff));

    for(int i = 0; i < sizeof(buff) - 1; i++) {
        if( buff[i] == '.' || buff[i] == '/' ) {
            buff[i] = '_';
        }
    }

    return snprintf(sBuffer, iBuffSize, "%s_%s.cache", buff, sFile);
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
