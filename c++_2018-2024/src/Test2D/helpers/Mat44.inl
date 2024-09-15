#include <math.h>
#include <string.h>

#include "Vector.h"


Mat44::Mat44() {
}


Mat44::Mat44(const Mat44 &other) {
    memcpy(&matrix, other.matrix, kNumElems * sizeof(f32));
}

Mat44::Mat44(const float (&m)[kNumElems]) {
    memcpy(&matrix, m, kNumElems * sizeof(f32));
}


Mat44::Mat44(eInitializerIdentity) {
    identity();
}


Mat44::~Mat44() {
}

void Mat44::identity() {
    matrix[0] = 1;    matrix[4] = 0;    matrix[8] = 0;    matrix[12] = 0;
    matrix[1] = 0;    matrix[5] = 1;    matrix[9] = 0;    matrix[13] = 0;
    matrix[2] = 0;    matrix[6] = 0;    matrix[10] = 1;   matrix[14] = 0;
    matrix[3] = 0;    matrix[7] = 0;    matrix[11] = 0;   matrix[15] = 1;
}


const Vec4& Mat44::getCol(const u8 col) const {
    return (*reinterpret_cast<const Vec4*>(&matrix[col * 4]));
}


Vec4& Mat44::getCol(const u8 col) {
    return (*reinterpret_cast<Vec4*>(&matrix[col * 4]));
}

f32& Mat44::operator()(int i, int j) {
    return element(i, j);
}

const f32& Mat44::operator()(int i, int j) const {
    return element(i, j);
}

f32& Mat44::element(int i, int j) {
    return matrix[i << 2 | j];
}

const f32& Mat44::element(int i, int j) const {
    return matrix[i << 2 | j];
}

void Mat44::setRow(int j, const Vec4 &p) {
    for (int i = 0; i < kNumRows; i++)
        element(i, j) = p[i];
}

void Mat44::setColumn(int i, const Vec4 &p) {
    for (int j = 0; j < kNumCols; j++)
        element(i, j) = p[j];
}

Mat44 matrixWithRotationOnAxis(f32 alpha, const Vec4 &a) {
    // center: rotation center
    // a: rotation axis
    //
    // By rotating over a default center, (0,0,0,1), we skip the last column of the matrix
    // (responsible for the traslation). This is useful when dealing with vector (in which
    // we don't care about their position)
    //
    // a vector is normalized, which means that operations like a.x^2 + a.y^2 
    // are equal to can be    to 1 - a.z^2, which considerably simplifies the code

    double s = sin(alpha);
    double c = cos(alpha);
    double axx = a.x*a.x;
    double ayy = a.y*a.y;
    double azz = a.z*a.z;

    Mat44 out;
    out.matrix[0] = axx + (1.f - axx) * c;
    out.matrix[1] = a.x * a.y * (1.f - c) - a.z * s;
    out.matrix[2] = a.x * a.z * (1.f - c) + a.y * s;
    out.matrix[3] = 0;

    out.matrix[4] = a.x * a.y * (1.f - c) + a.z * s;
    out.matrix[5] = ayy + (1.f - ayy) * c;
    out.matrix[6] = a.y * a.z * (1.f - c) - a.x * s;
    out.matrix[7] = 0;

    out.matrix[8] = a.x * a.z * (1.f - c) - a.y * s;
    out.matrix[9] = a.y * a.z * (1.f - c) + a.x * s;
    out.matrix[10] = azz + (1.f - azz) * c;
    out.matrix[11] = 0;

    out.matrix[12] = 0;
    out.matrix[13] = 0;
    out.matrix[14] = 0;
    out.matrix[15] = 1;

    return out;
}

Mat44 matrixWithReflectionOnAxis(const Vec4 &axisOrtho) {
    Mat44 out;
    out.matrix[0] = 1 - 2 * axisOrtho.x * axisOrtho.x;
    out.matrix[1] = -2 * axisOrtho.x * axisOrtho.y;
    out.matrix[2] = -2 * axisOrtho.x * axisOrtho.z;
    out.matrix[3] = 0;
    
    out.matrix[4] = -2 * axisOrtho.x * axisOrtho.y;
    out.matrix[5] = 1 - 2 * axisOrtho.y * axisOrtho.y;
    out.matrix[6] = -2 * axisOrtho.y + axisOrtho.z;
    out.matrix[7] = 0;
    
    out.matrix[8] = -2 * axisOrtho.x * axisOrtho.z;
    out.matrix[9] = -2 * axisOrtho.y * axisOrtho.z;
    out.matrix[10] = 1 - 2 * axisOrtho.z * axisOrtho.z;
    out.matrix[11] = 0;
    
    out.matrix[12] = 0;
    out.matrix[13] = 0;
    out.matrix[14] = 0;
    out.matrix[15] = 1;
    
    return out;
}

