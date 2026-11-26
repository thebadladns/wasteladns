struct VS_Output {
    float4 positionCS : SV_POSITION;
};
VS_Output VS(uint id : SV_VertexID) {
    VS_Output Output;
    float2 uv = float2((id << 1) & 2, id & 2);
    Output.positionCS = float4(uv * float2(2,-2) + float2(-1,1), 1, 1);  // clear depth
    return Output;
}