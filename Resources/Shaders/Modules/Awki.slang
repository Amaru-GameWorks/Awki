ByteAddressBuffer BindlessBuffers[] : register(t0, space0);
RWByteAddressBuffer RWBindlessBuffers[] : register(u0, space0);

Texture2D BindlessTextures2d[] : register(t0, space1);
RWTexture2D RWBindlessTextures2d[] : register(u0, space1);

SamplerState BindlessSamplers[] : register(s0, space2);

typealias BBuffer = uint;
typealias BTexture = int;

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