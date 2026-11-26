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
layout(std140) uniform type_PerJoint
{
    mat4 skinningMatrices[32];
} PerJoint;

layout(location = 0) in vec3 in_var_POSITION;
layout(location = 1) in vec2 in_var_TEXCOORD;
layout(location = 2) in vec4 in_var_JOINTINDICES;
layout(location = 3) in vec4 in_var_JOINTWEIGHTS;
layout(location = 0) out vec2 varying_TEXCOORD;

void main()
{
    varying_TEXCOORD = in_var_TEXCOORD;
    mat4 joint0 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.x)] * in_var_JOINTWEIGHTS.x;
    mat4 joint1 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.y)] * in_var_JOINTWEIGHTS.y;
    mat4 joint2 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.z)] * in_var_JOINTWEIGHTS.z;
    mat4 joint3 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.w)] * in_var_JOINTWEIGHTS.w;
    mat4 skinning = joint0 + joint1 + joint2 + joint3;
    gl_Position = PerScene.vpMatrix * ((PerGroup.modelMatrix * skinning) * vec4(in_var_POSITION, 1.0));
}
