#ifndef __WASTELADNS_SHADERS_DX11_H__
#define __WASTELADNS_SHADERS_DX11_H__

namespace gfx {
namespace shaders {

constexpr VS_src vs_color3d_unlit = {
"vs_color3d_unlit",
R"(
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
)"
};

constexpr VS_src vs_fullscreen_bufferless_clear_blit = {
"vs_fullscreen_bufferless_clear_blit",
R"(
struct VS_Output {
    float4 positionCS : SV_POSITION;
};
VS_Output VS(uint id : SV_VertexID) {
    VS_Output Output;
    float2 uv = float2((id << 1) & 2, id & 2);
    Output.positionCS = float4(uv * float2(2,-2) + float2(-1,1), 1, 1);  // clear depth
    return Output;
}
)"
};

constexpr VS_src vs_fullscreen_bufferless_textured_blit = {
"vs_fullscreen_bufferless_textured_blit",
R"(
struct VS_Output {
    float2 uv : TEXCOORD0;
    float4 positionCS : SV_POSITION;
};
VS_Output VS(uint id : SV_VertexID) {
    VS_Output Output;
    Output.uv = float2((id << 1) & 2, id & 2);
    Output.positionCS = float4(Output.uv * float2(2,-2) + float2(-1,1), 0, 1);
    return Output;
}
)"
};

constexpr PS_src ps_color3d_unlit = { // used for multiple vertex and buffer layouts
"ps_color3d_unlit",
R"(
struct VertexOut {
    float4 color : COLOR;
};
float4 PS(VertexOut OUT) : SV_TARGET {
    return OUT.color;
}
)"
};

constexpr PS_src ps_fullscreen_blit_clear_colored = {
"ps_fullscreen_blit_clear_colored",
R"(
cbuffer BlitColor : register(b0) {
    float4 color;
}
float4 PS() : SV_TARGET {
    return color;
}
)"
};

constexpr PS_src ps_fullscreen_blit_textured = {
"ps_fullscreen_blit_textured",
R"(
Texture2D texSrc : register(t0);
SamplerState texSrcSampler : register(s0);

struct VertexOut {
    float2 uv : TEXCOORD;
};
float4 PS(VertexOut IN) : SV_TARGET {

    float4 albedo = texSrc.Sample(texSrcSampler, IN.uv).rgba;
    return albedo.rgba;
}
)"
};

} // shaders
} // gfx
#endif // __WASTELADNS_SHADERS_DX11_H__
