#ifndef __WASTELADNS_ANGLE_H__
#define __WASTELADNS_ANGLE_H__

#ifndef UNITYBUILD
#include "vec.h"
#endif

namespace Math {
    
template <typename _T> const _T pi;
template <typename _T> const _T twopi;
template <typename _T> const _T r2d = 180.f / pi<_T>;
template <typename _T> const _T d2r = pi<_T> / 180.f;
template <> const f32 pi<f32> = 3.1415927410125732421875f;
template <> const f64 pi<f64> = 3.14159265358979311599796346854;
template <> const f32 twopi<f32> = 6.283185482025146484375f;
template <> const f64 twopi<f64> = 6.28318530717958623199592693709;

template <typename _T> _T sin(const _T v);
    
template <> f32 sin(const f32 v) {
    return ::sinf(v);
}
template <> f64 sin(const f64 v) {
    return ::sin(v);
}
    
template <typename _T> _T cos(const _T v);

template <> f32 cos(const f32 v) {
    return ::cosf(v);
}
template <> f64 cos(const f64 v) {
    return ::cos(v);
}

template <typename _T> _T tan(const _T v);

template <> f32 tan(const f32 v) {
    return ::tanf(v);
}
template <> f64 tan(const f64 v) {
    return ::tan(v);
}

template <typename _T> _T atan2(const _T a, const _T b);

template <> f32 atan2(const f32 a, const f32 b) {
    return ::atan2f(a, b);
}
template <> f64 atan2(const f64 a, const f64 b) {
    return ::atan2(a, b);
}

template <typename _T> _T mod(const _T a, const _T b);

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
template <typename _T> _T wrap(_T rad) {
    rad = mod(rad + pi<_T>, twopi<_T>);
    if (rad < 0.f) {
        rad += twopi<_T>;
    }
    return rad - pi<_T>;
}
    
template <typename _T> _T subtractShort(_T toRad, _T fromRad) {
    _T delta = toRad - fromRad;
    return wrap(delta);
}

}
    
#endif // __WASTELADNS_ANGLE_H__
