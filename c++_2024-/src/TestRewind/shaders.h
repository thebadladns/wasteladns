#ifndef __WASTELADNS_SHADERS_H__
#define __WASTELADNS_SHADERS_H__

namespace Renderer {
namespace Shaders {

#if __DX11

constexpr VS_src vs_3d_base = {
"vs_3d_base",
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

constexpr VS_src vs_color3d_base = {
"vs_color3d_base",
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

constexpr VS_src vs_color3d_skinned_base = {
"vs_color3d_skinned_base",
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
cbuffer type_PerJoint : register(b2) {
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

constexpr VS_src vs_3d_instanced_base = {
"vs_3d_instanced_base"
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
    matrix vp = mul(projectionMatrix, viewMatrix);
    float4 posWS = mul(mm, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vp, posWS);
    OUT.color = groupColor;
    return OUT;
}
)"
};

constexpr VS_src vs_textured3d_base = {
"vs_textured3d_base",
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

constexpr VS_src vs_textured3d_skinned_base = {
"vs_textured3d_skinned_base",
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
cbuffer type_PerJoint : register(b2) {
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

constexpr PS_src ps_textured3d_base = {
"ps_textured3d_base",
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

constexpr PS_src ps_textured3dalphaclip_base = {
"ps_textured3dalphaclip_base",
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
cbuffer type_PerJoint : register(b2) {
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

#elif __GL33

constexpr VS_src vs_3d_base = {
"vs_3d_base",
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(std140) uniform type_PerScene
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 viewPosWS;
    float padding1;
    vec3 lightPosWS;
    float padding2;
} PerScene;

layout(std140) uniform type_PerGroup
{
    mat4 modelMatrix;
    vec4 groupColor;
} PerGroup;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    varying_COLOR = PerGroup.groupColor;
    gl_Position = (PerScene.projectionMatrix * PerScene.viewMatrix) * (PerGroup.modelMatrix * vec4(in_var_POSITION, 1.0));
}
)"
};

constexpr VS_src vs_color3d_base = {
"vs_color3d_base",
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(std140) uniform type_PerScene
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 viewPosWS;
    float padding1;
    vec3 lightPosWS;
    float padding2;
} PerScene;

layout(std140) uniform type_PerGroup
{
    mat4 modelMatrix;
    vec4 groupColor;
} PerGroup;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 1) in vec4 in_var_COLOR;
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    varying_COLOR = in_var_COLOR;
    gl_Position = (PerScene.projectionMatrix * PerScene.viewMatrix) * (PerGroup.modelMatrix * vec4(in_var_POSITION, 1.0));
}
)"
};

constexpr VS_src vs_color3d_skinned_base = {
"vs_color3d_skinned_base",
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(std140) uniform type_PerScene
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 viewPosWS;
    float padding1;
    vec3 lightPosWS;
    float padding2;
} PerScene;

layout(std140) uniform type_PerGroup
{
    mat4 modelMatrix;
    vec4 groupColor;
} PerGroup;
layout(std140) uniform type_PerJoint
{
    mat4 skinningMatrices[256];
} PerJoint;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 1) in vec4 in_var_COLOR;
layout(location = 2) in vec4 in_var_JOINTINDICES;
layout(location = 3) in vec4 in_var_JOINTWEIGHTS;
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    varying_COLOR = in_var_COLOR;
    mat4 joint0 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.x)] * in_var_JOINTWEIGHTS.x;
    mat4 joint1 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.y)] * in_var_JOINTWEIGHTS.y;
    mat4 joint2 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.z)] * in_var_JOINTWEIGHTS.z;
    mat4 joint3 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.w)] * in_var_JOINTWEIGHTS.w;
    mat4 skinning = joint0 + joint1 + joint2 + joint3;
    gl_Position = (PerScene.projectionMatrix * PerScene.viewMatrix) * ((PerGroup.modelMatrix * skinning) * vec4(in_var_POSITION, 1.0));
}
)"
};

constexpr VS_src vs_3d_instanced_base = {
"vs_3d_instanced_base"
, R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(std140) uniform type_PerScene
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 viewPosWS;
    float padding1;
    vec3 lightPosWS;
    float padding2;
} PerScene;

layout(std140) uniform type_PerGroup
{
    mat4 modelMatrix;
    vec4 groupColor;
} PerGroup;

layout(std140) uniform type_PerInstance
{
    mat4 instanceMatrices[64];
} PerInstance;

