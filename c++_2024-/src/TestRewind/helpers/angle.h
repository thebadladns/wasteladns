#ifndef __WASTELADNS_ANGLE_H__
#define __WASTELADNS_ANGLE_H__

#ifndef UNITYBUILD
#include "vec.h"
#endif

namespace Math {
    
const f32 pi_f = 3.1415927410125732421875f;
const f64 pi_d = 3.14159265358979311599796346854;
const f32 twopi_f = 6.283185482025146484375f;
const f64 twopi_d = 6.28318530717958623199592693709;
const f32 halfpi_f = 1.57079637050628662109375f;
const f64 halfpi_d = 1.57079632679489655799898173427;
const f32 r2d_f = 180.f / pi_f;
const f64 r2d_d = 180.0 / pi_d;
const f32 d2r_f = pi_f / 180.f;
const f64 d2r_d = pi_d / 180.0;

f32 sin(const f32 v) { return ::sinf(v); }
f64 sin(const f64 v) { return ::sin(v); }
f32 cos(const f32 v) { return ::cosf(v); }
f64 cos(const f64 v) { return ::cos(v); }
f32 tan(const f32 v) { return ::tanf(v); }
f64 tan(const f64 v) { return ::tan(v); }
f32 atan2(const f32 a, const f32 b) { return ::atan2f(a, b); }
f64 atan2(const f64 a, const f64 b) { return ::atan2(a, b); }
f32 asin(const f32 v) { return ::asinf(v); }
f64 asin(const f64 v) { return ::asin(v); }
f32 mod(const f32 a, const f32 b) { return ::fmodf(a, b); }
f64 mod(const f64 a, const f64 b) { return ::fmod(a, b); }
float2 direction(f32 headingRad) { return float2(sin(headingRad), cos(headingRad)); }
f32 orientation(const float2& v) { return atan2(v.x, v.y); }
f32 wrap(f32 rad) { // TODO: understand this
    rad = mod(rad + pi_f, twopi_f);
    if (rad < 0.f) { rad += twopi_f; }
    return rad - pi_f;
}
f64 wrap(f64 rad) { // TODO: understand this
    rad = mod(rad + pi_d, twopi_d);
    if (rad < 0.f) { rad += twopi_d; }
    return rad - pi_d;
}
f32 subtractShort(f32 toRad, f32 fromRad) { return wrap(toRad - fromRad); }
f64 subtractShort(f64 toRad, f64 fromRad) { return wrap(toRad - fromRad); }

}
    
#endif // __WASTELADNS_ANGLE_H__
