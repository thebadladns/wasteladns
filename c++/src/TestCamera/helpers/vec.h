#ifndef __WASTELADNS_VEC_H__
#define __WASTELADNS_VEC_H__

#ifndef __WASTELADNS_TYPES_H__
#include "types.h"
#endif

#ifndef __WASTELADNS_TEMPLATE_DEFINES_H__
#include "template_defines.h"
#endif

// g=generic, s=specialized
#define VECTOR_TEMPLATES(g,s,...) \
    g(RT_PT2(mag, __VA_ARGS__)) \
    g(RT_PT3(mag, __VA_ARGS__)) \
    g(RT_PT4(mag, __VA_ARGS__)) \
    g(RT2_PT2(normalize, __VA_ARGS__)) \
    g(RT3_PT3(normalize, __VA_ARGS__)) \
    g(RT4_PT4(normalize, __VA_ARGS__)) \
    g(RB_PT2(normalizeSafe, __VA_ARGS__)) \
    g(RB_PT3(normalizeSafe, __VA_ARGS__)) \
    g(RB_PT4(normalizeSafe, __VA_ARGS__)) \
    g(RT2_PT2(negate, __VA_ARGS__)) \
    g(RT3_PT3(negate, __VA_ARGS__)) \
    g(RT4_PT4(negate, __VA_ARGS__)) \
    g(RT2_PT2T(scale, __VA_ARGS__)) \
    g(RT3_PT3T(scale, __VA_ARGS__)) \
    g(RT4_PT4T(scale, __VA_ARGS__)) \
    g(RT2_PT2T(invScale, __VA_ARGS__)) \
    g(RT3_PT3T(invScale, __VA_ARGS__)) \
    g(RT4_PT4T(invScale, __VA_ARGS__)) \
    g(RT_PT2T2(dot, __VA_ARGS__)) \
    g(RT_PT3T3(dot, __VA_ARGS__)) \
    g(RT_PT4T4(dot, __VA_ARGS__)) \
    g(RT2_PT2T2(add, __VA_ARGS__)) \
    g(RT3_PT3T3(add, __VA_ARGS__)) \
    g(RT4_PT4T4(add, __VA_ARGS__)) \
    g(RT2_PT2T2(subtract, __VA_ARGS__)) \
    g(RT3_PT3T3(subtract, __VA_ARGS__)) \
    g(RT4_PT4T4(subtract, __VA_ARGS__)) \
    g(RT2_PT2T2(cross, __VA_ARGS__)) \
    g(RT3_PT3T3(cross, __VA_ARGS__)) \
    g(RT4_PT4T4(cross, __VA_ARGS__)) \
    g(RT2_PT2T2(max, __VA_ARGS__)) \
    g(RT3_PT3T3(max, __VA_ARGS__)) \
    g(RT4_PT4T4(max, __VA_ARGS__)) \
    g(RT2_PT2T2(min, __VA_ARGS__)) \
    g(RT3_PT3T3(min, __VA_ARGS__)) \
    g(RT4_PT4T4(min, __VA_ARGS__))

#define VEC2_FORMAT(format) "(" format ", " format ")"
#define VEC3_FORMAT(format) "(" format ", " format ", " format ")"
#define VEC4_FORMAT(format) "(" format ", " format ", " format ", " format ")"
#define VEC2_PARAMS(v) v.x, v.y
#define VEC3_PARAMS(v) v.x, v.y, v.z
#define VEC4_PARAMS(v) v.x, v.y, v.z, v.w

template <typename _T>
struct Vector2 {
    Vector2();
    Vector2(const _T x, const _T y);
    Vector2(const Vector2<_T>& v);
    
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
    Vector3();
    Vector3(const _T x, const _T y, const _T z);
    Vector3(const _T x, const Vector2<_T>& yz);
    Vector3(const Vector2<_T>& xy, const _T z);
    Vector3(const Vector3<_T>& v);
    
    union {
        struct {
            Vector2<_T> xy;
        };
        struct {
            _T _padding;
            Vector2<_T> yz;
        };
        struct {
            _T x, y, z;
        };
        _T coords[3];
    };
};
typedef Vector3<f32> Vec3;

