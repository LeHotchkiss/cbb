#include "output.h"

#include <stdarg.h>
#include <stdio.h>

#include "utils.h"

const char* g_sHelpString = "CBB project builder\n\
Setups working directory automatically if needed.\n\
\n\
Arguments are:\n\
    [-p / --project]    <path-to-cml>   -   specify what project file to read, defaults to 'default.cml'\n\
    [-t / --add-target] <target-name>   -   add said target to the queue\n\
    --add-tabs          <N>             -   prefix all console output line with N tabulations\n\
    [-f / --force]                      -   ignore cache and rebuild forcefully\n\
    [-c / --clear]                      -   clear the cache and exit\n\
    [-a / --all]                        -   add all targets in the project to the queue\n\
    [-v / --version]                    -   print CBB version and exit\n\
    [-h / --help]                       -   print this message and exit\n\
    [-o / --output]     <0-2>           -   set the verbosiry level with 0 being complete silence and 2 violently touching your terminal\n\
    [-i / --no-headers]                 -   don`t check whether the included headers have changed\n\
    [-s / --safe]                       -   ignore the execution hooks";

void PutTabs(FILE* file) {
    for(int i = 0; i < g_cvars.addTabs; i++) {
        putc('\t', file);
    }
}

void Print(ECode iCode, const char* sFormat, ...) {
    const int verbosity = g_cvars.verbosity;
    if (verbosity <= 0 || (int)iCode < 4 - verbosity) {
        //return;
    }

    FILE* pOut = (iCode == ECode::Error) ? stderr : stdout;

    PutTabs(pOut);

    va_list args;
    va_start(args, sFormat);

    if(iCode > ECode::Info) {
        fprintf(pOut, "%s ", (iCode == ECode::Error) ? "[ERROR]" : "[WARNING]");
    }

    vfprintf(pOut, sFormat, args);

    va_end(args);
}
