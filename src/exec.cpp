#include "utils.h"

#include <stdlib.h>

#include "output.h"

using namespace cml;

bool Exec_Run(CObject args) {
    string_t cmd;

    if(args.Length() == 0) {
        return true;
    }

    #if defined(CBB_LINUX)
        for(size_t i = 0; i < args.Length(); i++) {
            CObject current = args.Index(i);
            if(current.Class() != EObjectClass::String) { continue; }

            cmd.PushFormatted("./%s ", args.Index(i).AsString());
        }
    #else
        static_assert(false, "w*ndows");
    #endif

    Print(ECode::Spam, "%s\n", cmd.String());
    return (system(cmd.String()) == 0);
}

bool Exec_Move(CObject args) {
    string_t cmd;

    if(args.Length() < 2) {
        return false;
    }

    #if defined(CBB_LINUX)
        cmd.PushFormatted("mv ./%s ./%s", args.Index(0).AsString(), args.Index(1).AsString());
    #else
        static_assert(false, "w*ndows");
    #endif

    Print(ECode::Spam, "%s\n", cmd.String());
    return (system(cmd.String()) == 0);
}

bool Exec_Copy(CObject args) {
    string_t cmd;

    if(args.Length() < 2) {
        return false;
    }

    #if defined(CBB_LINUX)
        cmd.PushFormatted("cp ./%s ./%s", args.Index(0).AsString(), args.Index(1).AsString());
    #else
        static_assert(false, "w*ndows");
    #endif

    Print(ECode::Spam, "%s\n", cmd.String());
    return (system(cmd.String()) == 0);
}

bool ExecCommand(ECommand cmd, CObject args) {
    if(g_cvars.safe) {
        return false;
    }

    switch(cmd) {
        case ECommand::Run:     return Exec_Run(args);
        case ECommand::Copy:    return Exec_Copy(args);
        case ECommand::Move:    return Exec_Move(args);

        default:                return false;
    }
}
