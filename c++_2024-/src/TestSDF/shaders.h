#ifndef __WASTELADNS_SHADERS_H__
#define __WASTELADNS_SHADERS_H__

namespace gfx {
namespace shaders {

#if __DX11

constexpr VS_src vs_3d_base = {
"vs_3d_base",
R"(
cbuffer PerScene : register(b0) {
    matrix vpMatrix;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
struct AppData {
    float3 posMS : POSITION;
};
struct VertexOutput {
    float4 color : COLOR;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    float4 posWS = mul(modelMatrix, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vpMatrix, posWS);
    OUT.color = groupColor;
    return OUT;
}
)"
};

constexpr VS_src vs_color2d_base = {
"vs_color3d_base",
R"(
cbuffer PerScene : register(b0) {
    matrix vpMatrix;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
struct AppData {
    float2 posMS : POSITION;
    float4 color : COLOR;
};
struct VertexOutput {
    float4 color : COLOR;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    float4 posWS = mul(modelMatrix, float4(IN.posMS, 0.f, 1.f));
    OUT.positionCS = mul(vpMatrix, posWS);
    OUT.color = IN.color.rgba * groupColor;
    return OUT;
}
)"
};

constexpr VS_src vs_color3d_base = {
"vs_color3d_base",
R"(
cbuffer PerScene : register(b0) {
    matrix vpMatrix;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
struct AppData {
    float3 posMS : POSITION;
    float4 color : COLOR;
};
struct VertexOutput {
    float4 color : COLOR;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    float4 posWS = mul(modelMatrix, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vpMatrix, posWS);
    OUT.color = IN.color.rgba * groupColor;
    return OUT;
}
)"
};

constexpr VS_src vs_color3d_skinned_base = {
"vs_color3d_skinned_base",
R"(
cbuffer PerScene : register(b0) {
    matrix vpMatrix;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
cbuffer type_PerJoint : register(b2) {
	matrix skinningMatrices[32];
};
struct AppData {
    float3 posMS : POSITION;
    float4 color : COLOR;
    int4 joint_indices : JOINTINDICES;
    float4 joint_weights : JOINTWEIGHTS;
};
struct VertexOutput {
    float4 color : COLOR;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    float4x4 joint0 = skinningMatrices[IN.joint_indices.x] * IN.joint_weights.x;
    float4x4 joint1 = skinningMatrices[IN.joint_indices.y] * IN.joint_weights.y;
    float4x4 joint2 = skinningMatrices[IN.joint_indices.z] * IN.joint_weights.z;
    float4x4 joint3 = skinningMatrices[IN.joint_indices.w] * IN.joint_weights.w;
    float4x4 skinning = joint0 + joint1 + joint2 + joint3;
    VertexOutput OUT;
    float4 posWS = mul(mul(modelMatrix,skinning), float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vpMatrix, posWS);
    OUT.color = IN.color.rgba * groupColor;
    return OUT;
}
)"
};

constexpr VS_src vs_3d_instanced_base = {
"vs_3d_instanced_base"
, R"(
cbuffer PerScene : register(b0) {
    matrix vpMatrix;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
cbuffer PerInstance : register(b2) {
	matrix instanceMatrices[64];
};
struct AppData {
    float3 posMS : POSITION;
    uint instanceID : SV_InstanceID;
};
struct VertexOutput {
    float4 color : COLOR;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    matrix instanceMatrix = instanceMatrices[IN.instanceID];
    matrix mm = mul(instanceMatrix, modelMatrix);
    float4 posWS = mul(mm, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vpMatrix, posWS);
    OUT.color = groupColor;
    return OUT;
}
)"
};

constexpr VS_src vs_textured3d_base = {
"vs_textured3d_base",
R"(
cbuffer PerScene : register(b0) {
    matrix vpMatrix;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
struct AppData {
    float3 posMS : POSITION;
    float2 uv : TEXCOORD;
};
struct VertexOutput {
    float2 uv : TEXCOORD;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    float4 posWS = mul(modelMatrix, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vpMatrix, posWS);
    OUT.uv = IN.uv;
    return OUT;
}
)"
};

constexpr VS_src vs_textured3d_skinned_base = {
"vs_textured3d_skinned_base",
R"(
cbuffer PerScene : register(b0) {
    matrix vpMatrix;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrix;
    float4 groupColor;
}
cbuffer type_PerJoint : register(b2) {
	matrix skinningMatrices[32];
};
struct AppData {
    float3 posMS : POSITION;
    float2 uv : TEXCOORD;
    int4 joint_indices : JOINTINDICES;
    float4 joint_weights : JOINTWEIGHTS;
};
struct VertexOutput {
    float2 uv : TEXCOORD;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    float4x4 joint0 = skinningMatrices[IN.joint_indices.x] * IN.joint_weights.x;
    float4x4 joint1 = skinningMatrices[IN.joint_indices.y] * IN.joint_weights.y;
    float4x4 joint2 = skinningMatrices[IN.joint_indices.z] * IN.joint_weights.z;
    float4x4 joint3 = skinningMatrices[IN.joint_indices.w] * IN.joint_weights.w;
    float4x4 skinning = joint0 + joint1 + joint2 + joint3;
    VertexOutput OUT;
    float4 posWS = mul(modelMatrix, mul(skinning, float4(IN.posMS, 1.f)));
    OUT.positionCS = mul(vpMatrix, posWS);
    OUT.uv = IN.uv;
    return OUT;
}
)"
};

constexpr PS_src ps_textured3d_base = {
"ps_textured3d_base",
R"(
cbuffer PerGroup : register(b0) {
    matrix modelMatrix;
    float4 groupColor;
}

Texture2D texDiffuse : register(t0);
SamplerState texDiffuseSampler : register(s0);

struct VertexOut {
    float2 uv : TEXCOORD;
};
float4 PS(VertexOut IN) : SV_TARGET {

    float4 albedo = texDiffuse.Sample(texDiffuseSampler, IN.uv).rgba;
    albedo = albedo * groupColor;
    //float3 color = albedo.a * albedo.rgb + (1.0 - albedo.a) * OUT.color.rgb;
    //return color;
    return albedo.rgba;
}
)"
};

