cbuffer PerScene : register(b0) {
    matrix vpMatrix;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
cbuffer PerInstance : register(b2) {
	matrix instanceMatrices[64];
};
struct AppData {
    float3 posMS : POSITION;
    uint instanceID : SV_InstanceID;
};
struct VertexOutput {
    float4 color : COLOR;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    matrix instanceMatrix = instanceMatrices[IN.instanceID];
    matrix mm = mul(instanceMatrix, modelMatrix);
    float4 posWS = mul(mm, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vpMatrix, posWS);
    OUT.color = groupColor;
    return OUT;
}