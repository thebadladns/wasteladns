#ifndef __WASTELADNS_RENDERER_TYPES_GLFW_H__
#define __WASTELADNS_RENDERER_TYPES_GLFW_H__

#ifndef UNITYBUILD
#include "core.h"
#endif

namespace Renderer {

    constexpr ProjectionType::Enum defaultProjectionType = ProjectionType::Zminus1to1;

namespace Driver {

    struct Type { enum Enum { Float = GL_FLOAT }; };
    struct InternalTextureFormat { enum Enum { V316 = GL_RGB16F }; };
    struct TextureFormat { enum Enum { V316 = GL_RGB }; };
    struct RasterizerFillMode { enum Enum { Fill = GL_FILL, Line = GL_LINE }; };
    struct RasterizerCullMode { enum Enum { CullFront = GL_FRONT, CullBack = GL_BACK, CullNone = 0 }; };
    struct BufferMemoryUsage { enum Enum { GPU = GL_STATIC_DRAW, CPU = GL_DYNAMIC_DRAW }; };
    struct BufferAccessType { enum Enum { GPU = GL_STATIC_DRAW, CPU = GL_DYNAMIC_DRAW }; }; // repeated, compatibility-only
    struct BufferItemType { enum Enum { U16 = GL_UNSIGNED_SHORT, U32 = GL_UNSIGNED_INT }; };
    struct BufferTopologyType { enum Enum { Triangles = GL_TRIANGLES, Lines = GL_LINES }; };

    struct RscMainRenderTarget {
        u32 mask;
    };
    
    struct RscRenderTarget {
        GLuint buffer;
        GLuint depthBuffer;
        u32 width, height;
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
        GLenum cullFace;
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
