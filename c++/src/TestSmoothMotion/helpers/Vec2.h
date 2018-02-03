#ifndef __Vec2_H__
#define __Vec2_H__

#include "TypeDefinitions.h"

class Vec2 {
public:
    Vec2(f32 x = 0, f32 y = 0);
    Vec2(const Vec2 &other);
    ~Vec2();
    
    // Getters
    __inline const f32& getX() const { return x; }
    __inline const f32& getY() const { return y; }
    
    __inline f32& getX() { return x; }
    __inline f32& getY() { return y; }
    
    __inline void setX(const f32& v) { x = v; }
    __inline void setY(const f32& v) { y = v; }
    
    // -> R2
    Vec2 operator-() const;
    // N -> R
    f32 &operator[](int i);
    const f32 &operator[](int i) const;
    // R2 -> R
    f32 module() const;
    // R2xR2 -> R2
    void operator-=(const Vec2 &other);
    void operator+=(const Vec2 &other);
    bool operator==(const Vec2 &other) const;
    
    union {
        struct {
            f32 x, y;
        };
        // allow array access too
        f32 coords[2];
    };
};

// R2 -> R2
Vec2 normalize(const Vec2 &v);
bool normalizeSafe(Vec2 &v);
// RxR2 -> R2
Vec2 operator*(const f32 &a, const Vec2 &v);
Vec2 operator*(const Vec2 &v, const f32 &a);
Vec2 operator/(const Vec2 &v, const f32 &a);
// R2xR2 -> R
f32 scalarDot(const Vec2 &a, const Vec2 &b);
// R2xR2 -> R2
Vec2 operator-(const Vec2 &a, const Vec2 &b);
Vec2 operator+(const Vec2 &a, const Vec2 &b);

/*******************
 * Implementation
 ******************/

#include "Vec2.inl"

#endif
