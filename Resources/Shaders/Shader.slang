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

    //Operator overload to get the actual texture object
   // float4 a = gMaterial.texture().Load(int2(0,0));

    //Accessor operators to Get/Set values in buffers
    //gMaterial.buffer.Set(0, float3(a.x, 0.f, 0.f));
    //gMaterial.buffer.Set(1, float3(1.f, 1.f, 1.f));
    //gMaterial.buffer.Set(2, float3(1.f, 0.f, 1.f));
    //gMaterial.buffer.Set(3, float3(0.f, 1.f, 0.f));

    //Can use either Get(i) or [i]
    //Cant use [] to set since the value is implicitly const as it comes from a constant buffer but can use Set()
    return float4(buffer[1], 1.f);
}
