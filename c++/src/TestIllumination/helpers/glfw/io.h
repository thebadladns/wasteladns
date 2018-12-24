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
}

#endif // __WASTELADNS_IO_GLFW_H__
