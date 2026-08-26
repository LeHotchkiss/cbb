#include "utils.h"

#include <stdlib.h>

#include "codes.h"
#include "utils.h"

bool TargetDone(const char* sName) {
    if( g_cvars.targetsStatus.HasKey(sName) ) {
        return g_cvars.targetsStatus[sName];
    }

    return false;
}

bool DoTarget(const char* sName, cml::CObject pData) {
    using namespace cml;

    if( TargetDone(sName) ) {
        printf("Target '%s' is already done, skipping...\n", sName);
        return true;
    }

    printf("\nWorking on: '%s'\n", sName);

    CObject pDepends = pData["depends"];

    if(pDepends != NIL) {
        if(pDepends.Class() == EObjectClass::Array) {
            printf("Resolving dependencies...");

            puts((pDepends.Length() > 0) ? "" : " None");

            for( size_t i = 0; i < pDepends.Length(); i++ ) {
                CObject pCurrent = pDepends.Index(i);

                if(pCurrent.Class() != EObjectClass::String) {
                    printf("\tDependency [%lu] is not a string, skipping\n", i);
                    continue;
                }

                const char* sDepName = pCurrent.AsString();

                CObject pTarget = g_cvars.targets[pCurrent.AsString()];

                if(pTarget == NIL) {
                    printf("\tDependency target '%s' is not found, skipping\n", sDepName);
                    continue;
                }

                bool code = DoTarget(pCurrent.AsString(), pTarget);

                if(!code && !g_cvars.ignoreErrors) {
                    printf("Failed to build target\n");
                    exit(RET_TARGET_ERROR);
                }
            }
        }
    }

    printf("Building %s...\n", sName);

    static char s_sCommand[1024];

    CObject pSources = pData["source"];

    if(pSources == NIL) {
        puts("\tNo source files, skipping");
        return true;
    }

    if(pSources.Class() != EObjectClass::Array) {
        puts("\tSource files data is not an array");
        return false;
    }

    cml::array_t<cml::string_t> aFiles;

    for(size_t i = 0; i < pSources.Length(); i++) {
        CObject pCurrent = pSources.Index(i);

        if(pCurrent.Class() != EObjectClass::String) {
            continue;
        }

        aFiles.Clear();
        ResolveFileMask(aFiles, pCurrent.AsString());

        for(size_t j = 0; j < aFiles.Length(); j++) {
            const char* sCurPath = aFiles[j].String();
            printf("\tFILE: '%s'\n", sCurPath);
        }
    }

    g_cvars.targetsStatus[sName] = true;

    return true;
}
