
#if __DX11
bool createRenderContext(HWND hWnd, const platform::State& platform) {
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
    renderer::driver::d3ddev = d3ddev;
    renderer::driver::d3dcontext = d3dcontext;
    renderer::driver::swapchain = swapchain;
    renderer::driver::perf = pPerf;
    
    return true;
}

void swapBuffers() {
	renderer::driver::swapchain->Present(1, 0);
}
#elif __GL33
struct LogLevel { enum Enum { None = -1, Debug, Low, Med, High }; };
struct LogData {
    u32 indent;
    LogLevel::Enum minLevel;
};
void debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    LogLevel::Enum logLevel;
    const char* severityStr = "";
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: severityStr = "HIGH"; logLevel = LogLevel::High; break;
    case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "MEDIUM"; logLevel = LogLevel::Med; break;
    case GL_DEBUG_SEVERITY_LOW: severityStr = "LOW"; logLevel = LogLevel::Low; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "NOTIFICATION"; logLevel = LogLevel::Debug; break;
    default: severityStr = "N/A"; logLevel = LogLevel::None; break;
    }
    LogData& logData = *(LogData*)userParam;
    if (logLevel < logData.minLevel) { return; }

    u32 indent = logData.indent;
    const char* sourceStr;
    switch (source) {
    case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "WINDOW SYSTEM"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "SHADER COMPILER"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "THIRD PARTY"; break;
    case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "APPLICATION"; break;
    case GL_DEBUG_SOURCE_OTHER: sourceStr = "OTHER"; break;
    default: sourceStr = "N/A"; break;
    }
    const char* typeStr = "";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR: typeStr = "ERROR"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "DEPRECATED BEHAVIOR"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "UNDEFINED BEHAVIOR"; break;
    case GL_DEBUG_TYPE_PORTABILITY: typeStr = "PORTABILITY"; break;
    case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "PERFORMANCE"; break;
    case GL_DEBUG_TYPE_OTHER: typeStr = "OTHER"; break;
    case GL_DEBUG_TYPE_MARKER: typeStr = "MARKER"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "PUSH"; logData.indent++; break;
    case GL_DEBUG_TYPE_POP_GROUP: typeStr = "POP"; logData.indent--, indent--; break;
    }

    platform::debuglog("%*s=>GL[%s,src:%s,severity:%s,id:%d]: %s\n",
        indent * 2, "", typeStr, sourceStr, severityStr, id, message);
}
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC, HGLRC, const int*);
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
typedef BOOL (WINAPI* PFNWGLGETPIXELFORMATARBPROC)(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);
PFNWGLGETPIXELFORMATARBPROC wglChoosePixelFormatARB;

#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_ALPHA_BITS_ARB 0x201b
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_TYPE_RGBA_ARB 0x202B

