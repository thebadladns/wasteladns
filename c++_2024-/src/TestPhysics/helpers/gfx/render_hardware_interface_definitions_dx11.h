#ifndef __WASTELADNS_RHI_DEFS_DX11_H__
#define __WASTELADNS_RHI_DEFS_DX11_H__

namespace gfx {
const auto generate_matrix_ortho = camera::generate_matrix_ortho_z0to1;
const auto generate_matrix_persp = camera::generate_matrix_persp_z0to1;
const auto add_oblique_plane_to_persp = camera::add_oblique_plane_to_persp_z0to1;
const auto extract_frustum_planes_from_vp = camera::extract_frustum_planes_from_vp_z0to1;
const f32 min_z = 0.f;
}

namespace gfx {
namespace rhi { // render hardware interface

struct Type { enum Enum { Float }; }; // unused, compatibility-only
struct InternalTextureFormat { enum Enum {
    V4_8 = DXGI_FORMAT_B8G8R8A8_UNORM, V316 = DXGI_FORMAT_R16G16B16A16_FLOAT }; };
struct TextureFormat { enum Enum {
    V4_8 = DXGI_FORMAT_B8G8R8A8_UNORM, V4_16 = DXGI_FORMAT_R16G16B16A16_FLOAT }; };
struct RenderTargetClearFlags { enum Enum {
    Stencil = D3D11_CLEAR_STENCIL, Depth = D3D11_CLEAR_DEPTH, Color = 0x4L }; };
struct RenderTargetWriteMask { enum Enum {
    All = D3D11_COLOR_WRITE_ENABLE_ALL, None = 0 }; };
struct RasterizerFillMode { enum Enum {
    Fill = D3D11_FILL_SOLID, Line = D3D11_FILL_WIREFRAME }; };
struct RasterizerCullMode { enum Enum {
    CullFront = D3D11_CULL_FRONT, CullBack = D3D11_CULL_BACK, CullNone = D3D11_CULL_NONE }; };
struct CompFunc { enum Enum {
    Never = D3D11_COMPARISON_NEVER, Always = D3D11_COMPARISON_ALWAYS,
    Less = D3D11_COMPARISON_LESS, LessEqual = D3D11_COMPARISON_LESS_EQUAL,
    Equal = D3D11_COMPARISON_EQUAL, NotEqual = D3D11_COMPARISON_NOT_EQUAL,
    Greater = D3D11_COMPARISON_GREATER, GreaterEqual = D3D11_COMPARISON_GREATER_EQUAL
}; };
struct DepthWriteMask { enum Enum {
    All = D3D11_DEPTH_WRITE_MASK_ALL, Zero = D3D11_DEPTH_WRITE_MASK_ZERO }; };
struct StencilOp { enum Enum {
    Keep = D3D11_STENCIL_OP_KEEP, Zero = D3D11_STENCIL_OP_ZERO,
    Replace = D3D11_STENCIL_OP_REPLACE, Invert = D3D11_STENCIL_OP_INVERT,
    Incr = D3D11_STENCIL_OP_INCR, Decr = D3D11_STENCIL_OP_DECR
}; };
struct BufferMemoryUsage { enum Enum {
    GPU = D3D11_USAGE_IMMUTABLE, CPU = D3D11_USAGE_DYNAMIC }; };
struct BufferAccessType { enum Enum {
    GPU = 0, CPU = D3D11_CPU_ACCESS_WRITE }; };
struct BufferItemType { enum Enum {
    U16 = DXGI_FORMAT_R16_UINT, U32 = DXGI_FORMAT_R32_UINT }; };
struct BufferTopologyType { enum Enum {
    Triangles = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    Lines = D3D_PRIMITIVE_TOPOLOGY_LINELIST }; };

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
    RscTexture depthStencil;
    ID3D11DepthStencilView* depthStencilView;
    u32 count;
};

struct ShaderCache {
    struct ByteCode { u8* data; u64 size; };
    allocator::PagedArena* arena;
    const char* path;
    ByteCode* shaderBytecode;
    u64 shaderBytecodeCount;
};
struct RscVertexShader {
    ID3D11VertexShader* impl;
    ID3D11InputLayout* inputLayout_impl;
};
struct RscPixelShader {
    ID3D11PixelShader* impl;
};
struct RscShaderSet {
    u32 cbuffer_bindings_vs[4];
    u32 cbuffer_bindings_ps[4];
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
    u32 indexOffset;
};
    
struct RscCBuffer {
    ID3D11Buffer* impl;
};

} // rhi
} // gfx
#endif // __WASTELADNS_RHI_DEFS_DX11_H__
