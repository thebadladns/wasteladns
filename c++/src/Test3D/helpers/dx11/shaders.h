#ifndef __WASTELADNS_SHADERS_DX11_H__
#define __WASTELADNS_SHADERS_DX11_H__

const char * coloredVertexShaderStr = R"(
cbuffer PerGroup : register(b0) {
    column_major matrix MVP;
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
struct VertexOut {
    float4 color : COLOR;
};
float4 PS(VertexOut OUT) : SV_TARGET {
    return OUT.color;
}
)";

const char* defaultVertexShaderStr = R"(

cbuffer PerScene : register(b0) {
    column_major matrix projectionMatrix;
    column_major matrix viewMatrix;
    float3 viewPosWS;
    float padding;
}
cbuffer PerGroup : register(b1) {
    column_major matrix modelMatrix;
    float4 groupColor;
}
cbuffer PerInstance : register(b2) {
	column_major matrix instanceMatrices[256];
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
    column_major matrix vp = mul(projectionMatrix, viewMatrix);
    float4 posWS = mul(modelMatrix, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vp, posWS);
    OUT.color = groupColor;
    return OUT;
}
)";

const char * defaultInstancedVertexShaderStr = R"(

cbuffer PerScene : register(b0) {
    column_major matrix projectionMatrix;
    column_major matrix viewMatrix;
    float3 viewPosWS;
    float padding;
}
cbuffer PerGroup : register(b1) {
    column_major matrix modelMatrix;
    float4 groupColor;
}
cbuffer PerInstance : register(b2) {
	column_major matrix instanceMatrices[256];
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
    column_major matrix instanceMatrix = instanceMatrices[IN.instanceID];
    column_major matrix mm = mul(instanceMatrix, modelMatrix);
    column_major matrix vp = mul(projectionMatrix, viewMatrix);
    float4 posWS = mul(mm, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vp, posWS);
    OUT.color = groupColor;
    return OUT;
}
)";

#endif // __WASTELADNS_SHADERS_DX11_H__
