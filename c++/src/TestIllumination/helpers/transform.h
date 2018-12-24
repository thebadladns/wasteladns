#ifndef __WASTELADNS_TRANSFORM_H__
#define __WASTELADNS_TRANSFORM_H__

#ifndef UNITYBUILD
#include "vec.h"
#endif

struct CoordinateSystem { enum Enum { RH_Zup, LH_Yup, RH_Yup_Zfront, Current = RH_Zup }; };

namespace Math {
    
    // Axis defined at the top to let the compiler use them as const expressions
    template <CoordinateSystem::Enum _system = CoordinateSystem::Current>
    constexpr const Vec3 rightAxis();
    template <CoordinateSystem::Enum _system = CoordinateSystem::Current>
    constexpr const Vec3 upAxis();
    template <CoordinateSystem::Enum _system = CoordinateSystem::Current>
    constexpr const Vec3 frontAxis();
    
    template <>
    constexpr const Vec3 rightAxis<CoordinateSystem::RH_Zup>() { return { 1.f, 0.f, 0.f }; };
    template <>
    constexpr const Vec3 frontAxis<CoordinateSystem::RH_Zup>() { return { 0.f, 1.f, 0.f }; };
    template <>
    constexpr const Vec3 upAxis<CoordinateSystem::RH_Zup>() { return { 0.f, 0.f, 1.f }; };
    
    template <>
    constexpr const Vec3 rightAxis<CoordinateSystem::LH_Yup>() { return { 1.f, 0.f, 0.f }; };
    template <>
    constexpr const Vec3 upAxis<CoordinateSystem::LH_Yup>() { return { 0.f, 1.f, 0.f }; };
    template <>
    constexpr const Vec3 frontAxis<CoordinateSystem::LH_Yup>() { return { 0.f, 0.f, 1.f }; };
}

template <CoordinateSystem::Enum _system>
struct TransformMatrix33;

template <>
struct TransformMatrix33<CoordinateSystem::RH_Zup> {
    union {
        struct {
            Vec3 right;
            Vec3 front;
            Vec3 up;
        };
        struct {
            Vec3 x;
            Vec3 y;
            Vec3 z;
        };
        Mat33 matrix;
    };
};
template <>
struct TransformMatrix33<CoordinateSystem::LH_Yup> {
    union {
        struct {
            Vec3 right;
            Vec3 up;
            Vec3 front;
        };
        struct {
            Vec3 x;
            Vec3 y;
            Vec3 z;
        };
        Mat33 matrix;
    };
};

template <CoordinateSystem::Enum _system>
struct TransformMatrix;

template <>
struct TransformMatrix<CoordinateSystem::RH_Zup> {
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

template <>
struct TransformMatrix<CoordinateSystem::LH_Yup> {
    union {
        struct {
            Vec3 right; f32 right_w;
            Vec3 up; f32 up_w;
            Vec3 front; f32 front_w;
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

template <>
struct TransformMatrix<CoordinateSystem::RH_Yup_Zfront> {
    union {
        struct {
            Vec3 right; f32 right_w;
            Vec3 up; f32 up_w;
            Vec3 back; f32 back_w;
            Vec3 pos; f32 pos_w;
        };
        struct {
            Vec3 x; f32 x_w;
            Vec3 z; f32 z_w;
            Vec3 y; f32 y_w;
        };
        f32 dataCM[16];
        Mat4 matrix;
    };
};

namespace Math {
    
    // Match a given ZY plane: Z dir is enforced, Y may be recomputed
    Mat33 fromYTowardsZ(const Vec3& y, const Vec3& z) {
        Mat33 t;
        t.col2 = z;
        t.col1 = Math::normalize(y);
        t.col0 = Math::cross(t.col1, t.col2);
        if (!Math::normalizeSafe(t.col0)) {
            t.col2 = Math::normalize(Math::cross(Vec3(1.f, 0.f, 0.f), t.col1));
            t.col0 = Math::cross(t.col1, t.col2);
        } else {
            t.col2 = Math::normalize(Math::cross(t.col0, t.col1));
        }
        return t;
    }
    
    // Match a given ZY plane: Y dir is enforced, Z may be recomputed
    Mat33 fromZTowardsY(const Vec3& z, const Vec3& y) {
        Mat33 t;
        t.col1 = y;
        t.col2 = Math::normalize(z);
        t.col0 = Math::cross(t.col1, t.col2);
        if (!Math::normalizeSafe(t.col0)) {
            t.col1 = Math::normalize(Math::cross(t.col2, Vec3(1.f, 0.f, 0.f)));
            t.col0 = Math::cross(t.col1, t.col2);
        } else {
            t.col1 = Math::normalize(Math::cross(t.col2, t.col0));
        }
        return t;
    }
    
