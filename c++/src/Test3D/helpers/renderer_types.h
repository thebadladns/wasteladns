#ifndef __WASTELADNS_RENDERER_TYPES_H__
#define __WASTELADNS_RENDERER_TYPES_H__

namespace Renderer {

    struct ProjectionType { enum Enum { Z0to1, Zminus1to1 }; };

    namespace Driver {
        struct RscMainRenderTarget;
        struct RscRenderTarget;
        struct RscTexture;
        struct RscBlendState;
        struct RscRasterizerState;
        template <typename _vertexLayout, typename _cbufferLayout>
        struct RscVertexShader;
        struct RscPixelShader;
        template <typename _vertexLayout, typename _cbufferLayout>
        struct RscShaderSet;
        template <typename _vertexLayout, typename _cbufferLayout>
        struct RscShaderSet;
        template <typename _layout>
        struct RscBuffer;
        template <typename _layout>
        struct RscIndexedBuffer;
        struct RscCBuffer;
    };

    // Supported layouts
    typedef Vec3 Layout_Vec3;
    struct Layout_TexturedVec2 {
        Vec2 pos;
        Vec2 uv;
    };
    struct Layout_TexturedVec3 {
        Vec3 pos;
        Vec2 uv;
        Vec3 normal;
        Vec3 tangent;
        Vec3 bitangent;
    };
    struct Layout_Vec2Color4B {
        Vec2 pos;
        u32 color;
    };
    struct Layout_Vec3Color4B {
        Vec3 pos;
        u32 color;
    };
    namespace Layout_CNone {
        struct Buffers { enum Enum { Count }; };
    };
    namespace Layout_CBuffer_3DScene {

        constexpr u32 max_instances = 256; // makes InstanceData 16kb, anything higher could be risky

        struct SceneData {
            Mat4 projectionMatrix;
            Mat4 viewMatrix;
            Vec3 viewPos;
            f32 padding1;
            Vec3 lightPos;
            f32 padding2;
        };
        struct GroupData {
            Mat4 worldMatrix;
            Vec4 groupColor;
        };
        struct InstanceData {
            Mat4 instanceMatrices[max_instances]; // xyz=pos z=padding
        };
        struct Buffers { enum Enum { SceneData, GroupData, InstanceData, Count }; };
    };
    namespace Layout_CBuffer_LightPass {
        struct SceneData {
            Vec4 viewPosWS;
            Vec4 lightPosWS;
        };
        struct Buffers { enum Enum { SceneData, Count }; };
    };
    namespace Layout_CBuffer_DebugScene {
        typedef Mat4 GroupData;
        struct Buffers { enum Enum { GroupData, Count }; };
    };

    // Convenience shapes
    struct RenderTargetTexturedQuad {
        Renderer::Layout_TexturedVec2 vertices[4];
        u16 indices[6];
    };
    struct TexturedCube {
        Renderer::Layout_TexturedVec3 vertices[24];
        u16 indices[36];
    };
    struct UntexturedCube {
        Renderer::Layout_Vec3 vertices[24];
        u16 indices[36];
    };

namespace Shaders {

    struct VSSrc {
        const char* name;
        const char* src;
    };
    struct VSDrawType { enum Enum { Standard, Instanced }; };

    struct VSTechnique { enum Enum {
        forward_base
    }; };
    template<VSTechnique::Enum _technique, typename _vertexLayout, typename _cbufferLayout, VSDrawType::Enum _type>
    VSSrc vsSrc;

    struct PSSrc {
        const char* name;
        const char** samplerNames;
        u32 numSamplers;
        const char* src;
    };
    struct PSTechnique { enum Enum {
          forward_untextured_unlit
        , forward_textured_lit_normalmapped
    }; };
    template <PSTechnique::Enum _technique, typename _vertexLayoutIn, typename _cbufferLayout>
    PSSrc psSrc;
}
}

#endif // __WASTELADNS_RENDERER_TYPES_H__
