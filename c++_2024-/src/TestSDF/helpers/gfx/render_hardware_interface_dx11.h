#ifndef __WASTELADNS_RHI_DX11_H__
#define __WASTELADNS_RHI_DX11_H__

namespace gfx {
namespace rhi { // render hardware interface

ID3D11Device1* d3ddev;
ID3D11DeviceContext1* d3dcontext;
IDXGISwapChain1* swapchain;
ID3DUserDefinedAnnotation* perf;

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
    }
}
void clear_main_RT(const RscMainRenderTarget& rt, Color32 color) {
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

    if (params.flags & RenderTargetParams::Flags::EnableDepth) {
        // same format as texture, 0 minmap
        D3D11_TEXTURE2D_DESC stencilDesc = {};
        stencilDesc.Width = params.width;
        stencilDesc.Height = params.height;
        stencilDesc.MipLevels = 1;
        stencilDesc.ArraySize = 1;
        stencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
        stencilDesc.SampleDesc.Count = 1;
        stencilDesc.SampleDesc.Quality = 0;
        stencilDesc.Usage = D3D11_USAGE_DEFAULT;
        stencilDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_DEPTH_STENCIL;
        stencilDesc.CPUAccessFlags = 0;
        stencilDesc.MiscFlags = 0;
        d3ddev->CreateTexture2D(&stencilDesc, nullptr, &rt.depthStencil.impl);

        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
        descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
        d3ddev->CreateDepthStencilView(rt.depthStencil.impl, &descDSV, &rt.depthStencilView);

        if (params.flags & RenderTargetParams::Flags::ReadDepth) {

            D3D11_SHADER_RESOURCE_VIEW_DESC texViewDesc = {};
            texViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            texViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            texViewDesc.Texture2D.MipLevels = 1;
            texViewDesc.Texture2D.MostDetailedMip = 0;
            d3ddev->CreateShaderResourceView(rt.depthStencil.impl, &texViewDesc, &rt.depthStencil.view);

            D3D11_SAMPLER_DESC samplerDesc = {};
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
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
            samplerDesc.MaxLOD = 0;

            d3ddev->CreateSamplerState(&samplerDesc, &rt.depthStencil.samplerState);
        }
    }

    {
        for (u32 i = 0; i < params.count; i++) {
            gfx::rhi::TextureRenderTargetCreateParams texParams;
            texParams.width = params.width;
            texParams.height = params.height;
            texParams.format = params.textureFormat;
            texParams.internalFormat = params.textureInternalFormat;
            texParams.type = params.textureFormatType;
            create_texture_empty(rt.textures[i], texParams);

            D3D11_RENDER_TARGET_VIEW_DESC rtViewDesc = {};
            rtViewDesc.Format = (DXGI_FORMAT) texParams.format;
            rtViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtViewDesc.Texture2D.MipSlice = 0;
            d3ddev->CreateRenderTargetView(rt.textures[i].impl, &rtViewDesc, &rt.views[i]);
        }
        rt.count = params.count;
    }
}
void bind_RT(const RscRenderTarget& rt) {
    d3dcontext->OMSetRenderTargets(rt.count, rt.views, rt.depthStencilView);
}
void clear_RT(const RscRenderTarget& rt, u32 flags) {
    if (flags & RenderTargetClearFlags::Color) {
        float colorv[] = { 0.f, 0.f, 0.f, 0.f };
        for (u32 i = 0; i < rt.count; i++) {
            d3dcontext->ClearRenderTargetView(rt.views[i], colorv);
        }
    }
    if (rt.depthStencil.view) {
        flags &= ~RenderTargetClearFlags::Color;
        d3dcontext->ClearDepthStencilView(rt.depthStencilView, flags, 1.f, 0);
    }
}
void clear_RT(const RscRenderTarget& rt, u32 flags, Color32 color) {
    float colorv[] = { color.getRf(), color.getGf(), color.getBf(), color.getAf() };
    for (u32 i = 0; i < rt.count; i++) {
        d3dcontext->ClearRenderTargetView(rt.views[i], colorv);
    }
    if (rt.depthStencilView) {
        flags &= ~RenderTargetClearFlags::Color;
        d3dcontext->ClearDepthStencilView(rt.depthStencilView, flags, 1.f, 0);
    }
}
// ONLY WORKS WITH SAME RESOLUTION
void copy_RT_to_main_RT(RscMainRenderTarget& dst, const RscRenderTarget& src, const RenderTargetCopyParams& params) {
    {
        ID3D11Resource* srcColor;
        ID3D11Resource* dstColor;
        src.views[0]->GetResource(&srcColor);
        dst.view->GetResource(&dstColor);
        d3dcontext->CopyResource(dstColor, srcColor);
    }
    if (params.depth) {
        ID3D11Resource* srcDepth;
        ID3D11Resource* dstDepth;
        src.depthStencilView->GetResource(&srcDepth);
        dst.depthStencilView->GetResource(&dstDepth);
        d3dcontext->CopyResource(dstDepth, srcDepth);
    }
}

