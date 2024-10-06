#ifndef __WASTELADNS_RENDERER_TYPES_DX11_H__
#define __WASTELADNS_RENDERER_TYPES_DX11_H__

namespace Renderer {

    const auto generate_matrix_ortho = generate_matrix_ortho_z0to1;
    const auto generate_matrix_persp = generate_matrix_persp_z0to1;

namespace Driver {

    struct Type { enum Enum { Float }; }; // unused, compatibility-only
    struct InternalTextureFormat { enum Enum { V4_8 = DXGI_FORMAT_B8G8R8A8_UNORM, V316 = DXGI_FORMAT_R16G16B16A16_FLOAT }; };
    struct TextureFormat { enum Enum { V4_8 = DXGI_FORMAT_B8G8R8A8_UNORM, V4_16 = DXGI_FORMAT_R16G16B16A16_FLOAT }; };
    struct RasterizerFillMode { enum Enum { Fill = D3D11_FILL_SOLID, Line = D3D11_FILL_WIREFRAME }; };
    struct RasterizerCullMode { enum Enum { CullFront = D3D11_CULL_FRONT, CullBack = D3D11_CULL_BACK, CullNone = D3D11_CULL_NONE }; };
    struct DepthFunc { enum Enum { Less = D3D11_COMPARISON_LESS }; }; // TODO
    struct DepthWriteMask { enum Enum { All = D3D11_DEPTH_WRITE_MASK_ALL, Zero = D3D11_DEPTH_WRITE_MASK_ZERO }; };
    struct BufferMemoryUsage { enum Enum { GPU = D3D11_USAGE_IMMUTABLE, CPU = D3D11_USAGE_DYNAMIC }; };
    struct BufferAccessType { enum Enum { GPU = 0, CPU = D3D11_CPU_ACCESS_WRITE }; };
    struct BufferItemType { enum Enum { U16 = DXGI_FORMAT_R16_UINT, U32 = DXGI_FORMAT_R32_UINT }; };
    struct BufferTopologyType { enum Enum { Triangles = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, Lines = D3D_PRIMITIVE_TOPOLOGY_LINELIST }; };


    struct RscTexture {
        ID3D11Texture2D* impl;
        ID3D11ShaderResourceView* view;
        ID3D11SamplerState* samplerState;
        DXGI_FORMAT format;
    };

    struct RscMainRenderTarget {
        ID3D11RenderTargetView* view;
        ID3D11DepthStencilView* depthStencilView;
    };
    struct RscRenderTarget {
        RscTexture textures[RenderTarget_MaxCount];
        ID3D11RenderTargetView* views[RenderTarget_MaxCount];
        ID3D11DepthStencilView* depthStencilView;
        u32 count;
    };

    struct RscVertexShader {
        ID3D11VertexShader* impl;
        ID3D11InputLayout* inputLayout_impl;
    };
    struct RscPixelShader {
        ID3D11PixelShader* impl;
    };
    struct RscShaderSet {
        ID3D11Buffer* cbuffers_vs[4];
        ID3D11Buffer* cbuffers_ps[4];
        RscVertexShader vs;
        RscPixelShader ps;
        u16 cbuffer_vs_count;
        u16 cbuffer_ps_count;
    };

    struct BufferAttributeFormat { enum Enum {
            R32G32B32_FLOAT = DXGI_FORMAT_R32G32B32_FLOAT,
            R32G32_FLOAT = DXGI_FORMAT_R32G32_FLOAT,
            R8G8B8A8_SINT = DXGI_FORMAT_R8G8B8A8_SINT,
            R8G8B8A8_UNORM = DXGI_FORMAT_R8G8B8A8_UNORM,
    }; };
    typedef D3D11_INPUT_ELEMENT_DESC VertexAttribDesc;
    VertexAttribDesc make_vertexAttribDesc(const char* name, size_t offset, size_t , BufferAttributeFormat::Enum format) {
        return D3D11_INPUT_ELEMENT_DESC{
            name, 0, (DXGI_FORMAT)format, 0, (u32)offset, D3D11_INPUT_PER_VERTEX_DATA, 0 };
    }
    struct RscInputLayout {
        ID3D11InputLayout* impl;
    };

    struct RscBlendState {
        ID3D11BlendState1* impl;
    };
    
    struct RscRasterizerState {
        ID3D11RasterizerState* impl;
    };

    struct RscDepthStencilState {
        ID3D11DepthStencilState* impl;
    };
    
    struct RscVertexBuffer {
        ID3D11Buffer* impl;
        D3D11_PRIMITIVE_TOPOLOGY type;
        u32 vertexStride;
        u32 vertexCount;
    };
    
    struct RscIndexedVertexBuffer {
        ID3D11Buffer* vertexBuffer_impl;
        ID3D11Buffer* indexBuffer_impl;
        D3D11_PRIMITIVE_TOPOLOGY type;
        DXGI_FORMAT indexType;
        u32 vertexStride;
        u32 indexCount;
    };
    
    struct RscCBuffer {
        ID3D11Buffer* impl;
    };

    typedef wchar_t Marker_t[64];
}
}
#endif // __WASTELADNS_RENDERER_TYPES_DX11_H__
