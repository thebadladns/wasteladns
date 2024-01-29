#ifndef __WASTELADNS_CORE_DX11_H__
#define __WASTELADNS_CORE_DX11_H__

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
#include <windef.h> // types used by winuser
#include <winuser.h> // api for windows stuff, PeekMessage, CreateWindowEx, etc 
typedef LONG HRESULT; // missing type in some of the includes

// types defined by winuser.h->libloaderapi.h->minwinbase.h,
// that directx will try to define again via oaidl.h->objidl.h->unknwn.h->wtypes.h->wtypesbase.h
#define _SECURITY_ATTRIBUTES_
#define _SYSTEMTIME_
#define BEGIN_INTERFACE
#define END_INTERFACE
#include <basetyps.h> // defines like "interface", DECLARE_INTERFACE and so on are used by dx11

#include <d3d11_1.h>

// runtime shader compilation hack
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#undef near
#undef far
#undef DELETE

#include <profileapi.h> // QueryPerformance funcs
#include <debugapi.h> // OutputDebugString 

// end of of windows shenanigans ----------------------------------------------------------------------

#define consoleLog OutputDebugString

namespace Platform {
    const char* name = "DX11";
}

#endif // __WASTELADNS_CORE_DX11_H__
