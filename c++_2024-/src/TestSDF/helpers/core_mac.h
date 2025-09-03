#ifndef __WASTELADNS_CORE_MACOS_H__
#define __WASTELADNS_CORE_MACOS_H__

#define _X86_
#if defined _M_X64 || defined _M_AMD64
#undef _X86_
#define _AMD64_
#endif
#define NOMINMAX
#define GLEXT_64_TYPES_DEFINED

#import <Cocoa/Cocoa.h>
#import <mach/mach_time.h> // for mach_absolute_time
#import <IOKit/hid/IOHIDLib.h>

#define consoleLog(a) printf("%s", a)

namespace platform {

const char* name = "MAC+GL";

void* mem_reserve(size_t size) {
    return mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
}
void mem_commit(void* ptr, size_t size) { /* no-op, OS will commit memory pages as needed */ }
}

#define __popcnt __builtin_popcount

#endif // __WASTELADNS_CORE_MACOS_H__
