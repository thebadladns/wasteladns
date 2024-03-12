#ifndef __WASTELADNS_RENDERER_DX11_H__
#define __WASTELADNS_RENDERER_DX11_H__

#ifndef UNITYBUILD
#include "core.h"
#endif

namespace Renderer {

    void create_textured_quad(RenderTargetTexturedQuad& q) {
        Renderer::Layout_TexturedVec2* v = q.vertices;
        u16* i = q.indices;
        v[0] = { { -1.f, 1.f }, { 0.f, 0.f } };
        v[1] = { { -1.f, -1.f }, { 0.f, 1.f } };
        v[2] = { { 1.f, 1.f }, { 1.f, 0.f } };
        v[3] = { { 1.f, -1.f }, { 1.f, 1.f } };
        i[0] = 0; i[1] = 1; i[2] = 2;
        i[3] = 2; i[4] = 1; i[5] = 3;
    }

namespace Driver {

    // Todo: clean this up
    ID3D11Device1* d3ddev;
    ID3D11DeviceContext1* d3dcontext;
    IDXGISwapChain1* swapchain;

    void create_main_RT(RscMainRenderTarget& rt, const MainRenderTargetParams& params) {
        ID3D11RenderTargetView* targetView;
        ID3D11Texture2D* backbuffer;
        swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &backbuffer);
        d3ddev->CreateRenderTargetView(backbuffer, nullptr, &targetView);
        rt.view = targetView;

        // Make depth buffer a part of the main render target, for now
        if (params.depth) {

            ID3D11Texture2D* stencil = nullptr;
            D3D11_TEXTURE2D_DESC stencilDesc = {};
            stencilDesc.Width = params.width;
            stencilDesc.Height = params.height;
            stencilDesc.MipLevels = 1;
            stencilDesc.ArraySize = 1;
            stencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            stencilDesc.SampleDesc.Count = 1;
            stencilDesc.SampleDesc.Quality = 0;
            stencilDesc.Usage = D3D11_USAGE_DEFAULT;
            stencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            stencilDesc.CPUAccessFlags = 0;
            stencilDesc.MiscFlags = 0;
            d3ddev->CreateTexture2D(&stencilDesc, nullptr, &stencil);

            // same format as texture, 0 minmap
            d3ddev->CreateDepthStencilView(stencil, nullptr, &rt.depthStencilView);

            ID3D11DepthStencilState* stencilState;
            D3D11_DEPTH_STENCIL_DESC stencilStateDesc = {};
            stencilStateDesc.DepthEnable = true;
            stencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            stencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
            stencilStateDesc.StencilEnable = false;
            d3ddev->CreateDepthStencilState(&stencilStateDesc, &stencilState);

            // Todo: eventually this may not belong here
            d3dcontext->OMSetDepthStencilState(stencilState, 1);
        }
    }
    void clear_main_RT(const RscMainRenderTarget& rt, Col color) {
        float colorv[] = { RGBA_PARAMS(color) };
        d3dcontext->ClearRenderTargetView(rt.view, colorv);
        if (rt.depthStencilView) {
            d3dcontext->ClearDepthStencilView(rt.depthStencilView, D3D11_CLEAR_DEPTH, 1.f, 0);
        }
    }
    void bind_main_RT(RscMainRenderTarget& rt) {
        d3dcontext->OMSetRenderTargets(1, &rt.view, rt.depthStencilView);
    }

