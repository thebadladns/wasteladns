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



        // Adjust window to account for menus
        const DWORD style = WS_OVERLAPPEDWINDOW;
        const DWORD exStyle = NULL;
        RECT rect = { 0, 0, (s32)windowConfig.window_width, (s32)windowConfig.window_height };
        AdjustWindowRectEx(&rect, style, FALSE, exStyle);

        hWnd = CreateWindowEx(
            exStyle
            , "WindowClass"
            , windowConfig.title
            , style
            , 0, 0, rect.right - rect.left, rect.bottom - rect.top
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

    

        struct MouseQueue {
            u8 buttons[::Input::Mouse::Keys::COUNT];
            f32 x, y;
            f32 scrollx, scrolly;
        };
        u8 keyboard_queue[::Input::Keyboard::Keys::COUNT]{};
        MouseQueue mouse_queue = {};

        // dinput gamepad
        //platform.input.padCount = init_dinput_pads(hWnd, platform.input.pads, COUNT_OF(platform.input.pads));
        ::Input::Gamepad::init_hid_pads_win(hWnd);

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
                    for (u32 i = 0; i < platform.input.padCount; i++) { // todo: improve release state
                        platform.input.pads[i].last_keys = platform.input.pads[i].curr_keys;
                    }
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
                                    KB::Keys::Enum keycode = (KB::Keys::Enum)(HIWORD(msg.lParam) & 0x1FF);
                                    ::Input::queueKeyUp(keyboard_queue, platform.input.keyboard.current, keycode);
                                }
                            }
                            break;
                            case WM_KEYDOWN:
                            case WM_SYSKEYDOWN: {
                                WPARAM wParam = msg.wParam;
                                if (wParam != VK_CONTROL && wParam != VK_PROCESSKEY) {
                                    KB::Keys::Enum keycode = (KB::Keys::Enum)(HIWORD(msg.lParam) & 0x1FF);
                                    ::Input::queueKeyDown(keyboard_queue, platform.input.keyboard.current, keycode);
                                }
                            }
                            break;
                            case WM_LBUTTONUP:
                            case WM_RBUTTONUP: {
                                MS::Keys::Enum keycode = (MS::Keys::Enum) ((msg.message >> 2) & 0x1);
                                ::Input::queueKeyUp(mouse_queue.buttons, platform.input.mouse.curr, keycode);
                            }
                            break;
                            case WM_LBUTTONDOWN:
                            case WM_RBUTTONDOWN: {
                                MS::Keys::Enum keycode = (MS::Keys::Enum) ((msg.message >> 2) & 0x1);
                                ::Input::queueKeyDown(mouse_queue.buttons, platform.input.mouse.curr, keycode);
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
                                ::Input::Gamepad::process_hid_pads_win(platform.memory.scratchArenaRoot, platform.input.pads, platform.input.padCount, COUNT_OF(platform.input.pads), (HRAWINPUT)msg.lParam);
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
                    memcpy(platform.input.mouse.last, platform.input.mouse.curr, sizeof(u8) * MS::Keys::COUNT);
                    for (int i = 0; i < MS::Keys::COUNT; i++) {
                        ::Input::unqueueKey(platform.input.mouse.curr, mouse_queue.buttons, i);
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

