#ifndef __Vec3_H__
#define __Vec3_H__

#include "TypeDefinitions.h"

class Vec2;


class Vec3 {
public:
    Vec3(f32 x = 0, f32 y = 0, f32 z = 0);
    Vec3(const Vec3 &other);
    explicit Vec3(const Vec2 &other, f32 z);
    explicit Vec3(f32 x, const Vec2 &other);
    ~Vec3();
    
    // -> R3
    Vec3 operator-() const;
    // N -> R
    f32 &operator[](int i);
    const f32 &operator[](int i) const;
    // R3 -> R
    f32 module() const;
    f32 moduleSq() const;
    // R3xR3 -> R3
    void operator-=(const Vec3 &other);
    void operator+=(const Vec3 &other);
    bool operator==(const Vec3 &other) const;

    union {
        struct {
            f32 x, y, z;
        };
        // allow array access too
        f32 coords[3];
    };
};

// R3 -> R3
Vec3 normalize(const Vec3 &v);
// RxR3 -> R3
Vec3 operator*(const f32 &a, const Vec3 &v);
Vec3 operator*(const Vec3 &v, const f32 &a);
Vec3 operator/(const Vec3 &v, const f32 &a);
// R3xR3 -> R
f32 scalarDot(const Vec3 &a, const Vec3 &b);
// R3xR3 -> R3
Vec3 operator-(const Vec3 &a, const Vec3 &b);
Vec3 operator+(const Vec3 &a, const Vec3 &b);
Vec3 crossProduct(const Vec3 &a, const Vec3 &b);

/*******************
 * Implementation
 ******************/

#include "Vec3.inl"

#endif