void set_VP(const ViewportParams& params) {
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = params.topLeftX;
    viewport.TopLeftY = params.topLeftY;
    viewport.Width = params.width;
    viewport.Height = params.height;
    viewport.MinDepth = 0.f; // todo: check
    viewport.MaxDepth = 1.f;
    d3dcontext->RSSetViewports(1, &viewport);
}

void create_texture_from_file(RscTexture& t, const TextureFromFileParams& params) {
    s32 w, h, channels;
    u8* data = stbi_load_arena(params.path, &w, &h, &channels, 4, params.arena);
    if (data) {
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        u32 typeSize = 4;
        switch (channels) {
        case 1: format = DXGI_FORMAT_R32_FLOAT; typeSize = 4; break;
        case 4: format = DXGI_FORMAT_R8G8B8A8_UNORM; typeSize = 4; break;
        default: assert("unhandled texture format");
        }
        t.format = format;

        // TODO: parameters!!
        bool mipsautogen = true;
        UINT miplevelsDesc = 0;
        UINT miplevelsGen = -1;
        UINT bindflags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        UINT miscflags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        FLOAT maxlod = D3D11_FLOAT32_MAX;
        if (w <= 64) {
            filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            maxlod = 0;
            mipsautogen = false;
            miscflags = 0;
            miplevelsDesc = miplevelsGen = 1;
            bindflags = D3D11_BIND_SHADER_RESOURCE;
        }

        // Do not initialize data here, since we'll use auto mipmapss
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = w;
        texDesc.Height = h;
        texDesc.MipLevels = miplevelsDesc;
        texDesc.ArraySize = 1;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.Format = format;
        texDesc.BindFlags = bindflags;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = miscflags;

        // Initialize data and generate mips
        if (mipsautogen) {
            d3ddev->CreateTexture2D(&texDesc, nullptr, &t.impl);
        }
        else {
            D3D11_SUBRESOURCE_DATA initData;
            initData.pSysMem = data;
            initData.SysMemPitch = (UINT)(typeSize * w);
            initData.SysMemSlicePitch = (UINT)(typeSize * w * h);
            d3ddev->CreateTexture2D(&texDesc, &initData, &t.impl);
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC texViewDesc = {};
        texViewDesc.Format = format;
        texViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        texViewDesc.Texture2D.MipLevels = miplevelsGen;
        texViewDesc.Texture2D.MostDetailedMip = 0;
        d3ddev->CreateShaderResourceView(t.impl, &texViewDesc, &t.view);

        if (mipsautogen) {
            d3dcontext->UpdateSubresource(t.impl, 0, nullptr, data, typeSize * w, typeSize * w * h);
            d3dcontext->GenerateMips(t.view);
        }
            
        // Sampler tied to texture resource, for now
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = filter;
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
        samplerDesc.MaxLOD = maxlod;

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
    d3ddev->CreateTexture2D(&texDesc, nullptr, &t.impl);

    D3D11_SHADER_RESOURCE_VIEW_DESC texViewDesc = {};
    texViewDesc.Format = format;
    texViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    texViewDesc.Texture2D.MipLevels = 1;
    texViewDesc.Texture2D.MostDetailedMip = 0;
    d3ddev->CreateShaderResourceView(t.impl, &texViewDesc, &t.view);

    // TODO: sampler params
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;// D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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
    samplerDesc.MaxLOD = 0;// D3D11_FLOAT32_MAX;

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

ShaderResult create_shader_vs(RscVertexShader& vs, const VertexShaderRuntimeCompileParams& params) {
    d3ddev->CreateVertexShader(params.shader_src, params.shader_length, nullptr, &vs.impl);
    d3ddev->CreateInputLayout(
        params.attribs, params.attrib_count, params.shader_src, params.shader_length,
        &vs.inputLayout_impl);
    ShaderResult result = {};
    result.compiled = true;
    return result;
}
ShaderResult create_shader_ps(RscPixelShader& ps, const PixelShaderRuntimeCompileParams& params) {
    d3ddev->CreatePixelShader(params.shader_src, params.shader_length, nullptr, &ps.impl);

    ShaderResult result = {};
    result.compiled = true;
    return result;
}
ShaderResult create_shader_set(RscShaderSet& ss, const ShaderSetRuntimeCompileParams& params) {
    ss.vs = params.vs;
    ss.ps = params.ps;
    for (u32 i = 0; i < params.cbuffer_count; i++) {
        const CBufferBindingDesc& binding = params.cbufferBindings[i];
        if (binding.stageMask & CBufferStageMask::VS) { ss.cbuffer_bindings_vs[ss.cbuffer_vs_count++] = i; }
        if (binding.stageMask & CBufferStageMask::PS) { ss.cbuffer_bindings_ps[ss.cbuffer_ps_count++] = i; }
    }
    ShaderResult result;
    result.compiled = true;
    return result;
}
void bind_shader(const RscShaderSet& ss) {
    d3dcontext->VSSetShader(ss.vs.impl, nullptr, 0);
    d3dcontext->PSSetShader(ss.ps.impl, nullptr, 0);
    d3dcontext->IASetInputLayout(ss.vs.inputLayout_impl);
}
    
void create_blend_state(RscBlendState& bs, const BlendStateParams& params) {
    // Todo: make parameters when needed
    ID3D11BlendState1* blendState;
    D3D11_BLEND_DESC1 blendStateDesc = {};
    blendStateDesc.RenderTarget[0].BlendEnable = params.blendEnable;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = params.renderTargetWriteMask;
    d3ddev->CreateBlendState1(&blendStateDesc, &blendState);

    bs.impl = blendState;
}
void bind_blend_state(const RscBlendState& bs) {
    d3dcontext->OMSetBlendState(bs.impl, 0, 0xffffffff);
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
    rasterizerDesc.ScissorEnable = params.scissor;
    rasterizerDesc.SlopeScaledDepthBias = 0.f;
    d3ddev->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
    rs.impl = rasterizerState;
}
void bind_RS(const RscRasterizerState& rs) {
    d3dcontext->RSSetState(rs.impl);
}
void set_scissor(const u32 left, const u32 top, const u32 right, const u32 bottom) {
    // todo: ensure appropriate rasterizer state?
    D3D11_RECT rect = {};
    rect.left = left;
    rect.bottom = bottom;
    rect.right = right;
    rect.top = top;
    d3dcontext->RSSetScissorRects(1, &rect);
}

void create_DS(RscDepthStencilState& ds, const DepthStencilStateParams& params) {
    ID3D11DepthStencilState* depthStencilState;
    D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc = {};
    depthStencilStateDesc.DepthEnable = params.depth_enable;
    depthStencilStateDesc.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK) params.depth_writemask;
    depthStencilStateDesc.DepthFunc = (D3D11_COMPARISON_FUNC) params.depth_func;
    depthStencilStateDesc.StencilEnable = params.stencil_enable;
    depthStencilStateDesc.StencilReadMask = params.stencil_readmask;
    depthStencilStateDesc.StencilWriteMask = params.stencil_writemask;
    depthStencilStateDesc.FrontFace.StencilFailOp = (D3D11_STENCIL_OP) params.stencil_failOp;
    depthStencilStateDesc.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)params.stencil_depthFailOp;
    depthStencilStateDesc.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)params.stencil_passOp;
    depthStencilStateDesc.FrontFace.StencilFunc = (D3D11_COMPARISON_FUNC) params.stencil_func;
    depthStencilStateDesc.BackFace.StencilFailOp = (D3D11_STENCIL_OP)params.stencil_failOp;
    depthStencilStateDesc.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)params.stencil_depthFailOp;
    depthStencilStateDesc.BackFace.StencilPassOp = (D3D11_STENCIL_OP)params.stencil_passOp;
    depthStencilStateDesc.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)params.stencil_func;
    depthStencilStateDesc.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC) params.stencil_func;
    d3ddev->CreateDepthStencilState(&depthStencilStateDesc, &depthStencilState);
    ds.impl = depthStencilState;
}
void bind_DS(const RscDepthStencilState& ds, const u32 stencilRef = 0) {
    d3dcontext->OMSetDepthStencilState(ds.impl, stencilRef);
}
    
