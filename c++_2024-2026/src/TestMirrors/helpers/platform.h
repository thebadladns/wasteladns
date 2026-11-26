#ifndef __WASTELADNS_PLATFORM_H__
#define __WASTELADNS_PLATFORM_H__

namespace platform
{
    struct LaunchConfig {
        const char* title;
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
        f32 window_scale = 1.f;
        bool fullscreen = false;
    };
    
    struct Time {
        f64 running;
        f64 start;
        f64 now;
    };
    
    struct Input {
        ::input::keyboard::State keyboard;
		::input::mouse::State mouse;
        ::input::gamepad::State pads[4];
        u32 padCount;
    };
    
    struct State {
        Screen screen;
        Time time;
        Input input;
    };
    
    void loadLaunchConfig(LaunchConfig& config);
    //void start(_GameData& game, platform::GameConfig& config, platform::State& platform);
    //void update(_GameData& game, platform::GameConfig& config, platform::State& platform);
}

#endif // __WASTELADNS_PLATFORM_H__
