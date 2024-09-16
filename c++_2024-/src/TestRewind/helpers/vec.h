#ifndef __WASTELADNS_VEC_H__
#define __WASTELADNS_VEC_H__

#ifndef UNITYBUILD
#include "types.h"
#include "math.h"
#endif

#define VEC2_FORMAT(format) "(" format ", " format ")"
#define float3_FORMAT(format) "(" format ", " format ", " format ")"
#define float4_FORMAT(format) "(" format ", " format ", " format ", " format ")"
#define VEC2_PARAMS(v) (v).x, (v).y
#define float3_PARAMS(v) (v).x, (v).y, (v).z
#define float4_PARAMS(v) (v).x, (v).y, (v).z, (v).w

struct float2 {
    float2() = default;
    constexpr float2(const f32 _x, const f32 _y) : x(_x), y(_y) {}
    
    union {
        struct {
            f32 x, y;
        };
        f32 coords[2];
    };
};

struct float3 {
    float3() = default;
    constexpr float3(const f32 _x, const f32 _y, const f32 _z) : x(_x), y(_y), z(_z) {}
    constexpr float3(const f32 _x, const float2& _yz) : x(_x), y(_yz.x), z(_yz.y) {}
    constexpr float3(const float2& _xy, const f32 _z) : x(_xy.x), y(_xy.y), z(_z) {}
    
    union {
        struct {
            f32 x, y, z;
        };
        f32 coords[3];
        struct {
            float2 xy;
        };
        struct {
            f32 _padding;
            float2 yz;
        };
    };
};

struct float4 {
    float4() = default;
    constexpr float4(const f32 _x, const f32 _y, const f32 _z, const f32 _w) : x(_x), y(_y), z(_z), w(_w) {}
    constexpr float4(const f32 _x, const float3& _yzw) : x(_x), y(_yzw.x), z(_yzw.y), w(_yzw.z) {}
    constexpr float4(const float3& _xyz, const f32 _w) : x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w) {}
    constexpr float4(const float2& _xy, const float2& _yz) : x(_xy.x), y(_xy.y), z(_yz.x), w(_yz.y) {}

    union {
        struct {
            f32 x, y, z, w;
        };
        f32 coords[4];
        struct {
            float2 xy, zw;
        };
        struct {
            f32 _padding0;
            float2 yz;
        };
        struct {
            float3 xyz;
        };
        struct {
            f32 _padding1;
            float3 yzw;
        };
    };
};

// Column Major
// 0  3  6
// 1  4  7
// 2  5  8
struct float3x3 {
    float3x3() = default;

    union {
        struct {
            float3 col0;
            float3 col1;
            float3 col2;
        };
        f32 dataCM[16];
    };
};

// Column Major
// 0  4  8  12
// 1  5  9  13
// 2  6  10 14
// 3  7  11 15
struct float4x4 {
    float4x4() = default;

    union {
        struct {
            float4 col0;
            float4 col1;
            float4 col2;
            float4 col3;
        };
        f32 dataCM[16];
    };
};

