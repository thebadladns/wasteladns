#ifndef UNITYBUILD
#include "../platform.h"
#include "core.h"
#include "input.h"
#include "game.h"
#endif

//void DI_log(HRESULT hr) {
//    Platform::debuglog("STATUS: ");
//
//    switch (hr) {
//    case DI_DOWNLOADSKIPPED: Platform::debuglog("DI_DOWNLOADSKIPPED"); break;
//    case DI_EFFECTRESTARTED: Platform::debuglog("DI_EFFECTRESTARTED"); break;
//    case DI_NOEFFECT: Platform::debuglog("DI_NOEFFECT or DI_BUFFEROVERFLOW or DI_NOTATTACHED or DI_PROPNOEFFECT"); break;
//    case DI_OK: Platform::debuglog("DI_OK"); break;
//    case DI_POLLEDDEVICE: Platform::debuglog("DI_POLLEDDEVICE"); break;
//    case DI_SETTINGSNOTSAVED: Platform::debuglog("DI_SETTINGSNOTSAVED"); break;
//    case DI_TRUNCATED: Platform::debuglog("DI_TRUNCATED"); break;
//    case DI_TRUNCATEDANDRESTARTED: Platform::debuglog("DI_TRUNCATEDANDRESTARTED"); break;
//    case DI_WRITEPROTECT: Platform::debuglog("DI_WRITEPROTECT"); break;
//    case DIERR_ACQUIRED: Platform::debuglog("DIERR_ACQUIRED"); break;
//    case DIERR_ALREADYINITIALIZED: Platform::debuglog("DIERR_ALREADYINITIALIZED"); break;
//    case DIERR_BADDRIVERVER: Platform::debuglog("DIERR_BADDRIVERVER"); break;
//    case DIERR_BETADIRECTINPUTVERSION: Platform::debuglog("DIERR_BETADIRECTINPUTVERSION"); break;
//    case DIERR_DEVICENOTREG: Platform::debuglog("DIERR_DEVICENOTREG"); break;
//    case DIERR_GENERIC: Platform::debuglog("DIERR_GENERIC"); break;
//    case DIERR_HANDLEEXISTS: Platform::debuglog("DIERR_HANDLEEXISTS OR DIERR_READONLY"); break;
//    case DIERR_INPUTLOST: Platform::debuglog("DIERR_INPUTLOST"); break;
//    case DIERR_INVALIDPARAM: Platform::debuglog("DIERR_INVALIDPARAM"); break;
//    case DIERR_NOAGGREGATION: Platform::debuglog("DIERR_NOAGGREGATION"); break;
//    case DIERR_NOINTERFACE: Platform::debuglog("DIERR_NOINTERFACE"); break;
//    case DIERR_NOTACQUIRED: Platform::debuglog("DIERR_NOTACQUIRED"); break;
//    case DIERR_NOTFOUND: Platform::debuglog("DIERR_NOTFOUND or DIERR_OBJECTNOTFOUND"); break;
//    case DIERR_NOTINITIALIZED: Platform::debuglog("DIERR_NOTINITIALIZED"); break;
//    case DIERR_OLDDIRECTINPUTVERSION: Platform::debuglog("DIERR_OLDDIRECTINPUTVERSION"); break;
//    case DIERR_OUTOFMEMORY: Platform::debuglog("DIERR_OUTOFMEMORY"); break;
//    case DIERR_UNSUPPORTED: Platform::debuglog("DIERR_UNSUPPORTED"); break;
//    case E_HANDLE: Platform::debuglog("E_HANDLE"); break;
//    case E_PENDING: Platform::debuglog("E_PENDING"); break;
//    case E_POINTER: Platform::debuglog("E_POINTER"); break;
//    }
//    Platform::debuglog("\n");
//}
//
//struct PadEnumContext {
//    ::Input::Gamepad::State* pads;
//    u32 maxpads;
//    u32 padcount;
//    LPDIRECTINPUT8 di;
//    DIJOYCONFIG* preferredJoyConfig;
//};
//BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* user) {
//
//    PadEnumContext& ctx = *(PadEnumContext*)user;
//    Platform::debuglog("Enumerating pad %s\n", pdidInstance->tszProductName);
//
//    u32 padindex = ctx.padcount;
//    // make room at index 0 if we have a preferred joystick
//    if (ctx.padcount > 0 && ctx.preferredJoyConfig && IsEqualGUID(pdidInstance->guidInstance, ctx.preferredJoyConfig->guidInstance)) {
//        memcpy(&(ctx.pads[ctx.padcount]), &(ctx.pads[0]), sizeof(ctx.pads[0]));
//        padindex = 0;
//    }
//
//    ::Input::Gamepad::State& pad = ctx.pads[padindex];
//    HRESULT hr = ctx.di->CreateDevice(pdidInstance->guidInstance, &pad.device, nullptr);
//    strncpy_s(pad.name, pdidInstance->tszProductName, sizeof(pad.name));
//
//    ctx.padcount++;
//
//    Platform::debuglog("Created pad\nProduct: %s\nInstance: %s\n", pdidInstance->tszProductName, pdidInstance->tszInstanceName);
//    DI_log(hr);
//
//    if (ctx.padcount >= ctx.maxpads) {
//        return DIENUM_STOP;
//    }
//
//    return DIENUM_CONTINUE;
//}
//
//struct JoystickObjectsEnumContext {
//    LPDIRECTINPUTDEVICE8 joydevice;
//};
//BOOL CALLBACK EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* user) {
//
//    JoystickObjectsEnumContext& ctx = *(JoystickObjectsEnumContext*)user;
//
//    if (pdidoi->dwType & DIDFT_AXIS) {
//        DIPROPRANGE diprg;
//        diprg.diph.dwSize = sizeof(DIPROPRANGE);
//        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
//        diprg.diph.dwHow = DIPH_BYID;
//        diprg.diph.dwObj = pdidoi->dwType; // Specify the enumerated axis
//        diprg.lMin = -1000;
//        diprg.lMax = +1000;
//
//        // Set the range for the axis
//        if (FAILED(ctx.joydevice->SetProperty(DIPROP_RANGE, &diprg.diph)))
//            return DIENUM_STOP;
//    }
//    return DIENUM_CONTINUE;
//}
//
//void checkJoystick(Input::Gamepad::State& state) {
//    namespace Pad = Input::Gamepad;
//
//    if (!state.device)
//        return;
//    HRESULT hr;
//    // Poll the device to read the current state
//    Platform::debuglog("Polling\n");
//    hr = state.device->Poll(); // todo: handle non-polling sticks
//    DI_log(hr);
//    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
//        Platform::debuglog("Reacquiring\n");
//        hr = state.device->Acquire();
//        DI_log(hr);
//
//        if (hr == DIERR_HANDLEEXISTS) {
//            Platform::debuglog("Unacquiring\n");
//            hr = state.device->Unacquire();
//            DI_log(hr);
//            Platform::debuglog("And reacquiring\n");
//            hr = state.device->Acquire();
//            DI_log(hr);
//        } 
//
//        if (SUCCEEDED(hr)) {
//            Platform::debuglog("Repolling\n");
//            hr = state.device->Poll();
//            DI_log(hr);
//        }
//    }
//
//    //DIJOYSTATE2 js;
//    //HRESULT hr = ctx.joydevice->GetDeviceState(sizeof(DIJOYSTATE2), &js);
//    //if (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST)
//    //{
//    //    // DInput is telling us that the input stream has been
//    //    // interrupted. We aren't tracking any state between polls, so
//    //    // we don't have any special reset that needs to be done. We
//    //    // just re-acquire and try again.
//    //    ctx.joydevice->Acquire();
//    //    ctx.joydevice->Poll();
//    //    hr = ctx.joydevice->GetDeviceState(sizeof(DIJOYSTATE2), &js);
//
//    //    // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
//    //    // may occur when the app is minimized or in the process of 
//    //    // switching, so just try again later 
//    //    //return;
//    //}
//
//    //if (FAILED(hr))
//    //    return; // The device should have been acquired during the Poll()
//
//    Platform::debuglog("Getting state\n");
//    DIJOYSTATE2 js;
//    hr = state.device->GetDeviceState(sizeof(DIJOYSTATE2), &js);
//    DI_log(hr);
//
//    state.sliders[Pad::Sliders::AXIS_X_LEFT] = js.lX / 1000.f;
//    state.sliders[Pad::Sliders::AXIS_Y_LEFT] = -js.lY / 1000.f;
//    state.sliders[Pad::Sliders::AXIS_X_RIGHT] = js.lZ / 1000.f;
//    state.sliders[Pad::Sliders::AXIS_Y_RIGHT] = -js.lRz / 1000.f;
//    state.sliders[Pad::Sliders::TRIGGER_LEFT] = 0.5f * (1.f + js.lRx / 1000.f);
//    state.sliders[Pad::Sliders::TRIGGER_RIGHT] = 0.5f * (1.f + js.lRy / 1000.f);
//    for (u16 key = 0; key < Pad::Keys::COUNT; key++) {
//        // todo
//    }
//
//    //// Axes
//    //Platform::debuglog("lX: %ld ", js.lX);
//    //Platform::debuglog("lY: %ld ", js.lY);
//    //Platform::debuglog("lZ: %ld ", js.lZ);
//    //Platform::debuglog("lRx: %ld ", js.lRx);
//    //Platform::debuglog("lRy: %ld ", js.lRy);
//    //Platform::debuglog("lRz: %ld ", js.lRz);
//
//    //// Slider controls
//    //Platform::debuglog("slider0: %ld ", js.rglSlider[0]);
//    //Platform::debuglog("slider1: %ld ", js.rglSlider[1]);
//
//    //// Points of view
//    //Platform::debuglog("POV0: %lu ", js.rgdwPOV[0]);
//    //Platform::debuglog("POV1: %lu ", js.rgdwPOV[1]);
//    //Platform::debuglog("POV2: %lu ", js.rgdwPOV[2]);
//    //Platform::debuglog("POV3: %lu ", js.rgdwPOV[3]);
//
//    // Fill up text with which buttons are pressed
//    // Display joystick state to dialog
//    u16 keys = 0;
//    //char strText[512] = {};
//    //Platform::format(strText, sizeof(strText), "buttons: ");
//    size_t button_count = Math::min((size_t)Pad::Keys::COUNT, sizeof(js.rgbButtons));
//    for (size_t i = 0; i < button_count; i++) {
//        if (js.rgbButtons[i] & 0x80) {
//
//            keys |= 1 << i;
//            //Platform::format(sz, sizeof(sz), "%02d ", i);
//            //Platform::format(strText, sizeof(strText), "%s%s", strText, sz);
//        }
//    }
//    state.last_keys = state.curr_keys;
//    state.curr_keys = keys;
//
//    //Platform::debuglog("%s", strText);
//    //Platform::debuglog("\\\\\\\\\\\\\\\\\n", strText);
//}
//
//u32 init_dinput_pads(HWND hWnd, ::Input::Gamepad::State* pads, u32 maxpads) {
//    LPDIRECTINPUT8 di = nullptr;
//    IDirectInputJoyConfig8* g_pJoystick = nullptr;
//
//    DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&di, nullptr);
//    di->QueryInterface(IID_IDirectInputJoyConfig8, (void**)&g_pJoystick);
//
//    PadEnumContext enumContext = {};
//    enumContext.di = di;
//
//    DIJOYCONFIG preferredJoyConfig = {};
//    preferredJoyConfig.dwSize = sizeof(preferredJoyConfig);
//    if SUCCEEDED((g_pJoystick->GetConfig(0, &preferredJoyConfig, DIJC_GUIDINSTANCE))) {
//        enumContext.preferredJoyConfig = &preferredJoyConfig;
//    }
//
//    enumContext.pads = pads;
//    enumContext.maxpads = maxpads;
//    di->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, &enumContext, DIEDFL_ATTACHEDONLY);
//    for (u32 i = 0; i < enumContext.padcount; i++) {
//
//        HRESULT hr;
//
//        LPDIRECTINPUTDEVICE8 joyDevice = enumContext.pads[i].device;
//        joyDevice->SetDataFormat(&c_dfDIJoystick2);
//        //if (joyDevice->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND) != DI_OK) { // this will make Acquire break when not in focus
//        if (joyDevice->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK) {
//            joyDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND);
//        }
//
//        //DIDEVCAPS devcaps = {};
//        //devcaps.dwSize = sizeof(DIDEVCAPS);
//        //hr = joyDevice->GetCapabilities(&devcaps);
//        //DI_log(hr);
//
//        JoystickObjectsEnumContext enumObjectsContext = {};
//        enumObjectsContext.joydevice = joyDevice;
//        joyDevice->EnumObjects(EnumObjectsCallback, (VOID*)&enumObjectsContext, DIDFT_ALL);
//
//        hr = pads[i].device->Acquire();
//        Platform::debuglog("Acquired pad %s\n", pads[i].name);
//        DI_log(hr);
//    }
//    return enumContext.padcount;
//}
//
//void poll_dinput_pads(::Input::Gamepad::State* pads, u32 padCount) {
//    for (u32 i = 0; i < padCount; i++) {
//        checkJoystick(pads[i]);
//    }
//}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    
    LRESULT result = 0;
    switch (message) {
    case WM_CLOSE:
    case WM_DESTROY: {
        PostQuitMessage(0);
    }
    break;
    default: {
        result = DefWindowProcA(hWnd, message, wParam, lParam);
    }
    break;
    }
    return result;
}
int WINAPI WinMain(
      HINSTANCE hInstance
    , HINSTANCE hPrevInstance
    , LPSTR lpCmdLine
    , int nCmdShow
) {
    Platform::State platform = {};
    Allocator::init_arena(platform.memory.scratchArenaRoot, 1 << 20); // 1MB
    __DEBUGEXP(platform.memory.scratchArenaHighmark = platform.memory.scratchArenaRoot.curr; platform.memory.scratchArenaRoot.highmark = &platform.memory.scratchArenaHighmark);

    HWND hWnd;
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
        Game::loadLaunchConfig(windowConfig);

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

        platform.screen.window_width = windowConfig.window_width;
        platform.screen.window_height = windowConfig.window_height;
        platform.screen.width = windowConfig.game_width;
        platform.screen.height = windowConfig.game_height;
        platform.screen.desiredRatio = platform.screen.width / (f32)platform.screen.height;
        platform.screen.fullscreen = windowConfig.fullscreen;
    }

    u64 frequency;
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&frequency)) {
        return 0;
    }

    ID3D11Device1* d3ddev;
    ID3D11DeviceContext1* d3dcontext;
    IDXGISwapChain1* swapchain;
    ID3DUserDefinedAnnotation* pPerf;
        
    u32 flags = 0;
