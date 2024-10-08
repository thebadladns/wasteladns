#ifndef __WASTELADNS_SHADERS_GL_H__
#define __WASTELADNS_SHADERS_GL_H__

namespace Renderer {
namespace Shaders {

constexpr VS_src vs_forward_base_Vec3Color4B_CBuffer_DebugScene_Standard = {
"forward_base_Vec3Color4B_CBuffer_DebugScene_Standard",
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(std140) uniform type_PerGroup
{
    mat4 MVP;
} PerGroup;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 1) in vec4 in_var_COLOR;
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    varying_COLOR = in_var_COLOR;
    gl_Position = PerGroup.MVP * vec4(in_var_POSITION, 1.0);
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

constexpr VS_src vs_forward_base_Vec3_CBuffer_3DScene_Standard = {
"forward_base_Vec3_CBuffer_3DScene_Sandard",
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

template <>
struct VS_src_selector<
    VSTechnique::forward_base,
    Layout_Vec3,
    Layout_CBuffer_3DScene,
    VSDrawType::Standard> {
    static const VS_src& value() { return vs_forward_base_Vec3_CBuffer_3DScene_Standard; }
};

constexpr VS_src vs_forward_base_Vec3Color4B_CBuffer_3DScene_Standard = {
"forward_base_Vec3Color4B_CBuffer_3DScene_Standard",
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
    mat4 skinningMatrices[256];
} PerInstance;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 1) in vec4 in_var_COLOR;
layout(location = 2) in vec4 in_var_JOINTINDICES;
layout(location = 3) in vec4 in_var_JOINTWEIGHTS;
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    varying_COLOR = in_var_COLOR;
    mat4 joint0 = PerInstance.skinningMatrices[int(in_var_JOINTINDICES.x)] * in_var_JOINTWEIGHTS.x;
    mat4 joint1 = PerInstance.skinningMatrices[int(in_var_JOINTINDICES.y)] * in_var_JOINTWEIGHTS.y;
    mat4 joint2 = PerInstance.skinningMatrices[int(in_var_JOINTINDICES.z)] * in_var_JOINTWEIGHTS.z;
    mat4 joint3 = PerInstance.skinningMatrices[int(in_var_JOINTINDICES.w)] * in_var_JOINTWEIGHTS.w;
    mat4 skinning = joint0 + joint1 + joint2 + joint3;
    gl_Position = (PerScene.projectionMatrix * PerScene.viewMatrix) * ((PerGroup.modelMatrix * skinning) * vec4(in_var_POSITION, 1.0));
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
    mat4 instanceMatrices[256];
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
    mat4 skinningMatrices[256];
} PerInstance;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 1) in vec2 in_var_TEXCOORD;
layout(location = 2) in vec4 in_var_JOINTINDICES;
layout(location = 3) in vec4 in_var_JOINTWEIGHTS;
layout(location = 0) out vec2 varying_TEXCOORD;

void main()
{
    varying_TEXCOORD = in_var_TEXCOORD;
    mat4 joint0 = PerInstance.skinningMatrices[int(in_var_JOINTINDICES.x)] * in_var_JOINTWEIGHTS.x;
    mat4 joint1 = PerInstance.skinningMatrices[int(in_var_JOINTINDICES.y)] * in_var_JOINTWEIGHTS.y;
    mat4 joint2 = PerInstance.skinningMatrices[int(in_var_JOINTINDICES.z)] * in_var_JOINTWEIGHTS.z;
    mat4 joint3 = PerInstance.skinningMatrices[int(in_var_JOINTINDICES.w)] * in_var_JOINTWEIGHTS.w;
    mat4 skinning = joint0 + joint1 + joint2 + joint3;
    gl_Position = (PerScene.projectionMatrix * PerScene.viewMatrix) * ((PerGroup.modelMatrix * skinning) * vec4(in_var_POSITION, 1.0));
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

constexpr VS_src vs_forward_base_Vec3TexturedMapped_CBuffer_3DScene_Standard = {
"forward_base_Vec3TexturedMapped_CBuffer_3DScene_Standard",
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
layout(location = 2) in vec3 in_var_NORMAL;
layout(location = 3) in vec3 in_var_TANGENT;
layout(location = 4) in vec3 in_var_BITANGENT;
layout(location = 0) out vec3 varying_POSITION;
layout(location = 1) out vec2 varying_TEXCOORD;
layout(location = 2) out mat3 varying_TBN;

void main()
{
    vec4 posWS = PerGroup.modelMatrix * vec4(in_var_POSITION, 1.f);
    vec3 normalWS = normalize(PerGroup.modelMatrix * vec4(in_var_NORMAL, 0.f)).xyz;
    vec3 tangentWS = normalize(PerGroup.modelMatrix * vec4(in_var_TANGENT, 0.f)).xyz;
    vec3 bitangentWS = normalize(PerGroup.modelMatrix * vec4(in_var_BITANGENT, 0.f)).xyz;
    
    // load vectors in the rows (transposed, becomes the inverse)
    varying_TBN = mat3(tangentWS, bitangentWS, normalWS);
    mat3 invtbn = transpose(varying_TBN);
    varying_TEXCOORD = in_var_TEXCOORD;
    varying_POSITION = posWS.xyz;
    mat4 vp = PerScene.projectionMatrix * PerScene.viewMatrix;
    gl_Position = vp * posWS;
}
)"
};

