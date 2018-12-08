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
    
    void generateMatrix(Mat4& matrix, const OrthoProjection::Config& config) {
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
    
    // Expects right-handed view matrix, z-coordinate point towards the viewer
    void generateMatrix(Mat4& matrixRHwithYup, const PerspProjection::Config& config) {
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
    
    struct Instance {
        OrthoProjection orthoProjection;
        PerspProjection perspProjection;
        Mat4 viewMatrix;
        Immediate::Buffer immediateBuffer;
    };
};

#endif // __WASTELADNS_RENDERER_H__
