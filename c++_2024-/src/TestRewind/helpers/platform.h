#ifndef __WASTELADNS_PLATFORM_H__
#define __WASTELADNS_PLATFORM_H__

#ifndef UNITYBUILD
#include "input.h"
#endif

namespace Platform
{
    struct LaunchConfig {
        const char* title;
        u64 scratchArena_size;
        u32 window_width;
        u32 window_height;
        u32 game_width;
        u32 game_height;
        bool fullscreen;
    };
    
    struct GameConfig {
        f64 nextFrame;
        bool quit;
    };
    
    struct Screen {
        f32 desiredRatio;
        u32 width; // game
        u32 height; // game
        u32 window_width;
        u32 window_height;
        __DEBUGDEF(f32 text_scale = 1.f;)
        bool fullscreen = false;
    };
    
    struct Time {
        f64 running;
        f64 start;
        f64 now;
    };
    
    struct Input {
        ::Input::Keyboard::State keyboard;
		::Input::Mouse::State mouse;
        ::Input::Gamepad::State pads[4];
        u32 padCount;
    };
    
    struct Memory {
        Allocator::Arena scratchArenaRoot;
        __DEBUGDEF(uintptr_t scratchArenaHighmark;)
    };

    struct State {
        Screen screen;
        Time time;
        Input input;
        Memory memory;
    };
    
    void loadLaunchConfig(LaunchConfig& config);
    //void start(_GameData& game, Platform::GameConfig& config, Platform::State& platform);
    //void update(_GameData& game, Platform::GameConfig& config, Platform::State& platform);
}
namespace OS = Platform;

#endif // __WASTELADNS_PLATFORM_H__
