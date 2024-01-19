#ifndef __WASTELADNS_SHADERS_DX11_H__
#define __WASTELADNS_SHADERS_DX11_H__

const char * coloredVertexShaderStr = R"(
cbuffer PerGroup : register(b0) {
    column_major matrix MVP;
}
struct AppData {
    float3 position : POSITION;
    float4 color : COLOR;
};
struct VertexOutput {
    float4 color : COLOR;
    float4 position : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    OUT.position = mul(MVP, float4(IN.position, 1.f));
    OUT.color = IN.color.rgba;
    return OUT;
}
)";

const char * defaultPixelShaderStr = R"(
struct VertexOut {
    float4 color : COLOR;
};
float4 PS(VertexOut OUT) : SV_TARGET {
    return OUT.color;
}
)";

const char* defaultVertexShaderStr = R"(

cbuffer PerScene : register(b0) {
    column_major matrix projectionMatrix;
    column_major matrix viewMatrix;
    float3 viewPosWS;
    float padding;
}
cbuffer PerGroup : register(b1) {
    column_major matrix modelMatrix;
    float4 groupColor;
}
cbuffer PerInstance : register(b2) {
	column_major matrix instanceMatrices[256];
};
struct AppData {
    float3 posMS : POSITION;
};
struct VertexOutput {
    float4 color : COLOR;
    float4 positionCS : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    column_major matrix vp = mul(projectionMatrix, viewMatrix);
    float4 posWS = mul(modelMatrix, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vp, posWS);
    OUT.color = groupColor;
    return OUT;
}
)";

const char* texturedVertexShaderStr = R"(

cbuffer PerScene : register(b0) {
    column_major matrix projectionMatrix;
    column_major matrix viewMatrix;
    float3 viewPosWS;
    float padding;
}
cbuffer PerGroup : register(b1) {
    column_major matrix modelMatrix;
    float4 groupColor;
}
cbuffer PerInstance : register(b2) {
	column_major matrix instanceMatrices[256];
};
struct AppData {
    float3 posMS : POSITION;
    float2 uv : TEXCOORD;
    float3 normalMS : NORMAL;
    float3 tangentMS : TANGENT;
    float3 bitangentMS : BITANGENT;
};
struct VertexOutput {
    float3 posWS : POSITION;
    float3 normalWS : NORMAL;
    float3 viewPosTS : VIEW_TS;
    float3 posTS : POSITION_TS;
    float2 uv : TEXCOORD;
    float4 positionCS : SV_POSITION;
    float3x3 tbnMatrix : TBN;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    matrix mv = mul(projectionMatrix, viewMatrix);
    OUT.posWS = mul(modelMatrix, float4(IN.posMS, 1.f)).xyz;
    OUT.normalWS = normalize(mul(modelMatrix, float4(IN.normalMS, 0.f))).xyz;
    float3 tangentWS = normalize(mul(modelMatrix, float4(IN.tangentMS, 0.f))).xyz;
    float3 bitangentWS = normalize(mul(modelMatrix, float4(IN.bitangentMS, 0.f))).xyz;
    
    // load vectors in the rows (transposed, becomes the inverse)
     OUT.tbnMatrix = float3x3(
          tangentWS.x, bitangentWS.x, OUT.normalWS.x
        , tangentWS.y, bitangentWS.y, OUT.normalWS.y
        , tangentWS.z, bitangentWS.z, OUT.normalWS.z
    );
    float3x3 invtbn = transpose(OUT.tbnMatrix);
    OUT.viewPosTS = mul(invtbn, viewPosWS);
    OUT.posTS = mul(invtbn, OUT.posWS);
    OUT.positionCS = mul(mv, float4(OUT.posWS, 1.f));
    OUT.uv = IN.uv;
    return OUT;
}
)";
const char* texturedPixelShaderStr = R"(

Texture2D texDiffuse : register(t0);
Texture2D texNormal : register(t1);
Texture2D texDepth : register(t2);
SamplerState texDiffuseSampler : register(s0);
SamplerState texNormalSampler : register(s1);
SamplerState texDepthSampler : register(s2);

cbuffer PerScene : register(b0) {
    column_major matrix projectionMatrix;
    column_major matrix viewMatrix;
    float3 viewPosWS;
    float padding;
}

cbuffer PerGroup : register(b1) {
    column_major matrix modelMatrix;
    float4 groupColor;
}

struct PixelIn {
    float3 posWS : POSITION;
    float3 normalWS : NORMAL;
    float3 viewPosTS : VIEW_TS;
    float3 posTS : POSITION_TS;
    float2 uv : TEXCOORD;
    float4 positionCS : SV_POSITION;
    float3x3 tbnMatrix : TBN;
};

float4 PS(PixelIn IN) : SV_TARGET {

    float3 lightWS = float3(3.f, 8.f, 15.f);
    float3 albedo = texDiffuse.Sample(texDiffuseSampler, IN.uv).rgb;
    float3 lightDir = normalize(lightWS - IN.posWS);
    float3 normalWS = texNormal.Sample(texNormalSampler, IN.uv).xyz;
    normalWS = normalWS * 2.f - 1.f;
    normalWS = normalize(mul(IN.tbnMatrix, normalWS));
    
    // Aggressive gooch-like shading
    float3 viewWS = viewPosWS.xyz;
    float3 viewDirWS = normalize(viewWS - IN.posWS);
    float diff = dot(lightDir, normalWS);
    float t = ( diff + 1.f ) * 0.5f;
    float3 r = 2.f * diff * normalWS - lightDir;
    float s = clamp(100.f * dot(r, viewDirWS) - 97.f, 0.f, 0.8f);
    float3 ccool = float3(0.f, 0.f, 0.3f) * albedo;
    float3 cwarm = clamp(float3(1.2f, 1.2f, 1.f) * albedo, 0.f, 1.f);
    float3 diffuse = 0.7f * diff * albedo;
    float3 ambient = 0.3f * albedo;

    float4 fragcolor = float4(s * float3(1.f,1.f,1.f) + (1 - s)*(t*cwarm + (1 - t)*ccool), 1.f);
    return fragcolor;
}
)";

const char * defaultInstancedVertexShaderStr = R"(

cbuffer PerScene : register(b0) {
    column_major matrix projectionMatrix;
    column_major matrix viewMatrix;
    float3 viewPosWS;
    float padding;
}
cbuffer PerGroup : register(b1) {
    column_major matrix modelMatrix;
    float4 groupColor;
}
cbuffer PerInstance : register(b2) {
	column_major matrix instanceMatrices[256];
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
    column_major matrix instanceMatrix = instanceMatrices[IN.instanceID];
    column_major matrix mm = mul(instanceMatrix, modelMatrix);
    column_major matrix vp = mul(projectionMatrix, viewMatrix);
    float4 posWS = mul(mm, float4(IN.posMS, 1.f));
    OUT.positionCS = mul(vp, posWS);
    OUT.color = groupColor;
    return OUT;
}
)";

#endif // __WASTELADNS_SHADERS_DX11_H__