namespace Math {

float2 negate(const float2& v) { return float2(-v.x, -v.y); }
float3 negate(const float3& v) { return float3(-v.x, -v.y, -v.z); }
float4 negate(const float4& v) { return float4(-v.x, -v.y, -v.z, -v.w); }
float2 scale(const float2& v, const f32 s) { return float2(v.x * s, v.y * s); }
float3 scale(const float3& v, const f32 s) { return float3(v.x * s, v.y * s, v.z * s); }
float4 scale(const float4& v, const f32 s) { return float4(v.x * s, v.y * s, v.z * s, v.w * s); }
float2 scale(const float2& v, const float2& s) { return float2(v.x * s.x, v.y * s.y); }
float3 scale(const float3& v, const float3& s) { return float3(v.x * s.x, v.y * s.y, v.z * s.z); }
float4 scale(const float4& v, const float4& s) { return float4(v.x * s.x, v.y * s.y, v.z * s.z, v.w * s.w); }
float2 invScale(const float2& v, const f32 s) { return float2(v.x / s, v.y / s); }
float3 invScale(const float3& v, const f32 s) { return float3(v.x / s, v.y / s, v.z / s); }
float4 invScale(const float4& v, const f32 s) { return float4(v.x / s, v.y / s, v.z / s, v.w / s); }
f32 mag(const float2& v) { return Math::sqrt(v.x * v.x + v.y * v.y); }
f32 mag(const float3& v) { return Math::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
f32 mag(const float4& v) { return Math::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }
f32 magSq(const float2& v) { return v.x * v.x + v.y * v.y; }
f32 magSq(const float3& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
f32 magSq(const float4& v) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }
float2 normalize(const float2& v) { return invScale(v, mag(v)); }
float3 normalize(const float3& v) { return invScale(v, mag(v)); }
float4 normalize(const float4& v) { return invScale(v, mag(v)); }
bool normalizeSafe(float2& v) { const f32 m = mag(v); if (m > Math::eps_f) { v = invScale(v, m); return true; } return false; }
bool normalizeSafe(float3& v) { const f32 m = mag(v); if (m > Math::eps_f) { v = invScale(v, m); return true; } return false; }
bool normalizeSafe(float4& v) { const f32 m = mag(v); if (m > Math::eps_f) { v = invScale(v, m); return true; } return false; }
f32 dot(const float2& a, const float2& b) { return a.x * b.x + a.y * b.y; }
f32 dot(const float3& a, const float3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
f32 dot(const float4& a, const float4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
float2 add(const float2& a, const float2& b) { return float2(a.x + b.x, a.y + b.y); }
float3 add(const float3& a, const float3& b) { return float3(a.x + b.x, a.y + b.y, a.z + b.z); }
float4 add(const float4& a, const float4& b) { return float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
float2 subtract(const float2& a, const float2& b) { return float2(a.x - b.x, a.y - b.y); }
float3 subtract(const float3& a, const float3& b) { return float3(a.x - b.x, a.y - b.y, a.z - b.z); }
float4 subtract(const float4& a, const float4& b) { return float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
f32 cross(const float2& a, const float2& b) { return a.x * b.y - a.y * b.x; }
float3 cross(const float3& a, const float3& b) { return float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
float4 cross(const float4& a, const float4& b) { return float4(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, 0.0); }
float2 max(const float2& a, const float2& b) { return float2(Math::max(a.x,b.x), Math::max(a.y, b.y)); }
float3 max(const float3& a, const float3& b) { return float3(Math::max(a.x,b.x), Math::max(a.y,b.y), Math::max(a.z,b.z)); }
float4 max(const float4& a, const float4& b) { return float4(Math::max(a.x,b.x), Math::max(a.y,b.y), Math::max(a.z,b.z), Math::max(a.w,b.w)); }
float2 min(const float2& a, const float2& b) { return float2(Math::min(a.x,b.x), Math::min(a.y, b.y)); }
float3 min(const float3& a, const float3& b) { return float3(Math::min(a.x,b.x), Math::min(a.y,b.y), Math::min(a.z,b.z)); }
float4 min(const float4& a, const float4& b) { return float4(Math::min(a.x,b.x), Math::min(a.y,b.y), Math::min(a.z,b.z), Math::min(a.w,b.w)); }
bool isCloseAll(const float2& a, const float2& b, const f32 d) { return Math::abs(a.x - b.x) < d && Math::abs(a.y - b.y) < d; }
bool isCloseAll(const float3& a, const float3& b, const f32 d) { return Math::abs(a.x - b.x) < d && Math::abs(a.y - b.y) < d && Math::abs(a.z - b.z) < d; }
bool isCloseAll(const float4& a, const float4& b, const f32 d) { return Math::abs(a.x - b.x) < d && Math::abs(a.y - b.y) < d && Math::abs(a.z - b.z) < d && Math::abs(a.w - b.w) < d; }
float2 lerp(const f32 t, const float2& a, const float2& b) { return float2(a.x+t*(b.x-a.x), a.y+t*(b.y-a.y)); }
float3 lerp(const f32 t, const float3& a, const float3& b) { return float3(a.x+t*(b.x-a.x), a.y+t*(b.y-a.y), a.z+t*(b.z-a.z)); }
float4 lerp(const f32 t, const float4& a, const float4& b) { return float4(a.x+t*(b.x-a.x), a.y+t*(b.y-a.y), a.z+t*(b.z-a.z), a.w+t*(b.w-a.w)); }

float3x3 mult(const float3x3& a, const float3x3& b) {
    float3 a_r0 ( a.dataCM[0], a.dataCM[3], a.dataCM[6] );
    float3 a_r1 ( a.dataCM[1], a.dataCM[4], a.dataCM[7] );
    float3 a_r2 ( a.dataCM[2], a.dataCM[5], a.dataCM[8] );
    
    float3x3 o;
    o.dataCM[0] = dot(a_r0, b.col0); o.dataCM[3] = dot(a_r0, b.col1); o.dataCM[6] = dot(a_r0, b.col2);
    o.dataCM[1] = dot(a_r1, b.col0); o.dataCM[4] = dot(a_r1, b.col1); o.dataCM[7] = dot(a_r1, b.col2);
    o.dataCM[2] = dot(a_r2, b.col0); o.dataCM[5] = dot(a_r2, b.col1); o.dataCM[8] = dot(a_r2, b.col2);
    return o;
}
float3x3 scale(const float3x3& m, const f32 s) {
    float3x3 o;
    o.col0 = scale(m.col0, s);
    o.col1 = scale(m.col1, s);
    o.col2 = scale(m.col2, s);
    return o;
}
float4x4 mult(const float4x4& a, const float4x4& b) {
    float4 a_r0 ( a.dataCM[0], a.dataCM[4], a.dataCM[8], a.dataCM[12] );
    float4 a_r1 ( a.dataCM[1], a.dataCM[5], a.dataCM[9], a.dataCM[13] );
    float4 a_r2 ( a.dataCM[2], a.dataCM[6], a.dataCM[10], a.dataCM[14] );
    float4 a_r3 ( a.dataCM[3], a.dataCM[7], a.dataCM[11], a.dataCM[15] );
    
    float4x4 o;
    o.dataCM[0] = dot(a_r0, b.col0); o.dataCM[4] = dot(a_r0, b.col1); o.dataCM[8] = dot(a_r0, b.col2); o.dataCM[12] = dot(a_r0, b.col3);
    o.dataCM[1] = dot(a_r1, b.col0); o.dataCM[5] = dot(a_r1, b.col1); o.dataCM[9] = dot(a_r1, b.col2); o.dataCM[13] = dot(a_r1, b.col3);
    o.dataCM[2] = dot(a_r2, b.col0); o.dataCM[6] = dot(a_r2, b.col1); o.dataCM[10] = dot(a_r2, b.col2); o.dataCM[14] = dot(a_r2, b.col3);
    o.dataCM[3] = dot(a_r3, b.col0); o.dataCM[7] = dot(a_r3, b.col1); o.dataCM[11] = dot(a_r3, b.col2); o.dataCM[15] = dot(a_r3, b.col3);
    return o;
}
float4x4 scale(const float4x4& a, const f32 s) {
    float4x4 o;
    o.col0 = scale(a.col0, s);
    o.col1 = scale(a.col1, s);
    o.col2 = scale(a.col2, s);
    o.col3 = scale(a.col3, s);
    return o;
}

}

#endif // __WASTELADNS_VEC_H__
