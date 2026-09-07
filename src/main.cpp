#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hlib/Array.h"
#include "hlib/String.h"

#include "CML.h"

#include "codes.h"
#include "utils.h"
#include "output.h"

cbb_state_t g_cvars;

#define CMP(_s, _ls) (strcmp(current, _s) == 0 || strcmp(current, _ls) == 0)

bool ParseCMD(int argc, char** argv) {
    char *current, *next;

    for(int i = 1; i < argc; i++) {
        current = argv[i];
        next = (i < argc-1) ? argv[i+1] : NULL;

        // Options without arguments

        if( CMP("-v", "--version") ) {
            Print(ECode::Info, "CBB v %s\n", CBB_VERSION);
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

        } else if( CMP("-h", "--help") ) {
            Print(ECode::Info, "%s\n", g_sHelpString);
            exit(RET_SUCCESS);
        }
        
        // Options with an argument

        if(next) {
            if( CMP("-t", "--add-target") ) {
                g_cvars.targetsRequest.PushBack(next);
                i++; continue;

            } else if( CMP("-p", "--project") ) {
                g_cvars.projectFile = (char*)next;
                i++; continue;
            } else if( strcmp(current, "--add-tabs") == 0 ) {
                g_cvars.addTabs = atoi(next);
                i++; continue;
            }

        } else {
            Print(ECode::Error, "ERROR: option '%s' requires an argument\n", current);
            return false;
        }
    }

    return true;
}

int main(int argc, char** argv) {
    if(!SetupProject()) {
        Print(ECode::Error, "%s\n", "Failed to setup project directory");
        exit(RET_PROJ_FAIL);
    }

    bool result = ParseCMD(argc, argv);
    if(result != true) {
        exit(RET_BAD_ARGS);
    }

    for(size_t i = 0; i < g_cvars.addTabs; i++) {
        g_cvars.gccPrefix += '\t';
    }

    Print(ECode::Info, "Working with project '%s'\n", g_cvars.projectFile);

    if( g_cvars.clear ) {
        Print(ECode::Info, "Clearing cache... %s\n", ClearCache() ? "Done" : "Failed" );
        exit(RET_SUCCESS);
    }

    if( g_cvars.targetsRequest.Length() > 0 ) {
        g_cvars.targetsRequest.Shrink();
        Print(ECode::Info, "%s\n", "Targets:");
        for(int i = 0; i < g_cvars.targetsRequest.Length(); i++) {
            Print(ECode::Info, "\t+ %s\n", g_cvars.targetsRequest[i]);
        }
        
    } else {
        Print(ECode::Info, "%s\n", "Nothing to do");
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
        Print(ECode::Error, "%s\n", "Project file does not have any targets defined");
        exit(RET_BAD_CML);
    }

    if(targetsTable.Class() != cml::EObjectClass::Object) {
        Print(ECode::Error, "%s\n", "'targets' field must be a table");
        exit(RET_BAD_CML);
    }

    g_cvars.targets = targetsTable;

    for(size_t i = 0; i < g_cvars.targetsRequest.Length(); i++) {
        hlib::CConstString sName = g_cvars.targetsRequest[i];
        cml::CObject target = g_cvars.targets[sName.String()];

        if(target.Class() != cml::EObjectClass::Object) {
            Print(ECode::Error, "Target '%s' is not a table\n", sName);
            exit(RET_BAD_CML);
        }

        if(!DoTarget(sName, target) && !g_cvars.ignoreErrors) {
            Print(ECode::Error, "%s\n", "Failed to build target");
            exit(RET_TARGET_ERROR);
        }
    }

    Print(ECode::Info, "%s\n", "Done");

    return RET_SUCCESS;
}
