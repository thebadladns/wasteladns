#ifndef __WASTELADNS_RENDERER_TYPES_GLFW_H__
#define __WASTELADNS_RENDERER_TYPES_GLFW_H__

#ifndef UNITYBUILD
#include "core.h"
#endif

namespace Renderer {
namespace Driver {
    
    // empty for now
    struct RscMainRenderTarget {
        u32 mask;
    };
    
    struct RscTexture {
        GLuint texId;
    };

    template <typename _vertexLayout, typename _cbufferLayout>
    struct RscVertexShader {
        GLuint shaderObject;
    };
    struct RscPixelShader {
        GLuint shaderObject;
    };
    template <typename _vertexLayout, typename _cbufferLayout>
    struct RscShaderSet {
        GLuint shaderObject;
    };
    
    struct RscBlendState {
        bool enable : 1;
    };
    
    struct RscRasterizerState {
        GLenum fillMode;
        bool cullFace : 1;
    };
    
    template <typename _layout>
    struct RscBuffer {
        GLsizei vertexCount;
        GLuint arrayObject;
        GLuint vertexBuffer;
        GLenum type;
    };
    
    template <typename _layout>
    struct RscIndexedBuffer {
        GLsizei indexCount;
        GLuint arrayObject;
        GLuint vertexBuffer;
        GLuint indexBuffer;
        GLenum type;
        GLenum indexType;
    };
    
    struct RscCBuffer {
        GLuint bufferObject;
        GLuint index;
    };
}
}
#endif // __WASTELADNS_RENDERER_TYPES_GLFW_H__