constexpr PS_src ps_textured3dalphaclip_base = {
"ps_textured3dalphaclip_base",
R"(
cbuffer PerGroup : register(b0) {
    matrix modelMatrix;
    float4 groupColor;
}
Texture2D texDiffuse : register(t0);
SamplerState texDiffuseSampler : register(s0);

struct VertexOut {
    float2 uv : TEXCOORD;
};
float4 PS(VertexOut IN) : SV_TARGET {

    float4 albedo = texDiffuse.Sample(texDiffuseSampler, IN.uv).rgba;
    albedo = albedo * groupColor;
    if (albedo.a < 0.1)
        discard;
    //float3 color = albedo.a * albedo.rgb + (1.0 - albedo.a) * OUT.color.rgb;
    //return color;
    return albedo.rgba;
}
)"
};

constexpr PS_src ps_fullscreen_blit_sdf = {
"ps_fullscreen_blit_sdf",
R"(
cbuffer BlitSDF : register(b0) {
    float4 viewMatrix0;
    float4 viewMatrix1;
    float4 viewMatrix2;
    float4 viewMatrix3;
    float4 proj_row2;
    float tanfov; float aspect; float near; float far;
    float time; float platformSize; float2 padding;
}

struct VertexOut {
    float2 uv : TEXCOORD0;
    float4 positionCS : SV_POSITION;
};
struct PS_OUT
{
    float4 color : SV_Target;
    float depth : SV_Depth;
};

float sdSphere( float3 p, float s )
{
  return length(p)-s;
}

float sdBox( float3 p, float3 b ) {
  float3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdCone( float3 p, float2 c, float h )
{
  float q = length(p.xz);
  return max(dot(c.xy,float2(q,p.y)),-h-p.y);
}

float sdThickDisk( float3 p, float3 n, in float r, in float thick )  
{
    float3 d = dot(p, n) * n;
    float3 o = p - d;
    o -= normalize( o ) * min( length( o ), r );
    return length( d + o ) - thick;
}

float2 sdBird(float3 pos) {

    float t = frac(0.7*time);
    
    float y = 0.7 + 0.2 * t * (1.0 - t);
    float3 center = float3(0.0, y, 0.0);
    float3 q = pos - center;
    
    // head
    float3 head = q - float3(-0.027, 0.685, 0.0);
    float d = sdSphere(head, 0.409);
    float2 res = float2(d, 2.0);
    
    // eyes
    float3 headSymm = float3(head.xy, abs(head.z));
    float d1 = sdSphere(headSymm - float3(0.0, 0.0, 0.2), 0.262);
    if (d1 < res.x) { res.y = 3.0; }
    d1 = sdSphere(headSymm - float3(0.0, 0.0, 0.35), 0.098);
    if (d1 < res.x) { res.y = 4.0; }
    
    // beak
    float3 cone = q - float3(0.77, 0.4, 0.0);
    cone.xy = mul(float2x2(-2, -6, 6, -2)/6.3245,cone.xy);
    float d2 = sdCone(cone, float2(0.6, 0.25), 0.528);
    if (d2 < res.x) { res = float2(d2, 5.0); }
    
    // body
    float d3 = sdBox(q - float3(-0.012, 0.01, 0), float3(0.123, 0.214, 0.14));
    if (d3 < res.x) { res = float2(d3, 6.0); }
    d3 = sdBox(q - float3(0.002, -0.1, 0), float3(0.203, 0.031, 0.203));
    if (d3 < res.x) { res.y = 7.0; }
    
    float3 bodySymm = float3(q.xy, abs(q.z));
    
    // arms
    float angle = -0.1 - 0.5 * t * (1.0 - t);
    //float angle = -0.2 - 0.15 * t;
    float sa = sin(angle);
    float ca = cos(angle);
    float3 u = float3(1.0, 0.0, 0.0);
    float3 v = float3(0.0, ca, sa);
    float3 w = float3(0.0, -sa, ca);
    float3 arms = bodySymm - float3(-0.012, 0.235, 0.21);
    float3 armsrot = float3(dot(arms,u), dot(arms,v), dot(arms,w));
    float3 armsextents = float3(0.034, 0.23, 0.028);
    float d4 = sdBox(armsrot + armsextents, armsextents); // rotation pivot at origin
    if (d4 < res.x) { res = float2(d4, 2.0); }
    
    // legs
    d4 = sdBox(bodySymm - float3(-0.012, -0.43, 0.1), float3(0.034, 0.18, 0.028));
    if (d4 < res.x) { res = float2(d4, 2.0); }
    
    return res;
}


float2 map(float3 pos) {

    pos.yz = float2(pos.z, -pos.y);

    // character
    float3 scaledPos = 0.35 * pos;
    float2 d1 = sdBird(scaledPos);

    // platform
    float d2 = sdThickDisk(pos - float3(0.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), platformSize, 0.286);
    float2 res = (d2 < d1.x) ? float2(d2, 1.0) : d1;

    return res;
}

float3 calcNormal(float3 pos) {
    float2 e = float2(0.0001, 0.0);
    return normalize(float3(map(pos+e.xyy).x-map(pos-e.xyy).x,
                            map(pos+e.yxy).x-map(pos-e.yxy).x,
                            map(pos+e.yyx).x-map(pos-e.yyx).x));
                              
}

float castShadow(float3 ro, float3 rd) {
    float res = 1.0;
    float t = 0.001;
    for (int i = 0; i < 100; i++) {
    
        float3 pos = ro + t * rd;
        float h = map(pos).x;
        
        res = min(res, 20.0 * h/t);
        if (res < 0.0001) break;
        t += h;
        
        if (t > 20.0) { t = -1.0; break; }
    }
    return clamp(res, 0.0, 1.0);
}

float2 castRay(float3 ro, float3 rd) {
    float t = 0.01;
    float m = -1.0;
    for (int i = 0; i < 250; i++) {
    
        float3 pos = ro + t * rd;
        
        float2 hm = map(pos);
        if (hm.x < 0.001) break; // inside object
    
        t += hm.x;
        m = hm.y;
        if (t > far) { m = -1.0; break; }
    }
    return float2(t,m);
}

float3 nearPos_WS(float2 uv) {
    // uv space ((0,0) top left, (1,1) bottom right) to NDC space ((0,0) center, y up, x right)
    float2 pos_NDC = uv * float2(2.0, -2.0) + float2(-1,1);
    // NDC to eye space (right handed, y up, z front)
    float4 pos_ES = float4(pos_NDC.x * aspect * tanfov, pos_NDC.y * tanfov, -1.f, 1.f / near);
    pos_ES = pos_ES / pos_ES.w;
    // to world space (right handed, z up)
    pos_ES.xyz = pos_ES.xyz - viewMatrix3.xyz;
    float3 pos_WS = float3(dot(viewMatrix0.xyz, pos_ES.xyz), dot(viewMatrix1.xyz, pos_ES.xyz), dot(viewMatrix2.xyz, pos_ES.xyz));
    
    return pos_WS;
}

PS_OUT PS(VertexOut IN) {

    // camera	
    float3 camPos_WS = float3(
        -dot(viewMatrix0.xyz, viewMatrix3.xyz),
        -dot(viewMatrix1.xyz, viewMatrix3.xyz),
        -dot(viewMatrix2.xyz, viewMatrix3.xyz));
    float3 ro = camPos_WS;
    float3 camToNear_WS = nearPos_WS(IN.uv) - camPos_WS;
    float3 rd = normalize(camToNear_WS);
    
    
    // raymarch scene
    float2 tm = castRay(ro, rd);
    
    PS_OUT output;
    output.depth = 1.0;
    output.color = float4(0.0, 0.0, 0.0, 0.0);
    if (tm.y > 0.0) {
        float3 pos = ro + tm.x * rd;
        float3 normal = calcNormal(pos);
        
        // query materials
        float3 mate = float3(1.0, 1.0, 1.0);
        if (tm.y > 6.5) {
            mate = mate * float3(1.0, 1.0, 1.0);
        } else if (tm.y > 5.5) {
            mate = mate * float3(1.0, 0.592, 1.0);
        } else if (tm.y > 4.5) {
            mate = mate * float3(1.0, 1.0, 0.33);
        } else if (tm.y > 3.5) {
            mate = mate * float3(0.05, 0.05, 0.05);
        } else if (tm.y > 2.5) {
            mate = mate * float3(0.79, 0.79, 0.79);
        } else if (tm.y > 1.5) {
            mate = mate * float3(0.1, 0.1, 0.1);
        } else /* if (tm.y > 0.5)*/ {
            mate = mate * float3(0.05, 0.09, 0.02);
        }

        // compute lighting
        float3 sun_dir = normalize(float3(sin(time), cos(time), 0.87));
        float sun_dif = clamp(dot(normal, sun_dir), 0.0, 1.0);
        float sun_sha = castShadow(pos + normal * 0.001, sun_dir);
        float sky_dif = clamp(0.5 + dot(normal, float3(0.0, 0.0, 1.0)), 0.0, 1.0);
        float bounce_dif = clamp(0.3 + 0.5 * dot(normal, float3(0.0, -1.0, 0.0)), 0.0, 1.0);
        float3 col = mate * 0.7*float3(7.0, 5.5, 4.0) * sun_dif * sun_sha;
        col += mate *  0.7*float3(0.7, 0.4, 1.1) * sky_dif;
        col += mate *  0.7*float3(0.18, 0.23, 0.17) * bounce_dif;
   
        // gamma correction
        col = pow(col, float3(0.4545, 0.4545, 0.4545));

        output.color = float4(col.xyz, 1.0 );

        // compute depth

        // project intersection position's z and w coordinate to clip space and perform perspective
        // divide, so we can extract the z component and write to the depth buffer
        float4 vpMatrix_row2 = float4(
            dot(proj_row2, viewMatrix0),
            dot(proj_row2, viewMatrix1),
            dot(proj_row2, viewMatrix2),
            dot(proj_row2, viewMatrix3));
        float4 vpMatrix_row3 = float4(  // todo: this assumes that proj_row3 is (0, 0, -1, 0),
            -viewMatrix0.z,             // but I think I'm sort of ok with that
            -viewMatrix1.z,
            -viewMatrix2.z,
            -viewMatrix3.z);
        float pos_CS_z = dot(vpMatrix_row2, float4(pos, 1.0));
        float pos_CS_w = dot(vpMatrix_row3, float4(pos, 1.0));
        float depth = pos_CS_z / pos_CS_w;

        // alternatively, if we want to pass the whole vp matrix
        //float zc = ( mul(vpMatrix, float4( pos, 1.0 )) ).z;
        //float wc = ( mul(vpMatrix, float4( pos, 1.0 )) ).w;
        //float depth = zc / wc;

        output.depth = depth;
    }

    return output;
}
)"
};

