#ifndef __WASTELADNS_CORE_WIN64_H__
#define __WASTELADNS_CORE_WIN64_H__

// start of windows shenanigans ----------------------------------------------------------------------

// define our own architecture, since we are not using windows.h
#define NOMINMAX
#define _X86_
#if defined _M_X64 || defined _M_AMD64
#undef _X86_
#define _AMD64_
#endif

#define RPC_NO_WINDOWS_H
#define COM_NO_WINDOWS_H
#include <windef.h> // types used by winuser // Wall time: 101.898ms
#include <winuser.h> // api for windows stuff, PeekMessage, CreateWindowEx, etc // Wall time: 38.148ms

#include <memoryapi.h> // for VirtualAlloc

#if __DX11
    // types defined by winuser.h->libloaderapi.h->minwinbase.h,
    // that directx will try to define again via oaidl.h->objidl.h->unknwn.h->wtypes.h->wtypesbase.h
    #define _SECURITY_ATTRIBUTES_
    #define _SYSTEMTIME_
    #define BEGIN_INTERFACE
    #define END_INTERFACE
    #include <basetyps.h> // defines like "interface", DECLARE_INTERFACE and so on are used by dx11

    #include <d3d11_1.h> // Wall time: 249.939ms
    #pragma comment(lib, "d3d11")

    // runtime shader compilation hack
    #include <d3dcompiler.h> // Wall time: 8.460ms
    #pragma comment(lib, "d3dcompiler.lib")

    // already included by stb_image, does not incur in extra cost
    #include <stdlib.h> // Wall time: 10ms

    namespace platform { const char* name = "WIN64+DX11"; }
#elif __GL33
    #include <wingdi.h> // defines like WINGDIAPI, used by opengl
    #pragma comment(lib, "opengl32.lib")
    #include "../renderer_gl33/loader_gl.h"

    namespace platform { const char* name = "WIN64+GL"; }
#endif

#undef near
#undef far
#undef DELETE

#include <profileapi.h> // QueryPerformance funcs
#include <debugapi.h> // OutputDebugString

// end of of windows shenanigans ----------------------------------------------------------------------

#define consoleLog OutputDebugString

namespace platform {
void* mem_reserve(size_t size) { return VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS); }
void mem_commit(void* ptr, size_t size) { VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE); }
}
#endif // __WASTELADNS_CORE_WIN64_H__