Mat44 matrixWithRotationOnAxisWithCenter(f32 alpha, const Vec4 &a, const Vec4 &center) {
    // center: rotation center
    // a: rotation axis
    //
    // See Mat44::matrixWithRotationOnAxis for further explanation on the code
    double s = sin(alpha);
    double c = cos(alpha);
    double axx = a.x*a.x;
    double ayy = a.y*a.y;
    double azz = a.z*a.z;

    Mat44 out;
    out.matrix[0] = axx + (1.f - axx) * c;
    out.matrix[1] = a.x * a.y * (1.f - c) - a.z * s;
    out.matrix[2] = a.x * a.z * (1.f - c) + a.y * s;
    out.matrix[3] = (center.x*(1.f - axx) - a.x*(center.y*a.y + center.z*a.z))*(1.f - c) + (center.y*a.z - center.z*a.y)*s;

    out.matrix[4] = a.x * a.y * (1.f - c) + a.z * s;
    out.matrix[5] = ayy + (1.f - ayy) * c;
    out.matrix[6] = a.y * a.z * (1.f - c) - a.x * s;
    out.matrix[7] = (center.y*(1.f - ayy) - a.y*(center.x*a.x + center.z*a.z))*(1.f - c) + (center.z*a.x - center.x*a.z)*s;

    out.matrix[8] = a.x * a.z * (1.f - c) - a.y * s;
    out.matrix[9] = a.y * a.z * (1.f - c) + a.x * s;
    out.matrix[10] = azz + (1.f - azz) * c;
    out.matrix[11] = (center.z*(1.f - azz) - a.z*(center.x*a.x + center.y*a.y))*(1.f - c) + (center.x*a.y - center.y*a.x)*s;

    out.matrix[12] = 0;
    out.matrix[13] = 0;
    out.matrix[14] = 0;
    out.matrix[15] = 1;

    return out;
}

// (R3->R3)x(R3->R3)->(R3->R3) Functions

Mat44 operator *(const Mat44 &A, const Mat44 &B) {
    
    Mat44 out;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            out(i, j) = 0;
            for (int k = 0; k < 4; k++) {
                out(i,j) += A(i, k) * B(k, j);
            }
        }
    }
    return out;
}

// (R3->R3)xR3->R3 Functions

Vec4 operator *(const Mat44 &A, const Vec4 &p) {
    Vec4 out;
    for (int j = 0; j < 4; j++) {
        out[j] = 0;
        for (int i = 0; i < 4; i++) {
            out[j] += A(i,j) * p[i];
        }
    }
    return out;
}

// (R3->R3)->(R3->R3)

