#ifndef __WASTELADNS_SHADERS_DX11_H__
#define __WASTELADNS_SHADERS_DX11_H__

const char * vertexShaderStr = R"(
cbuffer PerScene : register(b0) {
    matrix projectionMatrix;
    matrix viewMatrix;
}
cbuffer PerGroup : register(b1) {
    matrix worldView[64];
    float4 bgcolor;
}
struct AppData {
    float3 position : POSITION;
    uint instanceID : SV_InstanceID;
};
struct VertexOutput {
    float4 color : COLOR;
    float4 position : SV_POSITION;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldView[IN.instanceID]));
    OUT.position = mul(mvp, float4(IN.position, 1.f));
    OUT.color = bgcolor;
    return OUT;
}
)";

const char * coloredVertexShaderStr = R"(
cbuffer PerGroup : register(b0) {
    matrix MVP;
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

const char * pixelShaderStr = R"(
struct PixelIn {
    float4 color : COLOR;
};
float4 PS(PixelIn IN) : SV_TARGET {
    return IN.color;
}
)";

const char * geometryPassVShaderStr = R"(

cbuffer PerScene : register(b0) {
    matrix projectionMatrix;
    matrix viewMatrix;
    float3 viewPosWS;
    float padding;
}
cbuffer PerGroup : register(b1) {
    matrix modelMatrices[64];
    float depthTS;
}
struct AppData {
    float3 posMS : POSITION;
    float2 uv : TEXCOORD;
    float3 normalMS : NORMAL;
    float3 tangentMS : TANGENT;
    float3 bitangentMS : BITANGENT;
    uint instanceID : SV_InstanceID;
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
    matrix modelMatrix = modelMatrices[IN.instanceID];
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

const char * geometryPassPShaderStr = R"(

Texture2D texDiffuse : register(t0);
Texture2D texNormal : register(t1);
Texture2D texDepth : register(t2);
SamplerState texDiffuseSampler : register(s0);
SamplerState texNormalSampler : register(s1);
SamplerState texDepthSampler : register(s2);

cbuffer PerGroup : register(b1) {
    matrix modelMatrices[64];
    float depthTS;
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
struct PixelOut {
    float4 posWS : SV_Target0;
    float4 normalWS : SV_Target1;
    float4 diffuse : SV_Target2;
};

float2 binaryPOM(float2 uv, float2 uvextents, float mindepth, float maxdepth) {
    float2 curruv = uv;
    for (int sampleCount = 0; sampleCount < 16; sampleCount++) {
        float depth = (mindepth + maxdepth) * 0.5f;
        curruv = uv - depth * uvextents;
        // Compiler will unroll, no need to provide gradients
        float currsampleddepth = texDepth.Sample(texDepthSampler, curruv).r;

        if (currsampleddepth < depth) {
            maxdepth = depth;
        } else {
            mindepth = depth;
        }
    }

    return curruv;
}

float2 POM(float2 uv, float3 viewDir) {
    float layers = lerp(32.f, 8.f, abs(dot(float3(0.f, 0.f, 1.f), viewDir)));
    float depthstep = 1.f / layers;
    float2 uvextents = viewDir.xy * depthTS / viewDir.z;
    float2 uvstep = uvextents * depthstep;

    float2 curruv = uv;
    float2 nextuv = uv, prevuv = uv;
    float currdepth = 0.f;
    float nextdepth = currdepth, prevdepth = currdepth;
    float currsampleddepth = 0.1f;
    float prevsampleddepth = 0.f;
    float2 dx = ddx( uv );
    float2 dy = ddy( uv );
    do {
        prevuv = curruv;
        prevdepth = currdepth;
        prevsampleddepth = currsampleddepth;
        currdepth = nextdepth;
        curruv = nextuv;
        currsampleddepth = texDepth.SampleGrad(texDepthSampler, curruv, dx, dy).r;
        nextuv -= uvstep;
        nextdepth += depthstep;
    } while (currdepth < currsampleddepth);

    return binaryPOM(uv, uvextents, prevdepth, currdepth);
//    float prevdepthdelta = prevsampleddepth - prevdepth;
//    float currdepthdelta = currsampleddepth - currdepth;
//    float t = prevdepthdelta / (currdepthdelta - prevdepthdelta);
//    float2 outuv = prevuv * t + curruv * (1 - t);
//    return outuv;
}

PixelOut PS(PixelIn IN) {

    float3 viewDirTS = normalize(IN.viewPosTS - IN.posTS);
    float2 uv = IN.uv;
    uv = POM(IN.uv, viewDirTS);

    if (uv.x > 1.f || uv.y > 1.f || uv.x < 0.f || uv.y < 0.f) {
        clip(-1);
    }
    float4 albedo = texDiffuse.Sample(texDiffuseSampler, uv).rgba;
    if (albedo.a < 0.9) {
        discard;
    }

    PixelOut OUT;
    OUT.posWS = float4(IN.posWS, 1.f);
    OUT.diffuse = albedo;
    float3 normalWS = texNormal.Sample(texNormalSampler, uv).rgb;
    normalWS = normalWS * 2.f - 1.f;
    normalWS = normalize(mul(IN.tbnMatrix, normalWS));
    OUT.normalWS = float4(normalWS, 0.f);

    return OUT;
}
)";

const char * quadVShaderStr = R"(
struct AppData {
    float2 posCS : POSITION;
    float2 uv : TEXCOORD;
};
struct VertexOutput {
    float4 posCS : SV_POSITION;
    float2 uv : TEXCOORD;
};
VertexOutput VS(AppData IN) {
    VertexOutput OUT;
    OUT.posCS = float4(IN.posCS, 0.f, 1.f);
    OUT.uv = IN.uv;
    return OUT;
}
)";

const char * quadPShaderStr = R"(

cbuffer PerScene : register(b0) {
    float4 viewPosWS;
    float4 lightPosWS;
}

Texture2D albedoTex;
SamplerState samplerType;

// TODO: detach samplers from textures!!
Texture2D gPos : register(t0);
Texture2D gNormal : register(t1);
Texture2D gDiffuse : register(t2);
SamplerState gPosSampler : register(s0);
SamplerState gNormalSampler : register(s1);
SamplerState gDiffuseSampler : register(s2);

struct PixelIn {
    float4 posCS : SV_POSITION;
    float2 uv : TEXCOORD;
};
float4 PS(PixelIn IN) : SV_TARGET {

    float3 lightWS = lightPosWS.xyz;
    float3 albedo = gDiffuse.Sample(gDiffuseSampler, IN.uv).rgb;
    float3 posWS = gPos.Sample(gPosSampler, IN.uv).xyz;
    float3 lightDir = normalize(lightWS - posWS);
    float3 normalWS = gNormal.Sample(gNormalSampler, IN.uv).xyz;
    
    // Aggressive gooch-like shading
    float3 viewWS = viewPosWS.xyz;
    float3 viewDirWS = normalize(viewWS - posWS);
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

#endif // __WASTELADNS_SHADERS_DX11_H__
