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

const char * texturedVShaderStr = R"(

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
        float3 normal : NORMAL;
        float2 uv : TEXCOORD;
        uint instanceID : SV_InstanceID;
    };
    struct VertexOutput {
        float3 worldPos : POSITION;
        float3 normal : NORMAL;
        float2 uv : TEXCOORD;
        float4 position : SV_POSITION;
    };
    VertexOutput VS(AppData IN) {
        VertexOutput OUT;
        matrix m = worldView[IN.instanceID];
        OUT.worldPos = mul(m, float4(IN.position, 1.f)).xyz;
        OUT.normal = mul(m, float4(IN.normal, 0.f)).xyz;
        matrix mv = mul(projectionMatrix, viewMatrix);
        OUT.position = mul(mv, float4(OUT.worldPos, 1.f));
        OUT.uv = IN.uv;
        return OUT;
    }

)";

const char * texturedPShaderStr = R"(
    Texture2D albedoTex;
    SamplerState samplerType;

    struct PixelIn {
        float3 worldPos : POSITION;
        float3 normal : NORMAL;
        float2 uv : TEXCOORD;
    };
    float4 PS(PixelIn IN) : SV_TARGET {

        float3 lightPos = float3(0.f, 0.f, 100.f);
    
        float3 albedo = albedoTex.Sample(samplerType, IN.uv).rgb;
        float3 lightDir = normalize(lightPos - IN.worldPos);
        float3 normal = normalize(IN.normal);
    
        float diff = max(dot(lightDir, normal), 0.f);
        float3 diffuse = diff * albedo;
        float3 ambient = 0.5f * albedo;
    
        float4 FragColor = float4(diffuse + ambient, 1.f);

        return FragColor;

    }
)";

#endif // __WASTELADNS_SHADERS_DX11_H__
