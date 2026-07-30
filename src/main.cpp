#include <stdio.h>
#include <string.h>

#include "hlib/Array.h"
#include "hlib/String.h"

#include "codes.h"

#define CBB_DEFAULT_PROJECT     ("default.cml")
#define CBB_VERSION             ("1.0.0")

struct {
    char* projectFile = (char*)CBB_DEFAULT_PROJECT;
    bool force = false;

    hlib::CArray<hlib::CConstString> targets;
} g_cvars;

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
        }

        // Options requireing an argument

        if(next) {
            if( CMP("-a", "--add-target") ) {
                g_cvars.targets.PushBack(next);
                i++; continue;

            } else if( CMP("-p", "--project-file") ) {
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
    bool result = ParseCMD(argc, argv);
    if(result != true) {
        exit(RET_BAD_ARGS);
    }

    if( g_cvars.targets.Length() > 0 ) {
        puts("Targets: ");
        for(int i = 0; i < g_cvars.targets.Length(); i++) {
            printf("\t%s\n", g_cvars.targets[i]);
        }

    } else {
        printf("Nothing to do\n");
        exit(RET_SUCCESS);
    }

    return RET_SUCCESS;
}
