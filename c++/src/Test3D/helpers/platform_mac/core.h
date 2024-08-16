#ifndef __WASTELADNS_CORE_MACOS_H__
#define __WASTELADNS_CORE_MACOS_H__

#define _X86_
#if defined _M_X64 || defined _M_AMD64
#undef _X86_
#define _AMD64_
#endif
#define NOMINMAX
#define GLEXT_64_TYPES_DEFINED
#include "../../lib/glad/glad.c"

#import <Cocoa/Cocoa.h>
#import <mach/mach_time.h> // for mach_absolute_time

#define consoleLog printf

namespace Platform {
const char* name = "MACOS";
}

#endif // __WASTELADNS_CORE_MACOS_H__
