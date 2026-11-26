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

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 1) in vec2 in_var_TEXCOORD;
layout(location = 0) out vec2 varying_TEXCOORD;

void main()
{
    varying_TEXCOORD = in_var_TEXCOORD;
    gl_Position = PerScene.vpMatrix * (PerGroup.modelMatrix * vec4(in_var_POSITION, 1.0));
}
