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
        struct VSSkinType { enum Enum { Unskinned, Skinned }; };
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
    typedef float3 Layout_float3;
    struct Layout_Texturedfloat2 {
        float2 pos;
        float2 uv;
    };
    struct Layout_Texturedfloat3 {
        float3 pos;
        float2 uv;
    };
    struct Layout_TexturedSkinnedfloat3 {
        float3 pos;
        float2 uv;
        u8 joint_indices[4];
        u8 joint_weights[4];
    };
    struct Layout_float3TexturedMapped {
        float3 pos;
        float2 uv;
        float3 normal;
        float3 tangent;
        float3 bitangent;
    };
    struct Layout_float2Color4B {
        float2 pos;
        u32 color;
    };
    struct Layout_float3Color4B {
        float3 pos;
        u32 color;
    };
    struct Layout_float3Color4BSkinned {
        float3 pos;
        u32 color;
        u8 joint_indices[4];
        u8 joint_weights[4];
    };
    struct Layout_CNone {
        struct Buffers { enum Enum { Count }; };
    };
    constexpr u32 max_instances = 256; // makes InstanceData 16kb, anything higher could be risky
    struct Layout_CBuffer_3DScene {
        struct SceneData {
            float4x4 projectionMatrix;
            float4x4 viewMatrix;
            float3 viewPos;
            f32 padding1;
            float3 lightPos;
            f32 padding2;
        };
        struct GroupData {
            float4x4 worldMatrix;
            float4 groupColor;
        };
        typedef float4x4 Instance;
        struct InstanceData {
            Instance instanceMatrices[max_instances];
        };
        struct Buffers { enum Enum { SceneData, GroupData, InstanceData, Count }; };
    };
    struct Layout_CBuffer_LightPass {
        struct SceneData {
            float4 viewPosWS;
            float4 lightPosWS;
        };
        struct Buffers { enum Enum { SceneData, Count }; };
    };
    struct Layout_CBuffer_DebugScene {
        typedef float4x4 GroupData;
        struct Buffers { enum Enum { GroupData, Count }; };
    };

    namespace Shaders {
        template<typename _vsCBufferLayout, typename _psCBufferLayout>
        struct PSCBufferOpts;
        template<> struct PSCBufferOpts<Layout_CNone, Layout_CNone> { enum { cbufferUsage = PSCBufferUsage::None }; };
        template<typename _vsCBufferLayout> struct PSCBufferOpts<_vsCBufferLayout, _vsCBufferLayout> { enum { cbufferUsage = PSCBufferUsage::Uses }; };
        template<typename _vsCBufferLayout> struct PSCBufferOpts<_vsCBufferLayout, Layout_CNone> { enum { cbufferUsage = PSCBufferUsage::None }; };

        template <typename _vertexLayout> struct SkinnedType { enum Enum { type = VSSkinType::Unskinned }; };
        template<> struct SkinnedType<Layout_TexturedSkinnedfloat3> { enum Enum { type = VSSkinType::Skinned }; };
        template<> struct SkinnedType<Layout_float3Color4BSkinned> { enum Enum { type = VSSkinType::Skinned }; };

        template <typename _vertexLayout> struct MaterialUsage { enum Enum { type = PSMaterialUsage::None }; };
        template<> struct MaterialUsage<Layout_Texturedfloat2> { enum Enum { type = PSMaterialUsage::Uses }; };
        template<> struct MaterialUsage<Layout_Texturedfloat3> { enum Enum { type = PSMaterialUsage::Uses }; };
        template<> struct MaterialUsage<Layout_TexturedSkinnedfloat3> { enum Enum { type = PSMaterialUsage::Uses }; };
        template<> struct MaterialUsage<Layout_float3TexturedMapped> { enum Enum { type = PSMaterialUsage::Uses }; };
    }

    // Convenience shapes
    struct RenderTargetTexturedQuad {
        Renderer::Layout_Texturedfloat2 vertices[4];
        u16 indices[6];
    };
    struct TexturedCube {
        Renderer::Layout_float3TexturedMapped vertices[24];
        u16 indices[36];
    };
    struct UntexturedCube {
        Renderer::Layout_float3 vertices[24];
        u16 indices[36];
    };
}

#endif // __WASTELADNS_RENDERER_TYPES_H__
