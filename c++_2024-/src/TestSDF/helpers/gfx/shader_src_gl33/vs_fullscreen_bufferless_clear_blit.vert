#version 330
#extension GL_ARB_separate_shader_objects : require

void main()
{
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = 1.0 - float((gl_VertexID & 2) << 1);
    gl_Position = vec4(x, y, 1.0, 1.0); // clear depth
}