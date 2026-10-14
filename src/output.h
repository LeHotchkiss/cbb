#ifndef CBB_OUTPUT_H
#define CBB_OUTPUT_H

extern const char* g_sHelpString;

enum class ECode {
    Spam        = 0,
    Info        = 1,
    Warning     = 2,
    Error       = 3
};

void Print(ECode iCode, const char* sFormat, ...);

#endif
