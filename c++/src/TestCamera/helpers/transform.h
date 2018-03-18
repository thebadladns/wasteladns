#ifndef __WASTELADNS_TRANSFORM_H__
#define __WASTELADNS_TRANSFORM_H__

namespace Vector {
    
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
    
    Vec3 RIGHT_AXIS(1.f, 0.f, 0.f);
    Vec3 FRONT_AXIS(0.f, 1.f, 0.f);
    Vec3 UP_AXIS(0.f, 0.f, 1.f);
    
    void transformFromFront(Transform& t, const Vec3& v) {
        Vec3 tmpUp = UP_AXIS;
        Vec3 right = Vec::cross(v, tmpUp);
        if (!Vec::normalizeSafe(right)) {
            right = RIGHT_AXIS;
        }
        Vec3 up = Vec::normalize(Vec::cross(right, v));
        Vec3 front = Vec::cross(up, right);
        t.front = front;
        t.right = right;
        t.up = up;
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
        Vec2 out = Vec::add(
              Vec::scale(t.x.xy, Vec::dot(t.x.xy, v))
            , Vec::scale(t.y.xy, Vec::dot(t.y.xy, v))
        );
        return out;
    }
    Vec3 untransform3x3(const Transform& t, const Vec3& v) {
        Vec3 out = Vec::scale(t.x, Vec::dot(t.x, v));
        out = Vec::add(out, Vec::scale(t.y, Vec::dot(t.y, v)));
        out = Vec::add(out, Vec::scale(t.z, Vec::dot(t.z, v)));
        return out;
    }};

#endif // __WASTELADNS_TRANSFORM_H__
