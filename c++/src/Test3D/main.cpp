// C libs
#include <math.h>
#include <cstring>
#include "stdint.h"
#include <assert.h>

#include "lib/tinystl/vector.h"
// Printf
#include <stdio.h>
#if __GLFW
#include <stdarg.h>
#endif

// Debug
//#include <memory>
//#include <cxxabi.h>
//#include "debug/types.h"

#define STB_SPRINTF_IMPLEMENTATION 
#include "lib/stb/stb_sprintf.h"
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb/stb_image.h"

#define UFBX_REAL_IS_FLOAT
#include "lib/fbx/ufbx.c"

#define UNITYBUILD
#define __GPU_DEBUG 0


#if __MACOS || __GLFW
#define __GL 1
#endif

#if __DX11
#include "helpers/dx11/core.h"
#elif __MACOS
#include "helpers/platform_mac/core.h"
#elif __GLFW
#include "helpers/platform_glfw/core.h"
#endif

// Core
#include "helpers/types.h"
#if __DX11
#include "helpers/dx11/input_types.h"
#elif __MACOS
#include "helpers/platform_mac/input_types.h"
#elif __GLFW
#include "helpers/platform_glfw/input_types.h"
#endif


#include "helpers/math.h"
#include "helpers/easing.h"
#include "helpers/vec.h"
#include "helpers/angle.h"
#include "helpers/vec_ops.h"
#include "helpers/transform.h"
#include "helpers/color.h"
#define __WASTELADNS_HASH_DEBUG__
#include "helpers/hash.h"
#include "helpers/input.h"
#include "helpers/platform.h"
#include "helpers/camera.h"
#include "helpers/easing.h"

#if __DX11
#include "helpers/dx11/input.h"
#elif __MACOS
#include "helpers/platform_mac/input.h"
#elif __GLFW
#include "helpers/platform_glfw/input.h"
#endif

#include "helpers/io.h"

#include "helpers/renderer_types.h"
#if __DX11
#include "helpers/dx11/renderer_types.h"
#include "helpers/dx11/shaders.h"
#elif __GL
#include "helpers/gl/renderer_types.h"
#include "helpers/gl/shaders.h"
#endif

#include "helpers/renderer.h"
#if __DX11
#include "helpers/dx11/renderer.h"
#elif __GL
#include "helpers/gl/renderer.h"
#endif

#ifdef NDEBUG
#define __DEBUG 0
#define __DEBUGDEF(a)
#define __DEBUGEXP(a)
#else
#define __DEBUG 1
#define __WASTELADNS_DEBUG_TEXT__
#include "helpers/renderer_debug.h"
#define __DEBUGDEF(a) a
#define __DEBUGEXP(a) \
do { \
  (a); \
} while (0)
#endif

#include "game.h"

#if __DX11
#include "helpers/dx11/main.h"
#elif __MACOS
#include "helpers/platform_mac/main.mm"
#elif __GLFW
#include "helpers/platform_glfw/main.h"
#endif
