#ifndef __WASTELADNS_VEC_H__
#define __WASTELADNS_VEC_H__

#ifndef __WASTELADNS_TYPES_H__
#include "Types.h"
#endif

template <typename _T>
struct Vector2 {
    
    Vector2();
    Vector2(const _T x, const _T y);
    Vector2(const Vector2<_T>& v);
    
    union {
        struct {
            _T x, y;
        };
        // allow array access too
        _T coords[2];
    };
};
#ifndef __WASTELADNS_VEC_IMPL__
extern template struct Vector2<f32>;
extern template struct Vector2<f64>;
#endif
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
            _T x, y, z;
        };
        // allow array access too
        _T coords[3];
    };
};
#ifndef __WASTELADNS_VEC_IMPL__
extern template struct Vector3<f32>;
extern template struct Vector3<f64>;
#endif
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
            _T x, y, z, w;
        };
        // allow array access too
        _T coords[4];
    };
};
#ifndef __WASTELADNS_VEC_IMPL__
extern template struct Vector4<f32>;
extern template struct Vector4<f64>;
#endif
typedef Vector4<f32> Vec4;

template <typename _T>
struct Vector {
    static Vector2<_T> scale(const Vector2<_T>& v, const _T a);
    static Vector3<_T> scale(const Vector3<_T>& v, const _T a);
    static Vector4<_T> scale(const Vector4<_T>& v, const _T a);
    static Vector2<_T> invScale(const Vector2<_T>& v, const _T a);
    static Vector3<_T> invScale(const Vector3<_T>& v, const _T a);
    static Vector4<_T> invScale(const Vector4<_T>& v, const _T a);
    static _T dot(const Vector2<_T>& a, const Vector2<_T>& b);
    static _T dot(const Vector3<_T>& a, const Vector3<_T>& b);
    static _T dot(const Vector4<_T>& a, const Vector4<_T>& b);
    static Vector2<_T> add(const Vector2<_T>& a, const Vector2<_T>& b);
    static Vector3<_T> add(const Vector3<_T>& a, const Vector3<_T>& b);
    static Vector4<_T> add(const Vector4<_T>& a, const Vector4<_T>& b);
    static Vector2<_T> subtract(const Vector2<_T>& a, const Vector2<_T>& b);
    static Vector3<_T> subtract(const Vector3<_T>& a, const Vector3<_T>& b);
    static Vector4<_T> subtract(const Vector4<_T>& a, const Vector4<_T>& b);
    static Vector2<_T> negate(const Vector2<_T>& v);
    static Vector3<_T> negate(const Vector3<_T>& v);
    static Vector4<_T> negate(const Vector4<_T>& v);
    static _T mag(const Vector2<_T>& v);
    static _T mag(const Vector3<_T>& v);
    static _T mag(const Vector4<_T>& v);
    static Vector2<_T> normalize(const Vector2<_T>& v);
    static Vector3<_T> normalize(const Vector3<_T>& v);
    static Vector4<_T> normalize(const Vector4<_T>& v);
    static bool normalizeSafe(Vector2<_T>& v);
    static bool normalizeSafe(Vector3<_T>& v);
    static bool normalizeSafe(Vector4<_T>& v);
    static Vector2<_T> cross(const Vector2<_T>& a, const Vector2<_T>& b);
    static Vector3<_T> cross(const Vector3<_T>& a, const Vector3<_T>& b);
    static Vector4<_T> cross(const Vector4<_T>& a, const Vector4<_T>& b);
};
#ifndef __WASTELADNS_VEC_IMPL__
extern template struct Vector<f32>;
extern template struct Vector<f64>;
#endif
typedef Vector<f32> Vec;

#endif // __WASTELADNS_VEC_H__

#ifdef __WASTELADNS_VEC_IMPL__
#undef __WASTELADNS_VEC_IMPL__

#ifndef __WASTELADNS_MATH_H__
#include "Math.h"
#endif

template<typename _T>
Vector2<_T>::Vector2() {}
template<typename _T>
Vector2<_T>::Vector2(const _T x, const _T y) {
    this->x = x;
    this->y = y;
}
template<typename _T>
Vector2<_T>::Vector2(const Vector2<_T>& v) {
    x = v.x;
    y = v.y;
}