    void create_RT(RscRenderTarget& rt, const RenderTargetParams& params) {

        // Make depth buffer a part of the main render target, for now
        if (params.depth) {

            ID3D11Texture2D* stencil = nullptr;
            D3D11_TEXTURE2D_DESC stencilDesc = {};
            stencilDesc.Width = params.width;
            stencilDesc.Height = params.height;
            stencilDesc.MipLevels = 1;
            stencilDesc.ArraySize = 1;
            stencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            stencilDesc.SampleDesc.Count = 1;
            stencilDesc.SampleDesc.Quality = 0;
            stencilDesc.Usage = D3D11_USAGE_DEFAULT;
            stencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            stencilDesc.CPUAccessFlags = 0;
            stencilDesc.MiscFlags = 0;
            d3ddev->CreateTexture2D(&stencilDesc, nullptr, &stencil);

            // same format as texture, 0 minmap
            d3ddev->CreateDepthStencilView(stencil, nullptr, &rt.depthStencilView);
        }
    }
    void bind_RT(const RscRenderTarget& rt) {
        d3dcontext->OMSetRenderTargets(rt.viewCount, rt.views, rt.depthStencilView);
    }
    void clear_RT(const RscRenderTarget& rt) {
        float colorv[] = { 0.f, 0.f, 0.f, 0.f };
        for (u32 i = 0; i < rt.viewCount; i++) {
            d3dcontext->ClearRenderTargetView(rt.views[i], colorv);
        }
        if (rt.depthStencilView) {
            d3dcontext->ClearDepthStencilView(rt.depthStencilView, D3D11_CLEAR_DEPTH, 1.f, 0);
        }
    }
    void copy_RT_to_main_RT(RscMainRenderTarget& dst, const RscRenderTarget& src, const RenderTargetCopyParams& params) {
        if (params.depth) {
            ID3D11Resource* srcDepth;
            ID3D11Resource* dstDepth;
            src.depthStencilView->GetResource(&srcDepth);
            dst.depthStencilView->GetResource(&dstDepth);
            d3dcontext->CopyResource(dstDepth, srcDepth);
        }
    }

    void create_texture_from_file(RscTexture& t, const TextureFromFileParams& params) {
        s32 w, h, channels;
        u8* data = stbi_load(params.path, &w, &h, &channels, 0);
        if (data) {
            DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
            u32 typeSize = 0;
            switch (channels) {
            case 1: format = DXGI_FORMAT_R32_FLOAT; typeSize = 4; break;
            case 4: format = DXGI_FORMAT_R8G8B8A8_UNORM; typeSize = 4; break;
            default: assert("unhandled texture format");
            }
            t.format = format;

            // Do not initialize data here, since we'll use auto mipmapss
            ID3D11Texture2D* texture;
            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = w;
            texDesc.Height = h;
            texDesc.MipLevels = 0;
            texDesc.ArraySize = 1;
            texDesc.SampleDesc.Count = 1;
            texDesc.SampleDesc.Quality = 0;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.Format = format;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
            d3ddev->CreateTexture2D(&texDesc, nullptr, &texture);

            D3D11_SHADER_RESOURCE_VIEW_DESC texViewDesc = {};
            texViewDesc.Format = format;
            texViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            texViewDesc.Texture2D.MipLevels = -1;
            texViewDesc.Texture2D.MostDetailedMip = 0;
            d3ddev->CreateShaderResourceView(texture, &texViewDesc, &t.view);
            
            // Initialize data and generate mips
            d3dcontext->UpdateSubresource(texture, 0, nullptr, data, typeSize * w, typeSize * w * h);
            d3dcontext->GenerateMips(t.view);

            // Sampler tied to texture resource, for now
            D3D11_SAMPLER_DESC samplerDesc = {};
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDesc.MipLODBias = 0.0f;
            samplerDesc.MaxAnisotropy = 1;
            samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            samplerDesc.BorderColor[0] = 0;
            samplerDesc.BorderColor[1] = 0;
            samplerDesc.BorderColor[2] = 0;
            samplerDesc.BorderColor[3] = 0;
            samplerDesc.MinLOD = 0;
            samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

            d3ddev->CreateSamplerState(&samplerDesc, &t.samplerState);
            stbi_image_free(data);
        }
    }
    void create_texture_empty(RscTexture& t, const TextureRenderTargetCreateParams& params) {

        DXGI_FORMAT format = (DXGI_FORMAT)params.format;
        t.format = format;

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = params.width;
        texDesc.Height = params.height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = format;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;
        d3ddev->CreateTexture2D(&texDesc, nullptr, &t.texture);

        D3D11_SHADER_RESOURCE_VIEW_DESC texViewDesc = {};
        texViewDesc.Format = format;
        texViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        texViewDesc.Texture2D.MipLevels = 1;
        texViewDesc.Texture2D.MostDetailedMip = 0;
        d3ddev->CreateShaderResourceView(t.texture, &texViewDesc, &t.view);

        // Sampler tied to texture resource, for now
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.MipLODBias = 0.0f;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        samplerDesc.BorderColor[0] = 0;
        samplerDesc.BorderColor[1] = 0;
        samplerDesc.BorderColor[2] = 0;
        samplerDesc.BorderColor[3] = 0;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

        d3ddev->CreateSamplerState(&samplerDesc, &t.samplerState);
    }
    void bind_textures(const RscTexture* textures, const u32 count) {
        // hack, don't care for templates, dynamic arrays or alloca for now
        ID3D11ShaderResourceView* textureViews[16];
        ID3D11SamplerState* samplers[16];
        for (u32 i = 0; i < count; i++) {
            textureViews[i] = textures[i].view;
            samplers[i] = textures[i].samplerState;
        }
        d3dcontext->PSSetShaderResources(0, count, textureViews);
        d3dcontext->PSSetSamplers(0, count, samplers);
    }
    template <typename _vertexLayout, typename _cbufferLayout>
    void bind_shader_samplers(RscShaderSet<_vertexLayout, _cbufferLayout>& ss, const char** params, const u32 count) {}
    void bind_RTs(RscRenderTarget& b, const RscTexture* textures, const u32 count) {
        for (u32 i = 0; i < count; i++) {
            D3D11_RENDER_TARGET_VIEW_DESC rtViewDesc = {};
            rtViewDesc.Format = textures[i].format;
            rtViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtViewDesc.Texture2D.MipSlice = 0;
            d3ddev->CreateRenderTargetView(textures[i].texture, &rtViewDesc, &b.views[i]);
        }
        b.viewCount = count;
    }

