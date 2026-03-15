static const float2 kVerts[] = 
{
    float2( 0.0f,  0.5f),
    float2(-0.5f, -0.5f),
    float2( 0.5f, -0.5f)
};

struct VSOutput
{
    float4 Position : SV_POSITION;
};

struct UnlitMaterial
{
    float3 color;
}

ConstantBuffer<UnlitMaterial> ConstantData;

[shader("vertex")]
VSOutput main(uint vertexId: SV_VertexID)
{
    VSOutput output;
    output.Position = float4(kVerts[vertexId], 0.f, 1.f);
    return output;
}

[shader("fragment")]
float4 main(VSOutput input)
{
    return float4(ConstantData.color, 1.f);
}
