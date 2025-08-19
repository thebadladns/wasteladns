#ifndef __WASTELADNS_VEC_OPS_H__
#define __WASTELADNS_VEC_OPS_H__

namespace math {

force_inline f32 quaternionDot(const float4& a, const float4& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }
float4 quaternionSlerp(const f32 t, const float4& a, const float4& b) {
    f32 bx = b.x, by = b.y, bz = b.z, bw = b.w;
    f32 ax = a.x, ay = a.y, az = a.z, aw = a.w;

    // If negative dot, negate b so the interpolation takes the shorter arc
    f32 cosOmega = math::quaternionDot(a, b);
    if (cosOmega < 0.0f) {
        bx = -b.x; by = -b.y; bz = -b.z; bw = -b.w;
        cosOmega = -cosOmega;
    }

    f32 ka, kb;
    if (cosOmega > 0.9999f) { // Very close together: use linear interpolation
        ka = 1.0f - t;
        kb = t;
	} else {
        // sin^2(omega) + cos^2(omega) = 1, so
        f32 sinOmega = math::sqrt(1.0f - cosOmega * cosOmega);
        f32 omega = math::atan2(sinOmega, cosOmega);
        f32 oneOverSinOmega = 1.0f / sinOmega;

        ka = math::sin((1.0f - t) * omega) * oneOverSinOmega;
        kb = math::sin(t * omega) * oneOverSinOmega;
    }

    return float4(ax * ka + bx * kb
                , ay * ka + by * kb
                , az * ka + bz * kb
                , aw * ka + bw * kb);
}

float4 eulersToQuaterion(const float3& eulers)
{
    f32 pitch = eulers.x;
    f32 roll = eulers.y;
    f32 yaw = eulers.z;
    f32 sy = math::sin(yaw * 0.5f);
    f32 cy = math::cos(yaw * 0.5f);
    f32 cp = math::cos(pitch * 0.5f);
    f32 sp = math::sin(pitch * 0.5f);
    f32 cr = math::cos(roll * 0.5f);
    f32 sr = math::sin(roll * 0.5f);

    return float4(-cr * cp * cy + sr * sp * sy   // check math again
                , -sr * cp * sy + cr * sp * cy
                , -cr * sp * sy - sr * cp * cy
                , cr * cp * sy + sr * sp * cy);
}

float3 quaternionToEulers(const float4& q) {
    f32 sinr_cosp = 2.f * (q.w * q.x + q.y * q.z);
    f32 cosr_cosp = 1.f - 2.f * (q.x * q.x + q.y * q.y);
    f32 roll = math::atan2(sinr_cosp, cosr_cosp);

    f32 pitch;
    f32 sinp = 2.f * (q.w * q.y - q.z * q.x);
    if (sinp > 1.f) pitch = math::halfpi32;
    else if (sinp < 1.f) pitch = -math::halfpi32;
    else pitch = math::asin(sinp);

    f32 siny_cosp = 2.f * (q.w * q.z + q.x * q.y);
    f32 cosy_cosp = 1.f - 2.f * (q.y * q.y + q.z * q.z);
    f32 yaw = math::atan2(siny_cosp, cosy_cosp);

    return float3(roll, pitch, yaw);
}

float3x3 eulersToRotationMatrix(const float3& eulers) {
    float3x3 m;
    const f32 cx = math::cos(eulers.x);
    const f32 cy = math::cos(eulers.y);
    const f32 cz = math::cos(eulers.z);
    const f32 sx = math::sin(eulers.x);
    const f32 sy = math::sin(eulers.y);
    const f32 sz = math::sin(eulers.z);
    const f32 cxcz = cx * cz;
    const f32 cxsz = cx * sz;
    const f32 sxcz = sx * cz;
    const f32 sxsz = sx * sz;
    
    m.col0.x = cy * cz;
    m.col0.y = - cy * sz;
    m.col0.z = sy;

    m.col1.x = sxcz * sy + cxsz;
    m.col1.y = cxcz - sxsz * sy;
    m.col1.z = - sx * cy;

    m.col2.x = sxsz - cxcz * sy;
    m.col2.y = cxsz * sy + sxcz;
    m.col2.z = cx * cy;

    return m;
}

float3x3 quaternionToRotationMatrix(float4& q) {
    float3x3 m;

    f32 q0 = q.x;
    f32 q1 = q.y;
    f32 q2 = q.z;
    f32 q3 = q.w;

    // First row
    m.m[0] = 2 * (q0 * q0 + q1 * q1) - 1.f;
    m.m[3] = 2 * (q1 * q2 - q0 * q3);
    m.m[6] = 2 * (q1 * q3 + q0 * q2);

    // Second row
    m.m[1] = 2 * (q1 * q2 + q0 * q3);
    m.m[4] = 2 * (q0 * q0 + q2 * q2) - 1.f;
    m.m[7] = 2 * (q2 * q3 - q0 * q1);

    // Third row
    m.m[2] = 2 * (q1 * q3 - q0 * q2);
    m.m[5] = 2 * (q2 * q3 + q0 * q1);
    m.m[8] = 2 * (q0 * q0 + q3 * q3) - 1.f;

    return m;
}

float4x4 trsToMatrix(float3 translation, float4 q /*quaternion*/, float3 scale) {
	float4x4 mat;
    f32 xx = q.x * q.x, xy = q.x * q.y, xz = q.x * q.z, xw = q.x * q.w;
    f32 yy = q.y * q.y, yz = q.y * q.z, yw = q.y * q.w;
    f32 zz = q.z * q.z, zw = q.z * q.w;
    f32 sx = 2.0f * scale.x, sy = 2.0f * scale.y, sz = 2.0f * scale.z;
    mat.col0.x = sx * (-yy - zz + 0.5f);    mat.col1.x = sy * (-zw + xy);           mat.col2.x = sz * (+xz + yw);           mat.col3.x = translation.x;
    mat.col0.y = sx * (+xy + zw);           mat.col1.y = sy * (-xx - zz + 0.5f);    mat.col2.y = sz * (-xw + yz);           mat.col3.y = translation.y;
    mat.col0.z = sx * (-yw + xz);           mat.col1.z = sy * (+xw + yz);           mat.col2.z = sz * (-xx - yy + 0.5f);    mat.col3.z = translation.z,
    mat.col0.w = 0.f;                       mat.col1.w = 0.f;                       mat.col2.w = 0.f;                       mat.col3.w = 1.f;
    return mat;
}

float4 rotationMatrixToQuaternion(float3x3 m) {
    float4 q;
    f32* a = m.m;
    f32 trace = a[0] + a[4] + a[8];
    
    if (trace > 0.f) {
        f32 s = 0.5f / sqrtf(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (a[5] - a[7]) * s;
        q.y = (a[6] - a[2]) * s;
        q.z = (a[1] - a[3]) * s;
    }
    else {
        if (a[0] > a[4] && a[0] > a[8]) {
            f32 s = 2.0f * math::sqrt(1.0f + a[0] - a[4] - a[8]);
            q.w = (a[5] - a[7]) / s;
            q.x = 0.25f * s;
            q.y = (a[3] + a[1]) / s;
            q.z = (a[6] + a[2]) / s;
        }
        else if (a[4] > a[8]) {
            f32 s = 2.0f * math::sqrt(1.0f + a[4] - a[0] - a[8]);
            q.w = (a[6] - a[2]) / s;
            q.x = (a[3] + a[1]) / s;
            q.y = 0.25f * s;
            q.z = (a[7] + a[5]) / s;
        }
        else {
            f32 s = 2.0f * math::sqrt(1.0f + a[8] - a[0] - a[4]);
            q.w = (a[1] - a[3]) / s;
            q.x = (a[6] + a[2]) / s;
            q.y = (a[7] + a[5]) / s;
            q.z = 0.25f * s;
        }
    }
    return q;
}

bool inverse(float4x4& m) {
    
    float4x4 prev = m;
    memset(m.m, 0, sizeof(f32) * 16);
    
    f32* prev_m = prev.m;
    f32* next_m = m.m;
    
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
    
    f32 det = prev_m[0] * next_m[0] + prev_m[1] * next_m[4] + prev_m[2] * next_m[8] + prev_m[3] * next_m[12];
    if (det == 0.f) { return false; }
    
    f32 inv_det = 1.0f / det;
    for (int i = 0; i < 16; i++)
        next_m[i] = next_m[i] * inv_det;
    
    return true;
}
f32 poly_area(const float3* poly, const u32 count) {
    float3 normal = math::cross(math::subtract(poly[1], poly[0]), math::subtract(poly[2], poly[0]));
    math::normalizeSafe(normal);
    f32 area = 0.f;
    float3 prev_v = math::subtract(poly[0], poly[count - 1]);
    for (u32 c = 0; c < count; ++c) {
        float3 curr_v = math::subtract(poly[0], poly[c]);
        area += math::dot(math::cross(curr_v, prev_v), normal);
        prev_v = curr_v;
    }
    area = math::abs(0.5f*area);
    return area;
}

} // math

#endif // __WASTELADNS_VEC_OPS_H__
