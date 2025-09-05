#version 330
#extension GL_ARB_separate_shader_objects : require

uniform sampler2D texSrc;
uniform sampler2D depthSrc;

layout(location = 0) in vec2 varying_TEXCOORD;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    vec4 diffuse = texture(texSrc, varying_TEXCOORD).rgba;
    float depth = texture(depthSrc, varying_TEXCOORD).r;
    out_var_SV_TARGET = diffuse.rgba;
    gl_FragDepth = depth;
}