template <typename _T>
struct Vector4 {
    Vector4();
    Vector4(const _T x, const _T y, const _T z, const _T w);
    Vector4(const _T x, const Vector3<_T>& yzw);
    Vector4(const Vector3<_T>& xyz, const _T w);
    Vector4(const Vector2<_T>& xy, const Vector2<_T>& yz);
    Vector4(const Vector4<_T>& v);
    
    union {
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
        struct {
            _T x, y, z, w;
        };
        _T coords[4];
    };
};
typedef Vector4<f32> Vec4;

namespace Vector {
    DEFINE_TEMPLATES(VECTOR_TEMPLATES)
}
namespace Vec = Vector;

// g=generic, s=specialized
#define MATRIX44_TEMPLATES(g,s,...) \
    g(RB_PT44(inverse, __VA_ARGS__))

// Column Major
// 0  4  8  12
// 1  5  9  13
// 2  6  10 14
// 3  7  11 15
template <typename _T>
struct Matrix44 {
    Matrix44();
    Matrix44(const Matrix44<_T>& m);
    
    union {
        struct {
            Vector4<_T> fullcol0;
            Vector4<_T> fullcol1;
            Vector4<_T> fullcol2;
            Vector4<_T> fullcol3;
        };
        struct {
            Vector3<_T> col0; _T col0_w;
            Vector3<_T> col1; _T col1_w;
            Vector3<_T> col2; _T col2_w;
            Vector3<_T> col3; _T col3_w;
        };
        _T dataCM[16];
    };
};
typedef Matrix44<f32> Mat4;

#endif // #define __WASTELADNS_VEC_H__

#ifdef __WASTELADNS_VEC_IMPL__
#undef __WASTELADNS_VEC_IMPL__

#ifndef __WASTELADNS_MATH_H__
#include "math.h"
#endif

template <typename _T>
Vector2<_T>::Vector2() {}
template <typename _T>
Vector2<_T>::Vector2(const _T x, const _T y) {
    this->x = x;
    this->y = y;
}
template <typename _T>
Vector2<_T>::Vector2(const Vector2<_T>& v) {
    x = v.x;
    y = v.y;
}

template <typename _T>
Vector3<_T>::Vector3() {}
template <typename _T>
Vector3<_T>::Vector3(const _T x, const _T y, const _T z) {
    this->x = x;
    this->y = y;
    this->z = z;
}
template <typename _T>
Vector3<_T>::Vector3(const _T x, const Vector2<_T>& yz) {
    this->x = x;
    this->y = yz.x;
    this->z = yz.y;
}
template <typename _T>
Vector3<_T>::Vector3(const Vector2<_T>& xy, const _T z) {
    this->x = xy.x;
    this->y = xy.y;
    this->z = z;
}
template <typename _T>
Vector3<_T>::Vector3(const Vector3<_T>& v) {
    x = v.x;
    y = v.y;
    z = v.z;
}

template <typename _T>
Vector4<_T>::Vector4() {}
template <typename _T>
Vector4<_T>::Vector4(const _T x, const _T y, const _T z, const _T w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}
template <typename _T>
Vector4<_T>::Vector4(const _T x, const Vector3<_T>& yzw) {
    this->x = x;
    this->y = yzw.x;
    this->z = yzw.y;
    this->w = yzw.z;
}
template <typename _T>
Vector4<_T>::Vector4(const Vector3<_T>& xyz, const _T w) {
    this->x = xyz.z;
    this->y = xyz.y;
    this->z = xyz.z;
    this->w = w;
}
template <typename _T>
Vector4<_T>::Vector4(const Vector2<_T>& xy, const Vector2<_T>& yz) {
    x = xy.x;
    y = xy.y;
    z = yz.x;
    w = yz.y;
}
template <typename _T>
Vector4<_T>::Vector4(const Vector4<_T>& v) {
    x = v.x;
    y = v.y;
    z = v.z;
    w = v.w;
}

template <typename _T>
Matrix44<_T>::Matrix44() {}
template <typename _T>
Matrix44<_T>::Matrix44(const Matrix44<_T>& m) {
    memcpy(dataCM, m.dataCM, sizeof(_T) * 16);
}

