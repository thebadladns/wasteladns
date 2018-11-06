#ifndef __WASTELADNS_TRANSFORM_H__
#define __WASTELADNS_TRANSFORM_H__

#ifndef UNITYBUILD
#include "vec.h"
#endif

// Left-handed (right is to the right of front)
struct Transform33 {
    Transform33() {}
    Transform33(const Transform33& t) {
        matrix = t.matrix;
    }
    
    union {
        struct {
            Vec3 right;
            Vec3 front;
            Vec3 up;
        };
        Mat33 matrix;
    };
};

// Left-handed (right is to the right of front)
struct Transform {
    Transform() {}
    Transform(const Transform& t) {
        matrix = t.matrix;
    }
    
    union {
        struct {
            Vec3 right; f32 right_w;
            Vec3 front; f32 front_w;
            Vec3 up; f32 up_w;
            Vec3 pos; f32 pos_w;
        };
        struct {
            Vec3 x; f32 x_w;
            Vec3 y; f32 y_w;
            Vec3 z; f32 z_w;
        };
        f32 dataCM[16];
        Mat4 matrix;
    };
};

namespace Math {
    
    Vec3 RIGHT_AXIS(-1.f, 0.f, 0.f);
    Vec3 FRONT_AXIS(0.f, 1.f, 0.f);
    Vec3 UP_AXIS(0.f, 0.f, 1.f);
    
    Transform33 fromUpTowardsFront(const Vec3& up, const Vec3& front) {
        Transform33 t;
        t.front = front;
        t.up = Math::normalize(up);
        t.right = Math::cross(t.front, t.up);
        if (!Math::normalizeSafe(t.right)) {
            t.right = RIGHT_AXIS;
        }
        t.front = Math::normalize(Math::cross(t.up, t.right));
        
        return t;
    }
    
    Transform33 fromUp(const Vec3& up) {
        Transform33 t = fromUpTowardsFront(up, FRONT_AXIS);
        return t;
    }

    Transform33 fromFrontAndUpLH(const Vec3& front, const Vec3& up) {
        Transform33 t;
        t.up = up;
        t.front = Math::normalize(front);
        t.right = Math::cross(t.up, t.front);
        if (!Math::normalizeSafe(t.right)) {
            t.up = Math::normalize(Math::cross(t.front, RIGHT_AXIS));
            t.right = Math::cross(t.up, t.front);
        } else {
            t.up = Math::normalize(Math::cross(t.front, t.right));
        }
        return t;
    }

    Transform33 fromFront(const Vec3& front) {
        Transform33 t;
        t.up = UP_AXIS;
        t.front = Math::normalize(front);
        t.right = Math::cross(t.front, t.up);
        if (!Math::normalizeSafe(t.right)) {
            t.up = Math::normalize(Math::cross(RIGHT_AXIS, t.front));
            t.right = Math::cross(t.front, t.up);
        } else {
            t.up = Math::normalize(Math::cross(t.right, t.front));
        }
        return t;
    }
    
    void fromFront(Transform& t, const Vec3& v) {
        Transform33 tmp = fromFront(v);
        t.front = tmp.front;
        t.right = tmp.right;
        t.up = tmp.up;
    }

    void identity3x3(Transform& t) {
        t.right = RIGHT_AXIS;
        t.front = FRONT_AXIS;
        t.up = UP_AXIS;
    }
    
    void identity4x4(Transform& t) {
        identity3x3(t);
        t.pos = {};
        t.right_w = t.front_w = t.up_w = 0.f;
        t.pos_w = 1.f;
    }
    
    Vec2 transform3x3(const Transform& t, const Vec2& v) {
        Vec2 out(
              t.x.x * v.x + t.y.x * v.y
            , t.x.y * v.x + t.y.y * v.y
        );
        return out;
    }
    Vec3 transform3x3(const Transform& t, const Vec3& v) {
        Vec3 out(
              t.x.x * v.x + t.y.x * v.y + t.z.x * v.z
            , t.x.y * v.x + t.y.y * v.y + t.z.y * v.z
            , t.x.z * v.x + t.y.z * v.y + t.z.z * v.z
        );
        return out;
    }
    // TODO: probably will need to revise these when used
    Vec2 untransform3x3(const Transform& t, const Vec2& v) {
        Vec2 out = Math::add(
              Math::scale(t.x.xy, Math::dot(t.x.xy, v))
            , Math::scale(t.y.xy, Math::dot(t.y.xy, v))
        );
        return out;
    }
    Vec3 untransform3x3(const Transform& t, const Vec3& v) {
        Vec3 out = Math::scale(t.x, Math::dot(t.x, v));
        out = Math::add(out, Math::scale(t.y, Math::dot(t.y, v)));
        out = Math::add(out, Math::scale(t.z, Math::dot(t.z, v)));
        return out;
    }};

#endif // __WASTELADNS_TRANSFORM_H__
