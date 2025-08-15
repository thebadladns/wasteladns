#ifndef __WASTELADNS_VEC_H__
#define __WASTELADNS_VEC_H__

#define FLOAT2_FORMAT(format) "(" format ", " format ")"
#define FLOAT3_FORMAT(format) "(" format ", " format ", " format ")"
#define FLOAT4_FORMAT(format) "(" format ", " format ", " format ", " format ")"
#define FLOAT2_PARAMS(v) (v).x, (v).y
#define FLOAT3_PARAMS(v) (v).x, (v).y, (v).z
#define FLOAT4_PARAMS(v) (v).x, (v).y, (v).z, (v).w

struct float2 {
    float2() : x{}, y{} {}
    float2(const f32 x, const f32 y) : x(x), y(y) {}
    union {
        struct { f32 x, y; };
        f32 v[2];
    };
};

struct float3 {
    float3() : x{}, y{}, z{} {}
    float3(const f32 x, const f32 y, const f32 z) : x(x), y(y), z(z) {}
    float3(const f32 x, const float2& yz) : x(x), y(yz.x), z(yz.y) {}
    float3(const float2& xy, const f32 z) : x(xy.x), y(xy.y), z(z) {}
    union {
        struct { f32 x, y, z; };
        f32 v[3];
        struct { float2 xy; };
        struct { f32 padding; float2 yz; };
    };
};

