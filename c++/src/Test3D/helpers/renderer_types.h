#ifndef __WASTELADNS_RENDERER_TYPES_H__
#define __WASTELADNS_RENDERER_TYPES_H__

namespace Renderer {

    struct ProjectionType { enum Enum { Z0to1, Zminus1to1 }; };

    namespace Shaders {
        struct VS_src {
            const char* name;
            const char* src;
        };
        struct VSDrawType { enum Enum { Standard, Instanced }; };
        struct VSTechnique {
            enum Enum {
                  forward_base
                , fullscreen_blit
            };
        };
        template<VSTechnique::Enum _technique, typename _vertexLayout, typename _cbufferLayout, VSDrawType::Enum _type>
        struct VS_src_selector;

        struct PS_src {
            const char* name;
            const char** samplerNames;
            u32 numSamplers;
            const char* src;
        };
        struct PSMaterialUsage { enum Enum : bool { None, Uses }; };
        struct PSCBufferUsage { enum Enum : bool { None, Uses }; };
        struct PSTechnique {
            enum Enum {
                  forward_untextured_unlit
                , forward_textured_unlit
                , forward_textured_unlitalphaclip
                , forward_textured_lit_normalmapped
                , fullscreen_blit_textured
            };
        };
        template <PSTechnique::Enum _technique, typename _vertexLayoutIn, typename _cbufferLayout>
        struct PS_src_selector;
    }
    
    namespace Driver {
        struct RscMainRenderTarget;
        template<u32 _attachments>
        struct RscRenderTarget;
        struct RscTexture;
        struct RscBlendState;
        struct RscRasterizerState;
        struct RscDepthStencilState;
        template <typename _vertexLayout, typename _cbufferLayout, Shaders::VSDrawType::Enum _drawType>
        struct RscVertexShader;
        template <Shaders::PSCBufferUsage::Enum _cbufferUsage>
        struct RscPixelShader;
        template <typename _vertexLayout, typename _cbufferLayout, Shaders::PSCBufferUsage::Enum _cbufferUsage, Shaders::VSDrawType::Enum _drawType>
        struct RscShaderSet;
        template <typename _layout>
        struct RscBuffer;
        template <typename _layout>
        struct RscIndexedBuffer;
        struct RscCBuffer;
        //typedef (something) Marker_t;
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
    };
    struct Layout_Vec3TexturedMapped {
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
    struct Layout_CNone {
        struct Buffers { enum Enum { Count }; };
    };
    constexpr u32 max_instances = 256; // makes InstanceData 16kb, anything higher could be risky
    struct Layout_CBuffer_3DScene {
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
        typedef Mat4 Instance;
        struct InstanceData {
            Instance instanceMatrices[max_instances];
        };
        struct Buffers { enum Enum { SceneData, GroupData, InstanceData, Count }; };
    };
    struct Layout_CBuffer_LightPass {
        struct SceneData {
            Vec4 viewPosWS;
            Vec4 lightPosWS;
        };
        struct Buffers { enum Enum { SceneData, Count }; };
    };
    struct Layout_CBuffer_DebugScene {
        typedef Mat4 GroupData;
        struct Buffers { enum Enum { GroupData, Count }; };
    };

    namespace Shaders {
        template<typename _vsCBufferLayout, typename _psCBufferLayout>
        struct PSCBufferOpts;
        template<>
        struct PSCBufferOpts<Layout_CNone, Layout_CNone> {
            static const PSCBufferUsage::Enum cbufferUsage = PSCBufferUsage::None;
        };
        template<typename _vsCBufferLayout>
        struct PSCBufferOpts<_vsCBufferLayout, _vsCBufferLayout> {
            static const PSCBufferUsage::Enum cbufferUsage = PSCBufferUsage::Uses;
        };
        template<typename _vsCBufferLayout>
        struct PSCBufferOpts<_vsCBufferLayout, Layout_CNone> {
            static const PSCBufferUsage::Enum cbufferUsage = PSCBufferUsage::None;
        };
    }

    // Convenience shapes
    struct RenderTargetTexturedQuad {
        Renderer::Layout_TexturedVec2 vertices[4];
        u16 indices[6];
    };
    struct TexturedCube {
        Renderer::Layout_Vec3TexturedMapped vertices[24];
        u16 indices[36];
    };
    struct UntexturedCube {
        Renderer::Layout_Vec3 vertices[24];
        u16 indices[36];
    };
}

#endif // __WASTELADNS_RENDERER_TYPES_H__
