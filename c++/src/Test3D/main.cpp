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

#define UFBX_HEADER_PATH "lib/fbx/ufbx.h"
#define UFBX_REAL_IS_FLOAT
#include "lib/fbx/ufbx.c"

#define UNITYBUILD

// Core
#include "helpers/types.h"
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

#if __GLFW 
#include "helpers/glfw/core.h"
#include "helpers/glfw/input.h"
#include "helpers/glfw/main.h"
#include "helpers/glfw/shaders.h"
#elif __DX11
#include "helpers/dx11/core.h"
#include "helpers/dx11/input.h"
#include "helpers/dx11/main.h"
#include "helpers/dx11/shaders.h"
#endif

#include "helpers/io.h"

#include "helpers/renderer_types.h"
#if __GLFW
#include "helpers/glfw/renderer_types.h"
#elif __DX11
#include "helpers/dx11/renderer_types.h"
#endif

#include "helpers/renderer.h"
#if __GLFW
#include "helpers/glfw/renderer.h"
#elif __DX11
#include "helpers/dx11/renderer.h"
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

namespace Platform {
    void loadLaunchConfig(Platform::WindowConfig& config) {
        // hardcoded for now
        config.window_width = 640 * 1;
        config.window_height = 480 * 1;
        config.fullscreen = false;
        config.title = "3D Test";
    };
    template<>
    void start<Game::Instance>(Game::Instance& game, Platform::GameConfig& config, const Platform::State& platform) {
        Game::start(game, config, platform);
    };
    template<>
    void update<Game::Instance>(Game::Instance& game, Platform::GameConfig& config, const Platform::State& platform) {
        Game::update(game, config, platform);
    };
};

#if __GLFW
int main(int argc, char** argv) {
    int returnValue = 1;
    returnValue = Platform::GLFW::main<Game::Instance>(argc, argv);
    return returnValue;
}
#elif __DX11
int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow) {
    int returnValue = 1;
    returnValue = Platform::main<Game::Instance>(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    return returnValue;
}
#endif