#elif __GL33

constexpr VS_src vs_3d_base = {
"vs_3d_base",
R"(
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
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    varying_COLOR = PerGroup.groupColor;
    gl_Position = PerScene.vpMatrix * (PerGroup.modelMatrix * vec4(in_var_POSITION, 1.0));
}
)"
};

constexpr VS_src vs_color2d_base = {
"vs_color3d_base",
R"(
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

layout(location = 0) in vec2 in_var_POSITION;
layout(location = 1) in vec4 in_var_COLOR;
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    varying_COLOR = in_var_COLOR * PerGroup.groupColor;
    gl_Position = PerScene.vpMatrix * (PerGroup.modelMatrix * vec4(in_var_POSITION, -1.0, 1.0));
}
)"
};

constexpr VS_src vs_color3d_base = {
"vs_color3d_base",
R"(
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
layout(location = 1) in vec4 in_var_COLOR;
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    varying_COLOR = in_var_COLOR * PerGroup.groupColor;
    gl_Position = PerScene.vpMatrix * (PerGroup.modelMatrix * vec4(in_var_POSITION, 1.0));
}
)"
};

constexpr VS_src vs_color3d_skinned_base = {
"vs_color3d_skinned_base",
R"(
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
layout(location = 1) in vec4 in_var_COLOR;
layout(location = 2) in vec4 in_var_JOINTINDICES;
layout(location = 3) in vec4 in_var_JOINTWEIGHTS;
layout(location = 0) out vec4 varying_COLOR;

void main()
{
    varying_COLOR = in_var_COLOR * PerGroup.groupColor;
    mat4 joint0 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.x)] * in_var_JOINTWEIGHTS.x;
    mat4 joint1 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.y)] * in_var_JOINTWEIGHTS.y;
    mat4 joint2 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.z)] * in_var_JOINTWEIGHTS.z;
    mat4 joint3 = PerJoint.skinningMatrices[int(in_var_JOINTINDICES.w)] * in_var_JOINTWEIGHTS.w;
    mat4 skinning = joint0 + joint1 + joint2 + joint3;
    gl_Position = PerScene.vpMatrix * ((PerGroup.modelMatrix * skinning) * vec4(in_var_POSITION, 1.0));
}
)"
};

