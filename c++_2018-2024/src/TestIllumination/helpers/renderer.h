#ifndef __WASTELADNS_RENDERER_H__
#define __WASTELADNS_RENDERER_H__

#ifndef UNITYBUILD
#include <cstring>
#include "renderer_debug.h"
#endif

namespace Renderer {
    struct OrthoProjection {
        struct Config {
            f32 left;
            f32 right;
            f32 top;
            f32 bottom;
            f32 near;
            f32 far;
        };
        Config config;
        Mat4 matrix;
    };
    struct PerspProjection {
        struct Config {
            f32 fov;
            f32 aspect;
            f32 near;
            f32 far;
        };
        Config config;
        Mat4 matrix;
    };
    
    template <ProjectionType::Enum _type = defaultProjectionType>
    void generateMatrix(Mat4& matrix, const OrthoProjection::Config& config);

    template<>
    void generateMatrix<ProjectionType::Zminus1to1>(Mat4& matrix, const OrthoProjection::Config& config) {
        f32* matrixCM = matrix.dataCM;
        memset(matrixCM, 0, sizeof(f32) * 16);
        matrixCM[0] = 2.f / (config.right - config.left);
        matrixCM[5] = 2.f / (config.top - config.bottom);
        matrixCM[10] = -2.f / (config.far - config.near);
        matrixCM[12] = -(config.right + config.left) / (config.right - config.left);
        matrixCM[13] = -(config.top + config.bottom) / (config.top - config.bottom);
        matrixCM[14] = -(config.far + config.near) / (config.far - config.near);
        matrixCM[15] = 1.f;
    }
    template<>
    void generateMatrix<ProjectionType::Z0to1>(Mat4& matrix, const OrthoProjection::Config& config) {
        f32* matrixCM = matrix.dataCM;
        memset(matrixCM, 0, sizeof(f32) * 16);
        matrixCM[0] = 2.f / (config.right - config.left);
        matrixCM[5] = 2.f / (config.top - config.bottom);
        matrixCM[10] = -1.f / (config.far - config.near);
        matrixCM[12] = -(config.right + config.left) / (config.right - config.left);
        matrixCM[13] = -(config.top + config.bottom) / (config.top - config.bottom);
        matrixCM[14] = -config.near / (config.far - config.near);
        matrixCM[15] = 1.f;
    }
    
    // Expects right-handed view matrix, z-coordinate point towards the viewer
    template <ProjectionType::Enum _type = defaultProjectionType>
    void generateMatrix(Mat4& matrixRHwithYup, const PerspProjection::Config& config);
    template <>
    void generateMatrix<ProjectionType::Zminus1to1>(Mat4& matrixRHwithYup, const PerspProjection::Config& config) {
        const f32 h = 1.f / Math::tan(config.fov * 0.5f * Math::d2r<f32>);
        const f32 w = h / config.aspect;
        
        // maps each xyz axis to [-1,1] (left handed, y points up z moves away from the viewer)
        f32 (&matrixCM)[16] = matrixRHwithYup.dataCM;
        memset(matrixCM, 0, sizeof(f32) * 16);
        matrixCM[0] = w;
        matrixCM[5] = h;
        matrixCM[10] = -(config.far + config.near) / (config.far - config.near);
        matrixCM[11] = -1.f;
        matrixCM[14] = -(2.f * config.far * config.near) / (config.far - config.near);
    }
    template <>
    void generateMatrix<ProjectionType::Z0to1>(Mat4& matrixRHwithYup, const PerspProjection::Config& config) {
        const f32 h = 1.f / Math::tan(config.fov * 0.5f * Math::d2r<f32>);
        const f32 w = h / config.aspect;
        // maps each xyz axis to [0,1] (left handed, y points up z moves away from the viewer)
        f32(&matrixCM)[16] = matrixRHwithYup.dataCM;
        memset(matrixCM, 0, sizeof(f32) * 16);
        matrixCM[0] = w;
        matrixCM[5] = h;
        matrixCM[10] = config.far / (config.near - config.far);
        matrixCM[11] = -1.f;
        matrixCM[14] = config.far * config.near / (config.near - config.far);
    }

