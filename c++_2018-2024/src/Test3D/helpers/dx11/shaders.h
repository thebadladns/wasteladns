#ifndef __WASTELADNS_SHADERS_DX11_H__
#define __WASTELADNS_SHADERS_DX11_H__

namespace Renderer {
namespace Shaders {

constexpr VS_src vs_forward_base_Vec3Color4B_CBuffer_DebugScene_Standard = {
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
struct VS_src_selector<
    VSTechnique::forward_base,
    Layout_Vec3Color4B,
    Layout_CBuffer_DebugScene,
    VSDrawType::Standard> {
    static const VS_src& value() { return vs_forward_base_Vec3Color4B_CBuffer_DebugScene_Standard; }
};

template <>
struct VS_src_selector<
    VSTechnique::forward_base,
    Layout_Vec2Color4B,
    Layout_CBuffer_DebugScene,
    VSDrawType::Standard> {
    static const VS_src& value() { return vs_forward_base_Vec3Color4B_CBuffer_DebugScene_Standard; }
};

constexpr VS_src vs_forward_base_Vec3_CBuffer_3DScene_Sandard = {
"vs_forward_base_Vec3_CBuffer_3DScene_Sandard",
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
struct VS_src_selector<
    VSTechnique::forward_base,
    Layout_Vec3,
    Layout_CBuffer_3DScene,
    VSDrawType::Standard> {
    static const VS_src& value() { return vs_forward_base_Vec3_CBuffer_3DScene_Sandard; }
};

constexpr VS_src vs_forward_base_Vec3Color4B_CBuffer_3DScene_Standard = {
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
struct VS_src_selector<
    VSTechnique::forward_base,
    Layout_Vec3Color4B,
    Layout_CBuffer_3DScene,
    VSDrawType::Standard> {
    static const VS_src& value() { return vs_forward_base_Vec3Color4B_CBuffer_3DScene_Standard; }
};


constexpr VS_src vs_forward_base_Vec3Color4BSkinned_CBuffer_3DScene_Standard = {
"forward_base_Vec3Color4BSkinned_CBuffer_3DScene_Standard",
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
	matrix skinningMatrices[256];
};
struct AppData {
    float3 posMS : POSITION;
    float4 color : COLOR;
    int4 joint_indices : JOINTINDICES;
    float4 joint_weights : JOINTWEIGHTS;
};
struct VertexOutput {
    float4 color : COLOR;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    float4x4 joint0 = skinningMatrices[IN.joint_indices.x] * IN.joint_weights.x;
    float4x4 joint1 = skinningMatrices[IN.joint_indices.y] * IN.joint_weights.y;
    float4x4 joint2 = skinningMatrices[IN.joint_indices.z] * IN.joint_weights.z;
    float4x4 joint3 = skinningMatrices[IN.joint_indices.w] * IN.joint_weights.w;
    float4x4 skinning = joint0 + joint1 + joint2 + joint3;
    VertexOutput OUT;
    matrix vp = mul(projectionMatrix, viewMatrix);
    float4 posWS = mul(mul(modelMatrix,skinning), float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vp, posWS);
    OUT.color = IN.color.rgba;
    return OUT;
}
)"
};

template <>
struct VS_src_selector<
    VSTechnique::forward_base,
    Layout_Vec3Color4BSkinned,
    Layout_CBuffer_3DScene,
    VSDrawType::Standard> {
    static const VS_src& value() { return vs_forward_base_Vec3Color4BSkinned_CBuffer_3DScene_Standard; }
};

constexpr VS_src vs_forward_base_Vec3_CBuffer_3DScene_Instanced = {
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
struct VS_src_selector<
    VSTechnique::forward_base,
    Layout_Vec3,
    Layout_CBuffer_3DScene,
    VSDrawType::Instanced> {
    static const VS_src& value() { return vs_forward_base_Vec3_CBuffer_3DScene_Instanced; }
};
constexpr VS_src vs_forward_base_TexturedVec3_CBuffer_3DScene_Standard = {
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
};
struct VertexOutput {
    float2 uv : TEXCOORD;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    matrix vp = mul(projectionMatrix, viewMatrix);
    float4 posWS = mul(modelMatrix, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vp, posWS);
    OUT.uv = IN.uv;
    return OUT;
}
)"
};

template <>
struct VS_src_selector<
    VSTechnique::forward_base,
    Layout_TexturedVec3,
    Layout_CBuffer_3DScene,
    VSDrawType::Standard> {
    static const VS_src& value() { return vs_forward_base_TexturedVec3_CBuffer_3DScene_Standard; }
};

// TODO: REWRITE
constexpr VS_src vs_forward_base_TexturedSkinnedVec3_CBuffer_3DScene_Standard = {
"forward_base_TexturedSkinnedVec3_CBuffer_3DScene_Standard",
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
	matrix skinningMatrices[256];
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
    matrix vp = mul(projectionMatrix, viewMatrix);
    float4 posWS = mul(modelMatrix, mul(skinning, float4(IN.posMS, 1.f)));
    OUT.positionCS = mul(vp, posWS);
    OUT.uv = IN.uv;
    return OUT;
}
)"
};
template <>
struct VS_src_selector<
    VSTechnique::forward_base,
    Layout_TexturedSkinnedVec3,
    Layout_CBuffer_3DScene,
    VSDrawType::Standard> {
    static const VS_src& value() { return vs_forward_base_TexturedSkinnedVec3_CBuffer_3DScene_Standard; }
};

