#ifndef __WASTELADNS_TRANSFORM_H__
#define __WASTELADNS_TRANSFORM_H__

#ifndef UNITYBUILD
#include "vec.h"
#endif

// coordinate system = right handed Z up

namespace math {
    const float3 rightAxis() { return { 1.f, 0.f, 0.f }; };
    const float3 frontAxis() { return { 0.f, 1.f, 0.f }; };
    const float3 upAxis() { return { 0.f, 0.f, 1.f }; };
}

struct Transform33 {
    union {
        float3x3 matrix;
        struct {
            float3 right;
            float3 front;
            float3 up;
        };
        struct {
            float3 x;
            float3 y;
            float3 z;
        };
        f32 m[9];
    };
};

struct Transform {
    Transform() : matrix() {}
    union {
        float4x4 matrix;
        struct {
            float3 right; f32 right_w;
            float3 front; f32 front_w;
            float3 up; f32 up_w;
            float3 pos; f32 pos_w;
        };
        struct {
            float3 x; f32 x_w;
            float3 y; f32 y_w;
            float3 z; f32 z_w;
        };
        f32 m[16];
    };
};

struct Camera {
    Transform transform;
    float4x4 viewMatrix;
};

namespace math {
    
// Match a given ZY plane: Z dir is enforced, Y may be recomputed
float3x3 fromYTowardsZ(const float3& y, const float3& z) {
    float3x3 t;
    t.col2 = z;
    t.col1 = math::normalize(y);
    t.col0 = math::cross(t.col1, t.col2);
    if (!math::normalizeSafe(t.col0)) {
        t.col2 = math::normalize(math::cross(float3(1.f, 0.f, 0.f), t.col1));
        t.col0 = math::cross(t.col1, t.col2);
    } else {
        t.col2 = math::normalize(math::cross(t.col0, t.col1));
    }
    return t;
}
    
// Match a given ZY plane: Y dir is enforced, Z may be recomputed
float3x3 fromZTowardsY(const float3& z, const float3& y) {
    float3x3 t;
    t.col1 = y;
    t.col2 = math::normalize(z);
    t.col0 = math::cross(t.col1, t.col2);
    if (!math::normalizeSafe(t.col0)) {
        t.col1 = math::normalize(math::cross(t.col2, float3(1.f, 0.f, 0.f)));
        t.col0 = math::cross(t.col1, t.col2);
    } else {
        t.col1 = math::normalize(math::cross(t.col2, t.col0));
    }

    return t;
}

Transform33 fromFrontTowardsUp(const float3& front, const float3& up) { return { fromYTowardsZ(front, up) }; }
Transform33 fromUpTowardsFront(const float3& up, const float3& front) { return { fromZTowardsY(up, front) }; }
Transform33 fromFront(const float3& front) { return fromFrontTowardsUp(front, upAxis()); }
Transform33 fromUp(const float3& up) { return fromUpTowardsFront(up, frontAxis()); }
void fromFrontTowardsUp(Transform& t, const float3& front, const float3& up) {
    Transform33 tmp = fromFrontTowardsUp(front, up);
    t.x = tmp.x; t.y = tmp.y; t.z = tmp.z;
}
void fromFront(Transform& t, const float3& front) {
    Transform33 tmp = fromFrontTowardsUp(front, upAxis());
    t.x = tmp.x; t.y = tmp.y; t.z = tmp.z;
}
}

namespace math {
    
Transform fromPositionScaleAndRotationEulers(const float3& position, f32 scale, const float3& rotationEulers) {
       
    Transform t;
    float4 toRotation = math::eulersToQuaterion(rotationEulers);
    float3x3 rotation = math::quaternionToRotationMatrix(toRotation);
    t.matrix.col0.v[0] = scale * rotation.col0.v[0];
    t.matrix.col0.v[1] = scale * rotation.col0.v[1];
    t.matrix.col0.v[2] = scale * rotation.col0.v[2];
    t.matrix.col1.v[0] = scale * rotation.col1.v[0];
    t.matrix.col1.v[1] = scale * rotation.col1.v[1];
    t.matrix.col1.v[2] = scale * rotation.col1.v[2];
    t.matrix.col2.v[0] = scale * rotation.col2.v[0];
    t.matrix.col2.v[1] = scale * rotation.col2.v[1];
    t.matrix.col2.v[2] = scale * rotation.col2.v[2];
    t.matrix.col3.v[0] = position.v[0];
    t.matrix.col3.v[1] = position.v[1];
    t.matrix.col3.v[2] = position.v[2];
    t.matrix.col0.v[3] = 0.f;
    t.matrix.col1.v[3] = 0.f;
    t.matrix.col2.v[3] = 0.f;
    t.matrix.col3.v[3] = 1.f;

    return t;
}

float4x4 toEyeSpace(const Transform& tRHwithZup) { // RH_Zup to RH_Yup_negZfront
    float4x4 o_rh_yup_znegfront;
    float3& right = o_rh_yup_znegfront.col0.xyz;
    float3& up = o_rh_yup_znegfront.col1.xyz;
    float3& back = o_rh_yup_znegfront.col2.xyz;
    float3& pos = o_rh_yup_znegfront.col3.xyz;
    right = tRHwithZup.right;
    up = tRHwithZup.up;
    back = math::negate(tRHwithZup.front);
    pos = tRHwithZup.pos;
    o_rh_yup_znegfront.col0.w = 0.f;
    o_rh_yup_znegfront.col1.w = 0.f;
    o_rh_yup_znegfront.col2.w = 0.f;
    o_rh_yup_znegfront.col3.w  = 1.f;
    return o_rh_yup_znegfront;
}

float3 eyePosToWorldPos(const float3 pos_ES, const Transform& tRHwithZup) { // Eye is RH_Yup_negZfront
    const float4 mousepos_WS = math::mult(tRHwithZup.matrix, float4(pos_ES.x, -pos_ES.z, pos_ES.y, 1.f)); // todo: scale is wrong! this is in the direction of the ray, but not on the screen
    return mousepos_WS.xyz;
}
    
void identity3x3(Transform& t) {
    t.x = { 1.f, 0.f, 0.f };
    t.y = { 0.f, 1.f, 0.f };
    t.z = { 0.f, 0.f, 1.f };
}
    
void identity4x4(Transform& t) {
    identity3x3(t);
    t.x_w = t.y_w = t.z_w = 0.f;
    t.pos = {};
    t.pos_w = 1.f;
}
}
#endif // __WASTELADNS_TRANSFORM_H__
