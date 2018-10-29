// C libs
#include <math.h>
#include <cstring>
#include "stdint.h"
#include <assert.h>

// Debug
//#include <memory>
//#include <cxxabi.h>
//#include "debug/types.h"

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

// Platform-specific
#if PLATFORM_GLFW
#include "helpers/glfw/core.h"
#include "helpers/glfw/input.h"
#include "helpers/glfw/main.h"
#endif

#define __WASTELADNS_DEBUG_TEXT__
#include "helpers/renderer_debug.h"
#include "helpers/renderer.h"

// Game
#include "game.h"

namespace Platform {
    void loadLaunchConfig(Platform::WindowConfig& config) {
        // hardcoded for now
        config.window_width = 640;
        config.window_height = 480;
        config.fullscreen = false;
        config.title = "RPG Test";
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
#include <stdio.h>

#if PLATFORM_GLFW
int main(int argc, char** argv) {

    int returnValue = 1;
    returnValue = Platform::GLFW::main<Game::Instance>(argc, argv);
    
    return returnValue;
}
#endif
