#version 330
#extension GL_ARB_separate_shader_objects : require

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(std140) uniform type_PerScene
{
    mat4 vpMatrix;
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
    vec4 posWS = mm * vec4(in_var_POSITION, 1.0);
    gl_Position = PerScene.vpMatrix * posWS;
    varying_COLOR = PerGroup.groupColor;
}
