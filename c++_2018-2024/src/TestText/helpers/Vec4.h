#ifndef __Vec4_H__
#define __Vec4_H__

#include "TypeDefinitions.h"

class Vec2;
class Vec3;

class Vec4 {
public:
    Vec4(f32 x = 0.f, f32 y = 0.f, f32 z = 0.f, f32 w = 1.f);
    Vec4(const Vec4 &other);
    explicit Vec4(const Vec3 &other, const f32 v3);
    explicit Vec4(const Vec2 &other, const f32 v2, const f32 v3);
    ~Vec4();
    
    __inline const Vec2& xy() const;
    __inline const Vec3& xyz() const;
    __inline Vec2& xy();
    __inline Vec3& xyz();
    
    // -> R3
    Vec4 operator-() const;
    // N -> R
    f32 &operator[](int i);
    const f32 &operator[](int i) const;
    // R3 -> R
    f32 module() const;
    // R3xR3 -> R3
    void operator-=(const Vec4 &other);
    void operator+=(const Vec4 &other);
    bool operator==(const Vec4 &other) const;
    
    union {
        struct {
            float x, y, z, w;
        };
        // allow array access too
        float coords[4];
    };
};

// R3 -> R3
Vec4 normalize(const Vec4 &v);
// RxR3 -> R3
Vec4 operator*(const f32 &a, const Vec4 &v);
Vec4 operator*(const Vec4 &v, const f32 &a);
Vec4 operator/(const Vec4 &v, const f32 &a);
// R3xR3 -> R
f32 scalarDot(const Vec4 &a, const Vec4 &b);
// R3xR3 -> R3
Vec4 operator-(const Vec4 &a, const Vec4 &b);
Vec4 operator+(const Vec4 &a, const Vec4 &b);
Vec4 crossProduct(const Vec4 &a, const Vec4 &b);

/*******************
 * Implementation
 ******************/

#include "Vec4.inl"

#endif