#if __GPU_DEBUG
    flags = D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        (ID3D11Device**) &d3ddev,
        nullptr,
        (ID3D11DeviceContext**) &d3dcontext
    );

    if (d3ddev != NULL) {

        IDXGIDevice* dxgiDevice;
        d3ddev->QueryInterface(&dxgiDevice);
        IDXGIAdapter* dxgiAdapter;
        dxgiDevice->GetAdapter(&dxgiAdapter);
        IDXGIFactory2* dxgiFactory;
        dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**) &dxgiFactory);

        DXGI_SWAP_CHAIN_DESC1 scd = { 0 };
        scd.Width = platform.screen.window_width;
        scd.Height = platform.screen.window_height;
        scd.Scaling = DXGI_SCALING_STRETCH;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.BufferCount = 2;
        scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        scd.SampleDesc.Count = 1;

        dxgiFactory->CreateSwapChainForHwnd(
            d3ddev,
            hWnd,
            &scd,
            nullptr,
            nullptr,
            &swapchain);

        // todo: check for errors
        d3dcontext->QueryInterface(__uuidof(pPerf), reinterpret_cast<void**>(&pPerf));

        // TODO: Handle more nicely
        Renderer::Driver::d3ddev = d3ddev;
        Renderer::Driver::d3dcontext = d3dcontext;
        Renderer::Driver::swapchain = swapchain;
        Renderer::Driver::perf = pPerf;

        //::Input::Gamepad::State pads[1];
        //::Input::Gamepad::KeyboardMapping keyboardPadMappings[1];
        //platform.input.padCount = 1;
        //platform.input.pads = pads;
        //::Input::Gamepad::load(keyboardPadMappings[0]);

        struct MouseQueue {
            u8 buttons[::Input::Mouse::Keys::COUNT];
            f32 x, y;
            f32 scrollx, scrolly;
        };
        u8 keyboard_queue[::Input::Keyboard::Keys::COUNT]{};
        MouseQueue mouse_queue = {};

        // gamepad
        memset(platform.input.pads, 0, sizeof(platform.input.pads));
        //platform.input.padCount = init_dinput_pads(hWnd, platform.input.pads, COUNT_OF(platform.input.pads));

        RAWINPUTDEVICE rid;
        rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
        rid.usUsage = HID_USAGE_GENERIC_GAMEPAD;
        rid.dwFlags = 0;
        rid.hwndTarget = hWnd;
        RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));


        u64 start;
        QueryPerformanceCounter((LARGE_INTEGER*)&start);

        platform.time.running = 0.0;
        platform.time.now = platform.time.start = start / (f64)frequency;

        Game::Instance game;
        Platform::GameConfig config;
        Game::start(game, config, platform);

        MSG msg = {};

        do {
            if (platform.time.now >= config.nextFrame) {

                // Input
                {
                    namespace KB = ::Input::Keyboard;
                    namespace MS = ::Input::Mouse;

                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                        // Process some messages locally instead of through a callback
                        // Keystrokes only need TranslateMessage when handling text
                        switch (msg.message) {
                            case WM_QUIT: {
                                config.quit = true;
                            }
                            break;
                            case WM_KEYUP:
                            case WM_SYSKEYUP: {
                                WPARAM wParam = msg.wParam;
                                if (wParam != VK_CONTROL && wParam != VK_PROCESSKEY) {
                                    auto keycode = (KB::Keys::Enum)(HIWORD(msg.lParam) & 0x1FF);
                                    ::Input::queueKeyUp(keyboard_queue, platform.input.keyboard.current, keycode);
                                }
                            }
                            break;
                            case WM_KEYDOWN:
                            case WM_SYSKEYDOWN: {
                                WPARAM wParam = msg.wParam;
                                if (wParam != VK_CONTROL && wParam != VK_PROCESSKEY) {
                                    auto keycode = (KB::Keys::Enum)(HIWORD(msg.lParam) & 0x1FF);
                                    ::Input::queueKeyDown(keyboard_queue, platform.input.keyboard.current, keycode);
                                }
                            }
                            break;
                            case WM_LBUTTONUP:
                            case WM_RBUTTONUP: {
                                MS::Keys::Enum keycode = (MS::Keys::Enum) ((msg.message >> 2) & 0x1);
                                ::Input::queueKeyUp(mouse_queue.buttons, platform.input.mouse.current, keycode);

                            }
                            break;
                            case WM_LBUTTONDOWN:
                            case WM_RBUTTONDOWN: {
                                MS::Keys::Enum keycode = (MS::Keys::Enum) ((msg.message >> 2) & 0x1);
                                ::Input::queueKeyDown(mouse_queue.buttons, platform.input.mouse.current, keycode);
                            }
                            break;
                            case WM_MOUSEMOVE: {
                                LPARAM lParam = msg.lParam;
                                // avoid using MAKEPOINTS, since it requires undef far
                                POINTS mouseCoords = (*((POINTS*)&(lParam)));
                                mouse_queue.x = mouseCoords.x;
                                mouse_queue.y = mouseCoords.y;
                            }
                            break;
                            case WM_MOUSEWHEEL:
                            case WM_MOUSEHWHEEL: {
                                WPARAM wParam = msg.wParam;
                                short scroll = (short)HIWORD(wParam);
                                mouse_queue.scrollx = 0.01f * scroll * (msg.message == WM_MOUSEHWHEEL);
                                mouse_queue.scrolly = 0.01f * scroll * (msg.message == WM_MOUSEWHEEL);
                            }
                            break;
                            case WM_INPUT: {
                                // arena copy, not by value (there will be an implicit free when the function ends)
                                ::Input::Gamepad::processPads(platform.memory.scratchArenaRoot, platform.input.pads, platform.input.padCount, COUNT_OF(platform.input.pads), (HRAWINPUT)msg.lParam);
                            }
                            break;
                            default: {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                            break;
                        };
                    }
                    memcpy(platform.input.keyboard.last, platform.input.keyboard.current, sizeof(u8) * KB::Keys::COUNT);
                    for (int i = 0; i < KB::Keys::COUNT; i++) {
                        ::Input::unqueueKey(platform.input.keyboard.current, keyboard_queue, i);
                    }
                    memcpy(platform.input.mouse.last, platform.input.mouse.current, sizeof(u8) * MS::Keys::COUNT);
                    for (int i = 0; i < MS::Keys::COUNT; i++) {
                        ::Input::unqueueKey(platform.input.mouse.current, mouse_queue.buttons, i);
                    }
                    platform.input.mouse.dx = mouse_queue.x - platform.input.mouse.x;
                    platform.input.mouse.dy = mouse_queue.y - platform.input.mouse.y;
                    platform.input.mouse.x = mouse_queue.x;
                    platform.input.mouse.y = mouse_queue.y;
                    platform.input.mouse.scrolldx = mouse_queue.scrollx;
                    platform.input.mouse.scrolldy = mouse_queue.scrolly;
                    mouse_queue.scrollx = 0.f;
                    mouse_queue.scrolly = 0.f;
                }

                Game::update(game, config, platform);

                //poll_dinput_pads(platform.input.pads, platform.input.padCount);

                swapchain->Present(1, 0);
            }

            u64 now;
            QueryPerformanceCounter((LARGE_INTEGER*)&now);
            platform.time.now = now / (f64)frequency;
            platform.time.running = platform.time.now - platform.time.start;
        } while (!config.quit);
    }

    return 1;
}

