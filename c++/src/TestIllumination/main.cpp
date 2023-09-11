// C libs
#include <math.h>
#include <cstring>
#include "stdint.h"
#include <assert.h>

// Printf
#include <stdio.h>
#if __GLFW
#include <stdarg.h>
#endif

// Debug
//#include <memory>
//#include <cxxabi.h>
//#include "debug/types.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb/stb_image.h"

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
#include "helpers/glfw/io.h"
#include "helpers/glfw/main.h"
#include "helpers/glfw/shaders.h"
#elif __DX11
#include "helpers/dx11/core.h"
#include "helpers/dx11/input.h"
#include "helpers/dx11/io.h"
#include "helpers/dx11/main.h"
#include "helpers/dx11/shaders.h"
#endif

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

#define __WASTELADNS_DEBUG_TEXT__
#include "helpers/renderer_debug.h"


#include "game.h"

namespace Platform {
    void loadLaunchConfig(Platform::WindowConfig& config) {
        // hardcoded for now
        config.window_width = 640 * 1;
        config.window_height = 480 * 1;
        config.fullscreen = false;
        config.title = "Illumination Test";
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