HDC dc;
bool createRenderContext(HWND hWnd, const platform::State& platform) {

    {   // create a tmp render context / window pair, so we can load wgl extensions
        WNDCLASSA window_class = {};
        window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        window_class.lpfnWndProc = DefWindowProcA;
        window_class.hInstance = GetModuleHandle(0);
        window_class.lpszClassName = "dummy_wgl";
        if (!RegisterClassA(&window_class)) { return false; }
        HWND hWnd_tmp = CreateWindowExA(0, window_class.lpszClassName, "dummy gl", 0,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, window_class.hInstance, 0);
        if (!hWnd_tmp) { return false; }
        HDC dc_tmp = GetDC(hWnd_tmp);
        PIXELFORMATDESCRIPTOR pfd_tmp = {};
        pfd_tmp.nSize = sizeof(pfd_tmp);
        pfd_tmp.nVersion = 1;
        pfd_tmp.dwFlags = PFD_DRAW_TO_WINDOW | PFD_GENERIC_ACCELERATED | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd_tmp.iPixelType = PFD_TYPE_RGBA;
        pfd_tmp.cColorBits = 24;
        pfd_tmp.cAlphaBits = 8;
        pfd_tmp.cDepthBits = 24;
        pfd_tmp.cStencilBits = 8;
        SetPixelFormat(dc_tmp, ChoosePixelFormat(dc_tmp, &pfd_tmp), &pfd_tmp);
        HGLRC rc_tmp = wglCreateContext(dc_tmp);
        if (!rc_tmp) { return false; }
        if (!wglMakeCurrent(dc_tmp, rc_tmp)) { return false; }
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        wglChoosePixelFormatARB = (PFNWGLGETPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        wglMakeCurrent(dc_tmp, 0);
        wglDeleteContext(rc_tmp);
        ReleaseDC(hWnd_tmp, dc_tmp);
        DestroyWindow(hWnd_tmp);
    }

    // create the real context on the actual window using wglCreateContextAttribs, so apps like RenderDoc can process it
    dc = GetDC(hWnd);
    int pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        0
    };
    int pixel_format;
    UINT num_formats;
    wglChoosePixelFormatARB(dc, pixel_format_attribs, 0, 1, &pixel_format, &num_formats);
    if (!num_formats) { return false; }
    PIXELFORMATDESCRIPTOR pfd;
    if (!DescribePixelFormat(dc, pixel_format, sizeof(pfd), &pfd)) { return false; }
    if (!SetPixelFormat(dc, pixel_format, &pfd)) { return false; }
    int gl33_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };
    HGLRC rc = wglCreateContextAttribsARB(dc, 0, gl33_attribs);
    if (!rc) { return false; }
    if (!wglMakeCurrent(dc, rc)) { return false; }

    // initialize OpenGL function pointers
    renderer::driver::loadGLFramework();
    renderer::driver::loadGLExtensions();

#if __GPU_DEBUG
    if (glDebugMessageCallback) {//GLAD_GL_KHR_debug) {
        LogData logLevel = {};
        logLevel.minLevel = LogLevel::Debug;
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debugMessageCallback, &logLevel);
    }
    else {
        assert(0);
    }
