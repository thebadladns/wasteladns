#ifndef __WASTELADNS_CAMERA_H__
#define __WASTELADNS_CAMERA_H__

#ifndef UNITYBUILD
#include <cstring>
#include "vec_ops.h"
#include "transform.h"
#endif

struct Camera {
	Transform transform;
	Mat4 modelviewMatrix;
};

namespace CameraSystem {
    
    void generateModelViewMatrix(Mat4& modelview, const Transform& t) {
#if PLATFORM_DIRECTX9
        // Recompute camera transform taking into account the left-handedness
        Transform33 graphicsTransform = Math::fromFrontAndUpLH(t.front, t.up);
        modelview.col0 = graphicsTransform.right;
        modelview.col1 = graphicsTransform.up;
        modelview.col2 = graphicsTransform.front;
#else
        // We are left-handed, negate front for opengl
        modelview.col0 = t.right;
        modelview.col1 = t.up;
        modelview.col2 = Math::negate(t.front);
#endif
        modelview.col3 = t.pos;
        modelview.col0_w = 0.f;
        modelview.col1_w = 0.f;
        modelview.col2_w = 0.f;
        modelview.col3_w = 1.f;

        // TODO: it looks like these types of transforms can be inverted much easily
        // https://docs.microsoft.com/en-us/windows/desktop/direct3d9/d3dxmatrixlookatlh
        // Store the inverse modelview matrix for the renderer
        if (!Math::inverse(modelview)) {
            // Reset if something went wrong
            Transform fallbackTransform;
            Math::identity4x4(fallbackTransform);
            modelview.col0 = t.right;
            modelview.col1 = t.up;
#if PLATFORM_DIRECTX9
            modelview.col2 = t.front;
#else       
            modelview.col2 = Math::negate(t.front);
#endif
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