    template <CoordinateSystem::Enum _system>
    void generateModelViewMatrix(Mat4& modelview, const TransformMatrix<_system>& t) {
        
        const TransformMatrix<CoordinateSystem::RH_Yup_Zfront> tRHwithYUp = Math::toEyeSpace(t);
        const Mat4& mRHwithYUp = tRHwithYUp.matrix;
        
        // Simplified inverse
        // https://www.gamedev.net/forums/topic/647149-d3dxmatrixlookatlh-internally/?tab=comments#comment-5089654

        const f32 tx = -Math::dot(mRHwithYUp.col3, mRHwithYUp.col0);
        const f32 ty = -Math::dot(mRHwithYUp.col3, mRHwithYUp.col1);
        const f32 tz = -Math::dot(mRHwithYUp.col3, mRHwithYUp.col2);

        modelview.dataCM[0] = mRHwithYUp.col0.x;
        modelview.dataCM[4] = mRHwithYUp.col0.y;
        modelview.dataCM[8] = mRHwithYUp.col0.z;

        modelview.dataCM[1] = mRHwithYUp.col1.x;
        modelview.dataCM[5] = mRHwithYUp.col1.y;
        modelview.dataCM[9] = mRHwithYUp.col1.z;

        modelview.dataCM[2] = mRHwithYUp.col2.x;
        modelview.dataCM[6] = mRHwithYUp.col2.y;
        modelview.dataCM[10] = mRHwithYUp.col2.z;

        modelview.dataCM[12] = tx;
        modelview.dataCM[13] = ty;
        modelview.dataCM[14] = tz;

        modelview.dataCM[3] = mRHwithYUp.col0.w;
        modelview.dataCM[7] = mRHwithYUp.col1.w;;
        modelview.dataCM[11] = mRHwithYUp.col2.w;
        modelview.dataCM[15] = mRHwithYUp.col3.w;
    }

    void create(RenderTargetTexturedQuad& q);

