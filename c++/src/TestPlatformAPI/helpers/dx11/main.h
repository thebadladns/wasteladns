#ifndef UNITYBUILD
#include "../platform.h"
#include "core.h"
#include "input.h"
#endif

namespace Platform {

	const char* name = "DX11";

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

        ID3D11Device1* d3ddev;
        ID3D11DeviceContext1* d3dcontext;
        IDXGISwapChain1* swapchain;
        ID3D11RenderTargetView* renderTarget;
		
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

            ID3D11Texture2D* backbuffer;
            swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &backbuffer);
            d3ddev->CreateRenderTargetView(backbuffer, nullptr, &renderTarget);

            D3D11_VIEWPORT viewport = {};
            viewport.TopLeftX = 0;
            viewport.TopLeftY = 0;
            viewport.Width = (f32) platform.screen.width;
            viewport.Height = (f32)platform.screen.height;
            d3dcontext->RSSetViewports(1, &viewport);


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
}
