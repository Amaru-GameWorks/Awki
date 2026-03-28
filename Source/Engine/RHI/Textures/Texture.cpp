#include "Texture.h"
#include "RHI/Device.h"

#include <vulkan/vulkan.hpp>

extern vk::SampleCountFlagBits GetMSAA(const AkMSAA msaa)
{
	switch (msaa)
	{
		case AkMSAA::X1:			return vk::SampleCountFlagBits::e1;
		case AkMSAA::X2:			return vk::SampleCountFlagBits::e2;
		case AkMSAA::X4:			return vk::SampleCountFlagBits::e4;
		case AkMSAA::X8:			return vk::SampleCountFlagBits::e8;
		default:
			AkLogCritical("MSAA not registered in this function");
			return vk::SampleCountFlagBits::e1;
	}
}

extern vk::Format GetFormat(const AkPixelFormat format)
{
	switch (format)
	{
		case AkPixelFormat::R8_UINT:				return vk::Format::eR8Uint;
		case AkPixelFormat::R8_SINT:				return vk::Format::eR8Sint;
		case AkPixelFormat::R8_UNORM:				return vk::Format::eR8Unorm;
		case AkPixelFormat::R8_SNORM:				return vk::Format::eR8Snorm;
		case AkPixelFormat::RG8_UINT:				return vk::Format::eR8G8Uint;
		case AkPixelFormat::RG8_SINT:				return vk::Format::eR8G8Sint;
		case AkPixelFormat::RG8_UNORM:				return vk::Format::eR8G8Unorm;
		case AkPixelFormat::RG8_SNORM:				return vk::Format::eR8G8Snorm;
		case AkPixelFormat::RGBA8_UINT:				return vk::Format::eR8G8B8A8Uint;
		case AkPixelFormat::RGBA8_SINT:				return vk::Format::eR8G8B8A8Sint;
		case AkPixelFormat::RGBA8_UNORM:			return vk::Format::eR8G8B8A8Unorm;
		case AkPixelFormat::RGBA8_SNORM:			return vk::Format::eR8G8B8A8Snorm;
		case AkPixelFormat::RGBA8_SRGB:				return vk::Format::eR8G8B8A8Srgb;
		case AkPixelFormat::BGRA8_UNORM:			return vk::Format::eB8G8R8A8Unorm;
		case AkPixelFormat::BGRA8_SRGB:				return vk::Format::eB8G8R8A8Srgb;
		case AkPixelFormat::R10G10B10A2_UNORM:		return vk::Format::eA2B10G10R10UnormPack32;
		case AkPixelFormat::R16_UINT:				return vk::Format::eR16Uint;
		case AkPixelFormat::R16_SINT:				return vk::Format::eR16Sint;
		case AkPixelFormat::R16_UNORM:				return vk::Format::eR16Unorm;
		case AkPixelFormat::R16_SNORM:				return vk::Format::eR16Snorm;
		case AkPixelFormat::R16_FLOAT:				return vk::Format::eR16Sfloat;
		case AkPixelFormat::RG16_UINT:				return vk::Format::eR16G16Uint;
		case AkPixelFormat::RG16_SINT:				return vk::Format::eR16G16Sint;
		case AkPixelFormat::RG16_UNORM:				return vk::Format::eR16G16Unorm;
		case AkPixelFormat::RG16_SNORM:				return vk::Format::eR16G16Snorm;
		case AkPixelFormat::RG16_FLOAT:				return vk::Format::eR16G16Sfloat;
		case AkPixelFormat::RGBA16_UINT:			return vk::Format::eR16G16B16A16Uint;
		case AkPixelFormat::RGBA16_SINT:			return vk::Format::eR16G16B16A16Sint;
		case AkPixelFormat::RGBA16_UNORM:			return vk::Format::eR16G16B16A16Unorm;
		case AkPixelFormat::RGBA16_SNORM:			return vk::Format::eR16G16B16A16Snorm;
		case AkPixelFormat::RGBA16_FLOAT:			return vk::Format::eR16G16B16A16Sfloat;
		case AkPixelFormat::R32_UINT:				return vk::Format::eR32Uint;
		case AkPixelFormat::R32_SINT:				return vk::Format::eR32Sint;
		case AkPixelFormat::R32_FLOAT:				return vk::Format::eR32Sfloat;
		case AkPixelFormat::RG32_UINT:				return vk::Format::eR32G32Uint;
		case AkPixelFormat::RG32_SINT:				return vk::Format::eR32G32Sint;
		case AkPixelFormat::RG32_FLOAT:				return vk::Format::eR32G32Sfloat;
		case AkPixelFormat::RGBA32_UINT:			return vk::Format::eR32G32B32A32Uint;
		case AkPixelFormat::RGBA32_SINT:			return vk::Format::eR32G32B32A32Sint;
		case AkPixelFormat::RGBA32_FLOAT:			return vk::Format::eR32G32B32A32Sfloat;
		case AkPixelFormat::BC1_RGB_UNORM:			return vk::Format::eBc1RgbUnormBlock;
		case AkPixelFormat::BC1_RGB_SRGB:			return vk::Format::eBc1RgbSrgbBlock;
		case AkPixelFormat::BC1_RGBA_UNORM:			return vk::Format::eBc1RgbaUnormBlock;
		case AkPixelFormat::BC1_RGBA_SRGB:			return vk::Format::eBc1RgbaSrgbBlock;
		case AkPixelFormat::BC2_UNORM:				return vk::Format::eBc2UnormBlock;
		case AkPixelFormat::BC2_SRGB:				return vk::Format::eBc2SrgbBlock;
		case AkPixelFormat::BC3_UNORM:				return vk::Format::eBc3UnormBlock;
		case AkPixelFormat::BC3_SRGB:				return vk::Format::eBc3SrgbBlock;
		case AkPixelFormat::BC4_UNORM:				return vk::Format::eBc4UnormBlock;
		case AkPixelFormat::BC4_SNORM:				return vk::Format::eBc4SnormBlock;
		case AkPixelFormat::BC5_UNORM:				return vk::Format::eBc5UnormBlock;
		case AkPixelFormat::BC5_SNORM:				return vk::Format::eBc5SnormBlock;
		case AkPixelFormat::BC6H_UF16:				return vk::Format::eBc6HUfloatBlock;
		case AkPixelFormat::BC6H_SF16:				return vk::Format::eBc6HSfloatBlock;
		case AkPixelFormat::BC7_UNORM:				return vk::Format::eBc7UnormBlock;
		case AkPixelFormat::BC7_SRGB:				return vk::Format::eBc7SrgbBlock;
		case AkPixelFormat::D32_SFLOAT_S8_UINT:		return vk::Format::eD32SfloatS8Uint;
		case AkPixelFormat::D32_SFLOAT:				return vk::Format::eD32Sfloat;
		case AkPixelFormat::D24_UNORM_S8_UINT:		return vk::Format::eD24UnormS8Uint;
		case AkPixelFormat::D16_UNORM:				return vk::Format::eD16Unorm;
		default:
			AkLogCritical("Pixel format not registered in this function");
			return vk::Format::eUndefined;
	}
}

