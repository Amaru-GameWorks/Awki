import Awki;

struct UnlitMaterial
{
    float4 color;
    TextureHandle<Texture2D> diffuseHandle;
};

[vk::push_constant]
DrawCallData<UnlitMaterial> gDrawCall;

struct VSOutput
{
    precise float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

[shader("vertex")]
VSOutput main(uint vertexId: SV_VertexID)
{
    AwkiVertex vertex = gDrawCall.GetVertex(vertexId);

    VSOutput output;
    output.position = float4(vertex.position, 1.f);
    output.uv = vertex.uv;
    return output;
}

[shader("fragment")]
float4 main(VSOutput input)
{
    UnlitMaterial material = gDrawCall.GetMaterial();
    float4 color = material.color;

    if (material.diffuseHandle.IsValid())
    {
        Texture2D texture = material.diffuseHandle.Get();
        color *= texture.Sample(gLinearClampSampler, input.uv);
    }

    return float4(color.xyz, 1.f);
}
