#include "Texture.h"
#include "RHI/Device.h"
#include "RHI/UploadManager.h"
#include "RHI/Pipeline/BindlessResourcesManager.h"

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

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

extern vk::ImageAspectFlags GetImageAspectMask(const AkPixelFormat format)
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

extern vk::ImageViewType GetImageViewType(const AkTextureType type)
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

extern vk::ImageType GetImageType(const AkTextureType type)
{
	switch (type)
	{
		case AkTextureType::TEXTURE_1D:
		case AkTextureType::TEXTURE_ARRAY_1D:
			return vk::ImageType::e1D;

		case AkTextureType::CUBEMAP:
		case AkTextureType::CUBEMAP_ARRAY:
		case AkTextureType::TEXTURE_2D:
		case AkTextureType::TEXTURE_ARRAY_2D:
			return vk::ImageType::e2D;

		case AkTextureType::TEXTURE_3D:
			return vk::ImageType::e3D;

		default:
			AkLogCritical("Texture type not registered in this function");
			return vk::ImageType::e2D;
	}
}

extern vk::ImageUsageFlags GetTextureUsageFlags(const AkTextureFlags flags)
{
	vk::ImageUsageFlags usageFlags = {};

	if (flags & AkTextureFlags_BIND_AS_SHADER_RESOURCE)
		usageFlags |= vk::ImageUsageFlagBits::eSampled;

	if (flags & AkTextureFlags_BIND_AS_RENDER_TARGET)
		usageFlags |= vk::ImageUsageFlagBits::eColorAttachment;

	if (flags & AkTextureFlags_BIND_AS_DEPTH_STENCIL)
		usageFlags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;

	if (flags & AkTextureFlags_COPY_DESTINATION)
		usageFlags |= vk::ImageUsageFlagBits::eTransferDst;

	if (flags & AkTextureFlags_COPY_SOURCE)
		usageFlags |= vk::ImageUsageFlagBits::eTransferSrc;

	if (flags & AkTextureFlags_ALLOW_UNORDERED_ACCESS)
		usageFlags |= vk::ImageUsageFlagBits::eStorage;

	return usageFlags;
}

extern vk::ImageCreateFlags GetImageCreateFlags(const AkTextureType type)
{
	switch (type)
	{
		case AkTextureType::TEXTURE_1D:
		case AkTextureType::TEXTURE_2D:
		case AkTextureType::TEXTURE_3D:
		case AkTextureType::TEXTURE_ARRAY_1D:
		case AkTextureType::TEXTURE_ARRAY_2D:
			return {};

		case AkTextureType::CUBEMAP:
		case AkTextureType::CUBEMAP_ARRAY:
			return vk::ImageCreateFlagBits::eCubeCompatible;

		default:
			AkLogCritical("Texture type not registered in this function");
			return {};
	}
}

extern size_t CalculateTextureSize(const AkTextureDescriptor& descriptor)
{
	size_t size = 0;
	size_t offset = 0;
	bool isBlockCompressed = IsBlockCompressedPixelFormat(descriptor.format);
	for (uint32_t j = 0; j < descriptor.slices; ++j)
	{
		for (uint32_t i = 0; i < descriptor.mips; ++i)
		{
			size_t mipWidth = static_cast<size_t>(descriptor.width >> i);
			size_t mipHeight = static_cast<size_t>(descriptor.height >> i);

			size_t mipSize = 0;
			if (isBlockCompressed)
			{
				size_t numBlocksWide = (mipWidth + 3) / 4;
				size_t numBlocksHeight = (mipHeight + 3) / 4;
				mipSize = numBlocksWide * numBlocksHeight * GetCompressedBlockSize(descriptor.format);
			}
			else
			{
				size_t rowBytes = mipWidth * GetPixelSize(descriptor.format);
				mipSize = rowBytes * mipHeight;
			}

			mipSize *= static_cast<size_t>(descriptor.depth);
			offset += mipSize;
			size += mipSize;
		}
	}

	return size;
}

