#ifndef __WASTELADNS_SHADERS_DX11_H__
#define __WASTELADNS_SHADERS_DX11_H__

const char * coloredVertexShaderStr = R"(
cbuffer PerGroup : register(b0) {
    matrix MVP;
}
struct AppData {
    float3 position : POSITION;
    float4 color : COLOR;
};
struct VertexOutput {
    float4 color : COLOR;
    float4 position : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    OUT.position = mul(MVP, float4(IN.position, 1.f));
    OUT.color = IN.color.rgba;
    return OUT;
}
)";

const char * defaultPixelShaderStr = R"(
struct PixelIn {
    float4 color : COLOR;
};
float4 PS(PixelIn IN) : SV_TARGET {
    return IN.color;
}
)";

const char* defaultVertexShaderStr = R"(

cbuffer PerScene : register(b0) {
    matrix projectionMatrix;
    matrix viewMatrix;
    float3 viewPosWS;
    float padding;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
cbuffer PerInstance : register(b2) {
	matrix instanceMatrices[256];
};
struct AppData {
    float3 posMS : POSITION;
};
struct VertexOutput {
    float4 color : COLOR;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    matrix vp = mul(projectionMatrix, viewMatrix);
    float4 posWS = mul(modelMatrix, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vp, posWS);
    OUT.color = groupColor;
    return OUT;
}
)";

const char * defaultInstancedVertexShaderStr = R"(

cbuffer PerScene : register(b0) {
    matrix projectionMatrix;
    matrix viewMatrix;
    float3 viewPosWS;
    float padding;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
cbuffer PerInstance : register(b2) {
	matrix instanceMatrices[256];
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
    matrix vp = mul(projectionMatrix, viewMatrix);
    float4 posWS = mul(mm, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vp, posWS);
    OUT.color = groupColor;
    return OUT;
}
)";

#endif // __WASTELADNS_SHADERS_DX11_H__
