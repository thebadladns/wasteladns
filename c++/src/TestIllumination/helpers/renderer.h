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

    // TODO: clear this out, I think ProjTypeZMinus1to1 should work on dx11 too?
    struct ProjTypeZMinus1to1;
    struct ProjTypeZ0to1;

    template <typename _type = ProjTypeZ0to1>
    void generateMatrix(Mat4& matrix, const OrthoProjection::Config& config);

    template<>
    void generateMatrix<ProjTypeZMinus1to1>(Mat4& matrix, const OrthoProjection::Config& config) {
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
    void generateMatrix<ProjTypeZ0to1>(Mat4& matrix, const OrthoProjection::Config& config) {
        f32* matrixCM = matrix.dataCM;
        memset(matrixCM, 0, sizeof(f32) * 16);
        matrixCM[0] = 2.f / (config.right - config.left);
        matrixCM[5] = 2.f / (config.top - config.bottom);
        matrixCM[10] = 1.f / (config.far - config.near);
        matrixCM[12] = -(config.right + config.left) / (config.right - config.left);
        matrixCM[13] = -(config.top + config.bottom) / (config.top - config.bottom);
        matrixCM[14] = config.near / (config.near - config.far);
        matrixCM[15] = 1.f;
    }

    // Expects right-handed view matrix, z-coordinate point towards the viewer
    template <typename _type = ProjTypeZ0to1>
    void generateMatrix(Mat4& matrixRHwithYup, const PerspProjection::Config& config);

    template <>
    void generateMatrix<ProjTypeZMinus1to1>(Mat4& matrixRHwithYup, const PerspProjection::Config& config) {
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
    void generateMatrix<ProjTypeZ0to1>(Mat4& matrixRHwithYup, const PerspProjection::Config& config) {
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
};

namespace Renderer {
    
    struct RasterizerFillMode { enum Enum { Fill, Line }; };
    struct BufferMemoryMode { enum Enum { GPU, CPU }; };
    struct BufferItemType { enum Enum { U16, U32 }; };
    struct BufferTopologyType { enum Enum { Triangles, Lines }; };

namespace Driver {
    
    struct RenderTargetParams {
        u32 width;
        u32 height;
        bool depth;
    };
    void create(RscMainRenderTarget&, const RenderTargetParams&);
    void bind(RscMainRenderTarget& rt);

    struct TextureFromFileParams {
        const char* path;
    };
    void create(RscTexture& t, const TextureFromFileParams& params);

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
