struct VS_Output {
    float2 uv : TEXCOORD0;
    float4 positionCS : SV_POSITION;
};
VS_Output VS(uint id : SV_VertexID) {
    VS_Output Output;
    Output.uv = float2((id << 1) & 2, id & 2);
    Output.positionCS = float4(Output.uv * float2(2,-2) + float2(-1,1), 0, 1);
    return Output;
}