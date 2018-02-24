#ifndef __WASTELADNS_ANGLE_H__
#define __WASTELADNS_ANGLE_H__

#ifndef __WASTELADNS_C_MATH_H__
#include <math.h>
#define __WASTELADNS_C_MATH_H__
#endif

#ifndef __WASTELADNS_TYPES_H__
#include "types.h"
#endif

#ifndef __WASTELADNS_TEMPLATE_DEFINES_H__
#include "template_defines.h"
#endif

#ifndef __WASTELADNS_VEC_H__
#include "vec.h"
#endif

// g=generic, s=specialized
#define ANGLE_TEMPLATES(g,s,...) \
    s(RT_PT(sin, __VA_ARGS__)) \
    s(RT_PT(cos, __VA_ARGS__)) \
    s(RT_PTT(atan2, __VA_ARGS__)) \
    s(RT_PTT(mod, __VA_ARGS__)) \
    g(RT2_PT(direction, __VA_ARGS__)) \
    g(RT_PT2(orientation, __VA_ARGS__))

namespace Angle {
DEFINE_TEMPLATES(ANGLE_TEMPLATES)
    
template <typename _T> const _T pi;
template <typename _T> const _T twopi;
template <typename _T> const _T r2d = 180.f / pi<_T>;
template <typename _T> const _T d2r = pi<_T> / 180.f;
template <> const f32 pi<f32> = 3.1415927410125732421875f;
template <> const f64 pi<f64> = 3.14159265358979311599796346854;
template <> const f32 twopi<f32> = 6.283185482025146484375f;
template <> const f64 twopi<f64> = 6.28318530717958623199592693709;
    
}

#endif // __WASTELADNS_ANGLE_H__
    
#ifdef __WASTELADNS_ANGLE_IMPL__
#undef __WASTELADNS_ANGLE_IMPL__

namespace Angle {


template <> f32 sin(const f32 v) {
    return ::sinf(v);
}
template <> f64 sin(const f64 v) {
    return ::sin(v);
}
template <> f32 cos(const f32 v) {
    return ::cosf(v);
}
template <> f64 cos(const f64 v) {
    return ::cos(v);
}
template <> f32 atan2(const f32 a, const f32 b) {
    return ::atan2f(a, b);
}
template <> f64 atan2(const f64 a, const f64 b) {
    return ::atan2(a, b);
}
template <> f32 mod(const f32 a, const f32 b) {
    return ::fmodf(a, b);
}
template <> f64 mod(const f64 a, const f64 b) {
    return ::fmod(a, b);
}
template <typename _T> Vector2<_T> direction(_T headingRad) {
    return Vector2<_T>(sin(headingRad), cos(headingRad));
}

template <typename _T> _T orientation(const Vector2<_T>& v) {
    return atan2(v.x, v.y);
}

// TODO: understand this
template <typename _T> _T modpi(_T rad) {
    rad = mod(rad + pi<_T>, twopi<_T>);
    if (rad < 0.f) {
        rad += twopi<_T>;
    }
    return rad - pi<_T>;
}
    
template <typename _T> _T subtractShort(_T toRad, _T fromRad) {
    _T delta = toRad - fromRad;
    return modpi(delta);
}

INSTANTIATE_TEMPLATES(ANGLE_TEMPLATES, f32)
INSTANTIATE_TEMPLATES(ANGLE_TEMPLATES, f64)
    
}
    
#endif // __WASTELADNS_ANGLE_IMPL__
