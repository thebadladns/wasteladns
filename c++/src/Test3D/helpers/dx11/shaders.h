#ifndef __WASTELADNS_SHADERS_DX11_H__
#define __WASTELADNS_SHADERS_DX11_H__

namespace Renderer {
namespace Shaders {

template <>
const VSSrc vsSrc<
    VSTechnique::forward_base,
    Layout_Vec3Color4B,
    Layout_CBuffer_DebugScene::Buffers,
    VSDrawType::Standard> = {
"forward_base_Vec3Color4B_CBuffer_DebugScene_Standard",
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

template <>
const VSSrc vsSrc <
    VSTechnique::forward_base,
    Layout_Vec2Color4B,
    Layout_CBuffer_DebugScene::Buffers,
    VSDrawType::Standard> = vsSrc<
        VSTechnique::forward_base,
        Layout_Vec3Color4B,
        Layout_CBuffer_DebugScene::Buffers,
        VSDrawType::Standard>;

template <>
const VSSrc vsSrc <
    VSTechnique::forward_base,
    Layout_Vec3,
    Layout_CBuffer_3DScene::Buffers,
    VSDrawType::Standard> = {
"forward_base_Vec3_CBuffer_3DScene_Sandard",
R"(

cbuffer PerScene : register(b0) {
    matrix projectionMatrix;
    matrix viewMatrix;
    float3 viewPosWS;
    float padding1;
    float3 lightPosWS;
    float padding2;
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
)"
};

template <>
const VSSrc vsSrc <
    VSTechnique::forward_base,
    Layout_Vec3Color4B,
    Layout_CBuffer_3DScene::Buffers,
    VSDrawType::Standard> = {
"forward_base_Vec3Color4B_CBuffer_3DScene_Standard",
R"(
cbuffer PerScene : register(b0) {
    matrix projectionMatrix;
    matrix viewMatrix;
    float3 viewPosWS;
    float padding1;
    float3 lightPosWS;
    float padding2;
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
    float4 color : COLOR;
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
    OUT.color = IN.color.rgba;
    return OUT;
}
)"
};

template <>
const VSSrc vsSrc <
    VSTechnique::forward_base,
    Layout_Vec3,
    Layout_CBuffer_3DScene::Buffers,
    VSDrawType::Instanced> = {
"forward_base_Vec3_CBuffer_3DScene_Instanced"
, R"(
cbuffer PerScene : register(b0) {
    matrix projectionMatrix;
    matrix viewMatrix;
    float3 viewPosWS;
    float padding1;
    float3 lightPosWS;
    float padding2;
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
)"
};

template <>
const VSSrc vsSrc <
    VSTechnique::forward_base,
    Layout_TexturedVec3,
    Layout_CBuffer_3DScene::Buffers,
    VSDrawType::Standard> = {
"forward_base_TexturedVec3_CBuffer_3DScene_Standard",
R"(
cbuffer PerScene : register(b0) {
    matrix projectionMatrix;
    matrix viewMatrix;
    float3 viewPosWS;
    float padding1;
    float3 lightPosWS;
    float padding2;
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
    float2 uv : TEXCOORD;
    float3 normalMS : NORMAL;
    float3 tangentMS : TANGENT;
    float3 bitangentMS : BITANGENT;
};
struct VertexOutput {
    float3 posWS : POSITION;
    float3 normalWS : NORMAL;
    float3 viewPosTS : VIEW_TS;
    float3 posTS : POSITION_TS;
    float2 uv : TEXCOORD;
    float4 positionCS : SV_POSITION;
    float3x3 tbnMatrix : TBN;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    matrix mv = mul(projectionMatrix, viewMatrix);
    OUT.posWS = mul(modelMatrix, float4(IN.posMS, 1.f)).xyz;
    OUT.normalWS = normalize(mul(modelMatrix, float4(IN.normalMS, 0.f))).xyz;
    float3 tangentWS = normalize(mul(modelMatrix, float4(IN.tangentMS, 0.f))).xyz;
    float3 bitangentWS = normalize(mul(modelMatrix, float4(IN.bitangentMS, 0.f))).xyz;
    
    // load vectors in the rows (transposed, becomes the inverse)
     OUT.tbnMatrix = float3x3(
          tangentWS.x, bitangentWS.x, OUT.normalWS.x
        , tangentWS.y, bitangentWS.y, OUT.normalWS.y
        , tangentWS.z, bitangentWS.z, OUT.normalWS.z
    );
    float3x3 invtbn = transpose(OUT.tbnMatrix);
    OUT.viewPosTS = mul(invtbn, viewPosWS);
    OUT.posTS = mul(invtbn, OUT.posWS);
    OUT.positionCS = mul(mv, float4(OUT.posWS, 1.f));
    OUT.uv = IN.uv;
    return OUT;
}
)"
};

template <>
const PSSrc psSrc <
    PSTechnique::forward_untextured_unlit,
    Layout_Vec3,
    Layout_CBuffer_3DScene::Buffers> = {
"forward_untextured_unlit", // used for multiple vertex and buffer layouts
nullptr, 0, // no samplers
R"(
struct VertexOut {
    float4 color : COLOR;
};
float4 PS(VertexOut OUT) : SV_TARGET {
    return OUT.color;
}
)"
};
template <>
const PSSrc psSrc <
    PSTechnique::forward_untextured_unlit,
    Layout_Vec2Color4B,
    Layout_CBuffer_3DScene::Buffers> = psSrc <
        PSTechnique::forward_untextured_unlit,
        Layout_Vec3,
        Layout_CBuffer_3DScene::Buffers>;