void create_vertex_buffer(RscVertexBuffer& t, const VertexBufferDesc& params, const VertexAttribDesc*, const u32) {
    ID3D11Buffer* vertexBuffer;

    D3D11_SUBRESOURCE_DATA resourceData = { 0 };

    D3D11_BUFFER_DESC vertexVertexBufferDesc = { 0 };
    vertexVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexVertexBufferDesc.ByteWidth = params.vertexSize;
    vertexVertexBufferDesc.CPUAccessFlags = params.accessType;
    vertexVertexBufferDesc.Usage = (D3D11_USAGE) params.memoryUsage;
    resourceData = { params.vertexData, 0, 0 };
    HRESULT result = d3ddev->CreateBuffer(&vertexVertexBufferDesc, params.memoryUsage == BufferMemoryUsage::CPU ? nullptr : &resourceData, &vertexBuffer);
    (void)result;

    t.impl = vertexBuffer;
    t.type = (D3D11_PRIMITIVE_TOPOLOGY) params.type;
    t.vertexCount = params.vertexCount;
    t.vertexStride = params.vertexSize / params.vertexCount; // check
}
void update_vertex_buffer(RscVertexBuffer& b, const BufferUpdateParams& params) {
    D3D11_MAPPED_SUBRESOURCE mapped;
    d3dcontext->Map(b.impl, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapped);
    memcpy(mapped.pData, params.vertexData, params.vertexSize);
    d3dcontext->Unmap(b.impl, 0);
    b.vertexCount = params.vertexCount;
}
void bind_vertex_buffer(const RscVertexBuffer& b) {
    u32 offset = 0;
    d3dcontext->IASetVertexBuffers(0, 1, &b.impl, &b.vertexStride, &offset);
    d3dcontext->IASetPrimitiveTopology(b.type);
}
void draw_vertex_buffer(const RscVertexBuffer& b) {
    d3dcontext->Draw(b.vertexCount, 0);
}
    
