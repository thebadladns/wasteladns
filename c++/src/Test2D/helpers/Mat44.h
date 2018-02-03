#ifndef __Mat44f_H__
#define __Mat44f_H__

#include "TypeDefinitions.h"

class Vec4;


/**
 * Layout is column major
 *    i0 i1 i2 i3
 * j0 0  4  8  12
 * j1 1  5  9  13
 * j2 2  6  10 14
 * j3 3  7  11 15
 */

class Mat44 {
public:
    enum { kNumRows = 4, kNumCols = 4, kNumElems = kNumRows * kNumCols };
    enum eInitializerIdentity { kMatIdentity };
    
    Mat44();
    Mat44(const Mat44 &other);
    Mat44(const float (&m)[kNumElems]);
    Mat44(eInitializerIdentity);
    ~Mat44();

    void identity();
    
    const Vec4& getCol(const u8 col) const;
    Vec4& getCol(const u8 col);
    
    f32& element(int i, int j);
    f32& operator()(int i, int j);
    const f32& element(int i, int j) const;
    const f32& operator()(int i, int j) const;

    void setRow(int i, const Vec4 &p);
    void setColumn(int j, const Vec4 &p);

    // (R3->R3) -> (R3->R3)
    Mat44 inverse() const;
    
    f32 matrix[kNumElems];
};

// (R3->R3)x(R3->R3) -> (R3->R3)
Mat44 operator *(const Mat44 &A, const Mat44 &B);
// (R3->R3)xR3 -> R3
Vec4 operator *(const Mat44 &A, const Vec4 &p);

// Rotations
Mat44 matrixWithRotationOnAxisWithCenter(f32 alpha, const Vec4 &a, const Vec4 &center);
Mat44 matrixWithRotationOnAxis(f32 alpha, const Vec4 &a);
Mat44 matrixWithReflectionOnAxis(const Vec4 &axisOrtho);

/*******************
 * Implementation
 ******************/

#include "Mat44.inl"

#endif
