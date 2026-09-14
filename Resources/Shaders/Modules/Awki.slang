ByteAddressBuffer BindlessBuffers[];
RWByteAddressBuffer RWBindlessBuffers[];
Texture2D BindlessTextures2d[];
RWTexture2D RWBindlessTextures2d[];
SamplerState BindlessSamplers[];

property SamplerState gPointClampSampler { get { return BindlessSamplers[0]; } }
property SamplerState gLinearClampSampler { get { return BindlessSamplers[1]; } }

struct BufferHandle<T>
{
    int32_t index;

    T Get(int32_t i)
    {
        int32_t byteOffset = i * sizeof(T);
        return BindlessBuffers[index].Load<T>(byteOffset);
    }

    void Set(int32_t i, T newValue)
    {
        int32_t byteOffset = i * sizeof(T);
        RWBindlessBuffers[index].Store<T>(byteOffset, newValue);
    }

    __subscript(int32_t i)->T
    { 
        get { return Get(i); } 
        set { Set(i, newValue); }
    }

    bool IsValid()
    {
        return index != -1;
    }
}

struct TextureHandle<T>
{
    int32_t index;

    T Get()
    {
        if (T is Texture2D || T is Texture2D<float4>)
            return reinterpret<T>(BindlessTextures2d[index]);
        if (T is RWTexture2D || T is RWTexture2D<float4>)
            return reinterpret<T>(RWBindlessTextures2d[index]);
    }

    bool IsValid()
    {
        return index != -1;
    }
}

struct MaterialHandle<T>
{
    int32_t byteOffset;

    T Get()
    {
        // BindlessBuffers[0] is reserved exclusively for materials
        return BindlessBuffers[0].Load<T>(byteOffset);
    }

    bool IsValid()
    {
        return byteOffset != -1;
    }
}

struct AwkiVertex
{
    float3 position;
    float3 normal;
    float2 uv;
    float4 tangent;
    float4 _Padding;
};

struct VertexBufferHandle
{
    int32_t index;
    uint32_t offset;

    AwkiVertex Get(int32_t i)
    {
        int32_t byteOffset = (offset + i) * sizeof(AwkiVertex);
        return BindlessBuffers[index].Load<AwkiVertex>(byteOffset);
    }

    __subscript(int32_t i)->AwkiVertex
    { get { return Get(i); } }

    bool IsValid()
    {
        return index != -1;
    }
}

struct DrawCallData<T>
{
    MaterialHandle<T> material;
    VertexBufferHandle vertexBuffer;

    T GetMaterial()
    {
        return material.Get();
    }

    AwkiVertex GetVertex(uint32_t index)
    {
        return vertexBuffer[index];
    }
};