template <>
struct VS_src_selector<
    VSTechnique::forward_base,
    Layout_Vec3TexturedMapped,
    Layout_CBuffer_3DScene,
    VSDrawType::Standard> {
    static const VS_src& value() { return vs_forward_base_Vec3TexturedMapped_CBuffer_3DScene_Standard; }
};

constexpr VS_src vs_fullscreen_bufferless_blit = {
"fullscreen_bufferless_blit",
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

layout(location = 0) out vec2 varying_TEXCOORD;
 
void main()
{
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = 1.0 - float((gl_VertexID & 2) << 1);
    varying_TEXCOORD.x = (x+1.0)*0.5;
    varying_TEXCOORD.y = (y+1.0)*0.5;
    gl_Position = vec4(x, y, 0, 1);
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
#version 330
#extension GL_ARB_separate_shader_objects : require

layout(location = 0) in vec4 varying_COLOR;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    out_var_SV_TARGET = varying_COLOR;
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

const char* textured_unlit_samplers[] = { "texDiffuse" };
constexpr PS_src ps_forward_textured_unlit_TexturedVec3_CBuffer_3DScene = {
"forward_textured_unlit_TexturedVec3_CBuffer_3DScene",
textured_unlit_samplers,
sizeof(textured_unlit_samplers) / sizeof(textured_unlit_samplers[0]),
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
constexpr PS_src ps_forward_textured_lit_normalmapped_Vec3TexturedMapped_CBuffer_3DScene = {
"forward_textured_lit_normalmapped_TexturedVec3_CBuffer_3DScene",
textured_lit_normalmapped_samplers,
sizeof(textured_lit_normalmapped_samplers) / sizeof(textured_lit_normalmapped_samplers[0]),
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
uniform sampler2D texNormal;
// uniform sampler2D texDepth;

layout(location = 0) in vec3 varying_POSITION;
layout(location = 1) in vec2 varying_TEXCOORD;
layout(location = 2) in mat3 varying_TBN;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    vec3 lightWS = PerScene.lightPosWS;
    vec3 albedo = texture(texDiffuse, varying_TEXCOORD).rgb;
    vec3 lightDir = normalize(lightWS - varying_POSITION);
    vec3 normalWS = texture(texNormal, varying_TEXCOORD).xyz;
    normalWS = normalWS * 2.f - 1.f;
    normalWS = normalize(varying_TBN * normalWS);
    
    // Aggressive gooch-like shading
    vec3 viewWS = PerScene.viewPosWS.xyz;
    vec3 viewDirWS = normalize(viewWS - varying_POSITION);
    float diff = dot(lightDir, normalWS);
    float t = ( diff + 1.f ) * 0.5f;
    vec3 r = 2.f * diff * normalWS - lightDir;
    float s = clamp(100.f * dot(r, viewDirWS) - 97.f, 0.f, 0.8f);
    vec3 ccool = vec3(0.f, 0.f, 0.3f) * albedo;
    vec3 cwarm = clamp(vec3(1.2f, 1.2f, 1.f) * albedo, 0.f, 1.f);
    vec3 diffuse = 0.7f * diff * albedo;
    vec3 ambient = 0.3f * albedo;

    out_var_SV_TARGET = vec4(s * vec3(1.f,1.f,1.f) + (1 - s)*(t*cwarm + (1 - t)*ccool), 1.f);
}
)"
};

template <>
struct PS_src_selector<
    PSTechnique::forward_textured_lit_normalmapped,
    Layout_Vec3TexturedMapped,
    Layout_CBuffer_3DScene> {
    enum { materialUsage = PSMaterialUsage::Uses };
    static const PS_src& value() { return ps_forward_textured_lit_normalmapped_Vec3TexturedMapped_CBuffer_3DScene; }
};

const char* fullscreen_blit_samplers[] = { "texSrc" };
constexpr PS_src ps_fullscreen_blit_textured = {
"fullscreen_blit_textured",
fullscreen_blit_samplers,
sizeof(fullscreen_blit_samplers) / sizeof(fullscreen_blit_samplers[0]),
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

uniform sampler2D texSrc;

layout(location = 0) in vec2 varying_TEXCOORD;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    vec4 diffuse = texture(texSrc, varying_TEXCOORD).rgba;
    out_var_SV_TARGET = diffuse.rgba;
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
#endif // __WASTELADNS_SHADERS_GL_H__
