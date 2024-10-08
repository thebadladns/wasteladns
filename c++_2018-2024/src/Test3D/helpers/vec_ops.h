#ifndef __WASTELADNS_VEC_OPS_H__
#define __WASTELADNS_VEC_OPS_H__

#ifndef UNITYBUILD
#include "vec.h"
#include "angle.h"
#endif

namespace Math {

template <typename _T>
Vector4<_T> quaternionLerp(const f32 t, const Vector4<_T>& a, const Vector4<_T>& b) {
    _T af = 1.0f - t, bf = t;
    _T x = af * a.x + bf * b.x;
    _T y = af * a.y + bf * b.y;
    _T z = af * a.z + bf * b.z;
    _T w = af * a.w + bf * b.w;
    return Vector4<_T>(x, y, z, w);
}
template<typename _T>
_T quaternionDot(const Vector4<_T>& a, const Vector4<_T>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
template <typename _T>
Vector4<_T> quaternionSlerp(const f32 t, const Vector4<_T>& a, const Vector4<_T>& b) {
    _T x, y, z, w;
    _T bx = b.x, by = b.y, bz = b.z, bw = b.w;
    _T ax = a.x, ay = a.y, az = a.z, aw = a.w;

    // If negative dot, negate b so the interpolation takes the shorter arc
    _T cosOmega = Math::quaternionDot(a, b);
    if (cosOmega < 0.0f) {
        bx = -b.x; by = -b.y; bz = -b.z; bw = -b.w;
        cosOmega = -cosOmega;
    }

    _T ka, kb;
    if (cosOmega > 0.9999f) { // Very close together: use linear interpolation
        ka = 1.0f - t;
        kb = t;
	} else {
        // sin^2(omega) + cos^2(omega) = 1, so
        _T sinOmega = Math::sqrt(1.0f - cosOmega * cosOmega);
        _T omega = Math::atan2(sinOmega, cosOmega);
        _T oneOverSinOmega = 1.0f / sinOmega;

        ka = Math::sin((1.0f - t) * omega) * oneOverSinOmega;
        kb = Math::sin(t * omega) * oneOverSinOmega;
    }

    x = ax * ka + bx * kb;
    y = ay * ka + by * kb;
    z = az * ka + bz * kb;
    w = aw * ka + bw * kb;
    return Vector4<_T>(x, y, z, w);
}
template<typename _T>
Vector4<_T> quaternionNegate(const Vector4<_T>& q) {
	return Vector4<_T>(-q.x, -q.y, -q.z, -q.w);
}

template<typename _T>
Vector4<_T> eulersToQuaterion(const Vector3<_T>& eulers)
{
    _T roll = eulers.x;
    _T pitch = eulers.y;
    _T yaw = eulers.z;
    _T cy = Math::cos(yaw * 0.5f);
    _T sy = Math::sin(yaw * 0.5f);
    _T cp = Math::cos(pitch * 0.5f);
    _T sp = Math::sin(pitch * 0.5f);
    _T cr = Math::cos(roll * 0.5f);
    _T sr = Math::sin(roll * 0.5f);

    Vector4<_T> q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return q;
}

template<typename _T>
Vector3<_T> quaternionToEulers(const Vector4<_T>& q) {
    _T sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    _T cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    _T roll = Math::atan2(sinr_cosp, cosr_cosp);

    _T pitch;
    _T sinp = 2 * (q.w * q.y - q.z * q.x);
    if (sinp > 1) pitch = Math::halfpi<_T>;
    else if (sinp < 1) pitch = -Math::halfpi<_T>;
    else pitch = Math::asin(sinp);

    _T siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    _T cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    _T yaw = Math::atan2(siny_cosp, cosy_cosp);

    Vector3<_T> eulers(roll, pitch, yaw);
    return eulers;
}

template <typename _T>
Matrix33<_T> quaternionToRotationMatrix(Vector4<_T>& q) {
    Matrix33<_T> m;

    _T q0 = q.x;
    _T q1 = q.y;
    _T q2 = q.z;
    _T q3 = q.w;

    // First row
    m.dataCM[0] = 2 * (q0 * q0 + q1 * q1) - 1.f;
    m.dataCM[3] = 2 * (q1 * q2 - q0 * q3);
    m.dataCM[6] = 2 * (q1 * q3 + q0 * q2);

    // Second row
    m.dataCM[1] = 2 * (q1 * q2 + q0 * q3);
    m.dataCM[4] = 2 * (q0 * q0 + q2 * q2) - 1.f;
    m.dataCM[7] = 2 * (q2 * q3 - q0 * q1);

    // Third row
    m.dataCM[2] = 2 * (q1 * q3 - q0 * q2);
    m.dataCM[5] = 2 * (q2 * q3 + q0 * q1);
    m.dataCM[8] = 2 * (q0 * q0 + q3 * q3) - 1.f;

    //// First row
    //m.dataCM[0] = 2 * (q0 * q0 + q1 * q1) - 1.f;
    //m.dataCM[1] = 2 * (q1 * q2 - q0 * q3);
    //m.dataCM[2] = 2 * (q1 * q3 + q0 * q2);

    //// Second row
    //m.dataCM[3] = 2 * (q1 * q2 + q0 * q3);
    //m.dataCM[4] = 2 * (q0 * q0 + q2 * q2) - 1.f;
    //m.dataCM[5] = 2 * (q2 * q3 - q0 * q1);

    //// Third row
    //m.dataCM[6] = 2 * (q1 * q3 - q0 * q2);
    //m.dataCM[7] = 2 * (q2 * q3 + q0 * q1);
    //m.dataCM[8] = 2 * (q0 * q0 + q3 * q3) - 1.f;

    return m;
}

template <typename _T>
Matrix44<_T> trsToMatrix(Vector3<_T> translation, Vector4<_T> q /*quaternion*/, Vector3<_T> scale) {
	Mat4 mat = Matrix44<_T>();
    float xx = q.x * q.x, xy = q.x * q.y, xz = q.x * q.z, xw = q.x * q.w;
    float yy = q.y * q.y, yz = q.y * q.z, yw = q.y * q.w;
    float zz = q.z * q.z, zw = q.z * q.w;
    float sx = 2.0f * scale.x, sy = 2.0f * scale.y, sz = 2.0f * scale.z;
    mat.col0.x = sx * (-yy - zz + 0.5f);    mat.col1.x = sy * (-zw + xy);           mat.col2.x = sz * (+xz + yw);           mat.col3.x = translation.x;
    mat.col0.y = sx * (+xy + zw);           mat.col1.y = sy * (-xx - zz + 0.5f);    mat.col2.y = sz * (-xw + yz);           mat.col3.y = translation.y;
    mat.col0.z = sx * (-yw + xz);           mat.col1.z = sy * (+xw + yz);           mat.col2.z = sz * (-xx - yy + 0.5f);    mat.col3.z = translation.z,
    mat.col0.w = 0.f;                       mat.col1.w = 0.f;                       mat.col2.w = 0.f;                       mat.col3.w = 1.f;
    return mat;
}

template <typename _T>
Vector4<_T> rotationMatrixToQuaternion(Matrix33<_T> m) {
    Vector4<_T> q;
    _T* a = m.dataCM;
    _T trace = a[0] + a[4] + a[8];// a[0][0] + a[1][1] + a[2][2];


    /*    if (trace > 0) {
        _T s = 0.5f / sqrtf(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (a[7] - a[5]) * s;//(a[2][1] - a[1][2]) * s;
        q.y = (a[2] - a[6]) * s;//(a[0][2] - a[2][0]) * s;
        q.z = (a[3] - a[1]) * s;//(a[1][0] - a[0][1]) * s;
    }
    else {
        if (a[0] > a[4] && a[0] > a[8]) {
            float s = 2.0f * sqrtf(1.0f + a[0] - a[4] - a[8]);
            q.w = (a[7] - a[5]) / s;
            q.x = 0.25f * s;
            q.y = (a[1] + a[3]) / s;
            q.z = (a[2] + a[6]) / s;
        }
        else if (a[4] > a[8]) {
            float s = 2.0f * sqrtf(1.0f + a[4] - a[0] - a[8]);
            q.w = (a[2] - a[6]) / s;
            q.x = (a[1] + a[3]) / s;
            q.y = 0.25f * s;
            q.z = (a[5] + a[7]) / s;
        }
        else {
            float s = 2.0f * sqrtf(1.0f + a[8] - a[0] - a[4]);
            q.w = (a[3] - a[1]) / s;
            q.x = (a[2] + a[6]) / s;
            q.y = (a[5] + a[7]) / s;
            q.z = 0.25f * s;
        }*/

    if (trace > 0) {
        _T s = 0.5f / sqrtf(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (a[5] - a[7]) * s;//(a[2][1] - a[1][2]) * s;
        q.y = (a[6] - a[2]) * s;//(a[0][2] - a[2][0]) * s;
        q.z = (a[1] - a[3]) * s;//(a[1][0] - a[0][1]) * s;
    }
    else {
        if (a[0] > a[4] && a[0] > a[8]) {
            float s = 2.0f * Math::sqrt(1.0f + a[0] - a[4] - a[8]);
            q.w = (a[5] - a[7]) / s;
            q.x = 0.25f * s;
            q.y = (a[3] + a[1]) / s;
            q.z = (a[6] + a[2]) / s;
        }
        else if (a[4] > a[8]) {
            float s = 2.0f * Math::sqrt(1.0f + a[4] - a[0] - a[8]);
            q.w = (a[6] - a[2]) / s;
            q.x = (a[3] + a[1]) / s;
            q.y = 0.25f * s;
            q.z = (a[7] + a[5]) / s;
        }
        else {
            float s = 2.0f * Math::sqrt(1.0f + a[8] - a[0] - a[4]);
            q.w = (a[1] - a[3]) / s;
            q.x = (a[6] + a[2]) / s;
            q.y = (a[7] + a[5]) / s;
            q.z = 0.25f * s;
        }
        /*
                if (a[0][0] > a[1][1] && a[0][0] > a[2][2]) {
            float s = 2.0f * sqrtf(1.0f + a[0][0] - a[1][1] - a[2][2]);
            q.w = (a[2][1] - a[1][2]) / s;
            q.x = 0.25f * s;
            q.y = (a[0][1] + a[1][0]) / s;
            q.z = (a[0][2] + a[2][0]) / s;
        }
        else if (a[1][1] > a[2][2]) {
            float s = 2.0f * sqrtf(1.0f + a[1][1] - a[0][0] - a[2][2]);
            q.w = (a[0][2] - a[2][0]) / s;
            q.x = (a[0][1] + a[1][0]) / s;
            q.y = 0.25f * s;
            q.z = (a[1][2] + a[2][1]) / s;
        }
        else {
            float s = 2.0f * sqrtf(1.0f + a[2][2] - a[0][0] - a[1][1]);
            q.w = (a[1][0] - a[0][1]) / s;
            q.x = (a[0][2] + a[2][0]) / s;
            q.y = (a[1][2] + a[2][1]) / s;
            q.z = 0.25f * s;
        }
        */
    }
    return q;
}

template <typename _T>
bool inverse(Matrix44<_T>& m) {
    
    Matrix44<_T> prev = m;
    memset(m.dataCM, 0, sizeof(_T) * 16);
    
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

}

#endif // __WASTELADNS_VEC_OPS_H__
