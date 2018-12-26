#ifndef __WASTELADNS_RENDERER_TYPES_H__
#define __WASTELADNS_RENDERER_TYPES_H__

namespace Renderer {
    namespace Driver {
        struct RscMainRenderTarget;
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
    struct Layout_TexturedVec3 {
        Vec3 pos;
        Vec2 uv;
        Vec3 normal;
    };
    struct Layout_Vec2Color4B {
        Vec2 pos;
        u32 color;
    };
    struct Layout_Vec3Color4B {
        Vec3 pos;
        u32 color;
    };
    namespace Layout_CBuffer_3DScene {
        struct SceneData {
            Mat4 projectionMatrix;
            Mat4 viewMatrix;
        };
        struct GroupData {
            Mat4 worldMatrix[64];
            Vec4 color;
        };
        struct Buffers { enum Enum { SceneData, GroupData, Count }; };
    };
    namespace Layout_CBuffer_DebugScene {
        typedef Mat4 GroupData;
        struct Buffers { enum Enum { GroupData, Count }; } ;
    };
}

#endif // __WASTELADNS_RENDERER_TYPES_H__