namespace Vector {

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
Vector2<_T> cross(const Vector2<_T>& a, const Vector2<_T>& b) {
    return Vector2<_T>(a.x * b.y, a.y - b.x);
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

INSTANTIATE_TEMPLATES(VECTOR_TEMPLATES,f32);
INSTANTIATE_TEMPLATES(VECTOR_TEMPLATES,f64);

template <typename _T>
bool inverse(Matrix44<_T>& m) {
    
    Matrix44<_T> prev = m;
    memset(m.dataCM, 0.f, sizeof(_T) * 16);
    
    _T* prev_m = prev.dataCM;
    _T* next_m = m.dataCM;
    
    next_m[0] = prev_m[5]    * prev_m[10]   * prev_m[15] -
    prev_m[5]    * prev_m[11]   * prev_m[14] -
    prev_m[9]    * prev_m[6]    * prev_m[15] +
    prev_m[9]    * prev_m[7]    * prev_m[14] +
    prev_m[13]   * prev_m[6]    * prev_m[11] -
    prev_m[13]   * prev_m[7]    * prev_m[10];
    
    next_m[4] = -prev_m[4]  * prev_m[10]    * prev_m[15] +
    prev_m[4]   * prev_m[11]    * prev_m[14] +
    prev_m[8]   * prev_m[6]     * prev_m[15] -
    prev_m[8]   * prev_m[7]     * prev_m[14] -
    prev_m[12]  * prev_m[6]     * prev_m[11] +
    prev_m[12]  * prev_m[7]     * prev_m[10];
    
    next_m[8] = prev_m[4]   * prev_m[9]     * prev_m[15] -
    prev_m[4]   * prev_m[11]    * prev_m[13] -
    prev_m[8]   * prev_m[5]     * prev_m[15] +
    prev_m[8]   * prev_m[7]     * prev_m[13] +
    prev_m[12]  * prev_m[5]     * prev_m[11] -
    prev_m[12]  * prev_m[7]     * prev_m[9];
    
    next_m[12] = -prev_m[4] * prev_m[9]     * prev_m[14] +
    prev_m[4]  * prev_m[10]    * prev_m[13] +
    prev_m[8]  * prev_m[5]     * prev_m[14] -
    prev_m[8]  * prev_m[6]     * prev_m[13] -
    prev_m[12] * prev_m[5]     * prev_m[10] +
    prev_m[12] * prev_m[6]     * prev_m[9];
    
    next_m[1] = -prev_m[1]  * prev_m[10]    * prev_m[15] +
    prev_m[1]   * prev_m[11]    * prev_m[14] +
    prev_m[9]   * prev_m[2]     * prev_m[15] -
    prev_m[9]   * prev_m[3]     * prev_m[14] -
    prev_m[13]  * prev_m[2]     * prev_m[11] +
    prev_m[13]  * prev_m[3]     * prev_m[10];
    
    next_m[5] = prev_m[0]   * prev_m[10]    * prev_m[15] -
    prev_m[0]   * prev_m[11]    * prev_m[14] -
    prev_m[8]   * prev_m[2]     * prev_m[15] +
    prev_m[8]   * prev_m[3]     * prev_m[14] +
    prev_m[12]  * prev_m[2]     * prev_m[11] -
    prev_m[12]  * prev_m[3]     * prev_m[10];
    
    next_m[9] = -prev_m[0]  * prev_m[9]     * prev_m[15] +
    prev_m[0]   * prev_m[11]    * prev_m[13] +
    prev_m[8]   * prev_m[1]     * prev_m[15] -
    prev_m[8]   * prev_m[3]     * prev_m[13] -
    prev_m[12]  * prev_m[1]     * prev_m[11] +
    prev_m[12]  * prev_m[3]     * prev_m[9];
    
    next_m[13] = prev_m[0]  * prev_m[9]     * prev_m[14] -
    prev_m[0]  * prev_m[10]    * prev_m[13] -
    prev_m[8]  * prev_m[1]     * prev_m[14] +
    prev_m[8]  * prev_m[2]     * prev_m[13] +
    prev_m[12] * prev_m[1]     * prev_m[10] -
    prev_m[12] * prev_m[2]     * prev_m[9];
    
    next_m[2] = prev_m[1]   * prev_m[6]     * prev_m[15] -
    prev_m[1]   * prev_m[7]     * prev_m[14] -
    prev_m[5]   * prev_m[2]     * prev_m[15] +
    prev_m[5]   * prev_m[3]     * prev_m[14] +
    prev_m[13]  * prev_m[2]     * prev_m[7] -
    prev_m[13]  * prev_m[3]     * prev_m[6];
    
    next_m[6] = -prev_m[0]  * prev_m[6]     * prev_m[15] +
    prev_m[0]   * prev_m[7]     * prev_m[14] +
    prev_m[4]   * prev_m[2]     * prev_m[15] -
    prev_m[4]   * prev_m[3]     * prev_m[14] -
    prev_m[12]  * prev_m[2]     * prev_m[7] +
    prev_m[12]  * prev_m[3]     * prev_m[6];
    
    next_m[10] = prev_m[0]  * prev_m[5]     * prev_m[15] -
    prev_m[0]  * prev_m[7]     * prev_m[13] -
    prev_m[4]  * prev_m[1]     * prev_m[15] +
    prev_m[4]  * prev_m[3]     * prev_m[13] +
    prev_m[12] * prev_m[1]     * prev_m[7] -
    prev_m[12] * prev_m[3]     * prev_m[5];
    
    next_m[14] = -prev_m[0] * prev_m[5]     * prev_m[14] +
    prev_m[0]  * prev_m[6]     * prev_m[13] +
    prev_m[4]  * prev_m[1]     * prev_m[14] -
    prev_m[4]  * prev_m[2]     * prev_m[13] -
    prev_m[12] * prev_m[1]     * prev_m[6] +
    prev_m[12] * prev_m[2]     * prev_m[5];
    
    next_m[3] = -prev_m[1]  * prev_m[6]     * prev_m[11] +
    prev_m[1]   * prev_m[7]     * prev_m[10] +
    prev_m[5]   * prev_m[2]     * prev_m[11] -
    prev_m[5]   * prev_m[3]     * prev_m[10] -
    prev_m[9]   * prev_m[2]     * prev_m[7] +
    prev_m[9]   * prev_m[3]     * prev_m[6];
    
    next_m[7] = prev_m[0]   * prev_m[6]     * prev_m[11] -
    prev_m[0]   * prev_m[7]     * prev_m[10] -
    prev_m[4]   * prev_m[2]     * prev_m[11] +
    prev_m[4]   * prev_m[3]     * prev_m[10] +
    prev_m[8]   * prev_m[2]     * prev_m[7] -
    prev_m[8]   * prev_m[3]     * prev_m[6];
    
    next_m[11] = -prev_m[0] * prev_m[5]     * prev_m[11] +
    prev_m[0]  * prev_m[7]     * prev_m[9] +
    prev_m[4]  * prev_m[1]     * prev_m[11] -
    prev_m[4]  * prev_m[3]     * prev_m[9] -
    prev_m[8]  * prev_m[1]     * prev_m[7] +
    prev_m[8]  * prev_m[3]     * prev_m[5];
    
    next_m[15] = prev_m[0]  * prev_m[5]     * prev_m[10] -
    prev_m[0]  * prev_m[6]     * prev_m[9] -
    prev_m[4]  * prev_m[1]     * prev_m[10] +
    prev_m[4]  * prev_m[2]     * prev_m[9] +
    prev_m[8]  * prev_m[1]     * prev_m[6] -
    prev_m[8]  * prev_m[2]     * prev_m[5];
    
    _T det = prev_m[0] * next_m[0] + prev_m[1] * next_m[4] + prev_m[2] * next_m[8] + prev_m[3] * next_m[12];
    if (det == 0) {
        return false;
    }
    
    _T inv_det = 1.0f / det;
    for (int i = 0; i < 16; i++)
        next_m[i] = next_m[i] * inv_det;
    
    return true;
}

INSTANTIATE_TEMPLATES(MATRIX44_TEMPLATES,f32);
INSTANTIATE_TEMPLATES(MATRIX44_TEMPLATES,f64);
}

template struct Vector2<f32>;
template struct Vector3<f32>;
template struct Vector4<f32>;
template struct Vector2<f64>;
template struct Vector3<f64>;
template struct Vector4<f64>;

template struct Matrix44<f32>;
template struct Matrix44<f64>;

#endif // __WASTELADNS_VEC_IMPL__