void create_indexed_vertex_buffer(RscIndexedVertexBuffer& t, const IndexedVertexBufferDesc& params, const VertexAttribDesc*, const u32) {
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;

    D3D11_SUBRESOURCE_DATA resourceData = { 0 };

    D3D11_BUFFER_DESC vertexVertexBufferDesc = { 0 };
    vertexVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexVertexBufferDesc.ByteWidth = params.vertexSize;
    vertexVertexBufferDesc.CPUAccessFlags = params.accessType;
    vertexVertexBufferDesc.Usage = (D3D11_USAGE) params.memoryUsage;
    resourceData = { params.vertexData, 0, 0 };
    d3ddev->CreateBuffer(&vertexVertexBufferDesc, params.memoryUsage == BufferMemoryUsage::CPU ? nullptr : &resourceData, &vertexBuffer);

    D3D11_BUFFER_DESC indexVertexBufferDesc = { 0 };
    indexVertexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexVertexBufferDesc.ByteWidth = params.indexSize;
    indexVertexBufferDesc.CPUAccessFlags = params.accessType;
    indexVertexBufferDesc.Usage = (D3D11_USAGE) params.memoryUsage;
    resourceData = { params.indexData, 0, 0 };
    d3ddev->CreateBuffer(&indexVertexBufferDesc, params.memoryUsage == BufferMemoryUsage::CPU ? nullptr : &resourceData, &indexBuffer);

    t.vertexBuffer_impl = vertexBuffer;
    t.indexBuffer_impl = indexBuffer;
    t.indexType = (DXGI_FORMAT) params.indexType;
    t.type = (D3D11_PRIMITIVE_TOPOLOGY) params.type;
    t.indexCount = params.indexCount;
    t.vertexStride = params.vertexSize / params.vertexCount;
    t.indexOffset = 0;
}
void update_indexed_vertex_buffer(RscIndexedVertexBuffer& b, const IndexedBufferUpdateParams& params) {
    D3D11_MAPPED_SUBRESOURCE mapped;
    d3dcontext->Map(b.vertexBuffer_impl, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapped);
    memcpy(mapped.pData, params.vertexData, params.vertexSize);
    d3dcontext->Unmap(b.vertexBuffer_impl, 0);
    d3dcontext->Map(b.indexBuffer_impl, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapped);
    memcpy(mapped.pData, params.indexData, params.indexSize);
    d3dcontext->Unmap(b.indexBuffer_impl, 0);
    b.indexCount = params.indexCount;
}
void bind_indexed_vertex_buffer(const RscIndexedVertexBuffer& b) {
    u32 offset = 0;
    d3dcontext->IASetVertexBuffers(0, 1, &b.vertexBuffer_impl, &b.vertexStride, &offset);
    d3dcontext->IASetIndexBuffer(b.indexBuffer_impl, b.indexType, 0);
    d3dcontext->IASetPrimitiveTopology(b.type);
}
void draw_indexed_vertex_buffer(const RscIndexedVertexBuffer& b) {
    d3dcontext->DrawIndexed(b.indexCount, b.indexOffset, 0);

}
void draw_instances_indexed_vertex_buffer(const RscIndexedVertexBuffer& b, const u32 instanceCount) {
    d3dcontext->DrawIndexedInstanced(b.indexCount, instanceCount, 0, 0, 0);
}

