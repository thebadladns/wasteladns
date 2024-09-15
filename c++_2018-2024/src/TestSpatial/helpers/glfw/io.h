#ifndef __WASTELADNS_IO_GLFW_H__
#define __WASTELADNS_IO_GLFW_H__

#ifndef UNITYBUILD
#include <stdio.h>
#include <stdarg.h>
#endif

namespace Platform {
    void printf(const char* format, ...) {
        va_list va;
        va_start(va, format);
        vprintf(format, va);
        va_end(va);
    }
    int fopen(FILE **f, const char *name, const char *mode) {
        *f = ::fopen(name, mode);
        return *f == 0;
    }
    int fclose(FILE *f) { return ::fclose(f); }
    int fgetc(FILE* f) { return ::fgetc(f); }
    int fscanf(FILE *f, const char* format, ...) {
        va_list va;
        va_start(va, format);
        int r = ::vfscanf(f, format, va);
        va_end(va);
        return r;
    }
}

#endif // __WASTELADNS_IO_GLFW_H__