template <>
const PSSrc psSrc <
    PSTechnique::forward_untextured_unlit,
    Layout_Vec3Color4B,
    Layout_CBuffer_3DScene::Buffers> = psSrc <
        PSTechnique::forward_untextured_unlit,
        Layout_Vec3,
        Layout_CBuffer_3DScene::Buffers>;
template <>
const PSSrc psSrc <
    PSTechnique::forward_untextured_unlit,
    Layout_Vec2Color4B,
    Layout_CBuffer_DebugScene::Buffers> = psSrc <
        PSTechnique::forward_untextured_unlit,
        Layout_Vec3,
        Layout_CBuffer_3DScene::Buffers>;
template <>
const PSSrc psSrc <
    PSTechnique::forward_untextured_unlit,
    Layout_Vec3Color4B,
    Layout_CBuffer_DebugScene::Buffers> = psSrc <
        PSTechnique::forward_untextured_unlit,
        Layout_Vec3,
        Layout_CBuffer_3DScene::Buffers>;

const char* textured_lit_normalmapped_samplers[] = {"texDiffuse", "texNormal", "texDepth"};
template <>
const PSSrc psSrc <
    PSTechnique::forward_textured_lit_normalmapped,
    Layout_TexturedVec3,
    Layout_CBuffer_3DScene::Buffers> = {
"forward_textured_lit_normalmapped_TexturedVec3_CBuffer_3DScene",
textured_lit_normalmapped_samplers,
sizeof(textured_lit_normalmapped_samplers) / sizeof(textured_lit_normalmapped_samplers[0]),
R"(
cbuffer PerScene : register(b0) {
    matrix projectionMatrix;
    matrix viewMatrix;
    float3 viewPosWS;
    float padding1;
    float3 lightPosWS;
    float padding2;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
cbuffer PerInstance : register(b2) {
	matrix instanceMatrices[256];
};

Texture2D texDiffuse : register(t0);
Texture2D texNormal : register(t1);
Texture2D texDepth : register(t2);
SamplerState texDiffuseSampler : register(s0);
SamplerState texNormalSampler : register(s1);
SamplerState texDepthSampler : register(s2);


struct PixelIn {
    float3 posWS : POSITION;
    float3 normalWS : NORMAL;
    float3 viewPosTS : VIEW_TS;
    float3 posTS : POSITION_TS;
    float2 uv : TEXCOORD;
    float4 positionCS : SV_POSITION;
    float3x3 tbnMatrix : TBN;
};

float4 PS(PixelIn IN) : SV_TARGET {

    float3 lightWS = lightPosWS;
    float3 albedo = texDiffuse.Sample(texDiffuseSampler, IN.uv).rgb;
    float3 lightDir = normalize(lightWS - IN.posWS);
    float3 normalWS = texNormal.Sample(texNormalSampler, IN.uv).xyz;
    normalWS = normalWS * 2.f - 1.f;
    normalWS = normalize(mul(IN.tbnMatrix, normalWS));
    
    // Aggressive gooch-like shading
    float3 viewWS = viewPosWS.xyz;
    float3 viewDirWS = normalize(viewWS - IN.posWS);
    float diff = dot(lightDir, normalWS);
    float t = ( diff + 1.f ) * 0.5f;
    float3 r = 2.f * diff * normalWS - lightDir;
    float s = clamp(100.f * dot(r, viewDirWS) - 97.f, 0.f, 0.8f);
    float3 ccool = float3(0.f, 0.f, 0.3f) * albedo;
    float3 cwarm = clamp(float3(1.2f, 1.2f, 1.f) * albedo, 0.f, 1.f);
    float3 diffuse = 0.7f * diff * albedo;
    float3 ambient = 0.3f * albedo;

    float4 fragcolor = float4(s * float3(1.f,1.f,1.f) + (1 - s)*(t*cwarm + (1 - t)*ccool), 1.f);
    return fragcolor;
}
)"
};

}
}
#endif // __WASTELADNS_SHADERS_DX11_H__
