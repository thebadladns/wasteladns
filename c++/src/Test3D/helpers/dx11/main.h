#ifndef UNITYBUILD
#include "../platform.h"
#include "core.h"
#include "input.h"
#include "game.h"
#endif

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
            bool state = ((lParam >> 31) & 1) == 0;
            keyboardPollData->queue.keyStates[(KB::Keys::Enum)(HIWORD(lParam) & 0x1FF)] = state;
        }
    }
    break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP: {
        namespace MS = ::Input::Mouse;
        MS::PollData* mousePollData = (MS::PollData*)GetPropA(hWnd, "MousePollData");
        if (mousePollData) {
            MS::Keys::Enum key = (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP) ? MS::Keys::BUTTON_LEFT : MS::Keys::BUTTON_RIGHT;
            bool state = message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN;
            mousePollData->keyStates[key] = state;
        }
    }
    break;
    case WM_MOUSEMOVE: {
        namespace MS = ::Input::Mouse;
        MS::PollData* mousePollData = (MS::PollData*)GetPropA(hWnd, "MousePollData");
        if (mousePollData) {
            // avoid using MAKEPOINTS, since it requires undef far
            POINTS mouseCoords = (*((POINTS*)&(lParam)));
            mousePollData->x = mouseCoords.x;
            mousePollData->y = mouseCoords.y;
        }
    }
    break;
    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:{
        namespace MS = ::Input::Mouse;
        MS::PollData* mousePollData = (MS::PollData*)GetPropA(hWnd, "MousePollData");
        if (mousePollData) {
            short scroll = (short)HIWORD(wParam);
            mousePollData->scrollx = 0.01f * scroll * (message == WM_MOUSEHWHEEL);
            mousePollData->scrolly = 0.01f * scroll * (message == WM_MOUSEWHEEL);
        }
    }
    break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}
int WINAPI WinMain(
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

        platform.screen.width = windowConfig.window_width;
        platform.screen.height = windowConfig.window_height;
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
        
    u32 flags = 0;
    //flags = D3D11_CREATE_DEVICE_DEBUG;
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
        scd.Width = platform.screen.width;
        scd.Height = platform.screen.height;
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

        // TODO: Handle more nicely
        Renderer::Driver::d3ddev = d3ddev;
        Renderer::Driver::d3dcontext = d3dcontext;
        Renderer::Driver::swapchain = swapchain;

        // TODO: assumes depth usage, should go inside game code (as driver resource)
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = (f32) platform.screen.width;
        viewport.Height = (f32)platform.screen.height;
        viewport.MinDepth = 0.f;
        viewport.MaxDepth = 1.f;
        d3dcontext->RSSetViewports(1, &viewport);

        ::Input::Gamepad::State pads[1];
        ::Input::Gamepad::KeyboardMapping keyboardPadMappings[1];
        platform.input.padCount = 1;
        platform.input.pads = pads;
        ::Input::Gamepad::load(keyboardPadMappings[0]);

        ::Input::Keyboard::PollData keyboardPollData;
        keyboardPollData.queue = {};
        SetPropA(hWnd, "KeyboardPollData", &keyboardPollData);

        ::Input::Mouse::PollData mousePollData = {};
        SetPropA(hWnd, "MousePollData", &mousePollData);

        u64 start;
        QueryPerformanceCounter((LARGE_INTEGER*)&start);

        platform.time.running = 0.0;
        platform.time.now = platform.time.start = start / (f64)frequency;

        Game::Instance game;
        Platform::GameConfig config;
        Game::start(game, config, platform);

        MSG msg;

        do {
            if (platform.time.now >= config.nextFrame) {

                // Input
                {
                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    // Input
                    if ((config.requestFlags & (Platform::RequestFlags::PollKeyboard)) != 0) {
                        ::Input::Keyboard::pollState(platform.input.keyboard, keyboardPollData.queue);
                    }
                    if ((config.requestFlags & (Platform::RequestFlags::PollMouse)) != 0) {
                        ::Input::Mouse::pollState(platform.input.mouse, mousePollData);
                        ::Input::Mouse::resetState(mousePollData);
                    }
                    for (u32 i = 0; i < platform.input.padCount; i++) {
                        ::Input::Gamepad::pollState(platform.input.pads[i], keyboardPollData.queue, keyboardPadMappings[i]);
                    }
                }

                Game::update(game, config, platform);

                swapchain->Present(1, 0);

                if (config.quit) {
                    PostQuitMessage(0);
                }
            }

            u64 now;
            QueryPerformanceCounter((LARGE_INTEGER*)&now);
            platform.time.now = now / (f64)frequency;
            platform.time.running = platform.time.now - platform.time.start;
        } while (msg.message != WM_QUIT);
    }

    return 1;
}
