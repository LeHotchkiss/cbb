#include "utils.h"

using namespace cml;

void ParseWord(FILE* stream, array_t<char>& out) {
    out.Clear();

    int current;

    while ((current = fgetc(stream)) != EOF) {
        if (current == ' ' ||
            current == '\n' ||
            current == '\r' ||
            current == '\t') {
            break;
        }

        out.PushBack(current);
    }

    out.PushBack('\0');
}

string_t FindHeaderPath(CObject pIncludes, const char* sPath, bool& success) {
    FILE* file = NULL;
    string_t path;

    success = false;

    for(size_t i = 0; i < pIncludes.Length(); i++) {
        path.Clear();
        path.PushFormatted("%s/%s", pIncludes.Index(i).AsString(), sPath);

        file = fopen(path.String(), "rb");
        if(file) {
            fclose(file);
            success = true;
            break;
        }
    }

    return path;
}

string_t GetPath(array_t<char>& string, CObject pIncludes, bool& success) {
    char* data = string.Data();

    char* first = strchr(data, '"');

    if (first == NULL) {
        first = strchr(data, '<');
    }

    if (first == NULL) {
        return string_t(data);
    }

    ++first;

    char closing = (data[0] == '<') ? '>' : '"';
    char* second = strchr(first, closing);

    if (second) {
        *second = '\0';
    }

    return FindHeaderPath(pIncludes, first, success);
}

void ScanIncludes(table_t<string_t, bool>& filesReg, CObject pIncludes, const char* sPath, bool isHeader) {
    cml::array_t<char> command;
    cml::stack_t<FILE*> stack;

    FILE* source = fopen(sPath, "rb");
    if (!source) {
        return;
    }

    stack.Push(source);

    while (stack.Length() > 0) {
        FILE* file = stack.Head();
        stack.Pop();

        int current;

        while ((current = fgetc(file)) != EOF) {
            if (current != '#') {
                continue;
            }

            ParseWord(file, command);
            if (strcmp(command.Data(), "include") != 0) {
                continue;
            }

            ParseWord(file, command);

            bool success = false;
            string_t path = GetPath(command, pIncludes, success);
            if (!success) {
                continue;
            }

            path = GetAbsolutePath(path.String());

            if (filesReg.HasKey(path)) {
                continue;
            }

            filesReg.Insert(path, true);

            FILE* next = fopen(path.String(), "rb");
            if (!next) {
                continue;
            }

            stack.Push(next);
        }

        fclose(file);
    }
}
