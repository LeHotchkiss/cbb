#include "utils.h"

#include <stdlib.h>

#include "codes.h"
#include "utils.h"
#include "output.h"

bool TargetDone(const char* sName) {
    if( g_cvars.targetsStatus.HasKey(sName) ) {
        return g_cvars.targetsStatus[sName];
    }

    return false;
}

bool ProduceList(cml::string_t& sCommand, cml::CObject pData, const char* sName, const char* sFormat) {
    using namespace cml;

    CObject pArray = pData[sName];

    if(pArray.Class() != EObjectClass::Array || pArray == NIL) {
        Print(ECode::Error, "'%s' is not an array or does not exist\n", sName);
        return false;
    }

    for(size_t i = 0; i < pArray.Length(); i++) {
        CObject pCurrent = pArray.Index(i);

        if(pCurrent.Class() != EObjectClass::String) {
            continue;
        }

        sCommand.PushFormatted(sFormat, pCurrent.AsString());
    }

    return true;
}

bool ResolveExternalDeps(cml::CObject pData);

bool NeedsCompiling(const char* path) {
    time_t fileTimeCache = GetFileCachedTime(path);
    time_t fileTimeReal = GetFileModifTime(path);

    return (fileTimeCache != fileTimeReal || g_cvars.force);
}

bool CommandWrapper(cml::CObject command) {
    using namespace cml;

    CObject commandType = command["command"];
    if(commandType.Class() != EObjectClass::String) {
        return false;
    }

    CObject commandArgs = command["args"];
    if(commandArgs.Class() != EObjectClass::Array) {
        return false;
    }

    if( strcmp(commandType.AsString(), "run") == 0 ) {
        return ExecCommand(ECommand::Run, commandArgs);
    } else if( strcmp(commandType.AsString(), "move") == 0 ) {
        return ExecCommand(ECommand::Move, commandArgs);
    } else if( strcmp(commandType.AsString(), "copy") == 0 ) {
        return ExecCommand(ECommand::Copy, commandArgs);
    }

    return false;
}

