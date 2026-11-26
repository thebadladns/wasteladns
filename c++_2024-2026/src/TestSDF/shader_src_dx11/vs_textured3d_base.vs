cbuffer PerScene : register(b0) {
    matrix vpMatrix;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
struct AppData {
    float3 posMS : POSITION;
    float2 uv : TEXCOORD;
};
struct VertexOutput {
    float2 uv : TEXCOORD;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    float4 posWS = mul(modelMatrix, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vpMatrix, posWS);
    OUT.uv = IN.uv;
    return OUT;
}