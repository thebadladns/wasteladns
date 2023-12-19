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
