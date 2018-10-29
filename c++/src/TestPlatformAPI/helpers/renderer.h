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
        typedef Mat4 Matrix;
        Config config;
        Matrix matrix;
    };
    struct PerspProjection {
        struct Config {
            f32 fov;
            f32 aspect;
            f32 near;
            f32 far;
        };
        typedef Matrix44<f64> Matrix;
        Config config;
        Matrix matrix;
    };
    
    void generateMatrix(OrthoProjection::Matrix& matrix, const OrthoProjection::Config& config) {
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
    
    void generateMatrix(PerspProjection::Matrix& matrix, const PerspProjection::Config& config) {
        const f64 xMax = config.near * tan(config.fov * 2.f * Math::d2r<f64>);
        const f64 xMin = -xMax;
        const f64 yMin = xMin / config.aspect;
        const f64 yMax = xMax / config.aspect;
        
        f64* matrixCM = matrix.dataCM;
        memset(matrixCM, 0, sizeof(f64) * 16);
        matrixCM[0] = -(2.0 * config.near) / (xMax - xMin);
        matrixCM[5] = -(2.0 * config.near) / (yMax - yMin);
        matrixCM[10] = -(config.far + config.near) / (config.far - config.near);
        matrixCM[8] = -(xMax + xMin) / (xMax - xMin);
        matrixCM[9] = -(yMax + yMin) / (yMax - yMin);
        matrixCM[11] = -1.f;
        matrixCM[14] = -(2.0 * config.far * config.near) / (config.far - config.near);
    }
    
    struct Instance {
        OrthoProjection orthoProjection;
        PerspProjection perspProjection;
        Immediate::Buffer immediateBuffer;
    };
};

#endif // __WASTELADNS_RENDERER_H__