template<typename _T>
Vector3<_T>::Vector3() {}
template<typename _T>
Vector3<_T>::Vector3(const _T x, const _T y, const _T z) {
    this->x = x;
    this->y = y;
    this->z = z;
}
template<typename _T>
Vector3<_T>::Vector3(const _T x, const Vector2<_T>& yz) {
    this->x = x;
    this->y = yz.x;
    this->z = yz.y;
}
template<typename _T>
Vector3<_T>::Vector3(const Vector2<_T>& xy, const _T z) {
    this->x = xy.x;
    this->y = xy.y;
    this->z = z;
}
template<typename _T>
Vector3<_T>::Vector3(const Vector3<_T>& v) {
    x = v.x;
    y = v.y;
    z = v.z;
}

template<typename _T>
Vector4<_T>::Vector4() {}
template<typename _T>
Vector4<_T>::Vector4(const _T x, const _T y, const _T z, const _T w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}
template<typename _T>
Vector4<_T>::Vector4(const _T x, const Vector3<_T>& yzw) {
    this->x = x;
    this->y = yzw.x;
    this->z = yzw.y;
    this->w = yzw.z;
}
template<typename _T>
Vector4<_T>::Vector4(const Vector3<_T>& xyz, const _T w) {
    this->x = xyz.z;
    this->y = xyz.y;
    this->z = xyz.z;
    this->w = w;
}
template<typename _T>
Vector4<_T>::Vector4(const Vector2<_T>& xy, const Vector2<_T>& yz) {
    x = xy.x;
    y = xy.y;
    z = yz.x;
    w = yz.y;
}
template<typename _T>
Vector4<_T>::Vector4(const Vector4<_T>& v) {
    x = v.x;
    y = v.y;
    z = v.z;
    w = v.w;
}

