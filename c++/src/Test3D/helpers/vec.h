#ifndef __WASTELADNS_VEC_H__
#define __WASTELADNS_VEC_H__

#ifndef UNITYBUILD
#include "types.h"
#include "math.h"
#endif

#define VEC2_FORMAT(format) "(" format ", " format ")"
#define VEC3_FORMAT(format) "(" format ", " format ", " format ")"
#define VEC4_FORMAT(format) "(" format ", " format ", " format ", " format ")"
#define VEC2_PARAMS(v) v.x, v.y
#define VEC3_PARAMS(v) v.x, v.y, v.z
#define VEC4_PARAMS(v) v.x, v.y, v.z, v.w

template <typename _T>
struct Vector2 {
    constexpr Vector2() = default;
    constexpr Vector2(const _T x, const _T y);
    
    union {
        struct {
            _T x, y;
        };
        _T coords[2];
    };
};
typedef Vector2<f32> Vec2;

template <typename _T>
struct Vector3 {
    constexpr Vector3() = default;
    constexpr Vector3(const _T x, const _T y, const _T z);
    constexpr Vector3(const _T x, const Vector2<_T>& yz);
    constexpr Vector3(const Vector2<_T>& xy, const _T z);
    
    union {
        struct {
            _T x, y, z;
        };
        _T coords[3];
        struct {
            Vector2<_T> xy;
        };
        struct {
            _T _padding;
            Vector2<_T> yz;
        };
    };
};
typedef Vector3<f32> Vec3;

template <typename _T>
struct Vector4 {
    constexpr Vector4() = default;
    constexpr Vector4(const _T x, const _T y, const _T z, const _T w);
    constexpr Vector4(const _T x, const Vector3<_T>& yzw);
    constexpr Vector4(const Vector3<_T>& xyz, const _T w);
    constexpr Vector4(const Vector2<_T>& xy, const Vector2<_T>& yz);

    union {
        struct {
            _T x, y, z, w;
        };
        _T coords[4];
        struct {
            Vector2<_T> xy, zw;
        };
        struct {
            _T _padding0;
            Vector2<_T> yz;
        };
        struct {
            Vector3<_T> xyz;
        };
        struct {
            _T _padding1;
            Vector3<_T> yzw;
        };
    };
};
typedef Vector4<f32> Vec4;

// Column Major
// 0  3  6
// 1  4  7
// 2  5  8
template <typename _T>
struct Matrix33 {
    constexpr Matrix33() = default;

    union {
        struct {
            Vector3<_T> col0;
            Vector3<_T> col1;
            Vector3<_T> col2;
        };
        _T dataCM[16];
    };
};
typedef Matrix33<f32> Mat33;

// Column Major
// 0  4  8  12
// 1  5  9  13
// 2  6  10 14
// 3  7  11 15
template <typename _T>
struct Matrix44 {
    constexpr Matrix44() = default;

    union {
        struct {
            Vector4<_T> col0;
            Vector4<_T> col1;
            Vector4<_T> col2;
            Vector4<_T> col3;
        };
        _T dataCM[16];
    };
};
typedef Matrix44<f32> Mat4;

template <typename _T>
constexpr Vector2<_T>::Vector2(const _T _x, const _T _y)
: x(_x), y(_y) {}

template <typename _T>
constexpr Vector3<_T>::Vector3(const _T _x, const _T _y, const _T _z)
: x(_x), y(_y), z(_z) {}
template <typename _T>
constexpr Vector3<_T>::Vector3(const _T _x, const Vector2<_T>& _yz)
: x(_x), y(_yz.x), z(_yz.y) {}
template <typename _T>
constexpr Vector3<_T>::Vector3(const Vector2<_T>& _xy, const _T _z)
: x(_xy.x), y(_xy.y), z(_z) {}