bool DoTarget(const char* sName, cml::CObject pData) {
    using namespace cml;

    if( TargetDone(sName) ) {
        Print(ECode::Spam, "Target '%s' is already done, skipping...\n", sName);
        return true;
    }
    
    bool hasValidExec = false;

    CObject execList = pData["exec"];
    hasValidExec = execList.Class() == EObjectClass::Array;
    int execCounter = 0;

    CObject command = execList.Index(0);

    if(hasValidExec && command && command.Class() == EObjectClass::Object) {
        bool result = CommandWrapper(command);                                  // Pre-building hook command
        execCounter++;

        if(!result) {
            Print(ECode::Spam, "Failed to execute the prebuild hook\n");
        }
    }
    
    Print(ECode::Info, "Working on: '%s'\n", sName);

    Print(ECode::Spam, "Resolving external dependencies...\n");

    if(!ResolveExternalDeps(pData)) {
        return false;
    }

    CObject pDepends = pData["depends"];

    if(pDepends != NIL) {
        if(pDepends.Class() == EObjectClass::Array) {
            Print(ECode::Spam, "%s%s", "Resolving local dependencies...", (pDepends.Length() > 0) ? "\n" : " None\n");

            for( size_t i = 0; i < pDepends.Length(); i++ ) {
                CObject pCurrent = pDepends.Index(i);

                if(pCurrent.Class() != EObjectClass::String) {
                    Print(ECode::Warning, "\tDependency [%lu] is not a string, skipping\n", i);
                    continue;
                }

                const char* sDepName = pCurrent.AsString();

                CObject pTarget = g_cvars.targets[pCurrent.AsString()];

                if(pTarget == NIL) {
                    Print(ECode::Warning, "\tDependency target '%s' is not found, skipping\n", sDepName);
                    continue;
                }

                bool code = DoTarget(pCurrent.AsString(), pTarget);

                if(!code && !g_cvars.ignoreErrors) {
                    Print(ECode::Error, "%s\n", "Failed to build target");
                    exit(RET_TARGET_ERROR);
                }
            }
        }
    }
    
    Print(ECode::Info, "Building %s...\n", sName);

    CObject pSources = pData["source"];

    if(pSources == NIL) {
        Print(ECode::Info, "%s\n", "\tNo source files, skipping");
        return true;
    }

    if(pSources.Class() != EObjectClass::Array) {
        Print(ECode::Error, "%s\n", "\tSource files data is not an array");
        return false;
    }

    array_t<string_t> sourceFiles;
    array_t<string_t> objectFiles;

    CObject pClass = pData["class"];
    const char* targetClass = pClass ? pClass.AsString() : "exe";

    for(size_t i = 0; i < pSources.Length(); i++) {
        CObject pCurrent = pSources.Index(i);

        if(pCurrent.Class() != EObjectClass::String) {
            continue;
        }

        ResolveFileMask(sourceFiles, pCurrent.AsString());
    }

    char sObjBuff[256];

    CObject compiler = pData["compiler"];
    const char* sCompiler = compiler ? compiler.AsString() : "g++";

    table_t<string_t, bool> includesReg;
    table_t<string_t, bool> changedHeaders;

    CObject pIncludes = pData["includes"];

    bool failed = false;

    for(size_t i = 0; i < sourceFiles.Length(); i++) {      // compiling le files
        includesReg.Clear();
        const char* sPath = sourceFiles[i].String();

        int needsCompile = 0;

        ScanIncludes(includesReg, pIncludes, sPath, false);

        for(size_t k = 0; k < includesReg.Length(); k++) {
            const char* headerPath = includesReg.IndexKey(k);

            if (NeedsCompiling(headerPath)) {
                needsCompile++;

                if (!changedHeaders.HasKey(headerPath)) {
                    changedHeaders.Insert(headerPath, true);
                }
            }
        }

        needsCompile += NeedsCompiling(sPath);

        GetObjectFileName(sPath, sObjBuff, sizeof(sObjBuff));
        objectFiles.PushBack(sObjBuff);

        if( needsCompile > 0 || g_cvars.force ) {
            Print(ECode::Spam, "%s", "\n");
            Print(ECode::Info, "\tFile: '%s'\n", sPath);

            bool ok = true;

            string_t command;
            
            command.PushFormatted("%s ", sCompiler);
            command.PushFormatted("-c \"%s\" ", sPath);

            if( strcmp(targetClass, "shared") == 0 ) {
                command.PushFormatted("%s ", "-fPIC");
            }

            ProduceList(command, pData, "cc_args", "%s ");
            ProduceList(command, pData, "includes", "-I\"%s\" ");

            command.PushFormatted("-o \"%s\"", sObjBuff);

            Print(ECode::Spam, "\t\t %s\n", command.String());

            int code = system(command.String());
            ok = code == 0;

            if(ok) {
                CacheFile(sPath);
            } else {
                failed = true;
            }
        }
    }

    if(!failed) {
        for (size_t i = 0; i < changedHeaders.Length(); i++) {
            CacheFile(changedHeaders.IndexKey(i).String());
        }

        CObject postBuildCommand = execList.Index(execCounter);
        if(hasValidExec && postBuildCommand && postBuildCommand.Class() == EObjectClass::Object) {          // Post-compile hook
            bool result = CommandWrapper(postBuildCommand);
            execCounter++;

            if(!result) {
                Print(ECode::Spam, "Failed to execute the post-compile hook\n");
            }
        }
    } else {
        return false;
    }

    Print(ECode::Info, "%s\n", "Linking...");

    CObject pOutput = pData["out"];
    int code = -1;

    if( pOutput.Class() == EObjectClass::String ) {
        const char* sOutput = pOutput.AsString();

        string_t command;

        if( strcmp(targetClass, "static") == 0 ) {
            command = "ar rcs ";
            command.PushFormatted("%s ", sOutput);

            for(size_t i = 0; i < objectFiles.Length(); i++) {
                command.PushFormatted("%s ", objectFiles[i].String());
            }
        } else {
            command.PushFormatted("%s ", sCompiler);

            if( strcmp(targetClass, "shared") == 0 ) {
                command.PushFormatted("%s ", "-shared");
            }

            for(size_t i = 0; i < objectFiles.Length(); i++) {
                command.PushFormatted("%s ", objectFiles[i].String());
            }

            ProduceList(command, pData, "lib", " -L\"%s\"");
            ProduceList(command, pData, "link", " -l:\"%s\"");

            command.PushFormatted(" -o %s", sOutput);
        }

        Print(ECode::Spam, "\t%s\n", command.String());
        code = system(command.String());
    }

    if(code == 0) {
        CObject postLinkCommand = execList.Index(execCounter);
        if(hasValidExec && postLinkCommand && postLinkCommand.Class() == EObjectClass::Object) {
            bool result = CommandWrapper(postLinkCommand);
            execCounter++;

            if(!result) {
                Print(ECode::Spam, "Failed to execute the post-linking hook\n");
            }
        }

        g_cvars.targetsStatus[sName] = true;
        return true;
    } else {
        return false;
    }
}

bool ResolveExternalDeps(cml::CObject pData) {
    using namespace cml;

    CObject pDeps = pData["depends_ex"];

    if(pDeps == NIL || pDeps.Class() != EObjectClass::Array) {
        return true;
    }

    for(size_t i = 0; i < pDeps.Length(); i++) {
        CObject pCurrent = pDeps.Index(i);
        
        if(pCurrent.Class() != EObjectClass::Object) { continue; }

        CObject pPath = pCurrent["path"];
        CObject pTarget = pCurrent["target"];

        if(pPath == NIL || pPath.Class() != EObjectClass::String) { continue; }
        if(pTarget == NIL || pTarget.Class() != EObjectClass::String) { continue; }

        string_t temp = pPath.AsString();

        char* lastSlash = strrchr((char*)temp, '/');

        #if defined(CBB_WINDOWS)
        if(lastSlash == NULL) {
            lastSlash = strrchr((char*)temp, '\\');
        }
        #endif

        char* project = (char*)pPath.AsString();

        int code = 0;

        if(lastSlash != NULL) {
            *lastSlash = '\0';
            project = lastSlash+1;
            code = SetPWD(temp.String());

            if(code != 0) {
                return false;
            }
        }

        string_t command;
        command.PushFormatted("cbb -p \"%s\" -t \"%s\" --add-tabs %d", project, pTarget.AsString(), g_cvars.addTabs+2);

        if(g_cvars.force) {
            command.PushFormatted(" -f");
        }

        Print(ECode::Spam, "\t%s->%s:\n", project, pTarget.AsString());
        Print(ECode::Spam, "\t\t%s\n", command.String());
        
        code = system(command.String());

        if(code != 0) {
            return false;
        }

        if(lastSlash != NULL) {
            code = SetPWD("..");
            if(code != 0) {
                return false;
            }
        }
    }

    return true;
}