    template <typename _vertexLayout, typename _cbufferLayout>
    ShaderResult create_shader_vs(RscVertexShader<_vertexLayout, _cbufferLayout>& vs, const VertexShaderRuntimeCompileParams& params) {
        ID3D11VertexShader* vertexShader = nullptr;
        ID3DBlob* pShaderBlob = nullptr;
        ID3DBlob* pErrorBlob = nullptr;
        HRESULT hr = D3DCompile(
              params.shaderStr, params.length, "VS"
            , nullptr // defines
            , D3D_COMPILE_STANDARD_FILE_INCLUDE
            , "VS"
            , "vs_5_0"
            , D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0
            , &pShaderBlob, &pErrorBlob
        );

        ShaderResult result;
        result.compiled = !FAILED(hr);
        if (result.compiled) {
            d3ddev->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), nullptr, &vertexShader);
            create_vertex_layout(vs, (void*)pShaderBlob->GetBufferPointer(), (u32)pShaderBlob->GetBufferSize());
        } else {
            Platform::format(result.error, 128, "%.128s", pErrorBlob ? (char*)pErrorBlob->GetBufferPointer() : "Unknown shader error");
        }

        vs.shaderObject = vertexShader;

        if (pShaderBlob) pShaderBlob->Release();
        if (pErrorBlob) pErrorBlob->Release();

        return result;
    }
    ShaderResult create_shader_ps(RscPixelShader& ps, const PixelShaderRuntimeCompileParams& params) {
        ID3D11PixelShader* pixelShader = nullptr;
        ID3DBlob* pShaderBlob = nullptr;
        ID3DBlob* pErrorBlob = nullptr;
        HRESULT hr = D3DCompile(
              params.shaderStr, params.length, "PS"
            , nullptr // defines
            , D3D_COMPILE_STANDARD_FILE_INCLUDE
            , "PS"
            , "ps_5_0"
            , D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, 0
            , &pShaderBlob, &pErrorBlob
        );

        ShaderResult result;
        result.compiled = !FAILED(hr);
        if (result.compiled) {
            d3ddev->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), nullptr, &pixelShader);
        } else {
            Platform::format(result.error, 128, "%.128s", pErrorBlob ? (char*)pErrorBlob->GetBufferPointer() : "Unknown shader error");
        }

        ps.shaderObject = pixelShader;

        if (pShaderBlob) pShaderBlob->Release();
        if (pErrorBlob) pErrorBlob->Release();

        return result;
    }
    template <typename _vertexLayout, typename _cbufferLayout>
    ShaderResult create_shader_set(RscShaderSet<_vertexLayout, _cbufferLayout>& ss, const ShaderSetRuntimeCompileParams<_vertexLayout, _cbufferLayout>& params) {
        ss.vs = params.rscVS;
        ss.ps = params.rscPS;
        ShaderResult result;
        result.compiled = true;
        return result;
    }
    template <typename _vertexLayout, typename _cbufferLayout>
    void bind_shader(const RscShaderSet<_vertexLayout, _cbufferLayout>& ss) {
        d3dcontext->VSSetShader(ss.vs.shaderObject, nullptr, 0);
        d3dcontext->PSSetShader(ss.ps.shaderObject, nullptr, 0);
        d3dcontext->IASetInputLayout(ss.vs.inputLayout);
    }
    
    void create_blend_state(RscBlendState& bs, const BlendStateParams& params) {
        // Todo: make parameters when needed
        ID3D11BlendState1* blendState;
        D3D11_BLEND_DESC1 blendStateDesc = {};
        blendStateDesc.RenderTarget[0].BlendEnable = params.enable ? TRUE : FALSE;
        blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        d3ddev->CreateBlendState1(&blendStateDesc, &blendState);

        bs.bs = blendState;
    }
    void bind_blend_state(const RscBlendState bs) {
        d3dcontext->OMSetBlendState(bs.bs, 0, 0xffffffff);
    }
    
    void create_RS(RscRasterizerState& rs, const RasterizerStateParams& params) {
        ID3D11RasterizerState* rasterizerState;
        D3D11_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.AntialiasedLineEnable = FALSE;
        rasterizerDesc.CullMode = (D3D11_CULL_MODE) params.cull;
        rasterizerDesc.DepthBias = 0;
        rasterizerDesc.DepthBiasClamp = 0.f;
        rasterizerDesc.DepthClipEnable = TRUE;
        rasterizerDesc.FillMode = (D3D11_FILL_MODE) params.fill;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.MultisampleEnable = FALSE;
        rasterizerDesc.ScissorEnable = FALSE;
        rasterizerDesc.SlopeScaledDepthBias = 0.f;
        d3ddev->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
        rs.rs = rasterizerState;
    }
    void bind_RS(const RscRasterizerState rs) {
        d3dcontext->RSSetState(rs.rs);
    }
    
    template <typename _layout>
    void create_vertex_buffer(RscBuffer<_layout>& t, const BufferParams& params) {
        ID3D11Buffer* vertexBuffer;

        D3D11_SUBRESOURCE_DATA resourceData = { 0 };

        D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.ByteWidth = params.vertexSize;
        vertexBufferDesc.CPUAccessFlags = params.accessType;
        vertexBufferDesc.Usage = (D3D11_USAGE) params.memoryUsage;
        resourceData = { params.vertexData, 0, 0 };
        HRESULT result = d3ddev->CreateBuffer(&vertexBufferDesc, params.memoryUsage == BufferMemoryUsage::CPU ? nullptr : &resourceData, &vertexBuffer);
        (void)result;

        t.vertexBuffer = vertexBuffer;
        t.type = (D3D11_PRIMITIVE_TOPOLOGY) params.type;
        t.vertexCount = params.vertexCount;
        t.vertexStride = sizeof(_layout);
    }
    template <typename _layout>
    void update_vertex_buffer(RscBuffer<_layout>& b, const BufferUpdateParams& params) {
        D3D11_MAPPED_SUBRESOURCE mapped;
        d3dcontext->Map(b.vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapped);
        memcpy(mapped.pData, params.vertexData, params.vertexSize);
        d3dcontext->Unmap(b.vertexBuffer, 0);
        b.vertexCount = params.vertexCount;
    }
    template <typename _layout>
    void bind_vertex_buffer(const RscBuffer<_layout>& b) {
        u32 offset = 0;
        d3dcontext->IASetVertexBuffers(0, 1, &b.vertexBuffer, &b.vertexStride, &offset);
        d3dcontext->IASetPrimitiveTopology(b.type);
    }
    template <typename _layout>
    void draw_vertex_buffer(const RscBuffer<_layout>& b) {
        d3dcontext->Draw(b.vertexCount, 0);
    }
    
    template <typename _layout>
    void create_indexed_vertex_buffer(RscIndexedBuffer<_layout>& t, const IndexedBufferParams& params) {
        ID3D11Buffer* vertexBuffer;
        ID3D11Buffer* indexBuffer;

        D3D11_SUBRESOURCE_DATA resourceData = { 0 };

        D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.ByteWidth = params.vertexSize;
        vertexBufferDesc.CPUAccessFlags = params.accessType;
        vertexBufferDesc.Usage = (D3D11_USAGE) params.memoryUsage;
        resourceData = { params.vertexData, 0, 0 };
        d3ddev->CreateBuffer(&vertexBufferDesc, params.memoryUsage == BufferMemoryUsage::CPU ? nullptr : &resourceData, &vertexBuffer);

        D3D11_BUFFER_DESC indexBufferDesc = { 0 };
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.ByteWidth = params.indexSize;
        indexBufferDesc.CPUAccessFlags = params.accessType;
        indexBufferDesc.Usage = (D3D11_USAGE) params.memoryUsage;
        resourceData = { params.indexData, 0, 0 };
        d3ddev->CreateBuffer(&indexBufferDesc, params.memoryUsage == BufferMemoryUsage::CPU ? nullptr : &resourceData, &indexBuffer);

        t.vertexBuffer = vertexBuffer;
        t.indexBuffer = indexBuffer;
        t.indexType = (DXGI_FORMAT) params.indexType;
        t.type = (D3D11_PRIMITIVE_TOPOLOGY) params.type;
        t.indexCount = params.indexCount;
        t.vertexStride = sizeof(_layout);
    }
    template <typename _layout>
    void update_indexed_vertex_buffer(RscIndexedBuffer<_layout>& b, const IndexedBufferUpdateParams& params) {
        D3D11_MAPPED_SUBRESOURCE mapped;
        d3dcontext->Map(b.vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapped);
        memcpy(mapped.pData, params.vertexData, params.vertexSize);
        d3dcontext->Unmap(b.vertexBuffer, 0);
        d3dcontext->Map(b.indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapped);
        memcpy(mapped.pData, params.indexData, params.indexSize);
        d3dcontext->Unmap(b.indexBuffer, 0);
        b.indexCount = params.indexCount;
    }
    template <typename _layout>
    void bind_indexed_vertex_buffer(const RscIndexedBuffer<_layout>& b) {
        u32 offset = 0;
        d3dcontext->IASetVertexBuffers(0, 1, &b.vertexBuffer, &b.vertexStride, &offset);
        d3dcontext->IASetIndexBuffer(b.indexBuffer, b.indexType, 0);
        d3dcontext->IASetPrimitiveTopology(b.type);
    }
    template <typename _layout>
    void draw_indexed_vertex_buffer(const RscIndexedBuffer<_layout>& b) {
        d3dcontext->DrawIndexed(b.indexCount, 0, 0);

    }
    template <typename _layout>
    void draw_instances_indexed_vertex_buffer(const RscIndexedBuffer<_layout>& b, const u32 instanceCount) {
        d3dcontext->DrawIndexedInstanced(b.indexCount, instanceCount, 0, 0, 0);
    }

    template <typename _layout>
    void create_cbuffer(RscCBuffer& cb, const CBufferCreateParams& params) {
        ID3D11Buffer* bufferObject;
        D3D11_BUFFER_DESC constantBufferDesc = { 0 };
        constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        constantBufferDesc.ByteWidth = sizeof(_layout);
        constantBufferDesc.CPUAccessFlags = 0;
        constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        d3ddev->CreateBuffer(&constantBufferDesc, nullptr, &bufferObject);

        cb.bufferObject = bufferObject;
    }
    template <typename _layout>
    void update_cbuffer(RscCBuffer& cb, const _layout& data) {
        d3dcontext->UpdateSubresource(cb.bufferObject, 0, nullptr, &data, 0, 0);
    }
    void bind_cbuffers(const RscCBuffer* cb, const u32 count, const CBufferBindParams& params) {
        // this all works because an array of RscCBuffer maps to an array of tightly packed bufferObject
        if (params.vertex) {
            d3dcontext->VSSetConstantBuffers(0, count, &(cb[0].bufferObject));
        }
        if (params.pixel) {
            d3dcontext->PSSetConstantBuffers(0, count, &(cb[0].bufferObject));
        }
    }
}
}

