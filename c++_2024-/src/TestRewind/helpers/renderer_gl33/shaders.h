#ifndef __WASTELADNS_SHADERS_GL_H__
#define __WASTELADNS_SHADERS_GL_H__

namespace renderer {
namespace shaders {

constexpr VS_src vs_color3d_unlit = {
"vs_color3d_unlit",
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

constexpr VS_src vs_fullscreen_bufferless_clear_blit = {
"vs_fullscreen_bufferless_clear_blit",
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

void main()
{
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = 1.0 - float((gl_VertexID & 2) << 1);
    gl_Position = vec4(x, y, 1.0, 1.0); // clear depth
}
)"
};
constexpr VS_src vs_fullscreen_bufferless_textured_blit = {
"vs_fullscreen_bufferless_textured_blit",
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

constexpr PS_src ps_color3d_unlit = { // used for multiple vertex and buffer layouts
"ps_color3d_unlit",
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
constexpr PS_src ps_fullscreen_blit_white = {
"ps_fullscreen_blit_white",
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

uniform sampler2D texSrc;

layout(location = 0) in vec2 varying_TEXCOORD;
layout(location = 0) out vec4 out_var_SV_TARGET;

void main()
{
    out_var_SV_TARGET = vec4(1.0);
}
)"
};
constexpr PS_src ps_fullscreen_blit_textured = {
"ps_fullscreen_blit_textured",
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

}
}
#endif // __WASTELADNS_SHADERS_GL_H__