    void tangents(Vec3& t, Vec3& b, const Vec3& vbl, const Vec3& vtr, const Vec3& vtl) {
        const Vec2
            bl = { 0.f, 0.f }
            , tr = { 1.f, 1.f }
            , tl = { 0.f, 1.f }
        ;
        Vec3 worldH = Math::subtract(vbl, vtl);
        Vec3 worldV = Math::subtract(vtr, vtl);
        Vec2 uvV = Math::subtract(bl, tl);
        Vec2 uvH = Math::subtract(tr, tl);
        f32 invdet = 1.0f / (uvV.x * uvH.y - uvH.x * uvV.y);
        t.x = invdet * (uvH.y * worldH.x - uvV.y * worldV.x);
        t.y = invdet * (uvH.y * worldH.y - uvV.y * worldV.y);
        t.z = invdet * (uvH.y * worldH.z - uvV.y * worldV.z);
        t = Math::normalize(t);
        b.x = invdet * (-uvH.x * worldH.x + uvV.x * worldV.x);
        b.y = invdet * (-uvH.x * worldH.y + uvV.x * worldV.y);
        b.z = invdet * (-uvH.x * worldH.z + uvV.x * worldV.z);
        b = Math::normalize(b);
    }
    struct MappedCubeCreateParams {
        f32 width;
        f32 height;
        f32 depth;
    };
    void create(MappedCube& c, const MappedCubeCreateParams& params) {

        enum Normals { Y, NY, X, NX, Z, NZ };
        const Vec3 normals[] = {
              { 0.f, 1.f, 0.f }
            , { 0.f, -1.f, 0.f }
            , { 1.f, 0.f, 0.f }
            , { -1.f, 0.f, 0.f }
            , { 0.f, 0.f, 1.f }
            , { 0.f, 0.f, -1.f }
        };
        const Vec2
              bl = { 0.f, 0.f }
            , br = { 1.f, 0.f }
            , tr = { 1.f, 1.f }
            , tl = { 0.f, 1.f }
        ;
        f32 w = params.width;
        f32 h = params.height;
        f32 d = params.depth;
        Renderer::Layout_TexturedVec3* v = c.vertices;

        // vertex and texture coords
        v[0] = { { w, d, 0.f } }; v[1] = { { -w, d, 0.f } }; v[2] = { { -w, d, h } }; v[3] = { { w, d, h } };           // +y quad
        v[4] = { { -w, -d, 0.f } }; v[5] = { { w, -d, 0.f } }; v[6] = { { w, -d, h } }; v[7] = { { -w, -d, h } };       // -y quad

        v[8] = { { w, -d, 0.f} }; v[9] = { { w, d, 0.f } }; v[10] = { { w, d, h } }; v[11] = { { w, -d, h } };          // +x quad
        v[12] = { { -w, d, 0.f } }; v[13] = { { -w, -d, 0.f } }; v[14] = { { -w, -d, h } }; v[15] = { { -w, d, h } };   // -x quad

        v[16] = { { -w, -d, h } }; v[17] = { { w, -d, h } }; v[18] = { { w, d, h } }; v[19] = { { -w, d, h } };         // +z quad
        v[20] = { { w, d, 0.f } }; v[21] = { { w, -d, 0.f } }; v[22] = { { -w, -d, 0.f } }; v[23] = { { -w, d, 0.f } }; // -z quad

        // normals, tangents and bitangents
        Vec3 n, t, b;
        n = normals[Y]; t = normals[NX]; b = normals[NZ];
        // tangents(t, b, v[3].pos, v[1].pos, v[0].pos );
        v[0].normal = n; v[1].normal = n; v[2].normal = n; v[3].normal = n;     // +y quad
        v[0].tangent = t; v[1].tangent = t; v[2].tangent = t; v[3].tangent = t;
        v[0].bitangent = b; v[1].bitangent = b; v[2].bitangent = b; v[3].bitangent = b;
        v[0].uv = tl; v[1].uv = tr; v[2].uv = br; v[3].uv = bl;
        n = normals[NY]; t = normals[X]; b = normals[NZ];
        // tangents(t, b, v[7].pos, v[5].pos, v[4].pos );
        v[4].normal = n; v[5].normal = n; v[6].normal = n; v[7].normal = n;     // -y quad
        v[4].tangent = t; v[5].tangent = t; v[6].tangent = t; v[7].tangent = t;
        v[4].bitangent = b; v[5].bitangent = b; v[6].bitangent = b; v[7].bitangent = b;
        v[4].uv = tl; v[5].uv = tr; v[6].uv = br; v[7].uv = bl;
        n = normals[X]; t = normals[Y]; b = normals[NZ];
        // tangents(t, b, v[11].pos, v[9].pos, v[8].pos );
        v[8].normal = n; v[9].normal = n; v[10].normal = n; v[11].normal = n;   // +x quad
        v[8].tangent = t; v[9].tangent = t; v[10].tangent = t; v[11].tangent = t;
        v[8].bitangent = b; v[9].bitangent = b; v[10].bitangent = b; v[11].bitangent = b;
        v[8].uv = tl; v[9].uv = tr; v[10].uv = br; v[11].uv = bl;
        n = normals[NX]; t = normals[NY]; b = normals[NZ];
        // tangents(t, b, v[15].pos, v[13].pos, v[12].pos );
        v[12].normal = n; v[13].normal = n; v[14].normal = n; v[15].normal = n; // -x quad
        v[12].tangent = t; v[13].tangent = t; v[14].tangent = t; v[15].tangent = t;
        v[12].bitangent = b; v[13].bitangent = b; v[14].bitangent = b; v[15].bitangent = b;
        v[12].uv = tl; v[13].uv = tr; v[14].uv = br; v[15].uv = bl;
        n = normals[Z]; t = normals[X]; b = normals[NY];
        // tangents(t, b, v[19].pos, v[17].pos, v[16].pos );
        v[16].normal = n; v[17].normal = n; v[18].normal = n; v[19].normal = n; // +z quad
        v[16].tangent = t; v[17].tangent = t; v[18].tangent = t; v[19].tangent = t;
        v[16].bitangent = b; v[17].bitangent = b; v[18].bitangent = b; v[19].bitangent = b;
        v[16].uv = tl; v[17].uv = tr; v[18].uv = br; v[19].uv = bl;
        n = normals[NZ]; t = normals[NX]; b = normals[NY];
        // tangents(t, b, v[20].pos, v[22].pos, v[21].pos );
        v[20].normal = n; v[21].normal = n; v[22].normal = n; v[23].normal = n; // -z quad
        v[20].tangent = t; v[21].tangent = t; v[22].tangent = t; v[23].tangent = t;
        v[20].bitangent = b; v[21].bitangent = b; v[22].bitangent = b; v[23].bitangent = b;
        v[20].uv = bl; v[21].uv = tl; v[22].uv = tr; v[23].uv = br;

        u16* i = c.indices;

        i[0] = 0; i[1] = 1; i[2] = 2; i[3] = 3; i[4] = 0; i[5] = 2;             // +y tris
        i[6] = 4; i[7] = 5; i[8] = 6; i[9] = 7; i[10] = 4; i[11] = 6;           // -y tris

        i[12] = 8; i[13] = 9; i[14] = 10; i[15] = 11; i[16] = 8; i[17] = 10;    // +x tris
        i[18] = 12; i[19] = 13; i[20] = 14; i[21] = 15; i[22] = 12; i[23] = 14; // -x tris

        i[24] = 16; i[25] = 17; i[26] = 18; i[27] = 19; i[28] = 16; i[29] = 18; // +z tris
        i[30] = 20; i[31] = 21; i[32] = 22; i[33] = 23; i[34] = 20; i[35] = 22; // -z tris
    }
};

