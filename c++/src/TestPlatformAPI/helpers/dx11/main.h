#ifndef UNITYBUILD
#include "../platform.h"
#include "core.h"
#include "input.h"
#endif

const char * vertexColorShaderStr = R"(

    cbuffer ProjectionMatrixBuffer : register(b0) {
        matrix projectionMatrix;
    }
    cbuffer ViewMatrixBuffer : register(b1) {
        matrix viewMatrix;
    }
    cbuffer WorldMatrixBuffer : register(b2) {
        matrix worldMatrix;
    }
    cbuffer ColorBuffer : register(b3) {
        float4 bgcolor;
    }
    struct AppData {
        float3 position : POSITION;
    };
    struct VertexOutput {
        float4 color : COLOR;
        float4 position : SV_POSITION;
    };
    VertexOutput VS(AppData IN) {
        VertexOutput OUT;
        matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));
        OUT.position = mul(mvp, float4(IN.position, 1.f));
        OUT.color = bgcolor;
        return OUT;
    }

)";

const char * vertexShaderStr = R"(

    cbuffer ProjectionMatrixBuffer : register(b0) {
        matrix projectionMatrix;
    }
    cbuffer ViewMatrixBuffer : register(b1) {
        matrix viewMatrix;
    }
    cbuffer WorldMatrixBuffer : register(b2) {
        matrix worldMatrix;
    }
    struct AppData {
        float3 position : POSITION;
        float3 color: COLOR;
    };
    struct VertexOutput {
        float4 color : COLOR;
        float4 position : SV_POSITION;
    };
    VertexOutput VS(AppData IN) {
        VertexOutput OUT;
        matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));
        OUT.position = mul(mvp, float4(IN.position, 1.f));
        OUT.color = float4(IN.color, 1.f);
        return OUT;
    }

)";

