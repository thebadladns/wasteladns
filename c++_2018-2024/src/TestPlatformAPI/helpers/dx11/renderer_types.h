#ifndef __WASTELADNS_RENDERER_TYPES_DX11_H__
#define __WASTELADNS_RENDERER_TYPES_DX11_H__

#ifndef UNITYBUILD
#include "core.h"
#endif

namespace Renderer {
namespace Driver {
    
    struct RscMainRenderTarget {
        ID3D11RenderTargetView* view;
    };

    template <typename _vertexLayout, typename _cbufferLayout>
    struct RscVertexShader {
        ID3D11VertexShader* shaderObject;
        ID3D11InputLayout* inputLayout;
    };
    struct RscPixelShader {
        ID3D11PixelShader* shaderObject;
    };
    template <typename _vertexLayout, typename _cbufferLayout>
    struct RscShaderSet {
        ~RscShaderSet() {}
        RscVertexShader<_vertexLayout, _cbufferLayout> vs;
        RscPixelShader ps;
    };
    
    struct RscBlendState {
        ID3D11BlendState1* bs;
    };
    
    struct RscRasterizerState {
        ID3D11RasterizerState* rs;
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
}
}
#endif // __WASTELADNS_RENDERER_TYPES_DX11_H__
