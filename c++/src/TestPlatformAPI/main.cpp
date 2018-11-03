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

#if PLATFORM_GLFW

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

#endif

#if PLATFORM_GLFW
int main(int argc, char** argv) {

    int returnValue = 1;
    returnValue = Platform::GLFW::main<Game::Instance>(argc, argv);

    return returnValue;
}
#endif

#if PLATFORM_DIRECTX9

#include <stdio.h>
#define _X86_
#if defined _M_X64 || defined _M_AMD64
#undef _X86_
#define _AMD64_
#endif
#define NOMINMAX
#include <windef.h>
#include <d3d9.h>
#undef near
#undef far
#undef DELETE

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
struct Game {
    Time time;
};

namespace Platform {

    void printf(const char* format, ...) {
        char buffer[256];
        va_list va;
        va_start(va, format);
        vsprintf_s(buffer, format, va);
        va_end(va);
        OutputDebugString(buffer);
    }

}
namespace OS = Platform;

void start(Game& game, Platform::GameConfig& config, const Platform::State& platform) {
    game.time = {};
    game.time.config.maxFrameLength = 0.1;
    game.time.config.targetFramerate = 1.0 / 60.0;
    game.time.lastFrame = platform.time.now;

    config = {};
    config.nextFrame = platform.time.now;
    config.requestFlags = Platform::RequestFlags::PollKeyboard;

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

namespace Input {

namespace Keyboard
{
    // Use an explicit state queue in case message handling needs
    // to be moved outside the frame loop
    struct Queue {
        bool keyStates[Input::Keyboard::Keys::COUNT];
    };
};

namespace Gamepad {

    struct KeyboardMapping {
        Keyboard::Keys::Enum mapping[Keys::COUNT];
    };

    void load(KeyboardMapping& mapping) {
        memset(mapping.mapping, Keyboard::Keys::INVALID, sizeof(s32) * Keys::COUNT);
        // Hardcoded for now
        mapping.mapping[Keys::B_U] = Keyboard::Keys::I;
        mapping.mapping[Keys::B_D] = Keyboard::Keys::K;
        mapping.mapping[Keys::B_L] = Keyboard::Keys::J;
        mapping.mapping[Keys::B_R] = Keyboard::Keys::L;
    }

    void pollState(State& pad, const Keyboard::Queue& queue, const KeyboardMapping& mapping) {
        pad.active = true;
        pad.keys.last = pad.keys.current;
        pad.keys.current = 0;
        for (int i = 0; i < Keys::COUNT; i++) {
            Keyboard::Keys::Enum keyId = mapping.mapping[i];
            if (keyId != Keyboard::Keys::INVALID) {
                bool keyState = queue.keyStates[keyId];
                pad.keys.current = pad.keys.current | ((s32)keyState << i);
            }
        }

        // TODO: handle analog only if requested?
    }
}

namespace Keyboard
{
    struct Mapping {
        ::Input::Keyboard::Keys::Enum mapping[512];
    };
    struct PollData {
        Queue queue;
        Mapping mapping;
    };
    
    void load(Mapping& mapping) {
        memset(mapping.mapping, 0, sizeof(mapping.mapping));
        mapping.mapping[0x00B] = Keys::NUM0;
        mapping.mapping[0x002] = Keys::NUM1;
        mapping.mapping[0x003] = Keys::NUM2;
        mapping.mapping[0x004] = Keys::NUM3;
        mapping.mapping[0x005] = Keys::NUM4;
        mapping.mapping[0x006] = Keys::NUM5;
        mapping.mapping[0x007] = Keys::NUM6;
        mapping.mapping[0x008] = Keys::NUM7;
        mapping.mapping[0x009] = Keys::NUM8;
        mapping.mapping[0x00A] = Keys::NUM9;
        mapping.mapping[0x01E] = Keys::A;
        mapping.mapping[0x030] = Keys::B;
        mapping.mapping[0x02E] = Keys::C;
        mapping.mapping[0x020] = Keys::D;
        mapping.mapping[0x012] = Keys::E;
        mapping.mapping[0x021] = Keys::F;
        mapping.mapping[0x022] = Keys::G;
        mapping.mapping[0x023] = Keys::H;
        mapping.mapping[0x017] = Keys::I;
        mapping.mapping[0x024] = Keys::J;
        mapping.mapping[0x025] = Keys::K;
        mapping.mapping[0x026] = Keys::L;
        mapping.mapping[0x032] = Keys::M;
        mapping.mapping[0x031] = Keys::N;
        mapping.mapping[0x018] = Keys::O;
        mapping.mapping[0x019] = Keys::P;
        mapping.mapping[0x010] = Keys::Q;
        mapping.mapping[0x013] = Keys::R;
        mapping.mapping[0x01F] = Keys::S;
        mapping.mapping[0x014] = Keys::T;
        mapping.mapping[0x016] = Keys::U;
        mapping.mapping[0x02F] = Keys::V;
        mapping.mapping[0x011] = Keys::W;
        mapping.mapping[0x02D] = Keys::X;
        mapping.mapping[0x015] = Keys::Y;
        mapping.mapping[0x02C] = Keys::Z;

        mapping.mapping[0x028] = Keys::APOSTROPHE;
        mapping.mapping[0x02B] = Keys::BACKSLASH;
        mapping.mapping[0x033] = Keys::COMMA;
        mapping.mapping[0x00D] = Keys::EQUAL;
        mapping.mapping[0x029] = Keys::GRAVE_ACCENT;
        mapping.mapping[0x01A] = Keys::LEFT_BRACKET;
        mapping.mapping[0x00C] = Keys::MINUS;
        mapping.mapping[0x034] = Keys::PERIOD;
        mapping.mapping[0x01B] = Keys::RIGHT_BRACKET;
        mapping.mapping[0x027] = Keys::SEMICOLON;
        mapping.mapping[0x035] = Keys::SLASH;
        mapping.mapping[0x056] = Keys::WORLD_2;

        mapping.mapping[0x00E] = Keys::BACKSPACE;
        mapping.mapping[0x153] = Keys::DELETE;
        mapping.mapping[0x14F] = Keys::END;
        mapping.mapping[0x01C] = Keys::ENTER;
        mapping.mapping[0x001] = Keys::ESCAPE;
        mapping.mapping[0x147] = Keys::HOME;
        mapping.mapping[0x152] = Keys::INSERT;
        mapping.mapping[0x15D] = Keys::MENU;
        mapping.mapping[0x151] = Keys::PAGE_DOWN;
        mapping.mapping[0x149] = Keys::PAGE_UP;
        mapping.mapping[0x045] = Keys::PAUSE;
        mapping.mapping[0x146] = Keys::PAUSE;
        mapping.mapping[0x039] = Keys::SPACE;
        mapping.mapping[0x00F] = Keys::TAB;
        mapping.mapping[0x03A] = Keys::CAPS_LOCK;
        mapping.mapping[0x145] = Keys::NUM_LOCK;
        mapping.mapping[0x046] = Keys::SCROLL_LOCK;
        mapping.mapping[0x03B] = Keys::F1;
        mapping.mapping[0x03C] = Keys::F2;
        mapping.mapping[0x03D] = Keys::F3;
        mapping.mapping[0x03E] = Keys::F4;
        mapping.mapping[0x03F] = Keys::F5;
        mapping.mapping[0x040] = Keys::F6;
        mapping.mapping[0x041] = Keys::F7;
        mapping.mapping[0x042] = Keys::F8;
        mapping.mapping[0x043] = Keys::F9;
        mapping.mapping[0x044] = Keys::F10;
        mapping.mapping[0x057] = Keys::F11;
        mapping.mapping[0x058] = Keys::F12;
        mapping.mapping[0x064] = Keys::F13;
        mapping.mapping[0x065] = Keys::F14;
        mapping.mapping[0x066] = Keys::F15;
        mapping.mapping[0x067] = Keys::F16;
        mapping.mapping[0x068] = Keys::F17;
        mapping.mapping[0x069] = Keys::F18;
        mapping.mapping[0x06A] = Keys::F19;
        mapping.mapping[0x06B] = Keys::F20;
        mapping.mapping[0x06C] = Keys::F21;
        mapping.mapping[0x06D] = Keys::F22;
        mapping.mapping[0x06E] = Keys::F23;
        mapping.mapping[0x076] = Keys::F24;
        mapping.mapping[0x038] = Keys::LEFT_ALT;
        mapping.mapping[0x01D] = Keys::LEFT_CONTROL;
        mapping.mapping[0x02A] = Keys::LEFT_SHIFT;
        mapping.mapping[0x15B] = Keys::LEFT_SUPER;
        mapping.mapping[0x137] = Keys::PRINT_SCREEN;
        mapping.mapping[0x138] = Keys::RIGHT_ALT;
        mapping.mapping[0x11D] = Keys::RIGHT_CONTROL;
        mapping.mapping[0x036] = Keys::RIGHT_SHIFT;
        mapping.mapping[0x15C] = Keys::RIGHT_SUPER;
        mapping.mapping[0x150] = Keys::DOWN;
        mapping.mapping[0x14B] = Keys::LEFT;
        mapping.mapping[0x14D] = Keys::RIGHT;
        mapping.mapping[0x148] = Keys::UP;

        mapping.mapping[0x052] = Keys::KP_0;
        mapping.mapping[0x04F] = Keys::KP_1;
        mapping.mapping[0x050] = Keys::KP_2;
        mapping.mapping[0x051] = Keys::KP_3;
        mapping.mapping[0x04B] = Keys::KP_4;
        mapping.mapping[0x04C] = Keys::KP_5;
        mapping.mapping[0x04D] = Keys::KP_6;
        mapping.mapping[0x047] = Keys::KP_7;
        mapping.mapping[0x048] = Keys::KP_8;
        mapping.mapping[0x049] = Keys::KP_9;
        mapping.mapping[0x04E] = Keys::KP_ADD;
        mapping.mapping[0x053] = Keys::KP_DECIMAL;
        mapping.mapping[0x135] = Keys::KP_DIVIDE;
        mapping.mapping[0x11C] = Keys::KP_ENTER;
        mapping.mapping[0x037] = Keys::KP_MULTIPLY;
        mapping.mapping[0x04A] = Keys::KP_SUBTRACT;
    }

    void pollState(State& keyboard, Queue& queue) {
        memcpy(keyboard.last, keyboard.current, sizeof(u8) * Keys::COUNT);
        memset(keyboard.current, 0, sizeof(u8) * Keys::COUNT);
        for (int i = 0; i < Keys::COUNT; i++) {
            keyboard.current[i] = queue.keyStates[i];
        }
    }
};
};

namespace Platform {
    namespace DIRECTX9 {

        LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

            switch (message) {
            case WM_DESTROY: {
                PostQuitMessage(0);
                return 0;
            }
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP: {
                namespace KB = ::Input::Keyboard;
                KB::PollData* keyboardPollData = (KB::PollData*)GetPropA(hWnd, "KeyboardPollData");
                if (keyboardPollData && wParam != VK_CONTROL && wParam != VK_PROCESSKEY) {
                    const KB::Keys::Enum key = keyboardPollData->mapping.mapping[HIWORD(lParam) & 0x1FF];
                    bool state = ((lParam >> 31) & 1) == 0;
                    keyboardPollData->queue.keyStates[key] = state;
                }
            }
            break;
            }
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        template <typename _GameData>
        int main(
              HINSTANCE hInstance
            , HINSTANCE hPrevInstance
            , LPSTR lpCmdLine
            , int nCmdShow
        ) {
            (void)lpCmdLine;
            (void)hPrevInstance;

            HWND hWnd;
            Platform::State platform = {};
            {
                WNDCLASSEX wc;
                ZeroMemory(&wc, sizeof(WNDCLASSEX));
                wc.cbSize = sizeof(WNDCLASSEX);
                // Redraw on horizontal or vertical resize
                wc.style = CS_HREDRAW | CS_VREDRAW;
                wc.lpfnWndProc = WindowProc;
                wc.hInstance = hInstance;
                wc.hCursor = LoadCursor(NULL, IDC_ARROW);
                wc.lpszClassName = "WindowClass";
                RegisterClassEx(&wc);

                Platform::WindowConfig windowConfig;
                loadLaunchConfig(windowConfig);

                hWnd = CreateWindowEx(
                      NULL
                    , "WindowClass"
                    , windowConfig.title
                    , WS_OVERLAPPEDWINDOW
                    , 0, 0, windowConfig.window_width, windowConfig.window_height
                    , NULL /*parent*/
                    , NULL /*menu*/
                    , hInstance
                    , NULL
                );
                if (hWnd == NULL) {
                    return 0;
                }

                ShowWindow(hWnd, nCmdShow);

                platform.screen.width = windowConfig.window_width;
                platform.screen.height = windowConfig.window_height;
                platform.screen.desiredRatio = platform.screen.width / (f32)platform.screen.height;
                platform.screen.fullscreen = windowConfig.fullscreen;
            }

            u64 frequency;
            if (!QueryPerformanceFrequency((LARGE_INTEGER*)&frequency)) {
                return 0;
            }

            LPDIRECT3D9 d3d = Direct3DCreate9(D3D_SDK_VERSION);
            if (d3d == NULL) {
                return 0;
            }

            LPDIRECT3DDEVICE9 d3ddev;
            D3DPRESENT_PARAMETERS d3dpp;
            ZeroMemory(&d3dpp, sizeof(d3dpp));
            d3dpp.Windowed = TRUE; // TODO: support fullscreen
            d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            d3dpp.hDeviceWindow = hWnd;
            d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
            d3dpp.BackBufferWidth = platform.screen.width;
            d3dpp.BackBufferHeight = platform.screen.height;

            d3d->CreateDevice(
                D3DADAPTER_DEFAULT
                , D3DDEVTYPE_HAL
                , hWnd
                , D3DCREATE_SOFTWARE_VERTEXPROCESSING
                , &d3dpp
                , &d3ddev
            );

            if (d3ddev != NULL) {

                ::Input::Gamepad::State pads[1];
                ::Input::Gamepad::KeyboardMapping keyboardPadMappings[1];
                platform.input.padCount = 1;
                platform.input.pads = pads;
                ::Input::Gamepad::load(keyboardPadMappings[0]);

                ::Input::Keyboard::PollData keyboardPollData;
                ::Input::Keyboard::load(keyboardPollData.mapping);
                keyboardPollData.queue = {};
                SetPropA(hWnd, "KeyboardPollData", &keyboardPollData);

                u64 start;
                QueryPerformanceCounter((LARGE_INTEGER*)&start);

                platform.time.running = 0.0;
                platform.time.now = platform.time.start = start / (f64)frequency;

                _GameData game;
                Platform::GameConfig config;
                Platform::start<_GameData>(game, config, platform);

                MSG msg;

                do {
                    if (platform.time.now >= config.nextFrame) {

                        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }

                        // Input
                        if ((config.requestFlags & (Platform::RequestFlags::PollKeyboard)) != 0) {
                            ::Input::Keyboard::pollState(platform.input.keyboard, keyboardPollData.queue);
                        }
                        for (u32 i = 0; i < platform.input.padCount; i++) {
                            ::Input::Gamepad::pollState(platform.input.pads[i], keyboardPollData.queue, keyboardPadMappings[i]);
                        }

                        Platform::update<_GameData>(game, config, platform);
                    
                        // Render update
                        ::Input::Gamepad::State pad = platform.input.pads[0];
                        if (pad.pressed(::Input::Gamepad::Keys::B_U)) {
                            d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0);
                        }
                        else if (pad.released (::Input::Gamepad::Keys::B_U)) {
                            d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 255, 0), 1.0f, 0);
                        }
                        else if (pad.down(::Input::Gamepad::Keys::B_U)) {
                            d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);
                        }
                        else {
                            d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
                        }
                        d3ddev->BeginScene();

                        d3ddev->EndScene();
                        d3ddev->Present(NULL, NULL, NULL, NULL);

                        if (config.quit) {
                            PostQuitMessage(0);
                        }
                    }

                    u64 now;
                    QueryPerformanceCounter((LARGE_INTEGER*)&now);
                    platform.time.now = now / (f64)frequency;
                    platform.time.running = platform.time.now - platform.time.start;
                } while (msg.message != WM_QUIT);


                RemovePropA(hWnd, "KeyboardPollData");
                d3ddev->Release();
            }
            d3d->Release();

            return 1;
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow) {

    int returnValue = 1;
    returnValue = Platform::DIRECTX9::main<Game>(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

    return returnValue;

}

#endif