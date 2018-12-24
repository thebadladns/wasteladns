#ifndef __WASTELADNS_RENDERER_DX11_H__
#define __WASTELADNS_RENDERER_DX11_H__

#ifndef UNITYBUILD
#include "core.h"
#endif

namespace Renderer {
namespace Driver {

    // Todo: clean this up
    ID3D11Device1* d3ddev;
    ID3D11DeviceContext1* d3dcontext;
    IDXGISwapChain1* swapchain;

    D3D11_FILL_MODE toDXRasterizerFillMode[] = { D3D11_FILL_SOLID, D3D11_FILL_WIREFRAME };
    u32 toDXBufferAccessFlags[] = { 0, D3D11_CPU_ACCESS_WRITE };
    D3D11_USAGE toDXBufferCPUUsage[] = { D3D11_USAGE_IMMUTABLE, D3D11_USAGE_DYNAMIC };
    DXGI_FORMAT toDXBufferItemType[] = { DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R32_UINT };
    D3D_PRIMITIVE_TOPOLOGY toDXBufferTopologyType[] = { D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, D3D_PRIMITIVE_TOPOLOGY_LINELIST };
    
    void create(RscMainRenderTarget& rt, const RenderTargetParams& params) {
        ID3D11RenderTargetView* targetView;
        ID3D11Texture2D* backbuffer;
        swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &backbuffer);
        d3ddev->CreateRenderTargetView(backbuffer, nullptr, &targetView);

        rt.view = targetView;
    }
    void clear(const RscMainRenderTarget& rt, Col color) {
        Col bg(0.01f, 0.021f, 0.06f);
        d3dcontext->OMSetRenderTargets(1, &rt.view, nullptr);
        float colorv[] = { RGBA_PARAMS(color) };
        d3dcontext->ClearRenderTargetView(rt.view, colorv);
    }
    
    template <typename _vertexLayout, typename _cbufferLayout>
    ShaderResult create(RscVertexShader<_vertexLayout, _cbufferLayout>& vs, const VertexShaderRuntimeCompileParams& params) {
        ID3D11VertexShader* vertexShader = nullptr;
        ID3D11InputLayout* vertexInputLayout = nullptr;
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
            createLayout(vs, (void*)pShaderBlob->GetBufferPointer(), (u32)pShaderBlob->GetBufferSize());
        } else {
            snprintf(result.error, 128, "%.128s", pErrorBlob ? (char*)pErrorBlob->GetBufferPointer() : "Unknown shader error");
        }

        vs.shaderObject = vertexShader;

        if (pShaderBlob) pShaderBlob->Release();
        if (pErrorBlob) pErrorBlob->Release();

