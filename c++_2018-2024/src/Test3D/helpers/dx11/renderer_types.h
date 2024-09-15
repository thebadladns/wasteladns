#ifndef __WASTELADNS_RENDERER_TYPES_DX11_H__
#define __WASTELADNS_RENDERER_TYPES_DX11_H__

#ifndef UNITYBUILD
#include "core.h"
#endif

namespace Renderer {

    constexpr ProjectionType::Enum defaultProjectionType = ProjectionType::Z0to1;

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
        ID3D11Texture2D* texture;
        ID3D11ShaderResourceView* view;
        ID3D11SamplerState* samplerState;
        DXGI_FORMAT format;
    };

    struct RscMainRenderTarget {
        ID3D11RenderTargetView* view;
        ID3D11DepthStencilView* depthStencilView;
    };
    template<u32 _attachments>
    struct RscRenderTarget {
        RscTexture textures[_attachments];
        ID3D11RenderTargetView* views[_attachments];
        ID3D11DepthStencilView* depthStencilView;
    };

    template <typename _vertexLayout, typename _cbufferLayout, Shaders::VSDrawType::Enum _drawType>
    struct RscVertexShader {
        ID3D11VertexShader* shaderObject;
        ID3D11InputLayout* inputLayout;
    };
    template <Shaders::PSCBufferUsage::Enum _cbufferUsage>
    struct RscPixelShader {
        ID3D11PixelShader* shaderObject;
    };
    template <typename _vertexLayout, typename _cbufferLayout, Shaders::PSCBufferUsage::Enum _cbufferUsage, Shaders::VSDrawType::Enum _drawType>
    struct RscShaderSet {
        RscVertexShader<_vertexLayout, _cbufferLayout, _drawType> vs;
        RscPixelShader<_cbufferUsage> ps;
    };
    
    struct RscBlendState {
        ID3D11BlendState1* bs;
    };
    
    struct RscRasterizerState {
        ID3D11RasterizerState* rs;
    };

    struct RscDepthStencilState {
        ID3D11DepthStencilState* ds;
    };
    
    template <typename _layout>
    struct RscBuffer {
        ID3D11Buffer* vertexBuffer;
        D3D11_PRIMITIVE_TOPOLOGY type;
        u32 vertexStride;
        u32 vertexCount;
    };
    
    template <typename _layout>
    struct RscIndexedBuffer {
        ID3D11Buffer* vertexBuffer;
        ID3D11Buffer* indexBuffer;
        D3D11_PRIMITIVE_TOPOLOGY type;
        DXGI_FORMAT indexType;
        u32 vertexStride;
        u32 indexCount;
    };
    
    struct RscCBuffer {
        ID3D11Buffer* bufferObject;
    };

    typedef LPCWSTR Marker_t;
#define SET_MARKER_NAME(a, b) a = L##b;
    void set_marker(Marker_t data);
    void start_event(Marker_t data);
    void end_event();
}
}
#endif // __WASTELADNS_RENDERER_TYPES_DX11_H__
