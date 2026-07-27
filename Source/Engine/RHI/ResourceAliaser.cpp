#include "ResourceAliaser.h"

#include "RHI/Device.h"
#include "Graphics/RenderPipeline/PipelineResourceId.h"
#include "Graphics/RenderPipeline/DirectAcyclicRenderGraph.h"

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

extern vk::SampleCountFlagBits GetMSAA(const AkMSAA msaa);
extern vk::Format GetFormat(const AkPixelFormat format);
extern vk::ImageType GetImageType(const AkTextureType type);
extern vk::ImageUsageFlags GetTextureUsageFlags(const AkTextureFlags flags);
extern vk::ImageCreateFlags GetImageCreateFlags(const AkTextureType type);
extern vk::BufferUsageFlags GetBufferUsageFlags(const AkBufferFlags flags);

struct AkAliasedResources
{
	VmaAllocation allocation = VK_NULL_HANDLE;
	std::unordered_map<AkPipelineResourceId, vk::Image> textures;
	std::unordered_map<AkPipelineResourceId, vk::Buffer> buffers;
};

void AkResourceAliaser::CreateAliasedResources(AkAliasingGroup& aliasingGroup)
{
	AkAssert(!aliasingGroup.rhiResources, "Trying to allocate resources on an already allocated aliasing group");
	aliasingGroup.rhiResources = std::make_shared<AkAliasedResources>();

	const vk::Device& device = AkDevice::GetDevice();
	const vk::Instance& instance = AkDevice::GetInstance();
	const VmaAllocator& allocator = AkDevice::GetMemoryAllocator();

	std::vector<vk::MemoryRequirements> memoryRequirements;
	for (const AkAliasingRequest& aliasingRequest : aliasingGroup.requests)
	{
		const AkResourceRequest& resourceRequest = aliasingRequest.resourceRequest;
		if (resourceRequest.type == AkResourceRequest::Type::TEXTURE)
		{
			const AkTextureDescriptor& descriptor = resourceRequest.descriptor.texture;
			const vk::ImageCreateInfo imageCreateInfo =
			{
				.flags = GetImageCreateFlags(descriptor.type),
				.imageType = GetImageType(descriptor.type),
				.format = GetFormat(descriptor.format),
				.extent =
				{
					.width = descriptor.width,
					.height = descriptor.height,
					.depth = descriptor.depth,
				},
				.mipLevels = descriptor.mips,
				.arrayLayers = descriptor.slices,
				.samples = GetMSAA(descriptor.msaa),
				.tiling = vk::ImageTiling::eOptimal,
				.usage = GetTextureUsageFlags(descriptor.flags),
				.sharingMode = vk::SharingMode::eExclusive,
				.initialLayout = vk::ImageLayout::eUndefined,
			};

			vk::Image image = device.createImage(imageCreateInfo);
			aliasingGroup.rhiResources->textures[resourceRequest.id] = image;
			memoryRequirements.push_back(device.getImageMemoryRequirements(image));

		}
		else if (resourceRequest.type == AkResourceRequest::Type::BUFFER)
		{
			const AkBufferDescriptor& descriptor = resourceRequest.descriptor.buffer;
			vk::BufferCreateInfo bufferCreateInfo =
			{
				.size = descriptor.size,
				.usage = GetBufferUsageFlags(descriptor.flags),
				.sharingMode = vk::SharingMode::eExclusive,
			};

			vk::Buffer buffer = device.createBuffer(bufferCreateInfo);
			aliasingGroup.rhiResources->buffers[resourceRequest.id] = buffer;
			memoryRequirements.push_back(device.getBufferMemoryRequirements(buffer));
		}
	}

	vk::MemoryRequirements finalMemoryRequirements = {};
	finalMemoryRequirements.memoryTypeBits = ~0u;

	for (const vk::MemoryRequirements& requirement : memoryRequirements)
	{
		finalMemoryRequirements.size = std::max(finalMemoryRequirements.size, requirement.size);
		finalMemoryRequirements.alignment = std::max(finalMemoryRequirements.alignment, requirement.alignment);
		finalMemoryRequirements.memoryTypeBits &= requirement.memoryTypeBits;
	}

	std::unordered_map<AkPipelineResourceId, size_t> adjustedMemoryOffsets;
	adjustedMemoryOffsets[aliasingGroup.requests[0].resourceRequest.id] = 0;

	for (size_t i = 1; i < aliasingGroup.requests.size(); ++i)
	{
		const AkAliasingRequest& aliasRequest = aliasingGroup.requests[i];
		if (aliasRequest.offset != 0)
		{
			const AkAliasingRequest& lastRequest = aliasingGroup.requests[i - 1];
			adjustedMemoryOffsets[aliasRequest.resourceRequest.id] = adjustedMemoryOffsets[lastRequest.resourceRequest.id] + memoryRequirements[i - 1].size;
		}
	}

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
	allocCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	VkResult result = vmaAllocateMemory(allocator, finalMemoryRequirements, &allocCreateInfo, &aliasingGroup.rhiResources->allocation, nullptr);
	vk::detail::resultCheck(vk::Result(result), "vmaAllocateMemory");

	for (auto& [id, image] : aliasingGroup.rhiResources->textures)
	{
		result = vmaBindImageMemory2(allocator, aliasingGroup.rhiResources->allocation, adjustedMemoryOffsets[id], image, nullptr);
		vk::detail::resultCheck(vk::Result(result), "vmaBindMemory2");
	
		for (const AkAliasingRequest& aliasingRequest : aliasingGroup.requests)
		{
			if (aliasingRequest.resourceRequest.id == id)
			{
				AkTexture* texture = new AkTexture(aliasingRequest.resourceRequest.descriptor.texture, image);
				texture->SetDebugName(id.GetName());
				aliasingGroup.textures[id] = texture;
				break;
			}
		}
	}
	
	for (auto& [id, buffer] : aliasingGroup.rhiResources->buffers)
	{
		result = vmaBindBufferMemory2(allocator, aliasingGroup.rhiResources->allocation, adjustedMemoryOffsets[id], buffer, nullptr);
		vk::detail::resultCheck(vk::Result(result), "vmaBindMemory2");

		for (const AkAliasingRequest& aliasingRequest : aliasingGroup.requests)
		{
			if (aliasingRequest.resourceRequest.id == id)
			{
				AkBuffer* akBuffer = new AkBuffer(aliasingRequest.resourceRequest.descriptor.buffer, buffer);
				akBuffer->SetDebugName(id.GetName());
				aliasingGroup.buffers[id] = akBuffer;
				break;
			}
		}
	}
}

void AkResourceAliaser::FreeAliasedResources(const AkAliasingGroup& aliasingGroup)
{
	if (aliasingGroup.rhiResources)
	{
		const vk::Device& device = AkDevice::GetDevice();
		const VmaAllocator& allocator = AkDevice::GetMemoryAllocator();

		for (auto& [id, image] : aliasingGroup.rhiResources->textures)
		{
			device.destroyImage(image);
			delete aliasingGroup.textures.at(id);
		}

		for (auto& [id, buffer] : aliasingGroup.rhiResources->buffers)
		{
			device.destroyBuffer(buffer);
			delete aliasingGroup.buffers.at(id);
		}

		vmaFreeMemory(allocator, aliasingGroup.rhiResources->allocation);
	}
}
