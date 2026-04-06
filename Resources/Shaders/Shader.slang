import Awki;

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
    BBuffer buffer;
    BTexture texture;
};

ParameterBlock<UnlitMaterial> gMaterial;

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
    BufferHandle<float3> buffer = { gMaterial.buffer };
    TextureHandle<RWTexture2D> texture = { gMaterial.texture };

    // Operator() overload to get the actual texture object
    texture().Store(int2(0, 0), float4(0.f, 0.f, 1.f, 1.f));
    float4 a = texture().Load(int2(0, 0));

    // Accessor operators to Get/Set values in buffers
    // Can use either Get(i)/Set(i) or [i]
    buffer[0] = float3(1.f, 0.f, 0.f);
    // buffer[1] = float3(1.f, 1.f, 1.f);
    // buffer[2] = float3(1.f, 0.f, 1.f);
    // buffer[3] = float3(0.f, 1.f, 0.f);

    return float4(a.rgb + buffer[0], 1.f);
}
