// C libs
#include <math.h>
#include <cstring>
#include "stdint.h"
#include <assert.h>

// Printf
#include <stdio.h>
#if PLATFORM_GLFW
#include <stdarg.h>
#endif

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
#include "helpers/glfw/io.h"

#define __WASTELADNS_DEBUG_TEXT__
#include "helpers/renderer_debug.h"
#include "helpers/renderer.h"

#include "game.h"

namespace Platform {
    void loadLaunchConfig(Platform::WindowConfig& config) {
        // hardcoded for now
        config.window_width = 640;
        config.window_height = 480;
        config.fullscreen = false;
        config.title = "Platform API Test";
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

int main(int argc, char** argv) {
    int returnValue = 1;
    returnValue = Platform::GLFW::main<Game::Instance>(argc, argv);
    return returnValue;
}
#elif PLATFORM_DIRECTX9

#include "helpers/directx9/core.h"
#include "helpers/directx9/input.h"
#include "helpers/directx9/main.h"
#include "helpers/directx9/io.h"

#define __WASTELADNS_DEBUG_TEXT__
#include "helpers/renderer_debug.h"
#include "helpers/renderer.h"

struct Time {
    struct Config {
        f64 targetFramerate;
        f64 maxFrameLength;
    };

    Config config;
    f64 lastFrame;
    f64 lastFrameDelta;
    f64 nextFrame;
    s64 frameCount;
    bool paused;
    float far;
};
struct CameraManager {
    enum { FlyCam, CamCount };
    Camera cameras[CamCount];
    Camera* activeCam;
};
struct Game {
    Time time;
    CameraManager cameraMgr;
    Renderer::Instance renderMgr;
};

void start(Game& game, Platform::GameConfig& config, const Platform::State& platform) {
    game.time = {};
    game.time.config.maxFrameLength = 0.1;
    game.time.config.targetFramerate = 1.0 / 60.0;
    game.time.lastFrame = platform.time.now;

    config = {};
    config.nextFrame = platform.time.now;
    config.requestFlags = Platform::RequestFlags::PollKeyboard;

    game.renderMgr = {};
    {
        Renderer::Instance& mgr = game.renderMgr;
        using namespace Renderer;

        OrthoProjection::Config& ortho = mgr.orthoProjection.config;
        ortho.right = platform.screen.width * 0.5f;
        ortho.top = platform.screen.height * 0.5f;
        ortho.left = -ortho.right;
        ortho.bottom = -ortho.top;
        ortho.near = -1.f;
        ortho.far = 200.f;
        generateMatrix(mgr.orthoProjection.matrix, ortho);

        //PerspProjection::Config& frustum = mgr.perspProjection.config;
        //frustum.fov = 75.0;
        //frustum.aspect = 1.0;
        //frustum.near = 1.0;
        //frustum.far = 1500.0;
        //generateMatrix(mgr.perspProjection.matrix, frustum);

        PerspProjection::Config& frustum = mgr.perspProjection.config;
        frustum.fov = 45.0;
        frustum.aspect = (FLOAT)platform.screen.width / (FLOAT)platform.screen.height;
        frustum.near = 1.0;
        frustum.far = 100.0;
        generateMatrix(mgr.perspProjection.matrix, frustum);
    }

    game.cameraMgr = {};
    {
        CameraManager& mgr = game.cameraMgr;
        using namespace CameraSystem;

        Camera& cam = mgr.cameras[CameraManager::FlyCam];
        Math::identity4x4(cam.transform);
        cam.transform.pos = Vec3(0.f, -10.f, 0.f);
        Vec3 lookAt(0.f, 0.f, 0.f);
        Vec3 lookAtDir = Math::subtract(lookAt, cam.transform.pos);
        Transform33 tmp = Math::fromFront(lookAtDir);
        cam.transform.right = tmp.right;
        cam.transform.front = tmp.front;
        cam.transform.up = tmp.up;
        generateModelViewMatrix(cam.modelviewMatrix, cam.transform);

        mgr.activeCam = &cam;
    }
};
void update(Game& game, Platform::GameConfig& config, const Platform::State& platform) {

    f64 raw_dt = platform.time.now - game.time.lastFrame;
    game.time.lastFrameDelta = Math::min(raw_dt, game.time.config.maxFrameLength);
    game.time.lastFrame = platform.time.now;
    config.nextFrame = platform.time.now + game.time.config.targetFramerate;

    const ::Input::Keyboard::State& keyboard = platform.input.keyboard;
    if (keyboard.released(::Input::Keyboard::Keys::ESCAPE)) {
        config.quit = true;
    }

    OS::printf("[%.3f] running\n", platform.time.running);
};

namespace Platform {
    void loadLaunchConfig(Platform::WindowConfig& config) {
        // hardcoded for now
        config.window_width = 640;
        config.window_height = 480;
        config.fullscreen = false;
        config.title = "Platform API Test";
    };
    template<>
    void start<Game>(Game& game, Platform::GameConfig& config, const Platform::State& platform) {
        ::start(game, config, platform);
    };
    template<>
    void update<Game>(Game& game, Platform::GameConfig& config, const Platform::State& platform) {
        ::update(game, config, platform);
    };
};

int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow) {
    int returnValue = 1;
    returnValue = Platform::DIRECTX9::main<Game>(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    return returnValue;
}

#endif