struct float4 {
    float4() : x{}, y{}, z{}, w{} {}
    float4(const f32 x, const f32 y, const f32 z, const f32 w) : x(x), y(y), z(z), w(w) {}
    float4(const f32 x, const float3& yzw) : x(x), y(yzw.x), z(yzw.y), w(yzw.z) {}
    float4(const float3& xyz, const f32 w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
    float4(const float2& xy, const float2& yz) : x(xy.x), y(xy.y), z(yz.x), w(yz.y) {}

    union {
        struct { f32 x, y, z, w;
        };
        f32 v[4];
        struct { float2 xy, zw; };
        struct { f32 padding0; float2 yz; };
        struct { float3 xyz; };
        struct { f32 padding1; float3 yzw; };
    };
};

// Column Major
// 0  3  6
// 1  4  7
// 2  5  8
struct float3x3 {
    float3x3() : col0{}, col1{}, col2{} {}
    union {
        struct { float3 col0; float3 col1; float3 col2; };
        f32 m[9];
    };
};

// Row Major would be
//  0  1  2  3
//  4  5  6  7
//  8  9 10 11
// 12 13 14 15
// Column Major is the one we use
// 0  4  8  12
// 1  5  9  13
// 2  6  10 14
// 3  7  11 15
struct float4x4 {
    float4x4() : col0{}, col1{}, col2{}, col3{} {}
    union {
        struct { float4 col0; float4 col1; float4 col2; float4 col3; };
        f32 m[16];
    };
};

namespace math {

force_inline float2 negate(const float2& v) { return float2(-v.x, -v.y); }
force_inline float3 negate(const float3& v) { return float3(-v.x, -v.y, -v.z); }
force_inline float4 negate(const float4& v) { return float4(-v.x, -v.y, -v.z, -v.w); }
force_inline float2 scale(const float2& v, const f32 s) { return float2(v.x * s, v.y * s); }
force_inline float3 scale(const float3& v, const f32 s) { return float3(v.x * s, v.y * s, v.z * s); }
force_inline float4 scale(const float4& v, const f32 s) { return float4(v.x * s, v.y * s, v.z * s, v.w * s); }
force_inline float2 scale(const float2& v, const float2& s) { return float2(v.x * s.x, v.y * s.y); }
force_inline float3 scale(const float3& v, const float3& s) { return float3(v.x * s.x, v.y * s.y, v.z * s.z); }
force_inline float4 scale(const float4& v, const float4& s) { return float4(v.x * s.x, v.y * s.y, v.z * s.z, v.w * s.w); }
force_inline float2 invScale(const float2& v, const f32 s) { return float2(v.x / s, v.y / s); }
force_inline float3 invScale(const float3& v, const f32 s) { return float3(v.x / s, v.y / s, v.z / s); }
force_inline float4 invScale(const float4& v, const f32 s) { return float4(v.x / s, v.y / s, v.z / s, v.w / s); }
force_inline float2 invScale(const float2& v, const float2& s) { return float2(v.x / s.x, v.y / s.y); }
force_inline float3 invScale(const float3& v, const float3& s) { return float3(v.x / s.x, v.y / s.y, v.z / s.z); }
force_inline float4 invScale(const float4& v, const float4& s) { return float4(v.x / s.x, v.y / s.y, v.z / s.z, v.w / s.w); }
force_inline f32 mag(const float2& v) { return math::sqrt(v.x * v.x + v.y * v.y); }
force_inline f32 mag(const float3& v) { return math::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
force_inline f32 mag(const float4& v) { return math::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }
force_inline f32 magSq(const float2& v) { return v.x * v.x + v.y * v.y; }
force_inline f32 magSq(const float3& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
force_inline f32 magSq(const float4& v) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }
force_inline f32 invMag(const float2& v) { return math::rsqrt(v.x * v.x + v.y * v.y); }
force_inline f32 invMag(const float3& v) { return math::rsqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
force_inline f32 invMag(const float4& v) { return math::rsqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }
force_inline float2 normalize(const float2& v) { return scale(v, invMag(v)); }
force_inline float3 normalize(const float3& v) { return scale(v, invMag(v)); }
force_inline float4 normalize(const float4& v) { return scale(v, invMag(v)); }
force_inline bool normalizeSafe(float2& v) { const f32 m = mag(v); if (m > math::eps32) { v = invScale(v, m); return true; } return false; }
force_inline bool normalizeSafe(float3& v) { const f32 m = mag(v); if (m > math::eps32) { v = invScale(v, m); return true; } return false; }
force_inline bool normalizeSafe(float4& v) { const f32 m = mag(v); if (m > math::eps32) { v = invScale(v, m); return true; } return false; }
force_inline f32 dot(const float2& a, const float2& b) { return a.x * b.x + a.y * b.y; }
force_inline f32 dot(const float3& a, const float3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
force_inline f32 dot(const float4& a, const float4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
force_inline float2 add(const float2& a, const float2& b) { return float2(a.x + b.x, a.y + b.y); }
force_inline float3 add(const float3& a, const float3& b) { return float3(a.x + b.x, a.y + b.y, a.z + b.z); }
force_inline float4 add(const float4& a, const float4& b) { return float4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
force_inline float2 subtract(const float2& a, const float2& b) { return float2(a.x - b.x, a.y - b.y); }
force_inline float3 subtract(const float3& a, const float3& b) { return float3(a.x - b.x, a.y - b.y, a.z - b.z); }
force_inline float4 subtract(const float4& a, const float4& b) { return float4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
force_inline f32 cross(const float2& a, const float2& b) { return a.x * b.y - a.y * b.x; }
force_inline float3 cross(const float3& a, const float3& b) { return float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
force_inline float4 cross(const float4& a, const float4& b) { return float4(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x, 0.0); }
force_inline float2 max(const float2& a, const float2& b) { return float2(math::max(a.x,b.x), math::max(a.y, b.y)); }
force_inline float3 max(const float3& a, const float3& b) { return float3(math::max(a.x,b.x), math::max(a.y,b.y), math::max(a.z,b.z)); }
force_inline float4 max(const float4& a, const float4& b) { return float4(math::max(a.x,b.x), math::max(a.y,b.y), math::max(a.z,b.z), math::max(a.w,b.w)); }
force_inline float2 min(const float2& a, const float2& b) { return float2(math::min(a.x,b.x), math::min(a.y, b.y)); }
force_inline float3 min(const float3& a, const float3& b) { return float3(math::min(a.x,b.x), math::min(a.y,b.y), math::min(a.z,b.z)); }
force_inline float4 min(const float4& a, const float4& b) { return float4(math::min(a.x,b.x), math::min(a.y,b.y), math::min(a.z,b.z), math::min(a.w,b.w)); }
force_inline float2 clamp(const float2& a, const float2& l, const float2& h) { return float2(math::clamp(a.x, l.x, h.x), math::clamp(a.y, l.y, h.y)); }
force_inline float3 clamp(const float3& a, const float3& l, const float3& h) { return float3(math::clamp(a.x, l.x, h.x), math::clamp(a.y, l.y, h.y), math::clamp(a.z, l.z, h.z)); }
force_inline float4 clamp(const float4& a, const float4& l, const float4& h) { return float4(math::clamp(a.x, l.x, h.x), math::clamp(a.y, l.y, h.y), math::clamp(a.z, l.z, h.z), math::clamp(a.w, l.w, h.w)); }
force_inline bool isCloseAll(const float2& a, const float2& b, const f32 d) { return math::abs(a.x - b.x) < d && math::abs(a.y - b.y) < d; }
force_inline bool isCloseAll(const float3& a, const float3& b, const f32 d) { return math::abs(a.x - b.x) < d && math::abs(a.y - b.y) < d && math::abs(a.z - b.z) < d; }
force_inline bool isCloseAll(const float4& a, const float4& b, const f32 d) { return math::abs(a.x - b.x) < d && math::abs(a.y - b.y) < d && math::abs(a.z - b.z) < d && math::abs(a.w - b.w) < d; }
force_inline bool isZeroAll(const float2& a) { return a.x == 0.f && a.y == 0.f; }
force_inline bool isZeroAll(const float3& a) { return a.x == 0.f && a.y == 0.f && a.z == 0.f; }
force_inline bool isZeroAll(const float4& a) { return a.x == 0.f && a.y == 0.f && a.z == 0.f && a.w == 0.f; }
force_inline bool isZeroAll(const float3x3& m) {
    return m.m[0] == 0.f && m.m[1] == 0.f && m.m[2] == 0.f && m.m[3] == 0.f && m.m[4] == 0.f
        && m.m[5] == 0.f && m.m[6] == 0.f && m.m[7] == 0.f && m.m[8] == 0.f;
}
force_inline bool isZeroAll(const float4x4& m) {
    return m.m[0] == 0.f && m.m[1] == 0.f && m.m[2] == 0.f && m.m[3] == 0.f && m.m[4] == 0.f && m.m[5] == 0.f && m.m[6] == 0.f && m.m[7] == 0.f
        && m.m[8] == 0.f && m.m[9] == 0.f && m.m[10] == 0.f && m.m[11] == 0.f && m.m[12] == 0.f && m.m[13] == 0.f && m.m[14] == 0.f && m.m[15] == 0.f;
}
force_inline bool isNanAny(const float2& v) { return isnan(v.x) && isnan(v.x); }
force_inline bool isNanAny(const float3& v) { return isnan(v.x) && isnan(v.y) && isnan(v.z); }
force_inline bool isNanAny(const float4& v) { return isnan(v.x) && isnan(v.y) && isnan(v.z) && isnan(v.w); }
force_inline float2 lerp(const f32 t, const float2& a, const float2& b) { return float2(a.x+t*(b.x-a.x), a.y+t*(b.y-a.y)); }
force_inline float3 lerp(const f32 t, const float3& a, const float3& b) { return float3(a.x+t*(b.x-a.x), a.y+t*(b.y-a.y), a.z+t*(b.z-a.z)); }
force_inline float4 lerp(const f32 t, const float4& a, const float4& b) { return float4(a.x+t*(b.x-a.x), a.y+t*(b.y-a.y), a.z+t*(b.z-a.z), a.w+t*(b.w-a.w)); }

force_inline float3x3 mult(const float3x3& a, const float3x3& b) {
    float3 a_r0 ( a.m[0], a.m[3], a.m[6] );
    float3 a_r1 ( a.m[1], a.m[4], a.m[7] );
    float3 a_r2 ( a.m[2], a.m[5], a.m[8] );
    
    float3x3 o;
    o.m[0] = dot(a_r0, b.col0); o.m[3] = dot(a_r0, b.col1); o.m[6] = dot(a_r0, b.col2);
    o.m[1] = dot(a_r1, b.col0); o.m[4] = dot(a_r1, b.col1); o.m[7] = dot(a_r1, b.col2);
    o.m[2] = dot(a_r2, b.col0); o.m[5] = dot(a_r2, b.col1); o.m[8] = dot(a_r2, b.col2);
    return o;
}
force_inline float3 mult(const float3x3& m, const float3& v) {
    return float3(
          m.col0.x * v.x + m.col1.x * v.y + m.col2.x * v.z
        , m.col0.y * v.x + m.col1.y * v.y + m.col2.y * v.z
        , m.col0.z * v.x + m.col1.z * v.y + m.col2.z * v.z
    );
}
force_inline float3x3 scale(const float3x3& m, const f32 s) {
    float3x3 o;
    o.col0 = scale(m.col0, s);
    o.col1 = scale(m.col1, s);
    o.col2 = scale(m.col2, s);
    return o;
}
force_inline float3x3 transpose(const float3x3& m) {
    float3x3 o;
    o.col0 = { m.col0.x, m.col1.x, m.col2.x };
    o.col1 = { m.col0.y, m.col1.y, m.col2.y };
    o.col2 = { m.col0.z, m.col1.z, m.col2.z };
    return o;
}
force_inline float4x4 mult(const float4x4& a, const float4x4& b) {
    float4 a_r0 ( a.m[0], a.m[4], a.m[8], a.m[12] );
    float4 a_r1 ( a.m[1], a.m[5], a.m[9], a.m[13] );
    float4 a_r2 ( a.m[2], a.m[6], a.m[10], a.m[14] );
    float4 a_r3 ( a.m[3], a.m[7], a.m[11], a.m[15] );
    
    float4x4 o;
    o.m[0] = dot(a_r0, b.col0); o.m[4] = dot(a_r0, b.col1); o.m[8] = dot(a_r0, b.col2); o.m[12] = dot(a_r0, b.col3);
    o.m[1] = dot(a_r1, b.col0); o.m[5] = dot(a_r1, b.col1); o.m[9] = dot(a_r1, b.col2); o.m[13] = dot(a_r1, b.col3);
    o.m[2] = dot(a_r2, b.col0); o.m[6] = dot(a_r2, b.col1); o.m[10] = dot(a_r2, b.col2); o.m[14] = dot(a_r2, b.col3);
    o.m[3] = dot(a_r3, b.col0); o.m[7] = dot(a_r3, b.col1); o.m[11] = dot(a_r3, b.col2); o.m[15] = dot(a_r3, b.col3);
    return o;
}
force_inline float4 mult(const float4x4& m, const float4& v) {
    return float4(
          m.col0.x * v.x + m.col1.x * v.y + m.col2.x * v.z + m.col3.x * v.w
        , m.col0.y * v.x + m.col1.y * v.y + m.col2.y * v.z + m.col3.y * v.w
        , m.col0.z * v.x + m.col1.z * v.y + m.col2.z * v.z + m.col3.z * v.w
        , m.col0.w * v.x + m.col1.w * v.y + m.col2.w * v.z + m.col3.w * v.w
    );
}
force_inline float3 mult3x3(const float4x4& m, const float3& v) {
    return float3(
          m.col0.x * v.x + m.col1.x * v.y + m.col2.x * v.z
        , m.col0.y * v.x + m.col1.y * v.y + m.col2.y * v.z
        , m.col0.z * v.x + m.col1.z * v.y + m.col2.z * v.z
    );
}
force_inline float4x4 scale(const float4x4& a, const f32 s) {
    float4x4 o;
    o.col0 = scale(a.col0, s);
    o.col1 = scale(a.col1, s);
    o.col2 = scale(a.col2, s);
    o.col3 = scale(a.col3, s);
    return o;
}
force_inline float4x4 transpose(const float4x4& m) {
    float4x4 o;
    o.col0 = { m.col0.x, m.col1.x, m.col2.x, m.col3.x };
    o.col1 = { m.col0.y, m.col1.y, m.col2.y, m.col3.y };
    o.col2 = { m.col0.z, m.col1.z, m.col2.z, m.col3.z };
    o.col3 = { m.col0.w, m.col1.w, m.col2.w, m.col3.w };
    return o;
}

}

#endif // __WASTELADNS_VEC_H__
