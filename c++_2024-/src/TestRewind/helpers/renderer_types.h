#ifndef __WASTELADNS_RENDERER_TYPES_H__
#define __WASTELADNS_RENDERER_TYPES_H__

namespace Renderer {

    struct WindowProjection {
        struct Config {
            f32 left;
            f32 right;
            f32 top;
            f32 bottom;
            f32 near;
            f32 far;
        };
        Config config;
        float4x4 matrix;
    };
    struct PerspProjection {
        struct Config {
            f32 fov;
            f32 aspect;
            f32 near;
            f32 far;
        };
        Config config;
        float4x4 matrix;
    };
    //void generate_matrix_ortho(float4x4& matrix, const WindowProjection::Config& config);
    //void generate_matrix_persp(float4x4& matrixRHwithYup, const PerspProjection::Config& config);
    void generate_matrix_ortho_zneg1to1(float4x4& matrix, const WindowProjection::Config& config);
    void generate_matrix_ortho_z0to1(float4x4& matrix, const WindowProjection::Config& config);
    void generate_matrix_persp_zneg1to1(float4x4& matrixRHwithYup, const PerspProjection::Config& config);
    void generate_matrix_persp_z0to1(float4x4& matrixRHwithYup, const PerspProjection::Config& config);

    namespace Shaders {
        struct VS_src {
            const char* name;
            const char* src;
        };
        struct PS_src {
            const char* name;
            const char* src;
        };
    }
    
    namespace Driver {
        struct RscMainRenderTarget;
        enum { RenderTarget_MaxCount = 4 };
        struct RscRenderTarget;
        struct RscTexture;
        struct RscBlendState;
        struct RscRasterizerState;
        struct RscDepthStencilState;
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
    struct RenderTargetTexturedQuad {
        struct Vertex {
			float2 pos;
			float2 uv;
        };
        Vertex vertices[4];
        u16 indices[6];
    };
    struct TexturedCube {
        struct Vertex {
            float3 pos;
            float2 uv;
            float3 normal;
            float3 tangent;
            float3 bitangent;
        };
        Vertex vertices[24];
        u16 indices[36];
    };
    struct UntexturedCube {
        float3 vertices[24];
        u16 indices[36];
    };
}

#endif // __WASTELADNS_RENDERER_TYPES_H__
