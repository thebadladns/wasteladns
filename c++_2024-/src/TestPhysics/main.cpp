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

#include "helpers/core.h"
#include "helpers/math.h"
#include "helpers/allocator.h"

// allocators need to be defined before any libraries are included
#include "libs.h"

#include "helpers/io.h"
#include "helpers/easing.h"
#include "helpers/vec.h"
#include "helpers/angle.h"
#include "helpers/vec_ops.h"
#include "helpers/transform.h"
#include "helpers/color.h"
#include "helpers/bvh.h"
#include "helpers/input/input.h"
#include "helpers/platform.h"
#include "helpers/easing.h"
#include "helpers/gfx/graphics.h"

#if __DEBUG
#include "helpers/immediate_mode.h"
#endif

#include "game.h"
#include "helpers/main.h"