namespace Renderer {
    
    struct TextureFormat { enum Enum { V316 }; };
    struct Type { enum Enum { Float }; };
    struct RasterizerFillMode { enum Enum { Fill, Line }; };
    struct BufferMemoryMode { enum Enum { GPU, CPU }; };
    struct BufferItemType { enum Enum { U16, U32 }; };
    struct BufferTopologyType { enum Enum { Triangles, Lines }; };

namespace Driver {
    
    struct MainRenderTargetParams {
        u32 width;
        u32 height;
        bool depth;
    };
    void create(RscMainRenderTarget&, const MainRenderTargetParams&);
    void bind(RscMainRenderTarget& rt);
    
    struct RenderTargetParams {
        u32 width;
        u32 height;
        bool depth;
    };
    void create(RscRenderTarget& rt, const RenderTargetParams& params);
    void bind(const RscRenderTarget& rt);
    void clear(const RscRenderTarget& rt);
    struct RenderTargetCopyParams {
        // for now
        bool depth;
    };
    void copy(RscMainRenderTarget& dst, const RscRenderTarget& src, const RenderTargetCopyParams& params);

    struct TextureFromFileParams {
        const char* path;
    };
    void create(RscTexture& t, const TextureFromFileParams& params);
    struct TextureRenderTargetCreateParams {
        s32 width;
        s32 height;
        TextureFormat::Enum format;
        Type::Enum type;
    };
    void create(RscTexture& t, const TextureRenderTargetCreateParams& params);
    void bind(const RscTexture* textures, const u32 count);
    template <typename _vertexLayout, typename _cbufferLayout>
    void bind(RscShaderSet<_vertexLayout, _cbufferLayout>& ss, const char** params, const u32 count);
    void bind(RscRenderTarget& b, const RscTexture* textures, const u32 count);

