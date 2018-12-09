#ifndef __WASTELADNS_IO_DIRECTX9_H__
#define __WASTELADNS_IO_DIRECTX9_H__

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
}

#endif // __WASTELADNS_IO_DIRECTX9_H__