// supported vertex layouts
namespace Renderer {
namespace Driver {
    
    template <typename _bufferLayout>
    void create_vertex_layout(RscVertexShader<Layout_Vec3, _bufferLayout>& vs, void* shaderBufferPointer, u32 shaderBufferSize) {
        ID3D11InputLayout* inputLayout;
        D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        d3ddev->CreateInputLayout(vertexLayoutDesc, ARRAYSIZE(vertexLayoutDesc), shaderBufferPointer, shaderBufferSize, &inputLayout);

        vs.inputLayout = inputLayout;
    }
    template <typename _bufferLayout>
    void create_vertex_layout(RscVertexShader<Layout_TexturedVec2, _bufferLayout>& vs, void* shaderBufferPointer, u32 shaderBufferSize) {
        ID3D11InputLayout* inputLayout;
        D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
              { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Layout_TexturedVec2, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 }
            , { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Layout_TexturedVec2, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        d3ddev->CreateInputLayout(vertexLayoutDesc, ARRAYSIZE(vertexLayoutDesc), shaderBufferPointer, shaderBufferSize, &inputLayout);

        vs.inputLayout = inputLayout;
    }
    template <typename _bufferLayout>
    void create_vertex_layout(RscVertexShader<Layout_TexturedVec3, _bufferLayout>& vs, void* shaderBufferPointer, u32 shaderBufferSize) {
        ID3D11InputLayout* inputLayout;
        D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
              { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Layout_TexturedVec3, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 }
            , { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Layout_TexturedVec3, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 }
            , { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Layout_TexturedVec3, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 }
            , { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Layout_TexturedVec3, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0 }
            , { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Layout_TexturedVec3, bitangent), D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        d3ddev->CreateInputLayout(vertexLayoutDesc, ARRAYSIZE(vertexLayoutDesc), shaderBufferPointer, shaderBufferSize, &inputLayout);

        vs.inputLayout = inputLayout;
    }
    template <typename _bufferLayout>
    void create_vertex_layout(RscVertexShader<Layout_Vec2Color4B, _bufferLayout>& vs, void* shaderBufferPointer, u32 shaderBufferSize) {
        ID3D11InputLayout* inputLayout;
        D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
              { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Layout_Vec2Color4B, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 }
            , { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Layout_Vec2Color4B, color), D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        d3ddev->CreateInputLayout(vertexLayoutDesc, ARRAYSIZE(vertexLayoutDesc), shaderBufferPointer, shaderBufferSize, &inputLayout);

        vs.inputLayout = inputLayout;
    }
    template <typename _bufferLayout>
    void create_vertex_layout(RscVertexShader<Layout_Vec3Color4B, _bufferLayout>& vs, void* shaderBufferPointer, u32 shaderBufferSize) {
        ID3D11InputLayout* inputLayout;
        D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
              { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Layout_Vec3Color4B, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 }
            , { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Layout_Vec3Color4B, color), D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        d3ddev->CreateInputLayout(vertexLayoutDesc, ARRAYSIZE(vertexLayoutDesc), shaderBufferPointer, shaderBufferSize, &inputLayout);

        vs.inputLayout = inputLayout;
    }
}
}
#endif // __WASTELADNS_RENDERER_DX11_H__