constexpr vk::ImageViewType GetImageViewType(const AkTextureType type)
{
	switch (type)
	{
		case AkTextureType::TEXTURE_3D:			return vk::ImageViewType::e3D;
		case AkTextureType::TEXTURE_1D:			return vk::ImageViewType::e1D;
		case AkTextureType::TEXTURE_2D:			return vk::ImageViewType::e2D;
		case AkTextureType::TEXTURE_ARRAY_1D:	return vk::ImageViewType::e1DArray;
		case AkTextureType::TEXTURE_ARRAY_2D:	return vk::ImageViewType::e2DArray;
		case AkTextureType::CUBEMAP:			return vk::ImageViewType::eCube;
		case AkTextureType::CUBEMAP_ARRAY:		return vk::ImageViewType::eCubeArray;
		default:
			AkLogCritical("Texture type not registered in this function");
			return vk::ImageViewType::e2D;
	}
}

constexpr vk::ImageAspectFlags GetImageAspectMask(const AkPixelFormat format)
{
	vk::ImageAspectFlags aspectMask = {};
	if (IsDepthPixelFormat(format))
	{
		aspectMask = vk::ImageAspectFlagBits::eDepth;
		if (PixelFormatHasStencil(format))
			aspectMask |= vk::ImageAspectFlagBits::eStencil;
	}
	else
		aspectMask = vk::ImageAspectFlagBits::eColor;
	return aspectMask;
}

struct AkTextureStorage
{
	vk::Image image = nullptr;
	std::vector<vk::ImageView> views = {};
};

AkTexture::AkTexture(const AkTextureDescriptor& descriptor, const vk::Image& image)
	: m_Descriptor(descriptor)
{
	m_FromNative = true;
	m_Storage->image = image;
	
	CreateImageViews();
}

AkTexture::~AkTexture()
{
	const vk::Device& device = AkDevice::GetDevice();
	
	if (!m_FromNative)
		device.destroyImage(m_Storage->image);

	for (auto& view : m_Storage->views)
		device.destroyImageView(view);
}

const vk::Image& AkTexture::GetImage()
{
	return m_Storage->image;
}

const vk::ImageView& AkTexture::GetImageView(uint32_t mip, uint32_t slice)
{
	AkAssert(mip <= m_Descriptor.mips && slice <= m_Descriptor.slices, "Subresource index out of range");
	const uint32_t index = (slice * m_Descriptor.mips) + mip;
	return m_Storage->views[index];
}

void AkTexture::CreateImageViews()
{
	const vk::Device& device = AkDevice::GetDevice();
	m_Storage->views.resize(m_Descriptor.slices * m_Descriptor.mips);

	vk::ComponentMapping componentMapping = {};
	const size_t channelCount = GetChannelCount(m_Descriptor.format);

	if (channelCount >= 3)
		componentMapping = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	else if (channelCount == 2)
		componentMapping = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G };
	else if (channelCount == 1)
		componentMapping = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R };

	for (uint32_t i = 0; i < m_Descriptor.slices; ++i)
	{
		for (uint32_t j = 0; j < m_Descriptor.mips; ++j)
		{
			vk::ImageViewCreateInfo viewCreateInfo =
			{
				.image = m_Storage->image,
				.viewType = GetImageViewType(m_Descriptor.type),
				.format = GetFormat(m_Descriptor.format),
				.components = componentMapping,
				.subresourceRange =
				{
					.aspectMask = GetImageAspectMask(m_Descriptor.format),
					.baseMipLevel = j,
					.levelCount = VK_REMAINING_MIP_LEVELS,
					.baseArrayLayer = i,
					.layerCount = VK_REMAINING_ARRAY_LAYERS
				}
			};

			const uint32_t viewIndex = i * m_Descriptor.mips + j;
			m_Storage->views[viewIndex] = device.createImageView(viewCreateInfo);
		}
	}
}