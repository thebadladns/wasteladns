// C libs
#include <math.h>
#include <stdlib.h> // rand, mbstowcs_s
#include <stdint.h> // int8_t, int16_t, int32_t, etc
#include <assert.h>
#include <stdio.h> // printf

#define __GPU_DEBUG 0

#ifdef NDEBUG
#define __DEBUG 0
#define __DEBUGDEF(...)
#else
#define __DEBUG 1
#define __DEBUGDEF(...) __VA_ARGS__
#endif

#define __PROFILE 1
#if __PROFILE
#define __PROFILEONLY(...) __VA_ARGS__
#else
#define __PROFILEONLY(...)
#endif

#define DISABLE_INTRINSICS 0

#define WRITE_SHADERCACHE 0
#define READ_SHADERCACHE 0
#if __WIN64
	#if __DX11
		#undef WRITE_SHADERCACHE
		#undef READ_SHADERCACHE
		#define WRITE_SHADERCACHE __DEBUG
		#define READ_SHADERCACHE 1-WRITE_SHADERCACHE
	#endif
#endif

#if __WIN64
	#include "helpers/platform_win/core.h"
#elif __MACOS
	#include "helpers/platform_mac/core.h"
#endif
#include "helpers/types.h"
#include "helpers/math.h"
#include "helpers/allocator.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb/stb_image.h"

#include "lib/stb/stb_easy_font.h"

#define UFBX_REAL_IS_FLOAT
#define UFBX_MINIMAL
#define UFBXI_FEATURE_TRIANGULATION 1
#include "lib/fbx/ufbx.c"


#include "helpers/io.h"
#include "helpers/easing.h"
#include "helpers/vec.h"
#include "helpers/angle.h"
#include "helpers/vec_ops.h"
#include "helpers/transform.h"
#include "helpers/color.h"
#include "helpers/bvh.h"
#if __WIN64
	#include "helpers/platform_win/input_types.h"
#elif __MACOS
	#include "helpers/platform_mac/input_types.h"
#endif
#include "helpers/input.h"
#include "helpers/platform.h"
#include "helpers/easing.h"
#if __WIN64
	#include "helpers/platform_win/input.h"
#elif __MACOS
	#include "helpers/platform_mac/input.h"
#endif
#include "helpers/renderer_types.h"
#if __DX11
	#include "helpers/renderer_dx11/renderer_types.h"
	#include "helpers/renderer_dx11/shaders.h"
#elif __GL33
	#include "helpers/renderer_gl33/renderer_types.h"
	#include "helpers/renderer_gl33/shaders.h"
#endif
#include "helpers/renderer.h"
#if __DX11
	#include "helpers/renderer_dx11/renderer.h"
#elif __GL33
	#include "helpers/renderer_gl33/renderer.h"
#endif
#if __DEBUG
	#include "helpers/renderer_debug.h"
#endif

#include "game.h"
#if __WIN64
	#include "helpers/platform_win/main.h"
#elif __MACOS
	#include "helpers/platform_mac/main.mm"
#endif