template<typename _T>
Vector2<_T> Vector<_T>::scale(const Vector2<_T>& v, const _T a) {
    return Vector2<_T>(v.x * a, v.y * a);
}
template<typename _T>
Vector3<_T> Vector<_T>::scale(const Vector3<_T>& v, const _T a) {
    return Vector3<_T>(v.x * a, v.y * a, v.z * a);
}
template<typename _T>
Vector4<_T> Vector<_T>::scale(const Vector4<_T>& v, const _T a) {
    return Vector4<_T>(v.x * a, v.y * a, v.z * a, v.w * a);
}
template<typename _T>
Vector2<_T> Vector<_T>::invScale(const Vector2<_T>& v, const _T a) {
    return Vector2<_T>(v.x / a, v.y / a);
}
template<typename _T>
Vector3<_T> Vector<_T>::invScale(const Vector3<_T>& v, const _T a) {
    return Vector3<_T>(v.x / a, v.y / a, v.z / a);
}
template<typename _T>
Vector4<_T> Vector<_T>::invScale(const Vector4<_T>& v, const _T a) {
    return Vector4<_T>(v.x / a, v.y / a, v.z / a, v.w / a);
}
template<typename _T>
_T Vector<_T>::dot(const Vector2<_T>& a, const Vector2<_T>& b) {
     return a.x * b.x + a.y * b.y;
}
template<typename _T>
_T Vector<_T>::dot(const Vector3<_T>& a, const Vector3<_T>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
template<typename _T>
_T Vector<_T>::dot(const Vector4<_T>& a, const Vector4<_T>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
template<typename _T>
Vector2<_T> Vector<_T>::add(const Vector2<_T>& a, const Vector2<_T>& b) {
    return Vector2<_T>(a.x + b.x, a.y + b.y);
}
template<typename _T>
Vector3<_T> Vector<_T>::add(const Vector3<_T>& a, const Vector3<_T>& b) {
    return Vector3<_T>(a.x + b.x, a.y + b.y, a.z + b.z);
}
template<typename _T>
Vector4<_T> Vector<_T>::add(const Vector4<_T>& a, const Vector4<_T>& b) {
    return Vector4<_T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
template<typename _T>
Vector2<_T> Vector<_T>::subtract(const Vector2<_T>& a, const Vector2<_T>& b) {
    return Vector2<_T>(a.x - b.x, a.y - b.y);
}
template<typename _T>
Vector3<_T> Vector<_T>::subtract(const Vector3<_T>& a, const Vector3<_T>& b) {
    return Vector3<_T>(a.x - b.x, a.y - b.y, a.z - b.z);
}
template<typename _T>
Vector4<_T> Vector<_T>::subtract(const Vector4<_T>& a, const Vector4<_T>& b) {
    return Vector4<_T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
template<typename _T>
Vector2<_T> Vector<_T>::negate(const Vector2<_T>& v) {
    return Vector2<_T>(-v.x, -v.y);
}
template<typename _T>
Vector3<_T> Vector<_T>::negate(const Vector3<_T>& v) {
    return Vector3<_T>(-v.x, -v.y, -v.z);
}
template<typename _T>
Vector4<_T> Vector<_T>::negate(const Vector4<_T>& v) {
    return Vector4<_T>(-v.x, -v.y, -v.z, -v.w);
}
template<typename _T>
_T Vector<_T>::mag(const Vector2<_T>& v) {
    return Math<_T>::sqrt(v.x * v.x + v.y * v.y);
}
template<typename _T>
_T Vector<_T>::mag(const Vector3<_T>& v) {
    return Math<_T>::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
template<typename _T>
_T Vector<_T>::mag(const Vector4<_T>& v) {
    return Math<_T>::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}
template<typename _T>
Vector2<_T> Vector<_T>::normalize(const Vector2<_T>& v) {
    return invScale(v, mag(v));
}
template<typename _T>
Vector3<_T> Vector<_T>::normalize(const Vector3<_T>& v) {
    return invScale(v, mag(v));
}
template<typename _T>
Vector4<_T> Vector<_T>::normalize(const Vector4<_T>& v) {
    return invScale(v, mag(v));
}
template<typename _T>
bool Vector<_T>::normalizeSafe(Vector2<_T>& v) {
    const _T magnitude = mag(v);
    if (magnitude > Math<_T>::eps) {
        v.x /= magnitude;
        v.y /= magnitude;
        return true;
    }
    return true;
}
template<typename _T>
bool Vector<_T>::normalizeSafe(Vector3<_T>& v) {
    const _T magnitude = mag(v);
    if (magnitude > Math<_T>::eps) {
        v.x /= magnitude;
        v.y /= magnitude;
        v.z /= magnitude;
        return true;
    }
    return true;
}
template<typename _T>
bool Vector<_T>::normalizeSafe(Vector4<_T>& v) {
    const _T magnitude = mag(v);
    if (magnitude > Math<_T>::eps) {
        v.x /= magnitude;
        v.y /= magnitude;
        v.z /= magnitude;
        v.w /= magnitude;
        return true;
    }
    return true;
}
template<typename _T>
Vector2<_T> Vector<_T>::cross(const Vector2<_T>& a, const Vector2<_T>& b) {
    return Vector2<_T>(a.x * b.y, a.y - b.x);
}
template<typename _T>
Vector3<_T> Vector<_T>::cross(const Vector3<_T>& a, const Vector3<_T>& b) {
    return Vector3<_T>(a.y * b.z - a.z * b.y,
                       a.z * b.x - a.x * b.z,
                       a.x * b.y - a.y * b.x);
}
template<typename _T>
Vector4<_T> Vector<_T>::cross(const Vector4<_T>& a, const Vector4<_T>& b) {
    return Vector4<_T>(a.y * b.z - a.z * b.y,
                       a.z * b.x - a.x * b.z,
                       a.x * b.y - a.y * b.x,
                       0.0);
}

template struct Vector2<f32>;
template struct Vector3<f32>;
template struct Vector4<f32>;
template struct Vector2<f64>;
template struct Vector3<f64>;
template struct Vector4<f64>;
template struct Vector<f32>;
template struct Vector<f64>;

#endif // __WASTELADNS_VEC_IMPL__
