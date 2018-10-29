#ifndef __WASTELADNS_PLATFORM_H__
#define __WASTELADNS_PLATFORM_H__

#ifndef UNITYBUILD
#include "input.h"
#endif

namespace Platform
{
    struct WindowConfig {
        const char* title;
        u32 window_width;
        u32 window_height;
        bool fullscreen;
    };
    
    struct RequestFlags { enum Enum : u32 {
        PollKeyboard = 1 << 0
    }; };
    
    struct GameConfig {
        f32 nextFrame;
        RequestFlags::Enum requestFlags;
        bool quit;
    };
    
    struct Screen {
        f32 desiredRatio;
        u32 width;
        u32 height;
        bool fullscreen = false;
    };
    
    struct Time {
        f64 running;
        f64 start;
        f64 now;
    };
    
    struct Input {
        ::Input::Keyboard::State keyboard;
        ::Input::Gamepad::State* pads;
        s32 padCount;
    };
    
    struct State {
        Screen screen;
        Time time;
        Input input;
    };
    
    void loadLaunchConfig(WindowConfig& config);
    
    template <typename _GameData>
    void start(_GameData& game, Platform::GameConfig& config, const State& platform);
    
    template <typename _GameData>
    void update(_GameData& game, GameConfig& config, const State& platform);
}

#endif // __WASTELADNS_PLATFORM_H__
