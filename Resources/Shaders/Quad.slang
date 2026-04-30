import Awki;

static const float2 kVerts[] = 
{
    float2(-1.f,  1.f),
    float2(-1.f, -1.f),
    float2( 1.f,  1.f),
    float2( 1.f, -1.f)
};

static const float2 kUVs[] = 
{
    float2(0.f, 0.f),
    float2(0.f, 1.f),
    float2(1.f, 0.f),
    float2(1.f, 1.f)
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

struct QuadMaterial
{
    float4 color;
    TextureHandle<Texture2D> texture;
};

ParameterBlock<QuadMaterial> gMaterial;

[shader("vertex")]
VSOutput main(uint vertexId: SV_VertexID)
{
    VSOutput output;
    output.Position = float4(kVerts[vertexId], 0.f, 1.f);
    output.UV = kUVs[vertexId];
    return output;
}

[shader("fragment")]
float4 main(VSOutput input)
{
    float4 color = gMaterial.color;

    if (gMaterial.texture.IsValid())
    {
        Texture2D texture = gMaterial.texture.Get();
        color *= texture.Sample(gLinearClampSampler, input.UV);
    }

    return color;
}
