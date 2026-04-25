ByteAddressBuffer BindlessBuffers[];
RWByteAddressBuffer RWBindlessBuffers[];
Texture2D BindlessTextures2d[];
RWTexture2D RWBindlessTextures2d[];
SamplerState BindlessSamplers[];

property SamplerState gPointClampSampler { get { return BindlessSamplers[0]; } }
property SamplerState gLinearClampSampler { get { return BindlessSamplers[1]; } }

typealias BBuffer = int32_t;
typealias BTexture = int32_t;

struct BufferHandle<T>
{
    uint index;

    __init(BBuffer handle)
    {
        index = handle;
    }

    T Get(uint i)
    {
        uint byteOffset = i * sizeof(T);
        return BindlessBuffers[index].Load<T>(byteOffset);
    }

    void Set(uint i, T newValue)
    {
        uint byteOffset = i * sizeof(T);
        RWBindlessBuffers[index].Store<T>(byteOffset, newValue);
    }

    __subscript(uint i)->T
    { 
        get { return Get(i); } 
        set { Set(i, newValue); }
    }
}

struct TextureHandle<T>
{
    uint index;

    __init(BTexture handle)
    {
        index = handle;
    }

    T operator()()
    {
        if(T is Texture2D || T is Texture2D<float4>)
            return reinterpret<T>(BindlessTextures2d[index]);
        if(T is RWTexture2D || T is RWTexture2D<float4>)
            return reinterpret<T>(RWBindlessTextures2d[index]);
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