size_t CalculateTextureSizeInfo(const AkTextureDescriptor& descriptor, std::vector<AkMipInfo>& mipsInfo)
{
	mipsInfo.resize(descriptor.slices * descriptor.mips);

	size_t size = 0;
	size_t offset = 0;
	bool isBlockCompressed = IsBlockCompressedPixelFormat(descriptor.format);
	for (uint32_t j = 0; j < descriptor.slices; ++j)
	{
		for (uint32_t i = 0; i < descriptor.mips; ++i)
		{
			size_t mipWidth = static_cast<size_t>(descriptor.width >> i);
			size_t mipHeight = static_cast<size_t>(descriptor.height >> i);

			size_t mipSize = 0;
			if (isBlockCompressed)
			{
				size_t numBlocksWide = (mipWidth + 3) / 4;
				size_t numBlocksHeight = (mipHeight + 3) / 4;
				mipSize = numBlocksWide * numBlocksHeight * GetCompressedBlockSize(descriptor.format);
			}
			else
			{
				size_t rowBytes = mipWidth * GetPixelSize(descriptor.format);
				mipSize = rowBytes * mipHeight;
			}

			mipSize *= static_cast<size_t>(descriptor.depth);
			mipsInfo[i + (j * descriptor.slices)] = { mipSize, offset };

			offset += mipSize;
			size += mipSize;
		}
	}

	return size;
}

struct AkTextureStorage
{
	vk::Image image = nullptr;
	VmaAllocation allocation = nullptr;

	vk::Image multiSampledImage = nullptr;
	VmaAllocation multiSampledAllocation = nullptr;

	std::vector<vk::ImageView> views = {};
	std::vector<vk::ImageView> multiSampledViews = {};
};

AkTexture::AkTexture(const AkTextureDescriptor& descriptor, uint8_t* data)
	: m_Descriptor(descriptor)
{
	if (m_Descriptor.type == AkTextureType::CUBEMAP)
		m_Descriptor.slices = 6;

	if (m_Descriptor.type == AkTextureType::CUBEMAP_ARRAY)
		m_Descriptor.slices *= 6;

	if (m_Descriptor.flags & AkTextureFlags_SRGB_HINT)
		m_Descriptor.format = GetSRGB(m_Descriptor.format);

	m_Size = CalculateTextureSizeInfo(m_Descriptor, m_MipsInfo);

	const VmaAllocator& allocator = AkDevice::GetMemoryAllocator();
	const vk::ImageUsageFlags usageFlags = GetTextureUsageFlags(m_Descriptor.flags);
	bool autoResolveMSAA = m_Descriptor.msaa > AkMSAA::X1 && m_Descriptor.flags & AkTextureFlags_AUTO_RESOLVE_MSAA;

	vk::ImageCreateInfo imageCreateInfo =
	{
		.flags = GetImageCreateFlags(m_Descriptor.type),
		.imageType = GetImageType(m_Descriptor.type),
		.format = GetFormat(m_Descriptor.format),
		.extent =
		{
			.width = m_Descriptor.width,
			.height = m_Descriptor.height,
			.depth = m_Descriptor.depth,
		},
		.mipLevels = m_Descriptor.mips,
		.arrayLayers = m_Descriptor.slices,
		.samples = autoResolveMSAA ? vk::SampleCountFlagBits::e1 : GetMSAA(m_Descriptor.msaa),
		.tiling = vk::ImageTiling::eOptimal,
		.usage = usageFlags,
		.sharingMode = vk::SharingMode::eExclusive,
		.initialLayout = vk::ImageLayout::eUndefined,
	};

	VmaAllocationCreateInfo allocationInfo = {};
	allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocationInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	VkImage image = VK_NULL_HANDLE;
	VkResult result = vmaCreateImage(allocator, imageCreateInfo, &allocationInfo, &image, &m_Storage->allocation, nullptr);
	vk::detail::resultCheck(vk::Result(result), VULKAN_HPP_NAMESPACE_STRING "::Device::createImage");

	m_Storage->image = image;

	if (autoResolveMSAA)
	{
		constexpr vk::ImageUsageFlags kAttachmentMask = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment;

		imageCreateInfo.samples = GetMSAA(m_Descriptor.msaa);
		imageCreateInfo.usage = vk::ImageUsageFlagBits::eTransientAttachment | (usageFlags & kAttachmentMask);

#if PLATFORM_ANDROID
		allocationInfo.usage = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED;
		allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
#endif

		VkImage multiSampledImage = VK_NULL_HANDLE;
		result = vmaCreateImage(allocator, imageCreateInfo, &allocationInfo, &multiSampledImage, &m_Storage->multiSampledAllocation, nullptr);
		vk::detail::resultCheck(vk::Result(result), VULKAN_HPP_NAMESPACE_STRING "::Device::createImage");

		m_Storage->multiSampledImage = multiSampledImage;
	}

	CreateImageViews();

	if (data && (m_Descriptor.flags & AkTextureFlags_COPY_DESTINATION))
		AkUploadManager::QueueTextureUpload(this, data);

	if (m_Descriptor.flags & (AkTextureFlags_BIND_AS_SHADER_RESOURCE | AkTextureFlags_ALLOW_UNORDERED_ACCESS))
		AkBindlessResourcesManager::AddTexture(this);
}

