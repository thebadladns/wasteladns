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
    void generate_matrix_ortho(Mat4& matrix, const OrthoProjection::Config& config);

    template<>
    void generate_matrix_ortho<ProjectionType::Zminus1to1>(Mat4& matrix, const OrthoProjection::Config& config) {
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
    void generate_matrix_ortho<ProjectionType::Z0to1>(Mat4& matrix, const OrthoProjection::Config& config) {
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
    void generate_matrix_persp(Mat4& matrixRHwithYup, const PerspProjection::Config& config);
    template <>
    void generate_matrix_persp<ProjectionType::Zminus1to1>(Mat4& matrixRHwithYup, const PerspProjection::Config& config) {
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
    void generate_matrix_persp<ProjectionType::Z0to1>(Mat4& matrixRHwithYup, const PerspProjection::Config& config) {
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
    void generate_MV_matrix(Mat4& modelview, const TransformMatrix<_system>& t) {
        
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

    void create_RT(RenderTargetTexturedQuad& q);

    void calculate_tangents(Vec3& t, Vec3& b, const Vec3& vbl, const Vec3& vtr, const Vec3& vtl) {
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
    struct CubeCreateParams {
        f32 width;
        f32 height;
        f32 depth;
    };
    void create_textured_cube_coords(TexturedCube& c, const CubeCreateParams& params) {

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
        v[0] = { { w, d, -h } }; v[1] = { { -w, d, -h } }; v[2] = { { -w, d, h } }; v[3] = { { w, d, h } };           // +y quad
        v[4] = { { -w, -d, -h } }; v[5] = { { w, -d, -h } }; v[6] = { { w, -d, h } }; v[7] = { { -w, -d, h } };       // -y quad

        v[8] = { { w, -d, -h} }; v[9] = { { w, d, -h } }; v[10] = { { w, d, h } }; v[11] = { { w, -d, h } };          // +x quad
        v[12] = { { -w, d, -h } }; v[13] = { { -w, -d, -h } }; v[14] = { { -w, -d, h } }; v[15] = { { -w, d, h } };   // -x quad

        v[16] = { { -w, -d, h } }; v[17] = { { w, -d, h } }; v[18] = { { w, d, h } }; v[19] = { { -w, d, h } };         // +z quad
        v[20] = { { w, d, -h } }; v[21] = { { w, -d, -h } }; v[22] = { { -w, -d, -h } }; v[23] = { { -w, d, -h } }; // -z quad

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
	void create_untextured_cube_coords(UntexturedCube& c, const CubeCreateParams& params) {

		f32 w = params.width;
		f32 h = params.height;
		f32 d = params.depth;
		Renderer::Layout_Vec3* v = c.vertices;

		// vertex and texture coords
		v[0] = { w, d, -h }; v[1] = { -w, d, -h }; v[2] = { -w, d, h }; v[3] = { w, d, h };           // +y quad
		v[4] = { -w, -d, -h }; v[5] = { w, -d, -h }; v[6] = { w, -d, h }; v[7] = { -w, -d, h };       // -y quad

		v[8] = { w, -d, -h }; v[9] = { w, d, -h }; v[10] = { w, d, h }; v[11] = { w, -d, h };          // +x quad
		v[12] = { -w, d, -h }; v[13] = { -w, -d, -h };  v[14] = { -w, -d, h }; v[15] = { -w, d, h };   // -x quad

		v[16] = { -w, -d, h }; v[17] = { w, -d, h }; v[18] = { w, d, h }; v[19] = { -w, d, h };         // +z quad
		v[20] = { w, d, -h }; v[21] = { w, -d, -h }; v[22] = { -w, -d, -h }; v[23] = { -w, d, -h }; // -z quad

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

namespace Driver {
    
    struct MainRenderTargetParams {
        u32 width;
        u32 height;
        bool depth;
    };
    void create_main_RT(RscMainRenderTarget&, const MainRenderTargetParams&);
    void bind_main_RT(RscMainRenderTarget& rt);
    
    struct RenderTargetParams {
        u32 width;
        u32 height;
        bool depth;
    };
    void create_RT(RscRenderTarget& rt, const RenderTargetParams& params);
    void bind_RT(const RscRenderTarget& rt);
    void clear_RT(const RscRenderTarget& rt);
    struct RenderTargetCopyParams {
        // for now
        bool depth;
    };
    void copy_RT_to_main_RT(RscMainRenderTarget& dst, const RscRenderTarget& src, const RenderTargetCopyParams& params);

    struct TextureFromFileParams {
        const char* path;
    };
    void create_texture_from_file(RscTexture& t, const TextureFromFileParams& params);
    struct TextureRenderTargetCreateParams {
        s32 width;
        s32 height;
        TextureFormat::Enum format;
        InternalTextureFormat::Enum internalFormat;
        Type::Enum type;
    };
    void create_texture_empty(RscTexture& t, const TextureRenderTargetCreateParams& params);
    void bind_textures(const RscTexture* textures, const u32 count);
    template <typename _vertexLayout, typename _cbufferLayout>
    void bind_shader_samplers(RscShaderSet<_vertexLayout, _cbufferLayout>& ss, const char** params, const u32 count);
    void bind_RTs(RscRenderTarget& b, const RscTexture* textures, const u32 count);

    struct ShaderResult {
        char error[128];
        bool compiled;
    };
    struct VertexShaderRuntimeCompileParams {
        const char* shaderStr;
        u32 length;
    };
    template <typename _vertexLayout, typename _cbufferLayout>
    ShaderResult create_shader_vs(RscVertexShader<_vertexLayout, _cbufferLayout>&, const VertexShaderRuntimeCompileParams&);
    struct PixelShaderRuntimeCompileParams {
        const char* shaderStr;
        u32 length;
    };
    ShaderResult create_shader_ps(RscPixelShader&, const PixelShaderRuntimeCompileParams&);
    template <typename _vertexLayout, typename _cbufferLayout>
    struct ShaderSetRuntimeCompileParams {
        RscVertexShader<_vertexLayout, _cbufferLayout>& rscVS;
        RscPixelShader& rscPS;
        const RscCBuffer* cbuffers;
    };
    template <typename _vertexLayout, typename _cbufferLayout>
    ShaderResult create_shader_set(RscShaderSet<_vertexLayout, _cbufferLayout>&, const ShaderSetRuntimeCompileParams<_vertexLayout, _cbufferLayout>&);
    template <typename _vertexLayout, typename _cbufferLayout>
    void bind_shader(const RscShaderSet<_vertexLayout, _cbufferLayout>& ss);

    struct BlendStateParams {
        bool enable;
    };
    void create_blend_state(RscBlendState&, const BlendStateParams&);
    void bind_blend_state(const RscBlendState bs);

    struct RasterizerStateParams {
        RasterizerFillMode::Enum fill;
         RasterizerCullMode::Enum cull;
    };
    void create_RS(RscRasterizerState&, const RasterizerStateParams&);
    void bind_RS(const RscRasterizerState rs);

    struct BufferParams {
        void* vertexData;
        u32 vertexSize;
        BufferTopologyType::Enum type;
        BufferMemoryUsage::Enum memoryUsage;
        BufferAccessType::Enum accessType;
        u32 vertexCount;
    };
    template <typename _layout>
    void create_vertex_buffer(RscBuffer<_layout>&, const BufferParams&);
    struct BufferUpdateParams {
        void* vertexData;
        u32 vertexSize;
        u32 vertexCount;
    };
    template <typename _layout>
    void update_vertex_buffer(RscBuffer<_layout>&, const BufferUpdateParams&);
    template <typename _layout>
    void bind_vertex_buffer(const RscBuffer<_layout>&);
    template <typename _layout>
    void draw_vertex_buffer(const RscBuffer<_layout>&);
    
    struct IndexedBufferParams {
        void* vertexData;
        void* indexData;
        u32 vertexSize;
        u32 indexSize;
        u32 indexCount;
        BufferItemType::Enum indexType;
        BufferTopologyType::Enum type;
        BufferMemoryUsage::Enum memoryUsage;
        BufferAccessType::Enum accessType;
    };
    template <typename _layout>
    void create_indexed_vertex_buffer(RscIndexedBuffer<_layout>&, const IndexedBufferParams&);
    struct IndexedBufferUpdateParams {
        void* vertexData;
        void* indexData;
        u32 vertexSize;
        u32 indexSize;
        u32 indexCount;
    };
    template <typename _layout>
    void update_indexed_vertex_buffer(RscIndexedBuffer<_layout>&, const IndexedBufferUpdateParams&);
    template <typename _layout>
    void draw_indexed_vertex_buffer(const RscIndexedBuffer<_layout>&);
    template <typename _layout>
    void draw_instances_indexed_vertex_buffer(const RscIndexedBuffer<_layout>&, const u32);

    struct CBufferCreateParams {
    };
    template <typename _layout>
    void create_cbuffer(RscCBuffer& cb, const CBufferCreateParams& params);
    template <typename _layout>
    void update_cbuffer(RscCBuffer& cb, const _layout& data);
    struct CBufferBindParams {
        bool vertex;
        bool pixel;
    };
    void bind_cbuffers(const RscCBuffer* cb, const u32 count, const CBufferBindParams& params);

    template <typename _vertexLayout, typename _bufferLayout>
    void create_vertex_layout(RscVertexShader<_vertexLayout, _bufferLayout>& vs, void* shaderBufferPointer, u32 shaderBufferSize);
    template <typename _layout>
    void bind_vertex_layout();
    template <typename _vertexLayout, typename _bufferLayout>
    void bind_cbuffers_to_shader(RscShaderSet<_vertexLayout, _bufferLayout>&, const RscCBuffer* cbuffers);
}
template <typename _vertexLayout, typename _cbufferLayout>
struct TechniqueSrcParams {
    const Renderer::Driver::RscCBuffer* cbuffers;
    const char* vertexSrc; const char* vertexShaderName;
    const char* pixelSrc; const char* pixelShaderName;
    const char** samplerNames; u32 numSamplers;
};
template <Shaders::VSTechnique::Enum _vsTechnique, Shaders::PSTechnique::Enum _psTechnique
        , Shaders::VSDrawType::Enum _drawType
        , typename _vertexLayout, typename _cbufferLayout>
void create_technique(TechniqueSrcParams< _vertexLayout, _cbufferLayout>& params
          , const Renderer::Driver::RscCBuffer* cbuffers) {
    params.cbuffers = cbuffers;
    const Shaders::VSSrc& vsSrc = Shaders::vsSrc<_vsTechnique, _vertexLayout, _cbufferLayout, _drawType>;
    const Shaders::PSSrc& psSrc = Shaders::psSrc<_psTechnique, _vertexLayout, _cbufferLayout>;
    params.vertexSrc = vsSrc.src;
    params.vertexShaderName = vsSrc.name;
    params.pixelSrc = psSrc.src;
    params.pixelShaderName = psSrc.name;
    params.samplerNames = psSrc.samplerNames; params.numSamplers = psSrc.numSamplers;
}
template <typename _vertexLayout, typename _cbufferLayout>
void create_shader_from_technique(Renderer::Driver::RscShaderSet<_vertexLayout, _cbufferLayout>& shaderSet, const TechniqueSrcParams<_vertexLayout, _cbufferLayout>& params) {
    Renderer::Driver::RscVertexShader<_vertexLayout, _cbufferLayout> rscVS;
    Renderer::Driver::RscPixelShader rscPS;
    Renderer::Driver::ShaderResult pixelResult;
    Renderer::Driver::ShaderResult vertexResult;
    pixelResult = Renderer::Driver::create_shader_ps(rscPS, { params.pixelSrc, (u32)strlen(params.pixelSrc) });
    if (!pixelResult.compiled) {
        Platform::debuglog("%s: %s\n", params.pixelShaderName, pixelResult.error);
        return;
    }
    vertexResult = Renderer::Driver::create_shader_vs(rscVS, { params.vertexSrc, (u32)strlen(params.vertexSrc) });
    if (!vertexResult.compiled) {
        Platform::debuglog("%s: %s\n", params.vertexShaderName, vertexResult.error);
        return;
    }
    // Todo: can't reuse pixel or vertex shader after this. Is that bad?
    Renderer::Driver::ShaderResult result = Renderer::Driver::create_shader_set(shaderSet, { rscVS, rscPS, params.cbuffers });
    if (!result.compiled) {
        Platform::debuglog("Linking %s & %s: %s\n", params.vertexShaderName, params.pixelShaderName, result.error);
    }

    if (params.numSamplers) {
        Renderer::Driver::bind_shader_samplers(shaderSet, params.samplerNames, params.numSamplers);
    }
}
void create_cbuffers_3DScene(Renderer::Driver::RscCBuffer (&buffers)[Renderer::Layout_CBuffer_3DScene::Buffers::Count]) {
    Driver::create_cbuffer<Renderer::Layout_CBuffer_3DScene::SceneData>(buffers[Renderer::Layout_CBuffer_3DScene::Buffers::SceneData], {});
    Driver::create_cbuffer<Renderer::Layout_CBuffer_3DScene::GroupData>(buffers[Renderer::Layout_CBuffer_3DScene::Buffers::GroupData], {});
    Driver::create_cbuffer<Renderer::Layout_CBuffer_3DScene::InstanceData>(buffers[Renderer::Layout_CBuffer_3DScene::Buffers::InstanceData], {});
}
void create_indexed_vertex_buffer_from_untextured_cube(Renderer::Driver::RscIndexedBuffer<Renderer::Layout_Vec3>& buffer, const CubeCreateParams& params) {
    Renderer::UntexturedCube cube;
    Renderer::create_untextured_cube_coords(cube, { 1.f, 1.f, 1.f });
    Renderer::Driver::IndexedBufferParams bufferParams;
    bufferParams.vertexData = cube.vertices;
    bufferParams.indexData = cube.indices;
    bufferParams.vertexSize = sizeof(cube.vertices);
    bufferParams.indexSize = sizeof(cube.indices);
    bufferParams.indexCount = bufferParams.indexSize / sizeof(cube.indices[0]);
    bufferParams.memoryUsage = Renderer::Driver::BufferMemoryUsage::GPU;
    bufferParams.accessType = Renderer::Driver::BufferAccessType::GPU;
    bufferParams.indexType = Renderer::Driver::BufferItemType::U16;
    bufferParams.type = Renderer::Driver::BufferTopologyType::Triangles;
    Renderer::Driver::create_indexed_vertex_buffer(buffer, bufferParams);
}
void create_indexed_vertex_buffer_from_textured_cube(Renderer::Driver::RscIndexedBuffer<Renderer::Layout_TexturedVec3>& buffer, const CubeCreateParams& params) {
    Renderer::TexturedCube cube;
    Renderer::create_textured_cube_coords(cube, { 1.f, 1.f, 1.f });
    Renderer::Driver::IndexedBufferParams bufferParams;
    bufferParams.vertexData = cube.vertices;
    bufferParams.indexData = cube.indices;
    bufferParams.vertexSize = sizeof(cube.vertices);
    bufferParams.indexSize = sizeof(cube.indices);
    bufferParams.indexCount = bufferParams.indexSize / sizeof(cube.indices[0]);
    bufferParams.memoryUsage = Renderer::Driver::BufferMemoryUsage::GPU;
    bufferParams.accessType = Renderer::Driver::BufferAccessType::GPU;
    bufferParams.indexType = Renderer::Driver::BufferItemType::U16;
    bufferParams.type = Renderer::Driver::BufferTopologyType::Triangles;
    Renderer::Driver::create_indexed_vertex_buffer(buffer, bufferParams);
}
namespace FBX {
    void extract_vertex(Renderer::Layout_Vec3& vertexout, const ufbx_vec3& vertexin) {
        vertexout.x = vertexin.x;
        vertexout.y = vertexin.y;
        vertexout.z = vertexin.z;
    }
    void extract_vertex(Renderer::Layout_Vec3Color4B& vertexout, const ufbx_vec3& vertexin) {
        vertexout.pos.x = vertexin.x;
        vertexout.pos.y = vertexin.y;
        vertexout.pos.z = vertexin.z;
    }
    void extract_vertex_attrib(Renderer::Layout_Vec3* vertices, const ufbx_mesh& mesh) {}
    void extract_vertex_attrib(Renderer::Layout_Vec3Color4B* vertices, const ufbx_mesh& mesh) {
        if (mesh.vertex_color.exists) {
            for (size_t index = 0; index < mesh.num_indices; index++) {
                auto& c = mesh.vertex_color[index];
                Col32 color(c.x, c.y, c.z, c.w);
                vertices[mesh.vertex_indices[index]].color = color.ABGR();
            }
        }
    }
    template <typename _vertexLayout>
    void load(Driver::RscIndexedBuffer<_vertexLayout>& rscBuffer, const char* path) {
        tinystl::vector<_vertexLayout, Allocator::FrameArena> vertices;
        tinystl::vector<u32, Allocator::FrameArena> indices;
        ufbx_load_opts opts = {};
        opts.allow_null_material = true;
        ufbx_error error;
        ufbx_scene* scene = ufbx_load_file(path, &opts, &error);
        if (scene) {
            u32 maxVertices = 0;
            for (size_t i = 0; i < scene->meshes.count; i++) { maxVertices += (u32)scene->meshes.data[i]->num_vertices; }

            vertices.reserve(maxVertices);
            indices.clear(); // todo: reserve
            u32 vertexOffset = 0;
            for (size_t i = 0; i < scene->meshes.count; i++) {
                ufbx_mesh& mesh = *scene->meshes.data[i];

                for (size_t i = 0; i < mesh.num_vertices; i++) {
                    _vertexLayout vertex;
                    extract_vertex(vertex, mesh.vertices[i]);
                    vertices.push_back(vertex);
                }

                // can't copy the face indexes directly, need to de-triangulate
                for (size_t i = 0; i < mesh.num_faces; i++) {
                    ufbx_face& face = mesh.faces[i];
                    const u32 maxTriIndices = 32;
                    u32 triIndices[maxTriIndices];
                    u32 numTris = ufbx_triangulate_face(triIndices, maxTriIndices, &mesh, face);
                    for (u32 vi = 0; vi < numTris * 3; vi++) {
                        const uint32_t triangulatedFaceIndex = triIndices[vi];
                        const u32 vertexIndex = mesh.vertex_indices[triangulatedFaceIndex];
                        indices.push_back(vertexIndex + vertexOffset);
                    }
                }
                _vertexLayout* meshFirstVertex = &vertices[vertexOffset];
                extract_vertex_attrib(meshFirstVertex, mesh);
                vertexOffset += (u32)mesh.num_vertices;
            }

            if (vertices.size() > 0 && indices.size() > 0)
            {
                Renderer::Driver::IndexedBufferParams bufferParams;
                bufferParams.vertexData = &vertices[0];
                bufferParams.vertexSize = (u32)vertices.size() * sizeof(vertices[0]);
                bufferParams.indexData = &indices[0];
                bufferParams.indexSize = (u32)indices.size() * sizeof(indices[0]);
                bufferParams.indexCount = (u32)indices.size();
                bufferParams.memoryUsage = Renderer::Driver::BufferMemoryUsage::GPU;
                bufferParams.accessType = Renderer::Driver::BufferAccessType::GPU;
                bufferParams.indexType = Renderer::Driver::BufferItemType::U32;
                bufferParams.type = Renderer::Driver::BufferTopologyType::Triangles;
                Renderer::Driver::create_indexed_vertex_buffer(rscBuffer, bufferParams);
            }
        }
    }
}

};

#endif // __WASTELADNS_RENDERER_H__
