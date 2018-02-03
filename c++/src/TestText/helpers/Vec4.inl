#include "MathPrimitives.h"


Vec4::Vec4(const Vec4 &other) {
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
}

Vec4::Vec4(const Vec3 &other, const f32 v3) {
    this->x = other.x;
    this->y = other.y;
    this->z = other.z;
    this->w = v3;
}

Vec4::Vec4(const Vec2 &other, const f32 z, const f32 w) {
    this->x = other.x;
    this->y = other.y;
    this->z = z;
    this->w = w;
}

Vec4::Vec4(f32 x, f32 y, f32 z, f32 w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

Vec4::~Vec4() {
}

const Vec2& Vec4::xy() const {
    return (*reinterpret_cast<const Vec2*>(this));
}

const Vec3& Vec4::xyz() const {
    return (*reinterpret_cast<const Vec3*>(this));
}

Vec2& Vec4::xy() {
    return (*reinterpret_cast<Vec2*>(this));
}

Vec3& Vec4::xyz() {
    return (*reinterpret_cast<Vec3*>(this));
}

// -> R3
Vec4 Vec4::operator-() const {
    return Vec4(-x, -y, -z, -w);
}

// N->R Functions
f32& Vec4::operator[](int i) {
    return coords[i];
}

const f32& Vec4::operator[](int i) const {
    return coords[i];
}

// R3->R Functions
f32 Vec4::module() const {
    return sqrt(scalarDot(*this, *this));
}

// R3->R3 Functions
Vec4 normalize(const Vec4 &v) {
    f32 mod = v.module();
    return Vec4(v.x / mod,
                      v.y / mod,
                      v.z / mod,
                      v.w / mod);
}

// RxR3->R3 Functions
Vec4 operator*(const f32 &a, const Vec4 &v) {
    return Vec4(a * v.x,
                      a * v.y,
                      a * v.z,
                      a * v.w);
}

Vec4 operator*(const Vec4 &v, const f32 &a) {
    return Vec4(v.x * a,
                      v.y * a,
                      v.z * a,
                      v.w * a);
}

Vec4 operator/(const Vec4 &v, const f32 &a) {
    return Vec4(v.x / a,
                      v.y / a,
                      v.z / a,
                      v.w / a);
}

// R3xR3->R Functions
f32 scalarDot(const Vec4 &a, const Vec4& b) {
    return scalarDot(*reinterpret_cast<const Vec3*>(&a), *reinterpret_cast<const Vec3*>(&b));
}

// R3xR3->R3 Functions
Vec4 operator-(const Vec4 &a, const Vec4 &b) {
    return Vec4(a.x - b.x,
                      a.y - b.y,
                      a.z - b.z,
                      a.w - b.w);
}

Vec4 operator+(const Vec4 &a, const Vec4 &b) {
    return Vec4(a.x + b.x,
                      a.y + b.y,
                      a.z + b.z,
                      a.w + b.w);
}

void Vec4::operator-=(const Vec4 &other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
}

void Vec4::operator+=(const Vec4 &other) {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
}

bool Vec4::operator==(const Vec4 &other) const {
    return (fabs(x - other.x) < kVectorEqualityDelta)
        && (fabs(y - other.y) < kVectorEqualityDelta)
        && (fabs(z - other.z) < kVectorEqualityDelta)
        && (fabs(w - other.w) < kVectorEqualityDelta);
}

Vec4 crossProduct(const Vec4 &a, const Vec4 &b) {
    return Vec4(a.y*b.z - a.z*b.y,
                a.z*b.x - a.x*b.z,
                a.x*b.y - a.y*b.x,
                0.0); // CHECK?
}
