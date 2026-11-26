#version 330
#extension GL_ARB_separate_shader_objects : require

layout(std140) uniform type_PerGroup
{
    mat4 modelMatrix;
    vec4 groupColor;
} PerGroup;

uniform sampler2D texDiffuse;

layout(location = 0) in vec2 varying_TEXCOORD;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    vec4 diffuse = texture(texDiffuse, varying_TEXCOORD).rgba;
    diffuse = diffuse * PerGroup.groupColor;
    if (diffuse.w < 0.1)
        discard;
    //vec3 color = diffuse.a * diffuse.rgb + (1.0-diffuse.a) * varying_COLOR.rgb;
    //out_var_SV_TARGET = color;
    out_var_SV_TARGET = diffuse.rgba;
}
