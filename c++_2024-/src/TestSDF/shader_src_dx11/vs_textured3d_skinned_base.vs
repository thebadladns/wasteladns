cbuffer PerScene : register(b0) {
    matrix vpMatrix;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
cbuffer type_PerJoint : register(b2) {
	matrix skinningMatrices[32];
};
struct AppData {
    float3 posMS : POSITION;
    float2 uv : TEXCOORD;
    int4 joint_indices : JOINTINDICES;
    float4 joint_weights : JOINTWEIGHTS;
};
struct VertexOutput {
    float2 uv : TEXCOORD;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    float4x4 joint0 = skinningMatrices[IN.joint_indices.x] * IN.joint_weights.x;
    float4x4 joint1 = skinningMatrices[IN.joint_indices.y] * IN.joint_weights.y;
    float4x4 joint2 = skinningMatrices[IN.joint_indices.z] * IN.joint_weights.z;
    float4x4 joint3 = skinningMatrices[IN.joint_indices.w] * IN.joint_weights.w;
    float4x4 skinning = joint0 + joint1 + joint2 + joint3;
    VertexOutput OUT;
    float4 posWS = mul(modelMatrix, mul(skinning, float4(IN.posMS, 1.f)));
    OUT.positionCS = mul(vpMatrix, posWS);
    OUT.uv = IN.uv;
    return OUT;
}