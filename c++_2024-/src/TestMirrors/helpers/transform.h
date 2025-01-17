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
    
Transform fromOffsetAndOrbit(const float3 offset, const float3 eulers) {
    
    Transform transform;
    float3x3 rotation = math::eulersToRotationMatrix(eulers);
    transform.matrix.col0.xyz = rotation.col0;
    transform.matrix.col1.xyz = rotation.col1;
    transform.matrix.col2.xyz = rotation.col2;
    transform.pos = float3(
        math::dot(float3(transform.matrix.col0.x, transform.matrix.col1.x, transform.matrix.col2.x), offset),
        math::dot(float3(transform.matrix.col0.y, transform.matrix.col1.y, transform.matrix.col2.y), offset),
        math::dot(float3(transform.matrix.col0.z, transform.matrix.col1.z, transform.matrix.col2.z), offset)
    );
    transform.matrix.col0.w = transform.matrix.col1.w = transform.matrix.col2.w = 0.f;
    transform.matrix.col3.w = 1.f;

    return transform;
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

namespace camera {

struct WindowProjection {
    struct Config {
        f32 left;
        f32 right;
        f32 top;
        f32 bottom;
        f32 near;
        f32 far;
    };
    Config config;
    float4x4 matrix;
};
struct PerspProjection {
    struct Config {
        f32 fov;
        f32 aspect;
        f32 near;
        f32 far;
    };
    Config config;
    float4x4 matrix;
};

struct Camera {
    Transform transform; // right handed z up
    float4x4 viewMatrix; // right handed y up
};

void generate_matrix_ortho_zneg1to1(float4x4& matrix, const WindowProjection::Config& config) {
    f32* matrixCM = matrix.m;
    memset(matrixCM, 0, sizeof(f32) * 16);
    matrixCM[0] = 2.f / (config.right - config.left);
    matrixCM[5] = 2.f / (config.top - config.bottom);
    matrixCM[10] = -2.f / (config.far - config.near);
    matrixCM[12] = -(config.right + config.left) / (config.right - config.left);
    matrixCM[13] = -(config.top + config.bottom) / (config.top - config.bottom);
    matrixCM[14] = -(config.far + config.near) / (config.far - config.near);
    matrixCM[15] = 1.f;
}
void generate_matrix_ortho_z0to1(float4x4& matrix, const WindowProjection::Config& config) {
    f32* matrixCM = matrix.m;
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
void generate_matrix_persp_zneg1to1(float4x4& matrixRHwithYup, const PerspProjection::Config& config) {
    const f32 h = 1.f / math::tan(config.fov * 0.5f * math::d2r32);
    const f32 w = h / config.aspect;

    // maps each xyz axis to [-1,1], not reversed-z (left handed, y points up z moves away from the viewer)
    f32(&matrixCM)[16] = matrixRHwithYup.m;
    memset(matrixCM, 0, sizeof(f32) * 16);
    matrixCM[0] = w;
    matrixCM[5] = h;
    matrixCM[10] = -(config.far + config.near) / (config.far - config.near);
    matrixCM[11] = -1.f;
    matrixCM[14] = -(2.f * config.far * config.near) / (config.far - config.near);
    // inverse is
    //matrixCM[0] = 1.f / w;
    //matrixCM[5] = 1.f / h;
    //matrixCM[10] = 0.f;
    //matrixCM[11] = -(config.far - config.near) / (2.f * config.far * config.near);
    //matrixCM[14] = -1.f;
    //matrixCM[15] = (config.far + config.near) / (2.f * config.far * config.near);
}
void add_oblique_plane_to_persp_zneg1to1(float4x4& projectionMatrix, const float4& planeCameraSpace) {
    // from https://terathon.com/lengyel/Lengyel-Oblique.pdf
    // The near plane is (0, 0, 1, 1), so it can be expressed in the basis of the projection matrix as near = M3 (third row) + M4 (fourth row).
    // We want the near plane to be equal to a clip plane C, new_M3 = aC - M4. The original far plane is (0,0,-1,1), or with the basis above, F = M4-M3
    // new_F = M4 - new_M3 = 2*M4 - a*C. since M4 is aways (0,0,-1,0), a point P=(x,y,0,w) on our clip plane (dot(C,P)=0) will be such that
    // dot(new_F,P) = 2*dot(M4,P) - a*dot(C,P) = 0, which means the new far plane intersects with the C plane on P, which is on the xy plane (no z coord). We find a point Q_proj on the edge
    // of the original far plane, Q_proj=(sign(C_proj.x),sign(C_proj.y),1,1), and claim that the sign of C_proj will be the same that of C.
    // We want our new far plane to go through Q_proj=(sign(C.x),sign(C.y),1,1), so in camera space, dot(new_F,Q)=0, then dot(new_F,Q) = 2*dot(M4,Q) - a*dot(C,Q) --> a = 2*dot(M4,Q) / dot(C,Q)
    // Again, M4 is always (0,0,-1,0), so a = -2*Qz / dot(C,Q). With the projection P and inverse P_inv matrices in renderer::generate_matrix_persp_zneg1to1,
    // Q = P_inv * Q_proj = (sign(C.x)/w,sign(C.y)/h,-1,1/f), which is Q = (sign(C.x)/P[0], sign(C.y)/P[5],-1.f,(1 + P[10]) / P[14])
    // This makes a = 2 / dot(C,Q), which combined with M3 = a*C - M4 = a*C - (0,0,-1,0), gives us this
    float4 q;
    q.x = math::sign(planeCameraSpace.x) / projectionMatrix.m[0];
    q.y = math::sign(planeCameraSpace.y) / projectionMatrix.m[5];
    q.z = -1.f;
    q.w = (1.f + projectionMatrix.m[10]) / projectionMatrix.m[14];
    float4 c = math::scale(planeCameraSpace, (2.f / math::dot(planeCameraSpace, q)));
    projectionMatrix.m[2] = c.x;
    projectionMatrix.m[6] = c.y;
    projectionMatrix.m[10] = c.z + 1.f;
    projectionMatrix.m[14] = c.w;
}

void generate_matrix_persp_z0to1(float4x4& matrixRHwithYup, const PerspProjection::Config& config) {
    const f32 h = 1.f / math::tan(config.fov * 0.5f * math::d2r32);
    const f32 w = h / config.aspect;
    // maps each xyz axis to [0,1], not reversed-z (left handed, y points up z moves away from the viewer)
    f32(&matrixCM)[16] = matrixRHwithYup.m;
    memset(matrixCM, 0, sizeof(f32) * 16);
    matrixCM[0] = w;
    matrixCM[5] = h;
    matrixCM[10] = config.far / (config.near - config.far);
    matrixCM[11] = -1.f;
    matrixCM[14] = config.far * config.near / (config.near - config.far);
    // inverse is
    //matrixCM[0] = 1.f / w;
    //matrixCM[5] = 1.f / h;
    //matrixCM[10] = 0.f;
    //matrixCM[11] = (config.near - config.far) / (config.far * config.near);
    //matrixCM[14] = -1.f;
    //matrixCM[15] = 1.f / config.near;
}
void add_oblique_plane_to_persp_z0to1(float4x4& projectionMatrix, const float4& planeCameraSpace) {
    // adapted to work with z [0,1] from https://terathon.com/lengyel/Lengyel-Oblique.pdf
    // The near plane is (0, 0, 1, 0), so it can be expressed in the basis of the projection matrix as near = M3 (third row).
    // We want the near plane to be equal to a clip plane C, new_M3 = a*C. The original far plane is (0,0,-1,1), or with the basis above, F = M4 - M3
    // new_F = M4 - new_M3 = M4 - a*C. since M4 is aways (0,0,-1,0), a point P=(x,y,0,w) on our clip plane (dot(C,P) = 0) will be such that
    // dot(new_F,P) = dot(M4,P) - a*dot(C,P) = 0, which means the new far plane intersects with the C plane on P, which is on the xy plane (no z coord). We find a point Q_proj on the edge
    // of the original far plane, Q_proj=(sign(C_proj.x),sign(C_proj.y),1,1), and claim that the sign of C_proj will be the same that of C.
    // We want our new far plane to go through Q_proj=(sign(C.x),sign(C.y),1,1), so in camera space, dot(new_F,Q) = 0, then dot(new_F,Q) = dot(M4,Q) - a*dot(C,Q) -> a = dot(M4,Q) / dot(C,Q)
    // Again, M4 is always (0,0,-1,0), so a = -Qz / dot(C,Q). With the projection P and inverse P_inv matrices in renderer::generate_matrix_persp_z0to1,
    // Q = P_inv * Q_proj = (sign(C.x)/w,sign(C.y)/h,-1,1/f), which is Q = (sign(C.x)/P[0],sign(C.y)/P[5],-1.f,(1 + P[10]) / P[14])
    // This makes a = 1 / dot(C,Q), which combined with M3 = a*C, gives us this

    float4 q;
    q.x = math::sign(planeCameraSpace.x) / projectionMatrix.m[0];
    q.y = math::sign(planeCameraSpace.y) / projectionMatrix.m[5];
    q.z = -1.f;
    q.w = (1.f + projectionMatrix.m[10]) / projectionMatrix.m[14];
    float4 c = math::scale(planeCameraSpace, (1.f / math::dot(planeCameraSpace, q)));
    projectionMatrix.m[2] = c.x;
    projectionMatrix.m[6] = c.y;
    projectionMatrix.m[10] = c.z;
    projectionMatrix.m[14] = c.w;
}
void extract_frustum_planes_from_vp_zneg1to1(float4* planes, const float4x4& vpMatrix) {
    // using Gribb-Hartmann's method:
    // https://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf
    // note that this uses the normalize plane equation ax + by + cz + w = 0
    float4x4 transpose = math::transpose(vpMatrix);
    planes[0] = math::add(transpose.col3, transpose.col2);      // near
    planes[1] = math::subtract(transpose.col3, transpose.col2); // far
    planes[2] = math::add(transpose.col3, transpose.col0);      // left
    planes[3] = math::subtract(transpose.col3, transpose.col0); // right
    planes[4] = math::add(transpose.col3, transpose.col1);      // bottom
    planes[5] = math::subtract(transpose.col3, transpose.col1); // top
    // normalize (maybe we should only do this on demand)
    planes[0] = math::invScale(planes[0], math::mag(planes[0].xyz));
    planes[1] = math::invScale(planes[1], math::mag(planes[1].xyz));
    planes[2] = math::invScale(planes[2], math::mag(planes[2].xyz));
    planes[3] = math::invScale(planes[3], math::mag(planes[3].xyz));
    planes[4] = math::invScale(planes[4], math::mag(planes[4].xyz));
    planes[5] = math::invScale(planes[5], math::mag(planes[5].xyz));
}
void extract_frustum_planes_from_vp_z0to1(float4* planes, const float4x4& vpMatrix) {
    // using Gribb-Hartmann's method:
    // https://www8.cs.umu.se/kurser/5DV051/HT12/lab/plane_extraction.pdf
    // note that this uses the normalize plane equation ax + by + cz + w = 0
    float4x4 transpose = math::transpose(vpMatrix);
    planes[0] = transpose.col2;                                 // near
    planes[1] = math::subtract(transpose.col3, transpose.col2); // far
    planes[2] = math::add(transpose.col3, transpose.col0);      // left
    planes[3] = math::subtract(transpose.col3, transpose.col0); // right
    planes[4] = math::add(transpose.col3, transpose.col1);      // bottom
    planes[5] = math::subtract(transpose.col3, transpose.col1); // top
    // normalize (maybe we should only do this on demand)
    planes[0] = math::invScale(planes[0], math::mag(planes[0].xyz));
    planes[1] = math::invScale(planes[1], math::mag(planes[1].xyz));
    planes[2] = math::invScale(planes[2], math::mag(planes[2].xyz));
    planes[3] = math::invScale(planes[3], math::mag(planes[3].xyz));
    planes[4] = math::invScale(planes[4], math::mag(planes[4].xyz));
    planes[5] = math::invScale(planes[5], math::mag(planes[5].xyz));
}
void generate_persp_frustum_params(
    f32& l, f32& r, f32& t, f32& b, const PerspProjection::Config& config) {
    f32 near = config.near;
    f32 far = config.far;
    f32 fov = config.fov;
    f32 aspect = config.aspect;
    const f32 fovtan = math::tan(math::d2r32 * fov * 0.5f);
    t = -fovtan * near;
    b = -t; // todo: check whether top and bottom should be switched?
    l = -fovtan * aspect * near;
    r = -l;
    // for matrices like
    //f32 matrixCM[16];
    //matrixCM[0] = (2 * near) / (r - l);
    //matrixCM[5] = (2 * near) / (b - t);
    //matrixCM[8] = (l + r) / (r - l);
    //matrixCM[9] = (t + b) / (b - t);
    //matrixCM[10] = far / (near - far); // for [0,1]
    //matrixCM[10] = -(far + near) / (near - far); // for [-1,1]
    //matrixCM[11] = -1.f;
    //matrixCM[14] = (near * far) / (near - far); for [0,1]
    //matrixCM[14] = -(2.f * far * near) / (near - far); //for [-1,1]
}

void generate_matrix_view(float4x4& viewMatrix, const Transform& tRHwithZup) {

    // RH_Zup to RH_Yup_negZfront
    float4x4 rh_yup_znegfront;
    float3& right = rh_yup_znegfront.col0.xyz;
    float3& up = rh_yup_znegfront.col1.xyz;
    float3& back = rh_yup_znegfront.col2.xyz;
    float3& pos = rh_yup_znegfront.col3.xyz;
    right = tRHwithZup.right;
    up = tRHwithZup.up;
    back = math::negate(tRHwithZup.front);
    pos = tRHwithZup.pos;
    rh_yup_znegfront.col0.w = 0.f;
    rh_yup_znegfront.col1.w = 0.f;
    rh_yup_znegfront.col2.w = 0.f;
    rh_yup_znegfront.col3.w = 1.f;

    // Simplified inverse world_to_view from view_to_world
    // https://www.gamedev.net/forums/topic/647149-d3dxmatrixlookatlh-internally/?tab=comments#comment-5089654

    const f32 tx = -math::dot(rh_yup_znegfront.col3, rh_yup_znegfront.col0);
    const f32 ty = -math::dot(rh_yup_znegfront.col3, rh_yup_znegfront.col1);
    const f32 tz = -math::dot(rh_yup_znegfront.col3, rh_yup_znegfront.col2);

    viewMatrix.m[0] = rh_yup_znegfront.col0.x;
    viewMatrix.m[4] = rh_yup_znegfront.col0.y;
    viewMatrix.m[8] = rh_yup_znegfront.col0.z;

    viewMatrix.m[1] = rh_yup_znegfront.col1.x;
    viewMatrix.m[5] = rh_yup_znegfront.col1.y;
    viewMatrix.m[9] = rh_yup_znegfront.col1.z;

    viewMatrix.m[2] = rh_yup_znegfront.col2.x;
    viewMatrix.m[6] = rh_yup_znegfront.col2.y;
    viewMatrix.m[10] = rh_yup_znegfront.col2.z;

    viewMatrix.m[12] = tx;
    viewMatrix.m[13] = ty;
    viewMatrix.m[14] = tz;

    viewMatrix.m[3] = rh_yup_znegfront.col0.w;
    viewMatrix.m[7] = rh_yup_znegfront.col1.w;;
    viewMatrix.m[11] = rh_yup_znegfront.col2.w;
    viewMatrix.m[15] = rh_yup_znegfront.col3.w;
}

float3 screenPosToWorldPos(const f32 x, const f32 y, const u32 w, const u32 h, const PerspProjection::Config& persp, const float4x4& viewMatrix) {
    // screen space to NDC space
    const float2 screen((f32)w, (f32)h);
    const float2 pos_NDC = math::add(math::invScale(math::scale(math::add(float2(x, y), float2(0.5f, -0.5f)), float2(2.f, -2.f)), screen), float2(-1.f, 1.f));
    const f32 tanfov = math::tan(persp.fov * 0.5f * math::d2r32);
    // NDC to eye space (right handed, y up, z front)
    float4 pos_ES(pos_NDC.x * persp.aspect * tanfov, pos_NDC.y * tanfov, -1.f, 1.f / persp.near);
    pos_ES = math::invScale(pos_ES, pos_ES.w);
    // to world space (right handed, z up)
    pos_ES = math::subtract(pos_ES, viewMatrix.col3);
    const float3 pos_WS(math::dot(viewMatrix.col0, pos_ES), math::dot(viewMatrix.col1, pos_ES), math::dot(viewMatrix.col2, pos_ES));
    return pos_WS;
}

}

#endif // __WASTELADNS_TRANSFORM_H__