layout(location = 0) in vec3 in_var_POSITION;
uniform int SPIRV_Cross_BaseInstance;
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    mat4 mm = PerInstance.instanceMatrices[uint(gl_InstanceID + SPIRV_Cross_BaseInstance)] * PerGroup.modelMatrix;
    mat4 vp = PerScene.projectionMatrix * PerScene.viewMatrix;
    vec4 posWS = mm * vec4(in_var_POSITION, 1.0);
    gl_Position = vp * posWS;
    varying_COLOR = PerGroup.groupColor;
}
)"
};

constexpr VS_src vs_textured3d_base = {
"vs_textured3d_base",
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(std140) uniform type_PerScene
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 viewPosWS;
    float padding1;
    vec3 lightPosWS;
    float padding2;
} PerScene;

layout(std140) uniform type_PerGroup
{
    mat4 modelMatrix;
    vec4 groupColor;
} PerGroup;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 1) in vec2 in_var_TEXCOORD;
layout(location = 0) out vec2 varying_TEXCOORD;

void main()
{
    varying_TEXCOORD = in_var_TEXCOORD;
    gl_Position = (PerScene.projectionMatrix * PerScene.viewMatrix) * (PerGroup.modelMatrix * vec4(in_var_POSITION, 1.0));
}
)"
};

constexpr VS_src vs_textured3d_skinned_base = {
"vs_textured3d_skinned_base",
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

out gl_PerVertex
{
    vec4 gl_Position;
};
layout(std140) uniform type_PerScene
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 viewPosWS;
    float padding1;
    vec3 lightPosWS;
    float padding2;
} PerScene;
layout(std140) uniform type_PerGroup
{
    mat4 modelMatrix;
    vec4 groupColor;
} PerGroup;
layout(std140) uniform type_PerJoint
{
    mat4 skinningMatrices[256];
} PerJoint;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 1) in vec2 in_var_TEXCOORD;
layout(location = 2) in vec4 in_var_JOINTINDICES;
layout(location = 3) in vec4 in_var_JOINTWEIGHTS;
layout(location = 0) out vec2 varying_TEXCOORD;

void main()
{
    varying_TEXCOORD = in_var_TEXCOORD;
    mat4 joint0 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.x)] * in_var_JOINTWEIGHTS.x;
    mat4 joint1 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.y)] * in_var_JOINTWEIGHTS.y;
    mat4 joint2 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.z)] * in_var_JOINTWEIGHTS.z;
    mat4 joint3 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.w)] * in_var_JOINTWEIGHTS.w;
    mat4 skinning = joint0 + joint1 + joint2 + joint3;
    gl_Position = (PerScene.projectionMatrix * PerScene.viewMatrix) * ((PerGroup.modelMatrix * skinning) * vec4(in_var_POSITION, 1.0));
}
)"
};

constexpr PS_src ps_textured3d_base = {
"ps_textured3d_base",
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

layout(std140) uniform type_PerScene
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 viewPosWS;
    float padding1;
    vec3 lightPosWS;
    float padding2;
} PerScene;

uniform sampler2D texDiffuse;

layout(location = 0) in vec2 varying_TEXCOORD;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    vec4 diffuse = texture(texDiffuse, varying_TEXCOORD).rgba;
    //vec3 color = diffuse.a * diffuse.rgb + (1.0-diffuse.a) * varying_COLOR.rgb;
    //out_var_SV_TARGET = color;
    out_var_SV_TARGET = diffuse.rgba;
}
)"
};

constexpr PS_src ps_textured3dalphaclip_base = {
"forward_textured_unlitalphaclip_TexturedVec3_CBuffer_3DScene",
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

layout(std140) uniform type_PerScene
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
    vec3 viewPosWS;
    float padding1;
    vec3 lightPosWS;
    float padding2;
} PerScene;

uniform sampler2D texDiffuse;

layout(location = 0) in vec2 varying_TEXCOORD;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    vec4 diffuse = texture(texDiffuse, varying_TEXCOORD).rgba;
    if (diffuse.w < 0.1)
        discard;
    //vec3 color = diffuse.a * diffuse.rgb + (1.0-diffuse.a) * varying_COLOR.rgb;
    //out_var_SV_TARGET = color;
    out_var_SV_TARGET = diffuse.rgba;
}
)"
};
#endif

}
}
#endif // __WASTELADNS_SHADERS_H__