#endif

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
    platform::State platform = {};
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

        platform::LaunchConfig config;
        game::loadLaunchConfig(config);

        // Adjust window to account for menus
        const DWORD style = WS_OVERLAPPEDWINDOW;
        const DWORD exStyle = NULL;
		RECT rect = { 0, 0, (s32)config.window_width, (s32)config.window_height };
        AdjustWindowRectEx(&rect, style, FALSE, exStyle);

        hWnd = CreateWindowEx(
              exStyle
            , "WindowClass"
            , config.title
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

        platform.screen.window_width = config.window_width;
        platform.screen.window_height = config.window_height;
        platform.screen.width = config.game_width;
        platform.screen.height = config.game_height;
        platform.screen.desiredRatio = platform.screen.width / (f32)platform.screen.height;
        platform.screen.fullscreen = config.fullscreen;
    }

    u64 frequency;
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&frequency)) {
        return 0;
    }

    if (!createRenderContext(hWnd, platform)) {
        return 0;
    }

    ::input::gamepad::init_hid_pads_win(hWnd);

    u64 start;
    QueryPerformanceCounter((LARGE_INTEGER*)&start);
    const u32 desiredSchedulerMS = 1;
    // we only use one function from Winmm.dll, no need to load the whole lib
    typedef MMRESULT (*TIMEBEGINPERIODPROC)(_In_ UINT uPeriod);
    HMODULE winmmModule = LoadLibraryA("Winmm.dll");
    TIMEBEGINPERIODPROC timeBeginPeriod = 
        (TIMEBEGINPERIODPROC)GetProcAddress(winmmModule, "timeBeginPeriod");
    bool sleepIsGranular = (timeBeginPeriod(desiredSchedulerMS) == TIMERR_NOERROR);
    platform.time.running = 0.0;
    platform.time.now = platform.time.start = start / (f64)frequency;

    game::Instance game;
    platform::GameConfig config;
    game::start(game, config, platform);

    MSG msg = {};
    do {
        // Input
        {
        for (u32 i = 0; i < platform.input.padCount; i++)
        { platform.input.pads[i].last_keys = platform.input.pads[i].curr_keys; }
        memcpy(
            platform.input.keyboard.last, platform.input.keyboard.current,
            sizeof(u8) * ::input::keyboard::Keys::COUNT);
        memcpy(
            platform.input.mouse.last, platform.input.mouse.curr,
            sizeof(u8) * ::input::mouse::Keys::COUNT);
        const f32 mouse_prevx = platform.input.mouse.x, mouse_prevy = platform.input.mouse.y;
        platform.input.mouse.dx = platform.input.mouse.dy =
            platform.input.mouse.scrolldx = platform.input.mouse.scrolldy = 0.f;

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
                        const ::input::keyboard::Keys::Enum keycode =
                            (::input::keyboard::Keys::Enum)(HIWORD(msg.lParam) & 0x1FF);
						platform.input.keyboard.current[keycode] = 0;
                    }
                }
                break;
                case WM_KEYDOWN:
                case WM_SYSKEYDOWN: {
                    if (msg.wParam != VK_CONTROL && msg.wParam != VK_PROCESSKEY) {
                        const ::input::keyboard::Keys::Enum keycode =
                            (::input::keyboard::Keys::Enum)(HIWORD(msg.lParam) & 0x1FF);
                        platform.input.keyboard.current[keycode] = 1;
                    }
                }
                break;
                case WM_LBUTTONUP:
                case WM_RBUTTONUP: {
                    const ::input::mouse::Keys::Enum keycode =
                        (::input::mouse::Keys::Enum) ((msg.message >> 2) & 0x1);
					platform.input.mouse.curr[keycode] = 0;
                }
                break;
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN: {
                    const ::input::mouse::Keys::Enum keycode =
                        (::input::mouse::Keys::Enum) ((msg.message >> 2) & 0x1);
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
                    platform.input.mouse.scrolldx +=
                        0.01f * scroll * (msg.message == WM_MOUSEHWHEEL);
                    platform.input.mouse.scrolldy +=
                        0.01f * scroll * (msg.message == WM_MOUSEWHEEL);
                }
                break;
                case WM_INPUT: {
                    u8 mem[2048]; // local arena
                    allocator::Arena scratchArena = {};
                    allocator::init_arena(scratchArena, mem, sizeof(mem));
                    ::input::gamepad::process_hid_pads_win(
                        scratchArena, platform.input.pads,
                        platform.input.padCount, countof(platform.input.pads),
                        (HRAWINPUT)msg.lParam);
                }
                break;
                default: {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                break;
            };
        }
        } // Input

        // update and render game
        game::update(game, config, platform);

        // present game frame onscreen
        swapBuffers();

        // frame time handling: sleep a number of Windows scheduler periods until we hit our
        // next target time: https://blog.bearcats.nl/perfect-sleep-function/
        u64 now;
        QueryPerformanceCounter((LARGE_INTEGER*)&now);
        platform.time.now = now / (f64)frequency;
        if (platform.time.now < config.nextFrame) {
            if (sleepIsGranular) { // sleep with 1ms granularity away from the target time
                const f64 sleepSeconds = config.nextFrame - platform.time.now;
                // round up desired sleep time, to compensate for Windows' scheduler rounding up
                const f64 sleepMs = sleepSeconds * 1000. - (desiredSchedulerMS + 0.02);
                s32 sleepPeriodCount = s32(sleepMs / (f64)desiredSchedulerMS);
                if (sleepPeriodCount > 0) { Sleep(DWORD(sleepPeriodCount * desiredSchedulerMS)); }
            }
            do { // spin-lock until the next target time is hit
                YieldProcessor();
                QueryPerformanceCounter((LARGE_INTEGER*)&now);
                platform.time.now = now / (f64)frequency;
            } while (platform.time.now < config.nextFrame);
        }
        platform.time.running = platform.time.now - platform.time.start;
        
    } while (!config.quit);

    return 1;
}