const char * pixelShaderStr = R"(
    struct PixelIn {
        float4 color : COLOR;
    };
    float4 PS(PixelIn IN) : SV_TARGET {
        return IN.color;
    }
)";

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

        ID3D11Device1* d3ddev;
        ID3D11DeviceContext1* d3dcontext;
        IDXGISwapChain1* swapchain;
        ID3D11RenderTargetView* renderTarget;

        D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
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

            ID3D11Texture2D* backbuffer;
            swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &backbuffer);
            d3ddev->CreateRenderTargetView(backbuffer, nullptr, &renderTarget);

            ID3D11RasterizerState* rasterizerState;
            D3D11_RASTERIZER_DESC rasterizerDesc = { 0 };
            rasterizerDesc.AntialiasedLineEnable = FALSE;
            rasterizerDesc.CullMode = D3D11_CULL_FRONT;
            rasterizerDesc.DepthBias = 0;
            rasterizerDesc.DepthBiasClamp = 0.f;
            rasterizerDesc.DepthClipEnable = TRUE;
            rasterizerDesc.FillMode = D3D11_FILL_SOLID;
            rasterizerDesc.FrontCounterClockwise = FALSE;
            rasterizerDesc.MultisampleEnable = FALSE;
            rasterizerDesc.ScissorEnable = FALSE;
            rasterizerDesc.SlopeScaledDepthBias = 0.f;
            d3ddev->CreateRasterizerState(&rasterizerDesc, &rasterizerState);

            D3D11_VIEWPORT viewport = { 0 };
            viewport.TopLeftX = 0;
            viewport.TopLeftY = 0;
            viewport.Width = (f32) platform.screen.width;
            viewport.Height = (f32)platform.screen.height;

            d3dcontext->RSSetState(rasterizerState);
            d3dcontext->RSSetViewports(1, &viewport);

            ID3D11BlendState1* blendState;
            D3D11_BLEND_DESC1 blendStateDesc = { 0 };
            blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
            blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            d3ddev->CreateBlendState1(&blendStateDesc, &blendState);
            d3dcontext->OMSetBlendState(blendState, 0, 0xffffffff);

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

            // --------------------------------------------------------------

            struct CustomVertex1 {
                f32 x, y, z;
                f32 r, g, b;
            };
            ID3D11Buffer* vertexBuffer1;
            ID3D11Buffer* indexBuffer1;
            u32 indexCount1;
            {
                D3D11_SUBRESOURCE_DATA resourceData = { 0 };

                CustomVertex1 vertices[] = {
                    { 3.0f, -3.0f, 0.0f, 0.f, 0.f, 1.f, },
                    { 0.0f, 3.0f, 0.0f, 0.f, 1.f, 0.f, },
                    { -3.0f, -3.0f, 0.0f, 1.f, 0.f, 0.f },
                };
                WORD indices[] = {
                    0, 1, 2
                };
                indexCount1 = sizeof(indices) / sizeof(indices[0]);
                D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
                vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                vertexBufferDesc.ByteWidth = sizeof(vertices);
                vertexBufferDesc.CPUAccessFlags = 0;
                vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
                resourceData = { vertices, 0, 0 };
                d3ddev->CreateBuffer(&vertexBufferDesc, &resourceData, &vertexBuffer1);

                D3D11_BUFFER_DESC indexBufferDesc = { 0 };
                indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                indexBufferDesc.ByteWidth = sizeof(indices);
                indexBufferDesc.CPUAccessFlags = 0;
                indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
                resourceData.pSysMem = indices;
                d3ddev->CreateBuffer(&indexBufferDesc, &resourceData, &indexBuffer1);
            }
            ID3D11Buffer* vertexBuffer2;
            ID3D11Buffer* indexBuffer2;
            u32 indexCount2;
            {
                D3D11_SUBRESOURCE_DATA resourceData = { 0 };

                f32 pw = 5.f;
                f32 pz = 500.f;
                Vec3 pillarVertices[] = {
                      { -pw, -pw, 0.f },{ pw, -pw, 0.f },{ pw, -pw, pz },{ -pw, -pw, pz } // +y quad
                    , { pw, pw, 0.f },{ -pw, pw, 0.f },{ -pw, pw, pz },{ pw, pw, pz } // -y quad
                };
                u16 pillarIndexes[] = {
                    0, 1, 2, 3, 0, 2, // +y tris
                    4, 5, 6, 7, 4, 6, // -y tris
                    1, 4, 7, 2, 1, 7, // +x tris
                    5, 0, 3, 6, 5, 3, // -x tris
                };
                indexCount2 = sizeof(pillarIndexes) / sizeof(pillarIndexes[0]);
                D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
                vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                vertexBufferDesc.ByteWidth = sizeof(pillarVertices);
                vertexBufferDesc.CPUAccessFlags = 0;
                vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
                resourceData = { pillarVertices, 0, 0 };
                d3ddev->CreateBuffer(&vertexBufferDesc, &resourceData, &vertexBuffer2);

                D3D11_BUFFER_DESC indexBufferDesc = { 0 };
                indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                indexBufferDesc.ByteWidth = sizeof(pillarIndexes);
                indexBufferDesc.CPUAccessFlags = 0;
                indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
                resourceData.pSysMem = pillarIndexes;
                d3ddev->CreateBuffer(&indexBufferDesc, &resourceData, &indexBuffer2);
            }

            struct ConstantBuffer { enum { ProjectionMatrix, ViewMatrix, WorldMatrix, Color, Count }; };
            ID3D11Buffer* cbuffers[ConstantBuffer::Count];
            {
                D3D11_BUFFER_DESC constantBufferDesc = { 0 };
                constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                constantBufferDesc.ByteWidth = sizeof(Mat4);
                constantBufferDesc.CPUAccessFlags = 0;
                constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

                d3ddev->CreateBuffer(&constantBufferDesc, nullptr, &cbuffers[ConstantBuffer::ProjectionMatrix]);
                d3ddev->CreateBuffer(&constantBufferDesc, nullptr, &cbuffers[ConstantBuffer::ViewMatrix]);
                d3ddev->CreateBuffer(&constantBufferDesc, nullptr, &cbuffers[ConstantBuffer::WorldMatrix]);

                D3D11_BUFFER_DESC constantBufferDesc2 = { 0 };
                constantBufferDesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                constantBufferDesc2.ByteWidth = sizeof(Vec4);
                constantBufferDesc2.CPUAccessFlags = 0;
                constantBufferDesc2.Usage = D3D11_USAGE_DEFAULT;
                d3ddev->CreateBuffer(&constantBufferDesc2, nullptr, &cbuffers[ConstantBuffer::Color]);
            }

            ID3D11VertexShader* vertexShader = nullptr;
            ID3D11InputLayout* vertexInputLayout = nullptr;
            {
                ID3DBlob* pShaderBlob = nullptr;
                ID3DBlob* pErrorBlob = nullptr;
                HRESULT hr = D3DCompile(
                      vertexShaderStr, strlen(vertexShaderStr), "VS"
                    , nullptr // defines
                    , D3D_COMPILE_STANDARD_FILE_INCLUDE
                    , "VS"
                    , "vs_5_0"
                    , D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0
                    , &pShaderBlob, &pErrorBlob
                );
                if (!FAILED(hr)) {
                    d3ddev->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), nullptr, &vertexShader);
                } else {
                    Platform::printf("==== Shader compilation error: %.128s", pErrorBlob ? (char*)pErrorBlob->GetBufferPointer() : "Unknown shader error");
                }

                D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
                      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(CustomVertex1, x), D3D11_INPUT_PER_VERTEX_DATA, 0 }
                    , { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(CustomVertex1, r), D3D11_INPUT_PER_VERTEX_DATA, 0 }
                };
                d3ddev->CreateInputLayout(vertexLayoutDesc, ARRAYSIZE(vertexLayoutDesc), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), &vertexInputLayout);
                
                if (pShaderBlob) pShaderBlob->Release();
                if (pErrorBlob) pErrorBlob->Release();
            }
            ID3D11VertexShader* vertexColorShader = nullptr;
            ID3D11InputLayout* vertexColorInputLayout = nullptr;
            {
                ID3DBlob* pShaderBlob = nullptr;
                ID3DBlob* pErrorBlob = nullptr;
                HRESULT hr = D3DCompile(
                      vertexColorShaderStr, strlen(vertexColorShaderStr), "VS"
                    , nullptr // defines
                    , D3D_COMPILE_STANDARD_FILE_INCLUDE
                    , "VS"
                    , "vs_5_0"
                    , D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0
                    , &pShaderBlob, &pErrorBlob
                );
                if (!FAILED(hr)) {
                    d3ddev->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), nullptr, &vertexColorShader);
                }
                else {
                    Platform::printf("==== Shader compilation error: %.128s", pErrorBlob ? (char*)pErrorBlob->GetBufferPointer() : "Unknown shader error");
                }

                D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
                    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(CustomVertex1, x), D3D11_INPUT_PER_VERTEX_DATA, 0 }
                };
                d3ddev->CreateInputLayout(vertexLayoutDesc, ARRAYSIZE(vertexLayoutDesc), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), &vertexColorInputLayout);

                if (pShaderBlob) pShaderBlob->Release();
                if (pErrorBlob) pErrorBlob->Release();
            }
            ID3D11PixelShader* pixelShader = nullptr;
            {
                ID3DBlob* pShaderBlob = nullptr;
                ID3DBlob* pErrorBlob = nullptr;
                HRESULT hr = D3DCompile(
                      pixelShaderStr, strlen(pixelShaderStr), "PS"
                    , nullptr // defines
                    , D3D_COMPILE_STANDARD_FILE_INCLUDE
                    , "PS"
                    , "ps_5_0"
                    , D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0
                    , &pShaderBlob, &pErrorBlob
                );
                if (!FAILED(hr)) {
                    d3ddev->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), nullptr, &pixelShader);
                } else {
                    Platform::printf("==== Shader compilation error: %.128s", pErrorBlob ? (char*)pErrorBlob->GetBufferPointer() : "Unknown shader error");
                }
                if (pShaderBlob) pShaderBlob->Release();
                if (pErrorBlob) pErrorBlob->Release();
            }

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

                    // temp scene hack update
                    Col bg(0.01f, 0.021f, 0.06f);
                    d3dcontext->OMSetRenderTargets(1, &renderTarget, nullptr);
                    float color[] = { RGBA_PARAMS(bg) };
                    d3dcontext->ClearRenderTargetView(renderTarget, color);


                    // Update perspective projection matrix in the constant buffer
                    d3dcontext->UpdateSubresource(cbuffers[ConstantBuffer::ProjectionMatrix], 0, nullptr, game.renderMgr.perspProjection.matrix.dataCM, 0, 0);
                    // Camera view
                    d3dcontext->UpdateSubresource(cbuffers[ConstantBuffer::ViewMatrix], 0, nullptr, game.cameraMgr.activeCam->viewMatrix.dataCM, 0, 0);
                    {
                        // Buffer1
                        {
                            u32 vertexStride = sizeof(CustomVertex1);
                            u32 offset = 0;
                            Transform t;
                            Math::identity4x4(t);
                            d3dcontext->UpdateSubresource(cbuffers[ConstantBuffer::WorldMatrix], 0, nullptr, t.dataCM, 0, 0);
                            d3dcontext->IASetVertexBuffers(0, 1, &vertexBuffer1, &vertexStride, &offset);
                            d3dcontext->IASetInputLayout(vertexInputLayout);
                            d3dcontext->IASetIndexBuffer(indexBuffer1, DXGI_FORMAT_R16_UINT, 0);
                            d3dcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                            d3dcontext->VSSetShader(vertexShader, nullptr, 0);
                            d3dcontext->VSSetConstantBuffers(0, ConstantBuffer::Count, cbuffers);
                            d3dcontext->PSSetShader(pixelShader, nullptr, 0);
                            d3dcontext->OMSetBlendState(blendState, 0, 0xffffffff);
                            d3dcontext->DrawIndexed(indexCount1, 0, 0);
                        }

                        {
                            u32 vertexStride = sizeof(Vec3);
                            u32 offset = 0;
                            Transform t;
                            Math::identity4x4(t);
                            Vec4 colorbg = { 1.f, 1.f, 1.f, 0.5f };
                            d3dcontext->UpdateSubresource(cbuffers[ConstantBuffer::Color], 0, nullptr, &colorbg, 0, 0);
                            d3dcontext->IASetVertexBuffers(0, 1, &vertexBuffer2, &vertexStride, &offset);
                            d3dcontext->IASetInputLayout(vertexColorInputLayout);
                            d3dcontext->IASetIndexBuffer(indexBuffer2, DXGI_FORMAT_R16_UINT, 0);
                            d3dcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                            d3dcontext->VSSetShader(vertexColorShader, nullptr, 0);
                            d3dcontext->VSSetConstantBuffers(0, ConstantBuffer::Count, cbuffers);
                            d3dcontext->PSSetShader(pixelShader, nullptr, 0);
                            Vec2 pillarPos[] = {
                                 { -80.f, -20.f },{ 80.f, -20.f },{ -160.f, -20.f },{ 160.f, -20.f }
                                ,{ -240.f, -20.f },{ 240.f, -20.f },{ -300.f, -20.f },{ 300.f, -20.f },{ -80.f, -80.f }
                                ,{ 80.f, -80.f },{ -160.f, -80.f },{ 160.f, -80.f },{ -240.f, -80.f },{ 240.f, -80.f }
                                ,{ -300.f, -80.f },{ 300.f, -80.f },{ -20.f, 180.f },{ 20.f, 180.f },{ -100.f, 180.f }
                                ,{ 100.f, 180.f },{ -200.f, 180.f },{ 200.f, 180.f },{ -300.f, 180.f },{ 300.f, 180.f }
                            };
                            for (Vec2& pos : pillarPos) {
                                t.pos.xy = pos;
                                d3dcontext->UpdateSubresource(cbuffers[ConstantBuffer::WorldMatrix], 0, nullptr, t.dataCM, 0, 0);
                                d3dcontext->DrawIndexed(indexCount2, 0, 0);
                            }
                        }
                    }
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
}