Mat44 Mat44::inverse() const {
    // TODO: refactor to consider false cases
    Mat44 out;

    out.matrix[0] = matrix[5]    * matrix[10]   * matrix[15] -
                    matrix[5]    * matrix[11]   * matrix[14] -
                    matrix[9]    * matrix[6]    * matrix[15] +
                    matrix[9]    * matrix[7]    * matrix[14] +
                    matrix[13]   * matrix[6]    * matrix[11] -
                    matrix[13]   * matrix[7]    * matrix[10];

    out.matrix[4] = -matrix[4]  * matrix[10]    * matrix[15] +
                    matrix[4]   * matrix[11]    * matrix[14] +
                    matrix[8]   * matrix[6]     * matrix[15] -
                    matrix[8]   * matrix[7]     * matrix[14] -
                    matrix[12]  * matrix[6]     * matrix[11] +
                    matrix[12]  * matrix[7]     * matrix[10];

    out.matrix[8] = matrix[4]   * matrix[9]     * matrix[15] -
                    matrix[4]   * matrix[11]    * matrix[13] -
                    matrix[8]   * matrix[5]     * matrix[15] +
                    matrix[8]   * matrix[7]     * matrix[13] +
                    matrix[12]  * matrix[5]     * matrix[11] -
                    matrix[12]  * matrix[7]     * matrix[9];

    out.matrix[12] = -matrix[4] * matrix[9]     * matrix[14] +
                     matrix[4]  * matrix[10]    * matrix[13] +
                     matrix[8]  * matrix[5]     * matrix[14] -
                     matrix[8]  * matrix[6]     * matrix[13] -
                     matrix[12] * matrix[5]     * matrix[10] +
                     matrix[12] * matrix[6]     * matrix[9];

    out.matrix[1] = -matrix[1]  * matrix[10]    * matrix[15] +
                    matrix[1]   * matrix[11]    * matrix[14] +
                    matrix[9]   * matrix[2]     * matrix[15] -
                    matrix[9]   * matrix[3]     * matrix[14] -
                    matrix[13]  * matrix[2]     * matrix[11] +
                    matrix[13]  * matrix[3]     * matrix[10];

    out.matrix[5] = matrix[0]   * matrix[10]    * matrix[15] -
                    matrix[0]   * matrix[11]    * matrix[14] -
                    matrix[8]   * matrix[2]     * matrix[15] +
                    matrix[8]   * matrix[3]     * matrix[14] +
                    matrix[12]  * matrix[2]     * matrix[11] -
                    matrix[12]  * matrix[3]     * matrix[10];

    out.matrix[9] = -matrix[0]  * matrix[9]     * matrix[15] +
                    matrix[0]   * matrix[11]    * matrix[13] +
                    matrix[8]   * matrix[1]     * matrix[15] -
                    matrix[8]   * matrix[3]     * matrix[13] -
                    matrix[12]  * matrix[1]     * matrix[11] +
                    matrix[12]  * matrix[3]     * matrix[9];

    out.matrix[13] = matrix[0]  * matrix[9]     * matrix[14] -
                     matrix[0]  * matrix[10]    * matrix[13] -
                     matrix[8]  * matrix[1]     * matrix[14] +
                     matrix[8]  * matrix[2]     * matrix[13] +
                     matrix[12] * matrix[1]     * matrix[10] -
                     matrix[12] * matrix[2]     * matrix[9];

    out.matrix[2] = matrix[1]   * matrix[6]     * matrix[15] -
                    matrix[1]   * matrix[7]     * matrix[14] -
                    matrix[5]   * matrix[2]     * matrix[15] +
                    matrix[5]   * matrix[3]     * matrix[14] +
                    matrix[13]  * matrix[2]     * matrix[7] -
                    matrix[13]  * matrix[3]     * matrix[6];

    out.matrix[6] = -matrix[0]  * matrix[6]     * matrix[15] +
                    matrix[0]   * matrix[7]     * matrix[14] +
                    matrix[4]   * matrix[2]     * matrix[15] -
                    matrix[4]   * matrix[3]     * matrix[14] -
                    matrix[12]  * matrix[2]     * matrix[7] +
                    matrix[12]  * matrix[3]     * matrix[6];

    out.matrix[10] = matrix[0]  * matrix[5]     * matrix[15] -
                     matrix[0]  * matrix[7]     * matrix[13] -
                     matrix[4]  * matrix[1]     * matrix[15] +
                     matrix[4]  * matrix[3]     * matrix[13] +
                     matrix[12] * matrix[1]     * matrix[7] -
                     matrix[12] * matrix[3]     * matrix[5];

    out.matrix[14] = -matrix[0] * matrix[5]     * matrix[14] +
                     matrix[0]  * matrix[6]     * matrix[13] +
                     matrix[4]  * matrix[1]     * matrix[14] -
                     matrix[4]  * matrix[2]     * matrix[13] -
                     matrix[12] * matrix[1]     * matrix[6] +
                     matrix[12] * matrix[2]     * matrix[5];

    out.matrix[3] = -matrix[1]  * matrix[6]     * matrix[11] +
                    matrix[1]   * matrix[7]     * matrix[10] +
                    matrix[5]   * matrix[2]     * matrix[11] -
                    matrix[5]   * matrix[3]     * matrix[10] -
                    matrix[9]   * matrix[2]     * matrix[7] +
                    matrix[9]   * matrix[3]     * matrix[6];

    out.matrix[7] = matrix[0]   * matrix[6]     * matrix[11] -
                    matrix[0]   * matrix[7]     * matrix[10] -
                    matrix[4]   * matrix[2]     * matrix[11] +
                    matrix[4]   * matrix[3]     * matrix[10] +
                    matrix[8]   * matrix[2]     * matrix[7] -
                    matrix[8]   * matrix[3]     * matrix[6];

    out.matrix[11] = -matrix[0] * matrix[5]     * matrix[11] +
                     matrix[0]  * matrix[7]     * matrix[9] +
                     matrix[4]  * matrix[1]     * matrix[11] -
                     matrix[4]  * matrix[3]     * matrix[9] -
                     matrix[8]  * matrix[1]     * matrix[7] +
                     matrix[8]  * matrix[3]     * matrix[5];

    out.matrix[15] = matrix[0]  * matrix[5]     * matrix[10] -
                     matrix[0]  * matrix[6]     * matrix[9] -
                     matrix[4]  * matrix[1]     * matrix[10] +
                     matrix[4]  * matrix[2]     * matrix[9] +
                     matrix[8]  * matrix[1]     * matrix[6] -
                     matrix[8]  * matrix[2]     * matrix[5];

    f32 det;
    det = matrix[0] * out.matrix[0] + matrix[1] * out.matrix[4] + matrix[2] * out.matrix[8] + matrix[3] * out.matrix[12];
    if (det == 0)
        out.identity();
        return out;

    det = 1.0f / det;

    for (int i = 0; i < 16; i++)
        out.matrix[i] = out.matrix[i] * det;

    return out;
}
