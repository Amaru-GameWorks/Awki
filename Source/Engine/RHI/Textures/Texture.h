#pragma once
#include "PixelFormats.h"
#include "Utilities/ForwardStorage.h"

namespace vk 
{ 
	class Image; 
}

enum AkTextureFlagBits
{
	AkTextureFlags_SRGB_HINT					= 1 << 0,
	AkTextureFlags_BIND_AS_SHADER_RESOURCE		= 1 << 1,
	AkTextureFlags_BIND_AS_DEPTH_STENCIL		= 1 << 2,
	AkTextureFlags_BIND_AS_RENDER_TARGET		= 1 << 3,
	AkTextureFlags_ALLOW_UNORDERED_ACCESS		= 1 << 4,
	AkTextureFlags_COPY_DESTINATION				= 1 << 5,
	AkTextureFlags_COPY_SOURCE					= 1 << 6,
	AkTextureFlags_AUTO_RESOLVE_MSAA			= 1 << 7,
	AkTextureFlags_RENDER_INTO_SUB_RESOURCES	= 1 << 8,

	AkTextureFlags_DEFAULT			= AkTextureFlags_BIND_AS_SHADER_RESOURCE | AkTextureFlags_COPY_DESTINATION,
	AkTextureFlags_DEFAULT_RT		= AkTextureFlags_BIND_AS_RENDER_TARGET | AkTextureFlags_BIND_AS_SHADER_RESOURCE | AkTextureFlags_AUTO_RESOLVE_MSAA,
	AkTextureFlags_DEFAULT_DEPTH	= AkTextureFlags_BIND_AS_DEPTH_STENCIL | AkTextureFlags_BIND_AS_SHADER_RESOURCE
};
using AkTextureFlags = std::underlying_type_t<AkTextureFlagBits>;

enum class AkTextureType
{
	TEXTURE_1D,
	TEXTURE_2D,
	TEXTURE_3D,
	TEXTURE_ARRAY_1D,
	TEXTURE_ARRAY_2D,
	CUBEMAP,
	CUBEMAP_ARRAY
};

enum class AkMSAA
{
	X1,
	X2,
	X4,
	X8
};

struct AkTextureDescriptor
{
	uint32_t width = 1;
	uint32_t height = 1;
	uint32_t depth = 1;

	AkTextureFlags flags = AkTextureFlags_DEFAULT;
	AkTextureType type = AkTextureType::TEXTURE_2D;
	AkPixelFormat format = AkPixelFormat::RGBA8_UNORM;

	uint32_t mips = 1;
	uint32_t slices = 1;
	AkMSAA msaa = AkMSAA::X1;
};

class AkTexture
{
public:
	AkTexture(const AkTextureDescriptor& descriptor, const vk::Image& image);
	~AkTexture();

	const AkTextureDescriptor& GetDescriptor() const { return m_Descriptor; }
	const vk::Image& GetImage();

private:
	AkTextureDescriptor m_Descriptor;
	ForwardStorage<struct AkTextureStorage, 8> m_Storage;
};