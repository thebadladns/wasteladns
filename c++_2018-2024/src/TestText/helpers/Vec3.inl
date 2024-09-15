#include "MathPrimitives.h"

Vec3::Vec3(f32 x, f32 y, f32 z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

Vec3::Vec3(const Vec3 &other) {
    x = other.x;
    y = other.y;
    z = other.z;
}

Vec3::Vec3(const Vec2 &other, const f32 v2) {
    this->x = other.x;
    this->y = other.y;
    this->z = v2;
}

Vec3::~Vec3() {
}

Vec3 Vec3::operator-() const {
    return Vec3(-x, -y, -z);
}

// N->R Functions

f32 &Vec3::operator[](int i) {
    return coords[i];
}

const f32 &Vec3::operator[](int i) const {
    return coords[i];
}

// R3->R Functions

f32 Vec3::module() const {
    return sqrt(scalarDot(*this, *this));
}

f32 Vec3::moduleSq() const {
    return scalarDot(*this, *this);
}

// R3->R3 Functions

Vec3 normalize(const Vec3 &v) {
    f32 mod = v.module();
    return Vec3(v.x / mod,
                v.y / mod,
                v.z / mod);
}

// RxR3->R3 Functions

Vec3 operator*(const f32 &a, const Vec3 &v) {
    return Vec3(a * v.x,
                a * v.y,
                a * v.z);
}

Vec3 operator*(const Vec3 &v, const f32 &a) {
    return Vec3(v.x * a,
                v.y * a,
                v.z * a);
}

Vec3 operator/(const Vec3 &v, const f32 &a) {
    return Vec3(v.x / a,
                v.y / a,
                v.z / a);
}

// R3xR3->R Functions

f32 scalarDot(const Vec3 &a, const Vec3 &b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

// R3xR3->R3 Functions

Vec3 operator-(const Vec3 &a, const Vec3 &b) {
    return Vec3(a.x - b.x,
                a.y - b.y,
                a.z - b.z);
}

Vec3 operator+(const Vec3 &a, const Vec3 &b) {
    return Vec3(a.x + b.x,
                a.y + b.y,
                a.z + b.z);
}

void Vec3::operator-=(const Vec3 &other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
}

void Vec3::operator+=(const Vec3 &other) {
    x += other.x;
    y += other.y;
    z += other.z;
}

bool Vec3::operator==(const Vec3 &other) const {
    return (fabs(x - other.x) < kVectorEqualityDelta)
        && (fabs(y - other.y) < kVectorEqualityDelta)
        && (fabs(z - other.z) < kVectorEqualityDelta);
}

Vec3 crossProduct(const Vec3 &a, const Vec3 &b) {
    return Vec3(a.y*b.z - a.z*b.y,
                a.z*b.x - a.x*b.z,
                a.x*b.y - a.y*b.x);
}
