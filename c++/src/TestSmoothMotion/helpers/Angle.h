#ifndef __WASTELADNS_ANGLE_H__
#define __WASTELADNS_ANGLE_H__

#ifndef __WASTELADNS_C_MATH_H__
#include <math.h>
#define __WASTELADNS_C_MATH_H__
#endif

#ifndef __WASTELADNS_TYPES_H__
#include "Types.h"
#endif

#ifndef __WASTELADNS_VEC_H__
#include "Vec.h"
#endif

template <typename _T>
struct Angle {
    
    static const _T pi;
    static const _T twopi;
    static const _T r2d;
    static const _T d2r;
    
    static Vector2<_T> direction(_T headingRad);
    static _T heading(const Vector2<_T>& v);
    
    static _T modpi(_T rad);
    
    static _T shortestDelta(_T toRad, _T fromRad);
};

#ifndef __WASTELADNS_ANGLE_IMPL__
extern template struct Angle<f32>;
extern template struct Angle<f64>;
#endif

#endif // __WASTELADNS_ANGLE_H__

#ifdef __WASTELADNS_ANGLE_IMPL__
#undef __WASTELADNS_ANGLE_IMPL__

template <>
const f32 Angle<f32>::pi = 3.14159265358979323846264338327950288f;
template <>
const f32 Angle<f32>::twopi = 6.2831853071795864769252867665590576f;
template <>
const f32 Angle<f32>::r2d = 180.f / Angle<f32>::pi;
template <>
const f32 Angle<f32>::d2r = Angle<f32>::pi / 180.f;
template <>
const f64 Angle<f64>::pi = 3.14159265358979323846264338327950288;
template <>
const f64 Angle<f64>::twopi = 6.2831853071795864769252867665590576;
template <>
const f64 Angle<f64>::r2d = 180.0 / Angle<f64>::pi;
template <>
const f64 Angle<f64>::d2r = Angle<f64>::pi / 180.0;

template <>
Vector2<f32> Angle<f32>::direction(f32 headingRad) {
    return Vector2<f32>(sinf(headingRad), cosf(headingRad));
}
template <>
Vector2<f64> Angle<f64>::direction(f64 headingRad) {
    return Vector2<f64>(sin(headingRad), cos(headingRad));
}

template <>
f32 Angle<f32>::heading(const Vector2<f32>& v) {
    return atan2f(v.x, v.y);
}
template <>
f64 Angle<f64>::heading(const Vector2<f64>& v) {
    return atan2(v.x, v.y);
}

// TODO: understand this
template <>
f32 Angle<f32>::modpi(f32 rad) {
    rad = fmodf(rad + pi, twopi);
    if (rad < 0.f) {
        rad += twopi;
    }
    return rad - pi;
}
template <>
f64 Angle<f64>::modpi(f64 rad) {
    rad = fmod(rad + pi, twopi);
    if (rad < 0.f) {
        rad += twopi;
    }
    return rad - pi;
}

template <typename _T>
_T Angle<_T>::shortestDelta(_T toRad, _T fromRad) {
    _T delta = toRad - fromRad;
    return modpi(delta);
}

template struct Angle<f32>;
template struct Angle<f64>;

#endif // __WASTELADNS_ANGLE_IMPL__