        return result;
    }
    ShaderResult create(RscPixelShader& ps, const PixelShaderRuntimeCompileParams& params) {
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
            snprintf(result.error, 128, "%.128s", pErrorBlob ? (char*)pErrorBlob->GetBufferPointer() : "Unknown shader error");
        }

        ps.shaderObject = pixelShader;

        if (pShaderBlob) pShaderBlob->Release();
        if (pErrorBlob) pErrorBlob->Release();

        return result;
    }
    template <typename _vertexLayout, typename _cbufferLayout>
    ShaderResult create(RscShaderSet<_vertexLayout, _cbufferLayout>& ss, const ShaderSetRuntimeCompileParams<_vertexLayout, _cbufferLayout>& params) {
        ss.vs = params.rscVS;
        ss.ps = params.rscPS;
        ShaderResult result;
        result.compiled = true;
        return result;
    }
    template <typename _vertexLayout, typename _cbufferLayout>
    void bind(const RscShaderSet<_vertexLayout, _cbufferLayout>& ss) {
        d3dcontext->VSSetShader(ss.vs.shaderObject, nullptr, 0);
        d3dcontext->PSSetShader(ss.ps.shaderObject, nullptr, 0);
        d3dcontext->IASetInputLayout(ss.vs.inputLayout);
    }
    
    void create(RscBlendState& bs, const BlendStateParams& params) {
        // Todo: understand and make parameters
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
    void bind(const RscBlendState bs) {
        d3dcontext->OMSetBlendState(bs.bs, 0, 0xffffffff);
    }
    
    void create(RscRasterizerState& rs, const RasterizerStateParams& params) {
        ID3D11RasterizerState* rasterizerState;
        D3D11_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.AntialiasedLineEnable = FALSE;
        rasterizerDesc.CullMode = params.cullFace ? D3D11_CULL_FRONT : D3D11_CULL_NONE;
        rasterizerDesc.DepthBias = 0;
        rasterizerDesc.DepthBiasClamp = 0.f;
        rasterizerDesc.DepthClipEnable = TRUE;
        rasterizerDesc.FillMode = toDXRasterizerFillMode[params.fill];
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.MultisampleEnable = FALSE;
        rasterizerDesc.ScissorEnable = FALSE;
        rasterizerDesc.SlopeScaledDepthBias = 0.f;
        d3ddev->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
        rs.rs = rasterizerState;
    }
    void bind(const RscRasterizerState rs) {
        d3dcontext->RSSetState(rs.rs);
    }
    
    template <typename _layout>
    void create(RscBuffer<_layout>& t, const BufferParams& params) {
        ID3D11Buffer* vertexBuffer;

        D3D11_SUBRESOURCE_DATA resourceData = { 0 };

        D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.ByteWidth = params.vertexSize;
        vertexBufferDesc.CPUAccessFlags = toDXBufferAccessFlags[params.memoryMode];
        vertexBufferDesc.Usage = toDXBufferCPUUsage[params.memoryMode];
        resourceData = { params.vertexData, 0, 0 };
        HRESULT result = d3ddev->CreateBuffer(&vertexBufferDesc, params.memoryMode == BufferMemoryMode::CPU ? nullptr : &resourceData, &vertexBuffer);

        t.vertexBuffer = vertexBuffer;
        t.type = toDXBufferTopologyType[params.type];
        t.vertexCount = params.vertexCount;
        t.vertexStride = sizeof(_layout);
    }
    template <typename _layout>
    void update(RscBuffer<_layout>& b, const BufferUpdateParams& params) {
        D3D11_MAPPED_SUBRESOURCE mapped;
        d3dcontext->Map(b.vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapped);
        memcpy(mapped.pData, params.vertexData, params.vertexSize);
        d3dcontext->Unmap(b.vertexBuffer, 0);
        b.vertexCount = params.vertexCount;
    }
    template <typename _layout>
    void bind(const RscBuffer<_layout>& b) {
        u32 offset = 0;
        d3dcontext->IASetVertexBuffers(0, 1, &b.vertexBuffer, &b.vertexStride, &offset);
        d3dcontext->IASetPrimitiveTopology(b.type);
    }
    template <typename _layout>
    void draw(const RscBuffer<_layout>& b) {
        d3dcontext->Draw(b.vertexCount, 0);
    }
    
    template <typename _layout>
    void create(RscIndexedBuffer<_layout>& t, const IndexedBufferParams& params) {
        ID3D11Buffer* vertexBuffer;
        ID3D11Buffer* indexBuffer;

        D3D11_SUBRESOURCE_DATA resourceData = { 0 };

        D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.ByteWidth = params.vertexSize;
        vertexBufferDesc.CPUAccessFlags = toDXBufferAccessFlags[params.memoryMode];
        vertexBufferDesc.Usage = toDXBufferCPUUsage[params.memoryMode];
        resourceData = { params.vertexData, 0, 0 };
        d3ddev->CreateBuffer(&vertexBufferDesc, params.memoryMode == BufferMemoryMode::CPU ? nullptr : &resourceData, &vertexBuffer);

        D3D11_BUFFER_DESC indexBufferDesc = { 0 };
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.ByteWidth = params.indexSize;
        indexBufferDesc.CPUAccessFlags = toDXBufferAccessFlags[params.memoryMode];
        indexBufferDesc.Usage = toDXBufferCPUUsage[params.memoryMode];
        resourceData = { params.indexData, 0, 0 };
        d3ddev->CreateBuffer(&indexBufferDesc, params.memoryMode == BufferMemoryMode::CPU ? nullptr : &resourceData, &indexBuffer);

        t.vertexBuffer = vertexBuffer;
        t.indexBuffer = indexBuffer;
        t.indexType = toDXBufferItemType[params.indexType];
        t.type = toDXBufferTopologyType[params.type];
        t.indexCount = params.indexCount;
        t.vertexStride = sizeof(_layout);
    }
    template <typename _layout>
    void update(RscIndexedBuffer<_layout>& b, const IndexedBufferUpdateParams& params) {
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
    void bind(const RscIndexedBuffer<_layout>& b) {
        u32 offset = 0;
        d3dcontext->IASetVertexBuffers(0, 1, &b.vertexBuffer, &b.vertexStride, &offset);
        d3dcontext->IASetIndexBuffer(b.indexBuffer, b.indexType, 0);
        d3dcontext->IASetPrimitiveTopology(b.type);
    }
    template <typename _layout>
    void draw(const RscIndexedBuffer<_layout>& b) {
        d3dcontext->DrawIndexed(b.indexCount, 0, 0);

    }
    template <typename _layout>
    void drawInstances(const RscIndexedBuffer<_layout>& b, const u32 instanceCount) {
        d3dcontext->DrawIndexedInstanced(b.indexCount, instanceCount, 0, 0, 0);
    }

    template <typename _layout>
    void create(RscCBuffer& cb, const CBufferCreateParams& params) {
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
    void update(RscCBuffer& cb, const _layout& data) {
        d3dcontext->UpdateSubresource(cb.bufferObject, 0, nullptr, &data, 0, 0);
    }
    void bind(const RscCBuffer* cb, const u32 count) {
        // works because an array of RscCBuffer maps to an array of tightly packed bufferObject
        d3dcontext->VSSetConstantBuffers(0, count, &(cb[0].bufferObject));
    }
}
}

// supported vertex layouts
namespace Renderer {
namespace Driver {
    
    template <typename _bufferLayout>
    void createLayout(RscVertexShader<Layout_Vec3, _bufferLayout>& vs, void* shaderBufferPointer, u32 shaderBufferSize) {
        ID3D11InputLayout* inputLayout;
        D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        d3ddev->CreateInputLayout(vertexLayoutDesc, ARRAYSIZE(vertexLayoutDesc), shaderBufferPointer, shaderBufferSize, &inputLayout);

        vs.inputLayout = inputLayout;
    }
    template <typename _bufferLayout>
    void createLayout(RscVertexShader<Layout_Vec2Color4B, _bufferLayout>& vs, void* shaderBufferPointer, u32 shaderBufferSize) {
        ID3D11InputLayout* inputLayout;
        D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
              { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Layout_Vec2Color4B, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 }
            , { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Layout_Vec2Color4B, color), D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
        d3ddev->CreateInputLayout(vertexLayoutDesc, ARRAYSIZE(vertexLayoutDesc), shaderBufferPointer, shaderBufferSize, &inputLayout);

        vs.inputLayout = inputLayout;
    }
    template <typename _bufferLayout>
    void createLayout(RscVertexShader<Layout_Vec3Color4B, _bufferLayout>& vs, void* shaderBufferPointer, u32 shaderBufferSize) {
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