constexpr VS_src vs_3d_instanced_base = {
"vs_3d_instanced_base"
, R"(
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
)"
};

constexpr VS_src vs_textured3d_base = {
"vs_textured3d_base",
R"(
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
)"
};

constexpr VS_src vs_textured3d_skinned_base = {
"vs_textured3d_skinned_base",
R"(
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
)"
};

constexpr PS_src ps_textured3d_base = {
"ps_textured3d_base",
R"(
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
    //vec3 color = diffuse.a * diffuse.rgb + (1.0-diffuse.a) * varying_COLOR.rgb;
    //out_var_SV_TARGET = color;
    out_var_SV_TARGET = diffuse.rgba;
}
)"
};

constexpr PS_src ps_textured3dalphaclip_base = {
"forward_textured_unlitalphaclip_TexturedVec3_CBuffer_3DScene",
R"(
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
)"
};

constexpr PS_src ps_fullscreen_blit_sdf = {
"ps_fullscreen_blit_sdf",
R"(
#version 330
#extension GL_ARB_separate_shader_objects : require

layout(std140) uniform type_BlitSDF {
    vec4 viewMatrix0;
    vec4 viewMatrix1;
    vec4 viewMatrix2;
    vec4 viewMatrix3;
    vec4 proj_row2;
    float tanfov; float aspect; float near; float far;
    float time; float platformSize; vec2 padding;
} BlitSDF;

layout(location = 0) in vec2 varying_TEXCOORD;
layout(location = 1) in vec4 in_var_POSITION;
layout(location = 0) out vec4 out_var_SV_TARGET;

float sdSphere( vec3 p, float s )
{
  return length(p)-s;
}

float sdBox( vec3 p, vec3 b ) {
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float sdCone( vec3 p, vec2 c, float h )
{
  float q = length(p.xz);
  return max(dot(c.xy,vec2(q,p.y)),-h-p.y);
}

float sdThickDisk( vec3 p, vec3 n, in float r, in float thick )  
{
    vec3 d = dot(p, n) * n;
    vec3 o = p - d;
    o -= normalize( o ) * min( length( o ), r );
    return length( d + o ) - thick;
}

vec2 sdBird(vec3 pos) {

    float t = fract(0.7*BlitSDF.time);
    
    float y = 0.7 + 0.2 * t * (1.0 - t);
    vec3 center = vec3(0.0, y, 0.0);
    vec3 q = pos - center;
    
    // head
    vec3 head = q - vec3(0.027, 0.685, 0.0);
    float d = sdSphere(head, 0.409);
    vec2 res = vec2(d, 2.0);
    
    // eyes
    vec3 headSymm = vec3(head.xy, abs(head.z));
    float d1 = sdSphere(headSymm - vec3(0.0, 0.0, 0.2), 0.262);
    if (d1 < res.x) { res.y = 3.0; }
    d1 = sdSphere(headSymm - vec3(0.0, 0.0, 0.35), 0.098);
    if (d1 < res.x) { res.y = 4.0; }
    
    // beak
    vec3 cone = q - vec3(-0.77, 0.4, 0.0);
    cone.xy = (mat2(-2, -6, 6, -2)/6.3245) * cone.xy;
    float d2 = sdCone(cone, vec2(0.6, 0.25), 0.528);
    if (d2 < res.x) { res = vec2(d2, 5.0); }
    
    // body
    float d3 = sdBox(q - vec3(-0.012, 0.01, 0), vec3(0.123, 0.214, 0.14));
    if (d3 < res.x) { res = vec2(d3, 6.0); }
    d3 = sdBox(q - vec3(0.002, -0.1, 0), vec3(0.203, 0.031, 0.203));
    if (d3 < res.x) { res.y = 7.0; }
    
    vec3 bodySymm = vec3(q.xy, abs(q.z));
    
    // arms
    float angle = -0.1 - 0.5 * t * (1.0 - t);
    float sa = sin(angle);
    float ca = cos(angle);
    vec3 u = vec3(1.0, 0.0, 0.0);
    vec3 v = vec3(0.0, ca, sa);
    vec3 w = vec3(0.0, -sa, ca);
    vec3 arms = bodySymm - vec3(-0.012, 0.235, 0.21);
    vec3 armsrot = vec3(dot(arms,u), dot(arms,v), dot(arms,w));
    vec3 armsextents = vec3(0.034, 0.23, 0.028);
    float d4 = sdBox(armsrot + armsextents, armsextents); // rotation pivot at origin
    if (d4 < res.x) { res = vec2(d4, 2.0); }
    
    // legs
    d4 = sdBox(bodySymm - vec3(-0.012, -0.43, 0.1), vec3(0.034, 0.18, 0.028));
    if (d4 < res.x) { res = vec2(d4, 2.0); }
    
    return res;
}


vec2 map(vec3 pos) {

    pos.yz = vec2(pos.z, -pos.y);
    pos.x = -pos.x;

    // main character
    vec3 scaledPos = 0.35 * pos;
    vec2 d1 = sdBird(scaledPos);

    // platform
    float d2 = sdThickDisk(pos - vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), BlitSDF.platformSize, 0.286);

    return (d2 < d1.x) ? vec2(d2, 1.0) : d1;
}

vec3 calcNormal(vec3 pos) {
    vec2 e = vec2(0.0001, 0.0);
    return normalize(vec3(map(pos+e.xyy).x-map(pos-e.xyy).x,
                          map(pos+e.yxy).x-map(pos-e.yxy).x,
                          map(pos+e.yyx).x-map(pos-e.yyx).x));
                              
}

float castShadow(vec3 ro, vec3 rd) {
    float res = 1.0;
    float t = 0.001;
    for (int i = 0; i < 100; i++) {
    
        vec3 pos = ro + t * rd;
        float h = map(pos).x;
        
        res = min(res, 20.0 * h/t);
        if (res < 0.0001) break;
        t += h;
        
        if (t > 20.0) { t = -1.0; break; }
    }
    return clamp(res, 0.0, 1.0);
}

vec2 castRay(vec3 ro, vec3 rd) {
    float t = 0.01;
    float m = -1.0;
    for (int i = 0; i < 250; i++) {
    
        vec3 pos = ro + t * rd;
        
        vec2 hm = map(pos);
        if (hm.x < 0.001) break; // inside object
    
        t += hm.x;
        m = hm.y;
        if (t > BlitSDF.far) { m = -1.0; break; }
    }
    return vec2(t,m);
}

vec3 nearPos_WS(vec2 uv) {
    // uv space ((0,0) bottom left, (1,1) top right) to NDC space ((0,0) center, y up, x right)
    vec2 pos_NDC = uv * vec2(2.0, 2.0) + vec2(-1,-1);
    // NDC to eye space (right handed, y up, z front)
    vec4 pos_ES = vec4(pos_NDC.x * BlitSDF.aspect * BlitSDF.tanfov, pos_NDC.y * BlitSDF.tanfov, -1.f, 1.f / BlitSDF.near);
    pos_ES = pos_ES / pos_ES.w;
    // to world space (right handed, z up)
    pos_ES.xyz = pos_ES.xyz - BlitSDF.viewMatrix3.xyz;
    vec3 pos_WS = vec3(dot(BlitSDF.viewMatrix0.xyz, pos_ES.xyz), dot(BlitSDF.viewMatrix1.xyz, pos_ES.xyz), dot(BlitSDF.viewMatrix2.xyz, pos_ES.xyz));
    
    return pos_WS;
}

void main() {

    // camera	
    vec3 camPos_WS = vec3(
        -dot(BlitSDF.viewMatrix0.xyz, BlitSDF.viewMatrix3.xyz),
        -dot(BlitSDF.viewMatrix1.xyz, BlitSDF.viewMatrix3.xyz),
        -dot(BlitSDF.viewMatrix2.xyz, BlitSDF.viewMatrix3.xyz));
    vec3 ro = camPos_WS;
    vec3 camToNear_WS = nearPos_WS(varying_TEXCOORD) - camPos_WS;
    vec3 rd = normalize(camToNear_WS);
    
    // raymarch scene
    vec2 tm = castRay(ro, rd);
    
    vec4 output_color = vec4(0.0);
    float depth = 1.0;
    if (tm.y > 0.0) {
        vec3 pos = ro + tm.x * rd;
        vec3 normal = calcNormal(pos);

        // query materials
        vec3 mate = vec3(0.18, 0.18, 0.18);
        if (tm.y > 6.5) {
            mate = vec3(1.0, 1.00, 1.00);
        } else if (tm.y > 5.5) {
            mate = vec3(1.0, 0.592, 1.00);
        } else if (tm.y > 4.5) {
            mate = vec3(1.0, 1.0, 0.33);
        } else if (tm.y > 3.5) {
            mate = vec3(0.05, 0.05, 0.05);
        } else if (tm.y > 2.5) {
            mate = vec3(0.79, 0.79, 0.79);
        } else if (tm.y > 1.5) {
            mate = vec3(0.1, 0.1, 0.1);
        } else /* if (tm.y > 0.5)*/ {
            mate = vec3(0.05, 0.09, 0.02);
        }
        
        // compute lighting
        vec3 sun_dir = normalize(vec3(sin(BlitSDF.time), cos(BlitSDF.time), 0.87));
        float sun_dif = clamp(dot(normal, sun_dir), 0.0, 1.0);
        float sun_sha = castShadow(pos + normal * 0.001, sun_dir);
        float sky_dif = clamp(0.5 + dot(normal, vec3(0.0, 0.0, 1.0)), 0.0, 1.0);
        float bounce_dif = clamp(0.3 + 0.5 * dot(normal, vec3(0.0, -1.0, 0.0)), 0.0, 1.0);
        vec3 col = mate * 0.7*vec3(7.0, 5.5, 4.0) * sun_dif * sun_sha;
        col += mate *  0.7*vec3(0.7, 0.4, 1.1) * sky_dif;
        col += mate *  0.7*vec3(0.18, 0.23, 0.17) * bounce_dif;
   
        // gamma correction
        col = pow(col, vec3(0.4545, 0.4545, 0.4545));

        output_color = vec4(col.xyz, 1.0 );

        // compute depth

        // project intersection position's z and w coordinate to clip space and perform perspective
        // divide, so we can extract the z component and write to the depth buffer
        vec4 vpMatrix_row2 = vec4(
            dot(BlitSDF.proj_row2, BlitSDF.viewMatrix0),
            dot(BlitSDF.proj_row2, BlitSDF.viewMatrix1),
            dot(BlitSDF.proj_row2, BlitSDF.viewMatrix2),
            dot(BlitSDF.proj_row2, BlitSDF.viewMatrix3));
        vec4 vpMatrix_row3 = vec4(  // todo: this assumes that proj_row3 is (0, 0, -1, 0),
            -BlitSDF.viewMatrix0.z, // but I think I'm sort of ok with that
            -BlitSDF.viewMatrix1.z,
            -BlitSDF.viewMatrix2.z,
            -BlitSDF.viewMatrix3.z);
        float pos_CS_z = dot(vpMatrix_row2, vec4(pos, 1.0));
        float pos_CS_w = dot(vpMatrix_row3, vec4(pos, 1.0));

        depth = pos_CS_z / pos_CS_w;
        depth = (depth + 1.0) * 0.5; // move depth's [-1.0,1.0] range to texture's [0.0, 1.0] range

        // alternatively, if we want to pass the whole vp matrix
        //float zc = ( mul(vpMatrix, vec4( pos, 1.0 )) ).z;
        //float wc = ( mul(vpMatrix, vec4( pos, 1.0 )) ).w;
        //float depth = zc / wc;
    }

    out_var_SV_TARGET = output_color;
    gl_FragDepth = depth;
}
)"
};

#endif

}
}
#endif // __WASTELADNS_SHADERS_H__
