#ifndef __WASTELADNS_IO_DX11_H__
#define __WASTELADNS_IO_DX11_H__

#ifndef UNITYBUILD
#include <stdio.h>
#include "core.h"
#endif

namespace Platform {
    void printf(const char* format, ...) {
        char buffer[256];
        va_list va;
        va_start(va, format);
        vsprintf_s(buffer, format, va);
        va_end(va);
        OutputDebugString(buffer);
    }
    int fopen(FILE **f, const char *name, const char *mode) { return ::fopen_s(f, name, mode); }
    int fclose(FILE *f) { return ::fclose_s(f); }
    int fgetc(FILE* f) { return ::fgetc(f); }
    int fscanf(FILE *f, const char* format, ...) {
        va_list va;
        va_start(va, format);
        int r = ::vfscanf_s(f, format, va);
        va_end(va);
        return r;
    }
}

#endif // __WASTELADNS_IO_DX11_H__
