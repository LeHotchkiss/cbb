#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hlib/Array.h"
#include "hlib/String.h"

#include "CML.h"

#include "codes.h"
#include "utils.h"

#define CBB_DEFAULT_PROJECT     ("default.cml")
#define CBB_VERSION             ("1.0.0")

cbb_state_t g_cvars = {
    (char*)CBB_DEFAULT_PROJECT,
    false,
    false,
    false,
    false
};

#define CMP(_s, _ls) (strcmp(current, _s) == 0 || strcmp(current, _ls) == 0)

bool ParseCMD(int argc, char** argv) {
    char *current, *next;

    for(int i = 1; i < argc; i++) {
        current = argv[i];
        next = (i < argc-1) ? argv[i+1] : NULL;

        // Options without arguments

        if( CMP("-v", "--version") ) {
            printf("CBB v %s\n", CBB_VERSION);
            exit(RET_SUCCESS);

        } else if( CMP("-f", "--force") ) {
            g_cvars.force = true;
            continue;

        } else if( CMP("-a", "--all") ) {
            g_cvars.buildAll = true;
            continue;

        } else if( CMP("-c", "--clear") ) {
            g_cvars.clear = true;
            return true;
        }
        
        // Options requireing an argument

        if(next) {
            if( CMP("-t", "--add-target") ) {
                g_cvars.targetsRequest.PushBack(next);
                i++; continue;

            } else if( CMP("-p", "--project") ) {
                g_cvars.projectFile = (char*)next;
                i++; continue;
            }

        } else {
            fprintf(stderr, "ERROR: option '%s' requires an argument\n", current);
            return false;
        }
    }

    return true;
}

int main(int argc, char** argv) {
    if(!SetupProject()) {
        puts("Failed to setup project directory");
        exit(RET_PROJ_FAIL);
    }

    bool result = ParseCMD(argc, argv);
    if(result != true) {
        exit(RET_BAD_ARGS);
    }

    printf("Working with '%s'\n", g_cvars.projectFile);

    if( g_cvars.clear ) {
        printf("Clearing cache... %s\n", ClearCache() ? "Done" : "Failed" );
        exit(RET_SUCCESS);
    }

    if( g_cvars.targetsRequest.Length() > 0 ) {
        g_cvars.targetsRequest.Shrink();
        puts("Targets: ");
        for(int i = 0; i < g_cvars.targetsRequest.Length(); i++) {
            printf("\t+ %s\n", g_cvars.targetsRequest[i]);
        }

    } else {
        printf("Nothing to do\n");
        exit(RET_SUCCESS);
    }

    cml::CTextParser parser;

    cml::ETextError code = parser.Parse(g_cvars.projectFile, true);

    if(code != cml::ETextError::Ok) {
        char buff[64];
        parser.FormatError(code, buff, sizeof(buff));
        puts(buff);
        exit(RET_BAD_CML);
    }

    cml::CObject targetsTable = parser["targets"];

    if(!targetsTable) {
        printf("ERROR: project file does not have any targets defined\n");
        exit(RET_BAD_CML);
    }

    if(targetsTable.Class() != cml::EObjectClass::Object) {
        printf("ERROR: 'targets' field must be a table\n");
        exit(RET_BAD_CML);
    }

    g_cvars.targets = targetsTable;

    for(size_t i = 0; i < g_cvars.targetsRequest.Length(); i++) {
        hlib::CConstString sName = g_cvars.targetsRequest[i];
        cml::CObject target = g_cvars.targets[sName.String()];

        if(target.Class() != cml::EObjectClass::Object) {
            printf("ERROR: target '%s' is not a table\n", sName);
            exit(RET_BAD_CML);
        }

        if(!DoTarget(sName, target) && !g_cvars.ignoreErrors) {
            printf("Failed to build target\n");
            exit(RET_TARGET_ERROR);
        }
    }

    puts("Done");

    return RET_SUCCESS;
}