    // Functions needing specialization
    template <CoordinateSystem::Enum _system = CoordinateSystem::Current>
    TransformMatrix33<_system> fromFrontTowardsUp(const Vec3& front, const Vec3& up);
    template <CoordinateSystem::Enum _system = CoordinateSystem::Current>
    TransformMatrix33<_system> fromUpTowardsFront(const Vec3& up, const Vec3& front);
    template <CoordinateSystem::Enum _system>
    TransformMatrix<CoordinateSystem::RH_Yup_Zfront> toEyeSpace(const TransformMatrix<_system> t);
    
    // Helpers
    template <CoordinateSystem::Enum _system = CoordinateSystem::Current>
    TransformMatrix33<_system> fromFront(const Vec3& front) {
        return fromFrontTowardsUp<_system>(front, upAxis());
    }
    template <CoordinateSystem::Enum _system = CoordinateSystem::Current>
    TransformMatrix33<_system> fromUp(const Vec3& up) {
        return fromUpTowardsFront<_system>(up, frontAxis());
    }
    
    // Mat44 proxies
    template <CoordinateSystem::Enum _system>
    void fromFrontTowardsUp(TransformMatrix<_system>& t, const Vec3& front, const Vec3& up) {
        TransformMatrix33<_system> tmp = fromFrontTowardsUp<_system>(front, up);
        t.x = tmp.x; t.y = tmp.y; t.z = tmp.z;
    }
    template <CoordinateSystem::Enum _system>
    void fromFront(TransformMatrix<_system>& t, const Vec3& front) {
        TransformMatrix33<_system> tmp = fromFrontTowardsUp<_system>(front, upAxis());
        t.x = tmp.x; t.y = tmp.y; t.z = tmp.z;
    }
}

typedef TransformMatrix33<CoordinateSystem::Current> Transform33;
typedef TransformMatrix<CoordinateSystem::Current> Transform;

namespace Math {
    
    template <>
    TransformMatrix33<CoordinateSystem::RH_Zup> fromFrontTowardsUp<CoordinateSystem::RH_Zup>(const Vec3& front, const Vec3& up) {
        TransformMatrix33<CoordinateSystem::RH_Zup> t;
        t.matrix = fromYTowardsZ(front, up);
        return t;
    }
    template <>
    TransformMatrix33<CoordinateSystem::RH_Zup> fromUpTowardsFront<CoordinateSystem::RH_Zup>(const Vec3& up, const Vec3& front) {
        TransformMatrix33<CoordinateSystem::RH_Zup> t;
        t.matrix = fromZTowardsY(up, front);
        return t;
    }
    template <>
    TransformMatrix<CoordinateSystem::RH_Yup_Zfront> toEyeSpace(const TransformMatrix<CoordinateSystem::RH_Zup> tRHwithZup) {
        TransformMatrix<CoordinateSystem::RH_Yup_Zfront> tRHwithYUp;
        tRHwithYUp.right = tRHwithZup.right;
        tRHwithYUp.up = tRHwithZup.up;
        tRHwithYUp.back = Math::negate(tRHwithZup.front);
        tRHwithYUp.pos = tRHwithZup.pos;
        tRHwithYUp.right_w = 0.f;
        tRHwithYUp.up_w = 0.f;
        tRHwithYUp.back_w = 0.f;
        tRHwithYUp.pos_w = 1.f;
        return tRHwithYUp;
    }
    
    template <>
    TransformMatrix33<CoordinateSystem::LH_Yup> fromFrontTowardsUp<CoordinateSystem::LH_Yup>(const Vec3& front, const Vec3& up) {
        TransformMatrix33<CoordinateSystem::LH_Yup> t;
        t.matrix = fromZTowardsY(front, up);
        return t;
    }
    template <>
    TransformMatrix33<CoordinateSystem::LH_Yup> fromUpTowardsFront<CoordinateSystem::LH_Yup>(const Vec3& up, const Vec3& front) {
        TransformMatrix33<CoordinateSystem::LH_Yup> t;
        t.matrix = fromYTowardsZ(up, front);
        return t;
    }
    template <>
    TransformMatrix<CoordinateSystem::RH_Yup_Zfront> toEyeSpace(const TransformMatrix<CoordinateSystem::LH_Yup> tLHwithYup) {
        TransformMatrix<CoordinateSystem::RH_Yup_Zfront> tRHwithYUp;
        tRHwithYUp.right = tLHwithYup.right;
        tRHwithYUp.up = tLHwithYup.up;
        tRHwithYUp.back = Math::negate(tLHwithYup.front);
        tRHwithYUp.pos = tLHwithYup.pos;
        tRHwithYUp.right_w = 0.f;
        tRHwithYUp.up_w = 0.f;
        tRHwithYUp.back_w = 0.f;
        tRHwithYUp.pos_w = 1.f;
        return tRHwithYUp;
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
