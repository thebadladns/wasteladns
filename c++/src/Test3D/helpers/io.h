#ifndef __WASTELADNS_IO_GLFW_H__
#define __WASTELADNS_IO_GLFW_H__

#ifndef UNITYBUILD
#include <stdio.h>
#include <stdarg.h>
#endif

namespace Platform {
    const auto format = stbsp_snprintf;
    const auto format_va = stbsp_vsnprintf;
#if __DEBUG
    const void debuglog(const char* format, ...) {}
#else
    const void debuglog(const char* format, ...) {
        char text[1024];
        va_list va;
        va_start(va, format);
        Platform::format_va(text, sizeof(text), format, va);
        va_end(va);

        consoleLog(text);
    }
#endif
#if _MSC_VER
    int fopen(FILE** f, const char* name, const char* mode) { return ::fopen_s(f, name, mode); }
#else
    int fopen(FILE** f, const char* name, const char* mode) {
        *f = ::fopen(name, mode);
        return *f == 0;
    }
#endif
    const auto fclose = ::fclose;
    const auto fgetc = ::fgetc;
    const auto fscanf = ::fscanf;
}

#endif // __WASTELADNS_IO_GLFW_H__
