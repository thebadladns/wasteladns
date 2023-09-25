// C libs
#include <math.h>
#include <cstring>
#include "stdint.h"
#include <assert.h>

#include <vector>
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

#if __DX11
#include "helpers/dx11/core.h"
#elif __MACOS
#include "helpers/macos/core.h"
#elif __GLFW
#include "helpers/glfw/core.h"
#endif

// Core
#include "helpers/types.h"
#if __DX11
#include "helpers/dx11/input_types.h"
#elif __MACOS
#include "helpers/macos/input_types.h"
#elif __GLFW
#include "helpers/glfw/input_types.h"
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
#include "helpers/dx11/shaders.h"
#elif __MACOS
#include "helpers/macos/input.h"
#include "helpers/macos/shaders.h"
#elif __GLFW
#include "helpers/glfw/input.h"
#include "helpers/glfw/shaders.h"
#endif

#include "helpers/io.h"

#include "helpers/renderer_types.h"
#if __DX11
#include "helpers/dx11/renderer_types.h"
#elif __MACOS
#include "helpers/macos/renderer_types.h"
#elif __GLFW
#include "helpers/glfw/renderer_types.h"
#endif

#include "helpers/renderer.h"
#if __DX11
#include "helpers/dx11/renderer.h"
#elif __MACOS
#include "helpers/macos/renderer.h"
#elif __GLFW
#include "helpers/glfw/renderer.h"
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
#include "helpers/macos/main.mm"
#elif __GLFW
#include "helpers/glfw/main.h"
#endif
