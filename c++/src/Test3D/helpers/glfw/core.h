#ifndef __WASTELADNS_CORE_GLFW_H__
#define __WASTELADNS_CORE_GLFW_H__

#define _X86_
#if defined _M_X64 || defined _M_AMD64
#undef _X86_
#define _AMD64_
#endif
#define NOMINMAX
#define GLEXT_64_TYPES_DEFINED
#include "../../lib/glad/glad.c"
#include "GLFW/glfw3.h"

#define consoleLog printf

#undef near
#undef far
#undef DELETE

#endif // __WASTELADNS_CORE_GLFW_H__
