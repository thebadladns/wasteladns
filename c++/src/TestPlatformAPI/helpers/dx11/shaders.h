#ifndef __WASTELADNS_SHADERS_DX11_H__
#define __WASTELADNS_SHADERS_DX11_H__

const char * vertexColorShaderStr = R"(

    cbuffer PerSceneBuffer : register(b0) {
        matrix projectionMatrix;
        matrix viewMatrix;
    }
    cbuffer WorldMatrixBuffer : register(b1) {
        matrix worldMatrix;
    }
    cbuffer ColorBuffer : register(b2) {
        float4 bgcolor;
    }
    struct AppData {
        float3 position : POSITION;
    };
    struct VertexOutput {
        float4 color : COLOR;
        float4 position : SV_POSITION;
    };
    VertexOutput VS(AppData IN) {
        VertexOutput OUT;
        matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));
        OUT.position = mul(mvp, float4(IN.position, 1.f));
        OUT.color = bgcolor;
        return OUT;
    }

)";

const char * vertexShaderStr = R"(

    cbuffer PerSceneBuffer : register(b0) {
        matrix projectionMatrix;
        matrix viewMatrix;
    }
    cbuffer WorldMatrixBuffer : register(b1) {
        matrix worldMatrix;
    }
    struct AppData {
        float3 position : POSITION;
        float3 color: COLOR;
    };
    struct VertexOutput {
        float4 color : COLOR;
        float4 position : SV_POSITION;
    };
    VertexOutput VS(AppData IN) {
        VertexOutput OUT;
        matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));
        OUT.position = mul(mvp, float4(IN.position, 1.f));
        OUT.color = float4(IN.color, 1.f);
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
