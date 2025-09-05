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
