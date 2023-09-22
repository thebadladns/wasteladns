#ifndef __WASTELADNS_CORE_DX11_H__
#define __WASTELADNS_CORE_DX11_H__

#define _X86_
#if defined _M_X64 || defined _M_AMD64
#undef _X86_
#define _AMD64_
#endif
#define NOMINMAX
#include <windef.h>
#include <d3d11_1.h>

// runtime shader compilation hack
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#undef near
#undef far
#undef DELETE

#define consoleLog OutputDebugString

namespace Platform {
    const char* name = "DX11";
}

#endif // __WASTELADNS_CORE_DX11_H__
