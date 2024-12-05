#ifndef __WASTELADNS_RENDERER_TYPES_GL33_H__
#define __WASTELADNS_RENDERER_TYPES_GL33_H__

namespace renderer {

    const auto generate_matrix_ortho = camera::generate_matrix_ortho_zneg1to1;
    const auto generate_matrix_persp = camera::generate_matrix_persp_zneg1to1;
    const auto add_oblique_plane_to_persp = camera::add_oblique_plane_to_persp_zneg1to1;
    const f32 min_z = -1.f;

namespace driver {

    struct Type { enum Enum { Float = GL_FLOAT }; };
    struct InternalTextureFormat { enum Enum { V4_8 = GL_RGB8, V316 = GL_RGB16F }; };
    struct TextureFormat { enum Enum { V4_8 = GL_RGB, V4_16 = GL_RGB }; };
    struct RenderTargetClearFlags { enum Enum { Stencil = GL_STENCIL_BUFFER_BIT, Depth = GL_DEPTH_BUFFER_BIT, Color = GL_COLOR_BUFFER_BIT }; };
    struct RenderTargetWriteMask { enum Enum { All = 1, None = 0 }; };
    struct RasterizerFillMode { enum Enum { Fill = GL_FILL, Line = GL_LINE }; };
    struct RasterizerCullMode { enum Enum { CullFront = GL_FRONT, CullBack = GL_BACK, CullNone = 0 }; };
    struct CompFunc { enum Enum {
        Never = GL_NEVER, Always = GL_ALWAYS,
        Less = GL_LESS, LessEqual = GL_LEQUAL,
        Equal = GL_EQUAL, NotEqual = GL_NOTEQUAL,
        Greater = GL_GREATER, GreaterEqual = GL_GEQUAL
    }; };
    struct DepthWriteMask { enum Enum { All = GL_TRUE, Zero = GL_FALSE }; };
    struct StencilOp { enum Enum {
        Keep = GL_KEEP, Zero = GL_ZERO, Replace = GL_REPLACE, Invert = GL_INVERT,
        Incr = GL_INCR, Decr = GL_DECR
    }; };
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
        u32 count;
    };
    
    struct ShaderCache {};
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
    
    struct RscBlendState {
        bool writeColor; // todo: add more parameters to this
        bool blendEnable;
    };
    struct RscRasterizerState {
        GLenum fillMode;
        GLenum cullFace;
    };
    struct RscDepthStencilState {
        bool depth_enable;
        bool stencil_enable;
        u8 stencil_readmask;
        u8 stencil_writemask;
        GLenum depth_func;
        GLenum depth_writemask;
        GLenum stencil_failOp;
        GLenum stencil_depthFailOp;
        GLenum stencil_passOp;
        GLenum stencil_func;
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
