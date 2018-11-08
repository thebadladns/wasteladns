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

    // Assumes input transform is right-handed, z-coordinate is up
    // up = right x front
    // front = up x right
    // right = front x up
    void generateModelViewMatrix(Mat4& modelview, const Transform& tRHwithZup) {

        // Switching up to the y axis requires negating the front to retain right-handedness
        Mat4 tRHwithYUp;
        tRHwithYUp.col0 = tRHwithZup.right;
        tRHwithYUp.col1 = tRHwithZup.up;
        tRHwithYUp.col2 = Math::negate(tRHwithZup.front);
        tRHwithYUp.col3 = tRHwithZup.pos;
        tRHwithYUp.col0_w = 0.f;
        tRHwithYUp.col1_w = 0.f;
        tRHwithYUp.col2_w = 0.f;
        tRHwithYUp.col3_w = 1.f;

        // Simplified inverse
        // https://www.gamedev.net/forums/topic/647149-d3dxmatrixlookatlh-internally/?tab=comments#comment-5089654

        const f32 tx = -Math::dot(tRHwithYUp.col3, tRHwithYUp.col0);
        const f32 ty = -Math::dot(tRHwithYUp.col3, tRHwithYUp.col1);
        const f32 tz = -Math::dot(tRHwithYUp.col3, tRHwithYUp.col2);

        modelview.dataCM[0] = tRHwithYUp.col0.x;
        modelview.dataCM[4] = tRHwithYUp.col0.y;
        modelview.dataCM[8] = tRHwithYUp.col0.z;

        modelview.dataCM[1] = tRHwithYUp.col1.x;
        modelview.dataCM[5] = tRHwithYUp.col1.y;
        modelview.dataCM[9] = tRHwithYUp.col1.z;

        modelview.dataCM[2] = tRHwithYUp.col2.x;
        modelview.dataCM[6] = tRHwithYUp.col2.y;
        modelview.dataCM[10] = tRHwithYUp.col2.z;

        modelview.dataCM[12] = tx;
        modelview.dataCM[13] = ty;
        modelview.dataCM[14] = tz;

        modelview.dataCM[3] = tRHwithYUp.col0_w;
        modelview.dataCM[7] = tRHwithYUp.col1_w;;
        modelview.dataCM[11] = tRHwithYUp.col2_w;;
        modelview.dataCM[15] = tRHwithYUp.col3_w;
    }
    
    struct Instance {
        OrthoProjection orthoProjection;
        PerspProjection perspProjection;
        Mat4 viewMatrix;
        Immediate::Buffer immediateBuffer;
    };
};

#endif // __WASTELADNS_RENDERER_H__
