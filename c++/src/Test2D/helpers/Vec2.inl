#include "MathPrimitives.h"

Vec2::Vec2(f32 x, f32 y) {
    this->x = x;
    this->y = y;
}

Vec2::Vec2(const Vec2 &other) {
    x = other.x;
    y = other.y;
}

Vec2::~Vec2() {
}

// N->R Functions

f32 &Vec2::operator[](int i) {
    return coords[i];
}

const f32 &Vec2::operator[](int i) const {
    return coords[i];
}

// R2->R Functions

f32 Vec2::module() const {
    return sqrt(scalarDot(*this, *this));
}

// R2->R2 Functions

Vec2 Vec2::operator-() const {
    return Vec2(-x, -y);
}

Vec2 normalize(const Vec2 &v) {
    f32 mod = v.module();
    return Vec2(v.x / mod,
                v.y / mod);
}

bool normalizeSafe(Vec2& v) {
    f32 mod = v.module();
    if (mod > f32eps) {
        v.x = v.x / mod;
        v.y = v.y / mod;
        return true;
    } else {
        return false;
    }
}

// RxR2->R2 Functions

Vec2 operator*(const f32 &a, const Vec2 &v) {
    return Vec2(a * v.x,
                a * v.y);
}

Vec2 operator*(const Vec2 &v, const f32 &a) {
    return Vec2(v.x * a,
                v.y * a);
}

Vec2 operator/(const Vec2 &v, const f32 &a) {
    return Vec2(v.x / a,
                v.y / a);
}

// R2xR2->R Functions

f32 scalarDot(const Vec2 &a, const Vec2 &b) {
    return a.x*b.x + a.y*b.y;
}

// R2xR2->R2 Functions

Vec2 operator-(const Vec2 &a, const Vec2 &b) {
    return Vec2(a.x - b.x,
                a.y - b.y);
}

Vec2 operator+(const Vec2 &a, const Vec2 &b) {
    return Vec2(a.x + b.x,
                a.y + b.y);
}

void Vec2::operator-=(const Vec2 &other) {
    x -= other.x;
    y -= other.y;
}

void Vec2::operator+=(const Vec2 &other) {
    x += other.x;
    y += other.y;
}

bool Vec2::operator==(const Vec2 &other) const {
    return (fabs(x - other.x) < kVectorEqualityDelta)
        && (fabs(y - other.y) < kVectorEqualityDelta);
}