    struct ShaderResult {
        char error[128];
        bool compiled;
    };
    struct VertexShaderRuntimeCompileParams {
        const char* shaderStr;
        u32 length;
    };
    template <typename _vertexLayout, typename _cbufferLayout>
    ShaderResult create(RscVertexShader<_vertexLayout, _cbufferLayout>&, const VertexShaderRuntimeCompileParams&);
    struct PixelShaderRuntimeCompileParams {
        const char* shaderStr;
        u32 length;
    };
    ShaderResult create(RscPixelShader&, const PixelShaderRuntimeCompileParams&);
    template <typename _vertexLayout, typename _cbufferLayout>
    struct ShaderSetRuntimeCompileParams {
        RscVertexShader<_vertexLayout, _cbufferLayout>& rscVS;
        RscPixelShader& rscPS;
        const RscCBuffer* cbuffers;
    };
    template <typename _vertexLayout, typename _cbufferLayout>
    ShaderResult create(RscShaderSet<_vertexLayout, _cbufferLayout>&, const ShaderSetRuntimeCompileParams<_vertexLayout, _cbufferLayout>&);
    template <typename _vertexLayout, typename _cbufferLayout>
    void bind(const RscShaderSet<_vertexLayout, _cbufferLayout>& ss);

    struct BlendStateParams {
        bool enable;
    };
    void create(RscBlendState&, const BlendStateParams&);
    void bind(const RscBlendState bs);

    struct RasterizerStateParams {
        bool fill;
        bool cullFace;
    };
    void create(RscRasterizerState&, const RasterizerStateParams&);
    void bind(const RscRasterizerState rs);

    struct BufferParams {
        void* vertexData;
        u32 vertexSize;
        BufferTopologyType::Enum type;
        BufferMemoryMode::Enum memoryMode;
        u32 vertexCount;
    };
    template <typename _layout>
    void create(RscBuffer<_layout>&, const BufferParams&);
    struct BufferUpdateParams {
        void* vertexData;
        u32 vertexSize;
        u32 vertexCount;
    };
    template <typename _layout>
    void update(RscBuffer<_layout>&, const BufferUpdateParams&);
    template <typename _layout>
    void bind(const RscBuffer<_layout>& b);
    
    struct IndexedBufferParams {
        void* vertexData;
        void* indexData;
        u32 vertexSize;
        u32 indexSize;
        u32 indexCount;
        BufferItemType::Enum indexType;
        BufferTopologyType::Enum type;
        BufferMemoryMode::Enum memoryMode;
    };
    template <typename _layout>
    void create(RscIndexedBuffer<_layout>&, const IndexedBufferParams&);
    struct IndexedBufferUpdateParams {
        void* vertexData;
        void* indexData;
        u32 vertexSize;
        u32 indexSize;
        u32 indexCount;
    };
    template <typename _layout>
    void update(RscIndexedBuffer<_layout>&, const IndexedBufferUpdateParams&);
    template <typename _layout>
    void update(RscIndexedBuffer<_layout>& b, const IndexedBufferUpdateParams& params);
    template <typename _layout>
    void draw(const RscIndexedBuffer<_layout>& b);
    template <typename _layout>
    void drawInstances(const RscIndexedBuffer<_layout>& b, const u32 instanceCount);

    struct CBufferCreateParams {
    };
    template <typename _layout>
    void create(RscCBuffer& cb, const CBufferCreateParams& params);
    template <typename _layout>
    void update(RscCBuffer& cb, const _layout& data);
    struct CBufferBindParams {
        bool vertex;
        bool pixel;
    };
    void bind(const RscCBuffer* cb, const u32 count, const CBufferBindParams& params);

    template <typename _vertexLayout, typename _bufferLayout>
    void createLayout(RscVertexShader<_vertexLayout, _bufferLayout>& vs, void* shaderBufferPointer, u32 shaderBufferSize);
    template <typename _layout>
    void bindLayout();
    template <typename _vertexLayout, typename _bufferLayout>
    void bindCBuffers(RscShaderSet<_vertexLayout, _bufferLayout>&, const RscCBuffer* cbuffers);
}
};

#endif // __WASTELADNS_RENDERER_H__
