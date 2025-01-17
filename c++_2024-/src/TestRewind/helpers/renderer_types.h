#ifndef __WASTELADNS_RENDERER_TYPES_H__
#define __WASTELADNS_RENDERER_TYPES_H__

namespace renderer {

    //void generate_matrix_ortho(float4x4& matrix, const WindowProjection::Config& config);
    //void generate_matrix_persp(float4x4& matrixRHwithYup, const PerspProjection::Config& config);
    //void add_oblique_plane_to_persp(float4x4& projectionMatrix, const float4& planeCameraSpace);
    //void extract_frustum_planes_from_vp(float4* planes, const float4x4& vpMatrix);
    //const f32 min_z;

    namespace shaders {
        struct VS_src {
            const char* name;
            const char* src;
        };
        struct PS_src {
            const char* name;
            const char* src;
        };
    }
    
    namespace driver {
        struct RscMainRenderTarget;
        enum { RenderTarget_MaxCount = 4 };
        struct RscRenderTarget;
        struct RscTexture;
        struct RscBlendState;
        struct RscRasterizerState;
        struct RscDepthStencilState;
        struct ShaderCache;
        struct RscVertexShader;
        struct RscPixelShader;
        struct RscShaderSet;
        //typedef (something) VertexAttribDesc;
        //VertexAttribDesc make_vertexAttribDesc(const char* name, size_t offset, size_t stride, BufferAttributeFromat::Enum format);
        struct RscInputLayout;
        struct RscVertexBuffer;
        struct RscIndexedVertexBuffer;
        struct CBufferStageMask { enum Enum { VS = 1, PS = 2 }; };
        struct RscCBuffer;
        //typedef (something) Marker_t;
    };

    // Convenience shapes
    struct UntexturedCube {
        float3 vertices[24];
        u16 indices[36];
    };
    struct UntexturedSphere {
        float3 vertices[42];
        u16 indices[240];
    };
    struct ColoredVertex {
        float3 pos;
        u32 color;
    };
    struct ColoredCube {
        ColoredVertex vertices[24];
        u16 indices[36];
    };
    struct ColoredSphere {
        ColoredVertex vertices[42];
        u16 indices[240];
    };
}

#endif // __WASTELADNS_RENDERER_TYPES_H__
