#pragma once
#include "PixelFormats.h"
#include "Utilities/ForwardStorage.h"

#include <vector>

namespace vk
{
	class Image;
	class ImageView;
}

enum AkTextureFlagBits : uint16_t
{
	AkTextureFlags_SRGB_HINT				= 1 << 0,
	AkTextureFlags_BIND_AS_SHADER_RESOURCE	= 1 << 1,
	AkTextureFlags_BIND_AS_DEPTH_STENCIL	= 1 << 2,
	AkTextureFlags_BIND_AS_RENDER_TARGET	= 1 << 3,
	AkTextureFlags_ALLOW_UNORDERED_ACCESS	= 1 << 4,
	AkTextureFlags_COPY_DESTINATION			= 1 << 5,
	AkTextureFlags_COPY_SOURCE				= 1 << 6,
	AkTextureFlags_AUTO_RESOLVE_MSAA		= 1 << 7,

	AkTextureFlags_DEFAULT					= AkTextureFlags_BIND_AS_SHADER_RESOURCE | AkTextureFlags_COPY_DESTINATION,
	AkTextureFlags_DEFAULT_RT				= AkTextureFlags_BIND_AS_RENDER_TARGET | AkTextureFlags_BIND_AS_SHADER_RESOURCE | AkTextureFlags_AUTO_RESOLVE_MSAA,
	AkTextureFlags_DEFAULT_DEPTH			= AkTextureFlags_BIND_AS_DEPTH_STENCIL | AkTextureFlags_BIND_AS_SHADER_RESOURCE
};
using AkTextureFlags = std::underlying_type_t<AkTextureFlagBits>;

enum class AkTextureType : uint8_t
{
	TEXTURE_1D,
	TEXTURE_2D,
	TEXTURE_3D,
	TEXTURE_ARRAY_1D,
	TEXTURE_ARRAY_2D,
	CUBEMAP,
	CUBEMAP_ARRAY
};

enum class AkMSAA : uint8_t
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

struct AkMipInfo
{
	size_t size = 0;
	size_t offset = 0;
};

class AkTexture
{
	friend class AkBindlessResourcesManager;

public:
	AkTexture(const AkTextureDescriptor& descriptor, uint8_t* data);
	AkTexture(const AkTextureDescriptor& descriptor, const vk::Image& image);
	~AkTexture();

	size_t GetSize() const { return m_Size; }
	int32_t GetBindlessIndex() const { return m_BindlessIndex; }
	const std::vector<AkMipInfo>& GetMipsInfo() const { return m_MipsInfo; }

	const vk::Image& GetImage();
	const vk::ImageView& GetImageView(uint32_t mip = 0, uint32_t slice = 0);
	const AkTextureDescriptor& GetDescriptor() const { return m_Descriptor; }

private:
	bool m_FromNative = false;
	AkTextureDescriptor m_Descriptor;

	size_t m_Size = 0;
	int32_t m_BindlessIndex = -1;
	std::vector<AkMipInfo> m_MipsInfo = {};

	ForwardStorage<struct AkTextureStorage, 96> m_Storage;

	void CreateImageViews();
};