AkTexture::AkTexture(const AkTextureDescriptor& descriptor, const vk::Image& image)
	: m_Descriptor(descriptor)
{
	if (m_Descriptor.type == AkTextureType::CUBEMAP)
		m_Descriptor.slices = 6;

	if (m_Descriptor.type == AkTextureType::CUBEMAP_ARRAY)
		m_Descriptor.slices *= 6;

	if (m_Descriptor.flags & AkTextureFlags_SRGB_HINT)
		m_Descriptor.format = GetSRGB(m_Descriptor.format);

	m_Size = CalculateTextureSizeInfo(m_Descriptor, m_MipsInfo);

	m_FromNative = true;
	m_Storage->image = image;

	CreateImageViews();

	if (m_Descriptor.flags & (AkTextureFlags_BIND_AS_SHADER_RESOURCE | AkTextureFlags_ALLOW_UNORDERED_ACCESS))
		AkBindlessResourcesManager::AddTexture(this);
}

AkTexture::~AkTexture()
{
	AkBindlessResourcesManager::RemoveTexture(this);

	const vk::Device& device = AkDevice::GetDevice();

	if (!m_FromNative)
	{
		device.destroyImage(m_Storage->image);

		if(m_Storage->multiSampledImage)
			device.destroyImage(m_Storage->multiSampledImage);

		const VmaAllocator& allocator = AkDevice::GetMemoryAllocator();
		vmaFreeMemory(allocator, m_Storage->allocation);
	}

	for (auto& view : m_Storage->views)
		device.destroyImageView(view);

	for (auto& view : m_Storage->multiSampledViews)
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

void AkTexture::SetDebugName([[maybe_unused]] const std::string& name)
{
#if DEBUG
	const vk::Device& device = AkDevice::GetDevice();
	const vk::DebugUtilsObjectNameInfoEXT nameInfo =
	{
		.objectType = m_Storage->image.objectType,
		.objectHandle = reinterpret_cast<uint64_t>(static_cast<VkImage>(m_Storage->image)),
		.pObjectName = name.c_str()
	};
	device.setDebugUtilsObjectNameEXT(nameInfo);
#endif
}

void AkTexture::CreateImageViews()
{
	vk::ComponentMapping componentMapping = {};
	const size_t channelCount = GetChannelCount(m_Descriptor.format);

	if (channelCount >= 3)
		componentMapping = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
	else if (channelCount == 2)
		componentMapping = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG };
	else if (channelCount == 1)
		componentMapping = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eR };

	auto CreateViews = [this, componentMapping](vk::Image image, std::vector<vk::ImageView>& views)
	{
		const vk::Device& device = AkDevice::GetDevice();

		for (uint32_t i = 0; i < m_Descriptor.slices; ++i)
		{
			for (uint32_t j = 0; j < m_Descriptor.mips; ++j)
			{
				vk::ImageViewCreateInfo viewCreateInfo =
				{
					.image = image,
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
				views[viewIndex] = device.createImageView(viewCreateInfo);
			}
		}
	};

	m_Storage->views.resize(m_Descriptor.slices * m_Descriptor.mips);
	CreateViews(m_Storage->image, m_Storage->views);

	if (m_Storage->multiSampledImage)
	{
		m_Storage->multiSampledViews.resize(m_Descriptor.slices * m_Descriptor.mips);
		CreateViews(m_Storage->multiSampledImage, m_Storage->multiSampledViews);
	}
}