#ifndef CBB_OUTPUT_H
#define CBB_OUTPUT_H

extern const char* g_sHelpString;

enum class ECode {
    Info        = 0,
    Warning     = 1,
    Error       = 2
};

void Print(ECode iCode, const char* sFormat, ...);

#endif
