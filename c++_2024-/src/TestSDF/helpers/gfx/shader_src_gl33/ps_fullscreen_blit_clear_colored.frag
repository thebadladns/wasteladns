#version 330
#extension GL_ARB_separate_shader_objects : require

layout(std140) uniform type_BlitColor
{
    vec4 color;
} BlitColor;

layout(location = 0) in vec2 varying_TEXCOORD;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    out_var_SV_TARGET = BlitColor.color;
}
