import Awki;

struct VSOutput
{
    precise float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct UnlitMaterial
{
    float4 color;
    float4x4 viewProjection;
    BTexture diffuseHandle;
};

struct MeshData
{
    uint32_t vertexOffset;
    BBuffer vertexBuffer;
};

[[vk::push_constant]] MeshData gMeshData;
ParameterBlock<UnlitMaterial> gMaterial;

[shader("vertex")]
VSOutput main(uint vertexId: SV_VertexID)
{
    BufferHandle<AwkiVertex> vertexBuffer = { gMeshData.vertexBuffer };
    AwkiVertex vertex = vertexBuffer[gMeshData.vertexOffset + vertexId];

    VSOutput output;
    output.position = mul(gMaterial.viewProjection, float4(vertex.position, 1.f));
    output.uv = vertex.uv;
    return output;
}

[shader("fragment")]
float4 main(VSOutput input)
{
    float4 color = gMaterial.color;

    if (gMaterial.diffuseHandle != -1)
    {
        TextureHandle<Texture2D> texture = { gMaterial.diffuseHandle };
        color *= texture().Sample(gLinearClampSampler, input.uv);
    }

    return float4(color.xyz, 1.f);
}
