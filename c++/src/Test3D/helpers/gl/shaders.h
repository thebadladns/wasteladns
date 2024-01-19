#ifndef __WASTELADNS_SHADERS_GL_H__
#define __WASTELADNS_SHADERS_GL_H__

const char* coloredVertexShaderStr = R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(std140) uniform type_PerGroup
{
    layout(row_major) mat4 MVP;
} PerGroup;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 1) in vec4 in_var_COLOR;
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    varying_COLOR = in_var_COLOR;
    gl_Position = vec4(in_var_POSITION, 1.0) * PerGroup.MVP;
}

)";

const char* defaultPixelShaderStr = R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

layout(location = 0) in vec4 varying_COLOR;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    out_var_SV_TARGET = varying_COLOR;
}

)";

const char* defaultVertexShaderStr = R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(std140) uniform type_PerScene
{
    layout(row_major) mat4 projectionMatrix;
    layout(row_major) mat4 viewMatrix;
    vec3 viewPosWS;
    float padding;
} PerScene;

layout(std140) uniform type_PerGroup
{
    layout(row_major) mat4 modelMatrix;
    vec4 groupColor;
} PerGroup;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    varying_COLOR = PerGroup.groupColor;
    gl_Position = (vec4(in_var_POSITION, 1.0) * PerGroup.modelMatrix) * (PerScene.viewMatrix * PerScene.projectionMatrix);
}

)";

const char* texturedVertexShaderStr = R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(std140) uniform type_PerScene
{
    layout(row_major) mat4 projectionMatrix;
    layout(row_major) mat4 viewMatrix;
    vec3 viewPosWS;
    float padding;
} PerScene;

layout(std140) uniform type_PerGroup
{
    layout(row_major) mat4 modelMatrix;
    vec4 groupColor;
} PerGroup;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 1) in vec2 in_var_TEXCOORD;
layout(location = 2) in vec3 in_var_NORMAL;
layout(location = 3) in vec3 in_var_TANGENT;
layout(location = 4) in vec3 in_var_BITANGENT;
layout(location = 0) out vec3 varying_POSITION;
layout(location = 1) out vec3 varying_NORMAL;
layout(location = 2) out vec3 varying_VIEW_TS;
layout(location = 3) out vec3 varying_POSITION_TS;
layout(location = 4) out vec2 varying_TEXCOORD;
layout(location = 5) out mat3 varying_TBN;

void main()
{
    vec4 _60 = vec4(in_var_POSITION, 1.0) * PerGroup.modelMatrix;
    vec3 _61 = _60.xyz;
    vec4 _67 = normalize(vec4(in_var_NORMAL, 0.0) * PerGroup.modelMatrix);
    vec4 _74 = normalize(vec4(in_var_TANGENT, 0.0) * PerGroup.modelMatrix);
    vec4 _80 = normalize(vec4(in_var_BITANGENT, 0.0) * PerGroup.modelMatrix);
    mat3 _93 = mat3(vec3(_74.x, _80.x, _67.x), vec3(_74.y, _80.y, _67.y), vec3(_74.z, _80.z, _67.z));
    mat3 _94 = transpose(_93);
    varying_POSITION = _61;
    varying_NORMAL = _67.xyz;
    varying_VIEW_TS = PerScene.viewPosWS * _94;
    varying_POSITION_TS = _61 * _94;
    varying_TEXCOORD = in_var_TEXCOORD;
    gl_Position = vec4(_60.xyz, 1.0) * (PerScene.viewMatrix * PerScene.projectionMatrix);
    varying_TBN = _93;
}

)";

const char* texturedPixelShaderStr = R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

layout(std140) uniform type_PerScene
{
    layout(row_major) mat4 projectionMatrix;
    layout(row_major) mat4 viewMatrix;
    vec3 viewPosWS;
    float padding;
} PerScene;

uniform sampler2D SPIRV_Cross_CombinedtexDiffusetexDiffuseSampler;
uniform sampler2D SPIRV_Cross_CombinedtexNormaltexNormalSampler;

layout(location = 0) in vec3 varying_POSITION;
layout(location = 1) in vec3 varying_NORMAL;
layout(location = 2) in vec3 varying_VIEW_TS;
layout(location = 3) in vec3 varying_POSITION_TS;
layout(location = 4) in vec2 varying_TEXCOORD;
layout(location = 5) in mat3 varying_TBN;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    vec3 _64 = texture(SPIRV_Cross_CombinedtexDiffusetexDiffuseSampler, varying_TEXCOORD).xyz;
    vec3 _66 = normalize(vec3(3.0, 8.0, 15.0) - varying_POSITION);
    vec3 _75 = normalize(((texture(SPIRV_Cross_CombinedtexNormaltexNormalSampler, varying_TEXCOORD).xyz * 2.0) - vec3(1.0)) * varying_TBN);
    float _80 = dot(_66, _75);
    float _82 = (_80 + 1.0) * 0.5;
    float _89 = clamp((100.0 * dot((_75 * (2.0 * _80)) - _66, normalize(PerScene.viewPosWS - varying_POSITION))) - 97.0, 0.0, 0.800000011920928955078125);
    out_var_SV_TARGET = vec4((vec3(1.0) * _89) + (((clamp(vec3(1.2000000476837158203125, 1.2000000476837158203125, 1.0) * _64, vec3(0.0), vec3(1.0)) * _82) + ((vec3(0.0, 0.0, 0.300000011920928955078125) * _64) * (1.0 - _82))) * (1.0 - _89)), 1.0);
}

)";

const char* defaultInstancedVertexShaderStr = R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(std140) uniform type_PerScene
{
    layout(row_major) mat4 projectionMatrix;
    layout(row_major) mat4 viewMatrix;
    vec3 viewPosWS;
    float padding;
} PerScene;

layout(std140) uniform type_PerGroup
{
    layout(row_major) mat4 modelMatrix;
    vec4 groupColor;
} PerGroup;

layout(std140) uniform type_PerInstance
{
    layout(row_major) mat4 instanceMatrices[256];
} PerInstance;

layout(location = 0) in vec3 in_var_POSITION;
uniform int SPIRV_Cross_BaseInstance;
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    varying_COLOR = PerGroup.groupColor;
    gl_Position = (vec4(in_var_POSITION, 1.0) * (PerGroup.modelMatrix * PerInstance.instanceMatrices[uint((gl_InstanceID + SPIRV_Cross_BaseInstance))])) * (PerScene.viewMatrix * PerScene.projectionMatrix);
}

)";



#endif // __WASTELADNS_SHADERS_GL_H__
