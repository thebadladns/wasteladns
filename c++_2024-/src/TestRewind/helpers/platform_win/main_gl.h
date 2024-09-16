
#if __DX11
bool createRenderContext(HWND hWnd, const Platform::State& platform) {
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
        (ID3D11Device**)&d3ddev,
        nullptr,
        (ID3D11DeviceContext**)&d3dcontext
    );

    if (d3ddev == NULL) {
        return false;
    }

    IDXGIDevice* dxgiDevice;
    d3ddev->QueryInterface(&dxgiDevice);
    IDXGIAdapter* dxgiAdapter;
    dxgiDevice->GetAdapter(&dxgiAdapter);
    IDXGIFactory2* dxgiFactory;
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);

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
    
    return true;
}

void swapBuffers() {
	Renderer::Driver::swapchain->Present(1, 0);
}
#elif __GL33
HDC dc;
bool createRenderContext(HWND hWnd, const Platform::State& platform) {
    dc = GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_GENERIC_ACCELERATED | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd);
    HGLRC rc = wglCreateContext(dc);
    if (!rc) { return false; }
    wglMakeCurrent(dc, rc);

    // initialize OpenGL function pointers
    Renderer::Driver::loadGLFramework();
    Renderer::Driver::loadGLExtensions();

    return true;
}
void swapBuffers() {
    SwapBuffers(dc);
}
#endif


LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    
    LRESULT result = 0;
    switch (message) {
    case WM_CLOSE:
    case WM_DESTROY: { PostQuitMessage(0); } break;
    default: { result = DefWindowProcA(hWnd, message, wParam, lParam); } break;
    }
    return result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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
            , 0, 0, rect.right- rect.left, rect.bottom-rect.top
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

    if (!createRenderContext(hWnd, platform)) {
        return 0;
    }

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
                for (u32 i = 0; i < platform.input.padCount; i++) { platform.input.pads[i].last_keys = platform.input.pads[i].curr_keys; }
                memcpy(platform.input.keyboard.last, platform.input.keyboard.current, sizeof(u8)* ::Input::Keyboard::Keys::COUNT);
                memcpy(platform.input.mouse.last, platform.input.mouse.curr, sizeof(u8) * ::Input::Mouse::Keys::COUNT);
                const f32 mouse_prevx = platform.input.mouse.x, mouse_prevy = platform.input.mouse.y;
                platform.input.mouse.dx = platform.input.mouse.dy = platform.input.mouse.scrolldx = platform.input.mouse.scrolldy = 0.f;
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
                            if (msg.wParam != VK_CONTROL && msg.wParam != VK_PROCESSKEY) {
                                ::Input::Keyboard::Keys::Enum keycode = (::Input::Keyboard::Keys::Enum)(HIWORD(msg.lParam) & 0x1FF);
								platform.input.keyboard.current[keycode] = 0;
                            }
                        }
                        break;
                        case WM_KEYDOWN:
                        case WM_SYSKEYDOWN: {
                            if (msg.wParam != VK_CONTROL && msg.wParam != VK_PROCESSKEY) {
                                ::Input::Keyboard::Keys::Enum keycode = (::Input::Keyboard::Keys::Enum)(HIWORD(msg.lParam) & 0x1FF);
                                platform.input.keyboard.current[keycode] = 1;
                            }
                        }
                        break;
                        case WM_LBUTTONUP:
                        case WM_RBUTTONUP: {
                            ::Input::Mouse::Keys::Enum keycode = (::Input::Mouse::Keys::Enum) ((msg.message >> 2) & 0x1);
							platform.input.mouse.curr[keycode] = 0;
                        }
                        break;
                        case WM_LBUTTONDOWN:
                        case WM_RBUTTONDOWN: {
                            ::Input::Mouse::Keys::Enum keycode = (::Input::Mouse::Keys::Enum) ((msg.message >> 2) & 0x1);
                            platform.input.mouse.curr[keycode] = 1;
                        }
                        break;
                        case WM_MOUSEMOVE: {
                            LPARAM lParam = msg.lParam;
                            // avoid using MAKEPOINTS, since it requires undef far
                            POINTS mouseCoords = (*((POINTS*)&(lParam)));
                            platform.input.mouse.x = mouseCoords.x;
                            platform.input.mouse.y = mouseCoords.y;
							platform.input.mouse.dx = platform.input.mouse.x - mouse_prevx;
                            platform.input.mouse.dy = platform.input.mouse.y - mouse_prevy;
                        }
                        break;
                        case WM_MOUSEWHEEL:
                        case WM_MOUSEHWHEEL: {
                            short scroll = (short)HIWORD(msg.wParam);
                            platform.input.mouse.scrolldx += 0.01f * scroll * (msg.message == WM_MOUSEHWHEEL);
                            platform.input.mouse.scrolldy += 0.01f * scroll * (msg.message == WM_MOUSEWHEEL);
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
            }

            Game::update(game, config, platform);

            swapBuffers();
        }

        u64 now;
        QueryPerformanceCounter((LARGE_INTEGER*)&now);
        platform.time.now = now / (f64)frequency;
        platform.time.running = platform.time.now - platform.time.start;
    } while (!config.quit);

    return 1;
}