void draw_fullscreen() {
    d3dcontext->IASetInputLayout(nullptr);
    d3dcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3dcontext->Draw(3, 0);
}

void create_cbuffer(RscCBuffer& cb, const CBufferCreateParams& params) {
    ID3D11Buffer* bufferObject;
    D3D11_BUFFER_DESC constantVertexBufferDesc = { 0 };
    constantVertexBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantVertexBufferDesc.ByteWidth = params.byteWidth;
    constantVertexBufferDesc.CPUAccessFlags = 0;
    constantVertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    d3ddev->CreateBuffer(&constantVertexBufferDesc, nullptr, &bufferObject);

    cb.impl = bufferObject;
}
void update_cbuffer(RscCBuffer& cb, const void* data) {
    d3dcontext->UpdateSubresource(cb.impl, 0, nullptr, data, 0, 0); // todo: this should probably be map/unmap
}
void bind_cbuffers(const RscShaderSet& ss, const RscCBuffer* cb, const u32 count) {
    u32 vs_count = 0, ps_count = 0;
    ID3D11Buffer* vs_cbuffers[4];
    ID3D11Buffer* ps_cbuffers[4];
    for (u32 i = 0; i < ss.cbuffer_vs_count; i++) {
        vs_cbuffers[vs_count++] = cb[ss.cbuffer_bindings_vs[i]].impl;
    }
    for (u32 i = 0; i < ss.cbuffer_ps_count; i++) {
        ps_cbuffers[ps_count++] = cb[ss.cbuffer_bindings_ps[i]].impl;
    }
    if (vs_count) { d3dcontext->VSSetConstantBuffers(0, vs_count, vs_cbuffers); }
    if (ps_count) { d3dcontext->PSSetConstantBuffers(0, ps_count, ps_cbuffers); }
}
#if __PROFILE
void start_event(const char* ansi) {
    wchar_t wide[64];
    size_t converted;
    mbstowcs_s(&converted, wide, ansi, countof(wide));
    perf->BeginEvent(wide);
}
void end_event() {
    perf->EndEvent();
}
#endif

} // rhi
} // gfx
#endif // __WASTELADNS_RHI_DX11_H__