constexpr VS_src vs_forward_base_Layout_Vec3TexturedMapped_CBuffer_3DScene_Standard = {
"forward_base_Layout_Vec3TexturedMapped_CBuffer_3DScene_Standard",
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
struct VS_src_selector<
    VSTechnique::forward_base,
    Layout_Vec3TexturedMapped,
    Layout_CBuffer_3DScene,
    VSDrawType::Standard> {
    static const VS_src& value() { return vs_forward_base_Layout_Vec3TexturedMapped_CBuffer_3DScene_Standard; }
};

constexpr VS_src vs_fullscreen_bufferless_blit = {
"fullscreen_bufferless_blit",
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

template <>
struct VS_src_selector<
    VSTechnique::fullscreen_blit,
    Layout_TexturedVec3,
    Layout_CNone,
    VSDrawType::Standard> {
    static const VS_src& value() { return vs_fullscreen_bufferless_blit; }
};

constexpr PS_src ps_forward_untextured_unlit = { // used for multiple vertex and buffer layouts
"forward_untextured_unlit",
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
struct PS_src_selector<
    PSTechnique::forward_untextured_unlit,
    Layout_Vec3,
    Layout_CNone> {
    enum { materialUsage = PSMaterialUsage::None };
    static const PS_src& value() { return ps_forward_untextured_unlit; }
};

template <>
struct PS_src_selector<
    PSTechnique::forward_untextured_unlit,
    Layout_Vec2Color4B,
    Layout_CNone> {
    enum { materialUsage = PSMaterialUsage::None };
    static const PS_src& value() { return ps_forward_untextured_unlit; }
};
template <>
struct PS_src_selector<
    PSTechnique::forward_untextured_unlit,
    Layout_Vec3Color4B,
    Layout_CNone> {
    enum { materialUsage = PSMaterialUsage::None };
    static const PS_src& value() { return ps_forward_untextured_unlit; }
};
template <>
struct PS_src_selector<
    PSTechnique::forward_untextured_unlit,
    Layout_Vec3Color4BSkinned,
    Layout_CNone> {
    enum { materialUsage = PSMaterialUsage::None };
    static const PS_src& value() { return ps_forward_untextured_unlit; }
};

const char* textured_unlit_samplers[] = { "diffuse" };
constexpr PS_src ps_forward_textured_unlit_TexturedVec3_CBuffer_3DScene = {
"forward_textured_unlit_TexturedVec3_CBuffer_3DScene",
textured_unlit_samplers,
sizeof(textured_unlit_samplers) / sizeof(textured_unlit_samplers[0]),
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
SamplerState texDiffuseSampler : register(s0);

struct VertexOut {
    float2 uv : TEXCOORD;
};
float4 PS(VertexOut IN) : SV_TARGET {

    float4 albedo = texDiffuse.Sample(texDiffuseSampler, IN.uv).rgba;
    //float3 color = albedo.a * albedo.rgb + (1.0 - albedo.a) * OUT.color.rgb;
    //return color;
    return albedo.rgba;
}
)"
};

template <>
struct PS_src_selector<
    PSTechnique::forward_textured_unlit,
    Layout_TexturedVec3,
    Layout_CNone> {
    enum { materialUsage = PSMaterialUsage::Uses };
    static const PS_src& value() { return ps_forward_textured_unlit_TexturedVec3_CBuffer_3DScene; }
};
template <>
struct PS_src_selector<
    PSTechnique::forward_textured_unlit,
    Layout_TexturedSkinnedVec3,
    Layout_CNone> {
    enum { materialUsage = PSMaterialUsage::Uses };
    static const PS_src& value() { return ps_forward_textured_unlit_TexturedVec3_CBuffer_3DScene; }
};

constexpr PS_src ps_forward_textured_unlitalphaclip_TexturedVec3_CBuffer_3DScene = {
"forward_textured_unlitalphaclip_TexturedVec3_CBuffer_3DScene",
textured_unlit_samplers,
sizeof(textured_unlit_samplers) / sizeof(textured_unlit_samplers[0]),
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
SamplerState texDiffuseSampler : register(s0);

struct VertexOut {
    float2 uv : TEXCOORD;
};
float4 PS(VertexOut IN) : SV_TARGET {

    float4 albedo = texDiffuse.Sample(texDiffuseSampler, IN.uv).rgba;
    if (albedo.a < 0.1)
        discard;
    //float3 color = albedo.a * albedo.rgb + (1.0 - albedo.a) * OUT.color.rgb;
    //return color;
    return albedo.rgba;
}
)"
};

template <>
struct PS_src_selector<
    PSTechnique::forward_textured_unlitalphaclip,
    Layout_TexturedVec3,
    Layout_CNone> {
    enum { materialUsage = PSMaterialUsage::Uses };
    static const PS_src& value() { return ps_forward_textured_unlitalphaclip_TexturedVec3_CBuffer_3DScene; }
};
template <>
struct PS_src_selector<
    PSTechnique::forward_textured_unlitalphaclip,
    Layout_TexturedSkinnedVec3,
    Layout_CNone> {
    enum { materialUsage = PSMaterialUsage::Uses };
    static const PS_src& value() { return ps_forward_textured_unlitalphaclip_TexturedVec3_CBuffer_3DScene; }
};


const char* textured_lit_normalmapped_samplers[] = { "texDiffuse", "texNormal", "texDepth" };
constexpr PS_src ps_forward_textured_lit_normalmapped_Layout_Vec3TexturedMapped_CBuffer_3DScene = {
"forward_textured_lit_normalmapped_Layout_Vec3TexturedMapped_CBuffer_3DScene",
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

template <>
struct PS_src_selector<
    PSTechnique::forward_textured_lit_normalmapped,
    Layout_Vec3TexturedMapped,
    Layout_CBuffer_3DScene> {
    enum { materialUsage = PSMaterialUsage::Uses };
    static const PS_src& value() { return ps_forward_textured_lit_normalmapped_Layout_Vec3TexturedMapped_CBuffer_3DScene; }
};

const char* fullscreen_blit_samplers[] = { "texSrc" };
constexpr PS_src ps_fullscreen_blit_textured = {
"fullscreen_blit_textured",
fullscreen_blit_samplers,
sizeof(fullscreen_blit_samplers) / sizeof(fullscreen_blit_samplers[0]),
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

template <>
struct PS_src_selector<
    PSTechnique::fullscreen_blit_textured,
    Layout_TexturedVec3,
    Layout_CNone> {
    enum { materialUsage = PSMaterialUsage::Uses };
    static const PS_src& value() { return ps_fullscreen_blit_textured; }
};

}
}
#endif // __WASTELADNS_SHADERS_DX11_H__
