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

#endif // __WASTELADNS_SHADERS_DX11_H__
