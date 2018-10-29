#ifndef __WASTELADNS_CAMERA_H__
#define __WASTELADNS_CAMERA_H__

#ifndef UNITYBUILD
#include <cstring>
#include "vec_ops.h"
#include "transform.h"
#endif

namespace CameraSystem {
    
    struct Camera {
        Transform transform;
        Mat4 modelviewMatrix;
    };
    
    void generateModelViewMatrix(Mat4& modelview, const Transform& t) {
        // We are left-handed, negate front for opengl
        modelview.col0 = t.right;
        modelview.col1 = t.up;
        modelview.col2 = Math::negate(t.front);
        modelview.col3 = t.pos;
        modelview.col0_w = 0.f;
        modelview.col1_w = 0.f;
        modelview.col2_w = 0.f;
        modelview.col3_w = 1.f;
        
        // Store the inverse modelview matrix for the renderer
        if (!Math::inverse(modelview)) {
            // Reset if something went wrong
            Transform fallbackTransform;
            Math::identity4x4(fallbackTransform);
            modelview.col0 = t.right;
            modelview.col1 = t.up;
            modelview.col2 = Math::negate(t.front);
            modelview.col3 = t.pos;
            modelview.col0_w = 0.f;
            modelview.col1_w = 0.f;
            modelview.col2_w = 0.f;
            modelview.col3_w = 1.f;
            Math::inverse(modelview);
        }
    }
};

#endif // __WASTELADNS_CAMERA_H__
