#ifndef __WASTELADNS_CORE_DIRECTX9_H__
#define __WASTELADNS_CORE_DIRECTX9_H__

#define _X86_
#if defined _M_X64 || defined _M_AMD64
#undef _X86_
#define _AMD64_
#endif
#define NOMINMAX
#include <windef.h>
#include <d3d9.h>
#undef near
#undef far
#undef DELETE

#endif // __WASTELADNS_CORE_DIRECTX9_H__
