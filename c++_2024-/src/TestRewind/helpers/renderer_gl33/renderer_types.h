#ifndef __WASTELADNS_RENDERER_TYPES_GL33_H__
#define __WASTELADNS_RENDERER_TYPES_GL33_H__

namespace Renderer {

    const auto generate_matrix_ortho = generate_matrix_ortho_zneg1to1;
    const auto generate_matrix_persp = generate_matrix_persp_zneg1to1;
    const f32 min_z = -1.f;

namespace Driver {

    struct Type { enum Enum { Float = GL_FLOAT }; };
    struct InternalTextureFormat { enum Enum { V4_8 = GL_RGB8, V316 = GL_RGB16F }; };
    struct TextureFormat { enum Enum { V4_8 = GL_RGB, V4_16 = GL_RGB }; };
    struct RasterizerFillMode { enum Enum { Fill = GL_FILL, Line = GL_LINE }; };
    struct RasterizerCullMode { enum Enum { CullFront = GL_FRONT, CullBack = GL_BACK, CullNone = 0 }; };
    struct DepthFunc { enum Enum { Less = GL_LESS }; }; // TODO
    struct DepthWriteMask { enum Enum { All = GL_TRUE, Zero = GL_FALSE }; };
    struct BufferMemoryUsage { enum Enum { GPU = GL_STATIC_DRAW, CPU = GL_DYNAMIC_DRAW }; };
    struct BufferAccessType { enum Enum { GPU = GL_STATIC_DRAW, CPU = GL_DYNAMIC_DRAW }; }; // repeated, compatibility-only
    struct BufferItemType { enum Enum { U16 = GL_UNSIGNED_SHORT, U32 = GL_UNSIGNED_INT }; };
    struct BufferTopologyType { enum Enum { Triangles = GL_TRIANGLES, Lines = GL_LINES }; };

    struct RscTexture { GLuint id; };

    struct RscMainRenderTarget { u32 mask; };
    struct RscRenderTarget {
        RscTexture textures[RenderTarget_MaxCount];
        GLuint buffer;
        GLuint depthBuffer;
        u32 width, height;
        u32 mask;
        u32 count;
    };

    struct RscVertexShader { GLuint id; };
    struct RscPixelShader { GLuint id; };
    struct RscShaderSet { GLuint id; };

    struct BufferAttributeFormat { enum Enum { R32G32B32_FLOAT, R32G32_FLOAT, R8G8B8A8_SINT, R8G8B8A8_UNORM }; };
    struct VertexAttribDesc {
        const char* name;
        size_t offset;
        size_t stride;
        s32 size;
        GLenum type;
        GLenum normalized;
    };
    VertexAttribDesc make_vertexAttribDesc(const char* name, size_t offset, size_t stride, BufferAttributeFormat::Enum format) {
        const s32 sizes[] = {3, 2, 4, 4};
        const GLenum types[] = {GL_FLOAT, GL_FLOAT, GL_BYTE, GL_UNSIGNED_BYTE};
        const GLenum normalized[] = {GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE};
        return VertexAttribDesc{
            name, offset, stride, sizes[format], types[format], normalized[format]
        };
    }
    struct RscInputLayout {};
    
    struct RscBlendState { bool enable; };
    struct RscRasterizerState {
        GLenum fillMode;
        GLenum cullFace;
    };
    struct RscDepthStencilState {
        bool enable;
        GLenum func;
        GLenum writemask;
    };
    
    struct RscVertexBuffer {
        GLsizei vertexCount;
        GLuint arrayObject;
        GLuint vertexBuffer;
        GLenum type;
    };
    
    struct RscIndexedVertexBuffer {
        GLsizei indexCount;
        GLuint arrayObject;
        GLuint vertexBuffer;
        GLuint indexBuffer;
        GLenum type;
        GLenum indexType;
    };
    
    struct RscCBuffer {
        GLuint id;
        u32 byteWidth;
    };

    typedef const char* Marker_t;
}
}
#endif // __WASTELADNS_RENDERER_TYPES_GL33_H__