template <typename _T>
constexpr Vector4<_T>::Vector4(const _T _x, const _T _y, const _T _z, const _T _w)
: x(_x), y(_y), z(_z), w(_w) {}
template <typename _T>
constexpr Vector4<_T>::Vector4(const _T _x, const Vector3<_T>& _yzw)
: x(_x), y(_yzw.x), z(_yzw.y), w(_yzw.z) {}
template <typename _T>
constexpr Vector4<_T>::Vector4(const Vector3<_T>& _xyz, const _T _w)
: x(_xyz.x), y(_xyz.y), z(_xyz.z), w(_w) {}
template <typename _T>
constexpr Vector4<_T>::Vector4(const Vector2<_T>& _xy, const Vector2<_T>& _zw)
: x(_xy.x), y(_xy.y), z(_zw.x), w(_zw.y) {}

namespace Math {
    
template <typename _T>
Vector2<_T> negate(const Vector2<_T>& v) {
    return Vector2<_T>(-v.x, -v.y);
}
template <typename _T>
Vector3<_T> negate(const Vector3<_T>& v) {
    return Vector3<_T>(-v.x, -v.y, -v.z);
}
template <typename _T>
Vector4<_T> negate(const Vector4<_T>& v) {
    return Vector4<_T>(-v.x, -v.y, -v.z, -v.w);
}
template <typename _T>
Vector2<_T> scale(const Vector2<_T>& v, const _T a) {
    return Vector2<_T>(v.x * a, v.y * a);
}
template <typename _T>
Vector3<_T> scale(const Vector3<_T>& v, const _T a) {
    return Vector3<_T>(v.x * a, v.y * a, v.z * a);
}
template <typename _T>
Vector4<_T> scale(const Vector4<_T>& v, const _T a) {
    return Vector4<_T>(v.x * a, v.y * a, v.z * a, v.w * a);
}
template <typename _T>
Vector2<_T> scale(const Vector2<_T>& v, const Vector2<_T>& s) {
    return Vector2<_T>(v.x * s.x, v.y * s.y);
}
template <typename _T>
Vector3<_T> scale(const Vector3<_T>& v, const Vector3<_T>& s) {
    return Vector3<_T>(v.x * s.x, v.y * s.y, v.z * s.z);
}
template <typename _T>
Vector4<_T> scale(const Vector4<_T>& v, const Vector4<_T>& s) {
    return Vector4<_T>(v.x * s.x, v.y * s.y, v.z * s.z, v.w * s.w);
}
template <typename _T>
Vector2<_T> invScale(const Vector2<_T>& v, const _T a) {
    return Vector2<_T>(v.x / a, v.y / a);
}
template <typename _T>
Vector3<_T> invScale(const Vector3<_T>& v, const _T a) {
    return Vector3<_T>(v.x / a, v.y / a, v.z / a);
}
template <typename _T>
Vector4<_T> invScale(const Vector4<_T>& v, const _T a) {
    return Vector4<_T>(v.x / a, v.y / a, v.z / a, v.w / a);
}
template <typename _T>
_T mag(const Vector2<_T>& v) {
    return Math::sqrt(v.x * v.x + v.y * v.y);
}
template <typename _T>
_T mag(const Vector3<_T>& v) {
    return Math::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
template <typename _T>
_T mag(const Vector4<_T>& v) {
    return Math::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}
template <typename _T>
_T magSq(const Vector2<_T>& v) {
    return v.x * v.x + v.y * v.y;
}
template <typename _T>
_T magSq(const Vector3<_T>& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
template <typename _T>
_T magSq(const Vector4<_T>& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}
template <typename _T>
Vector2<_T> normalize(const Vector2<_T>& v) {
    return invScale(v, mag(v));
}
template <typename _T>
Vector3<_T> normalize(const Vector3<_T>& v) {
    return invScale(v, mag(v));
}
template <typename _T>
Vector4<_T> normalize(const Vector4<_T>& v) {
    return invScale(v, mag(v));
}
template <typename _T>
bool normalizeSafe(Vector2<_T>& v) {
    const _T magnitude = mag(v);
    if (magnitude > Math::eps<_T>) {
        v.x /= magnitude;
        v.y /= magnitude;
        return true;
    }
    return false;
}
template <typename _T>
bool normalizeSafe(Vector3<_T>& v) {
    const _T magnitude = mag(v);
    if (magnitude > Math::eps<_T>) {
        v.x /= magnitude;
        v.y /= magnitude;
        v.z /= magnitude;
        return true;
    }
    return false;
}
template <typename _T>
bool normalizeSafe(Vector4<_T>& v) {
    const _T magnitude = mag(v);
    if (magnitude > Math::eps<_T>) {
        v.x /= magnitude;
        v.y /= magnitude;
        v.z /= magnitude;
        v.w /= magnitude;
        return true;
    }
    return false;
}
template <typename _T>
_T dot(const Vector2<_T>& a, const Vector2<_T>& b) {
     return a.x * b.x + a.y * b.y;
}
template <typename _T>
_T dot(const Vector3<_T>& a, const Vector3<_T>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
template <typename _T>
_T dot(const Vector4<_T>& a, const Vector4<_T>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
template <typename _T>
Vector2<_T> add(const Vector2<_T>& a, const Vector2<_T>& b) {
    return Vector2<_T>(a.x + b.x, a.y + b.y);
}
template <typename _T>
Vector3<_T> add(const Vector3<_T>& a, const Vector3<_T>& b) {
    return Vector3<_T>(a.x + b.x, a.y + b.y, a.z + b.z);
}
template <typename _T>
Vector4<_T> add(const Vector4<_T>& a, const Vector4<_T>& b) {
    return Vector4<_T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
template <typename _T>
Vector2<_T> subtract(const Vector2<_T>& a, const Vector2<_T>& b) {
    return Vector2<_T>(a.x - b.x, a.y - b.y);
}
template <typename _T>
Vector3<_T> subtract(const Vector3<_T>& a, const Vector3<_T>& b) {
    return Vector3<_T>(a.x - b.x, a.y - b.y, a.z - b.z);
}
template <typename _T>
Vector4<_T> subtract(const Vector4<_T>& a, const Vector4<_T>& b) {
    return Vector4<_T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
template <typename _T>
_T cross(const Vector2<_T>& a, const Vector2<_T>& b) {
    return a.x * b.y - a.y * b.x;
}
template <typename _T>
Vector3<_T> cross(const Vector3<_T>& a, const Vector3<_T>& b) {
    return Vector3<_T>(a.y * b.z - a.z * b.y,
                       a.z * b.x - a.x * b.z,
                       a.x * b.y - a.y * b.x);
}
template <typename _T>
Vector4<_T> cross(const Vector4<_T>& a, const Vector4<_T>& b) {
    return Vector4<_T>(a.y * b.z - a.z * b.y,
                       a.z * b.x - a.x * b.z,
                       a.x * b.y - a.y * b.x,
                       0.0);
}
template <typename _T>
Vector2<_T> max(const Vector2<_T>& a, const Vector2<_T>& b) {
    return Vector2<_T>(Math::max(a.x,b.x), Math::max(a.y, b.y));
}
template <typename _T>
Vector3<_T> max(const Vector3<_T>& a, const Vector3<_T>& b) {
    return Vector3<_T>(Math::max(a.x,b.x),
                       Math::max(a.y,b.y),
                       Math::max(a.z,b.z));
}
template <typename _T>
Vector4<_T> max(const Vector4<_T>& a, const Vector4<_T>& b) {
    return Vector4<_T>(Math::max(a.x,b.x),
                       Math::max(a.y,b.y),
                       Math::max(a.z,b.z),
                       Math::max(a.w,b.w));
}
template <typename _T>
Vector2<_T> min(const Vector2<_T>& a, const Vector2<_T>& b) {
    return Vector2<_T>(Math::min(a.x,b.x), Math::min(a.y, b.y));
}
template <typename _T>
Vector3<_T> min(const Vector3<_T>& a, const Vector3<_T>& b) {
    return Vector3<_T>(Math::min(a.x,b.x),
                       Math::min(a.y,b.y),
                       Math::min(a.z,b.z));
}
template <typename _T>
Vector4<_T> min(const Vector4<_T>& a, const Vector4<_T>& b) {
    return Vector4<_T>(Math::min(a.x,b.x),
                       Math::min(a.y,b.y),
                       Math::min(a.z,b.z),
                       Math::min(a.w,b.w));
}
template <typename _T>
bool isCloseAll(const Vector2<_T>& a, const Vector2<_T>& b, const _T d) {
    return Math::abs(a.x - b.x) < d
        && Math::abs(a.y - b.y) < d;
}
template <typename _T>
bool isCloseAll(const Vector3<_T>& a, const Vector3<_T>& b, const _T d) {
    return Math::abs(a.x - b.x) < d
        && Math::abs(a.y - b.y) < d
        && Math::abs(a.z - b.z) < d;
}
template <typename _T>
bool isCloseAll(const Vector4<_T>& a, const Vector4<_T>& b, const _T d) {
    return Math::abs(a.x - b.x) < d
        && Math::abs(a.y - b.y) < d
        && Math::abs(a.z - b.z) < d
        && Math::abs(a.w - b.w) < d;
}
    
template <typename _T>
Matrix33<_T> mult(const Matrix33<_T>& a, const Matrix33<_T>& b) {
    Vector3<_T> a_r0 ( a.dataCM[0], a.dataCM[3], a.dataCM[6] );
    Vector3<_T> a_r1 ( a.dataCM[1], a.dataCM[4], a.dataCM[7] );
    Vector3<_T> a_r2 ( a.dataCM[2], a.dataCM[5], a.dataCM[8] );
    
    Matrix33<_T> o;
    o.dataCM[0] = dot(a_r0, b.col0); o.dataCM[3] = dot(a_r0, b.col1); o.dataCM[6] = dot(a_r0, b.col2);
    o.dataCM[1] = dot(a_r1, b.col0); o.dataCM[4] = dot(a_r1, b.col1); o.dataCM[7] = dot(a_r1, b.col2);
    o.dataCM[2] = dot(a_r2, b.col0); o.dataCM[5] = dot(a_r2, b.col1); o.dataCM[8] = dot(a_r2, b.col2);
    
    return o;
}
template <typename _T>
Matrix33<_T> mult(const Matrix33<_T>& a, const _T s) {

    Matrix33<_T> o;
    o.col0 = scale(a.col0, s);
    o.col1 = scale(a.col1, s);
    o.col2 = scale(a.col2, s);
    return o;
}
template <typename _T>
Matrix44<_T> mult(const Matrix44<_T>& a, const Matrix44<_T>& b) {
    Vector4<_T> a_r0 ( a.dataCM[0], a.dataCM[4], a.dataCM[8], a.dataCM[12] );
    Vector4<_T> a_r1 ( a.dataCM[1], a.dataCM[5], a.dataCM[9], a.dataCM[13] );
    Vector4<_T> a_r2 ( a.dataCM[2], a.dataCM[6], a.dataCM[10], a.dataCM[14] );
    Vector4<_T> a_r3 ( a.dataCM[3], a.dataCM[7], a.dataCM[11], a.dataCM[15] );
    
    Matrix44<_T> o;
    o.dataCM[0] = dot(a_r0, b.col0); o.dataCM[4] = dot(a_r0, b.col1); o.dataCM[8] = dot(a_r0, b.col2); o.dataCM[12] = dot(a_r0, b.col3);
    o.dataCM[1] = dot(a_r1, b.col0); o.dataCM[5] = dot(a_r1, b.col1); o.dataCM[9] = dot(a_r1, b.col2); o.dataCM[13] = dot(a_r1, b.col3);
    o.dataCM[2] = dot(a_r2, b.col0); o.dataCM[6] = dot(a_r2, b.col1); o.dataCM[10] = dot(a_r2, b.col2); o.dataCM[14] = dot(a_r2, b.col3);
    o.dataCM[3] = dot(a_r3, b.col0); o.dataCM[7] = dot(a_r3, b.col1); o.dataCM[11] = dot(a_r3, b.col2); o.dataCM[15] = dot(a_r3, b.col3);
    
    return o;
}
template <typename _T>
Matrix44<_T> mult(const Matrix44<_T>& a, const _T s) {

    Matrix44<_T> o;
    o.col0 = scale(a.col0, s);
    o.col1 = scale(a.col1, s);
    o.col2 = scale(a.col2, s);
    o.col3 = scale(a.col3, s);
    return o;
}

}

#endif // __WASTELADNS_VEC_H__
