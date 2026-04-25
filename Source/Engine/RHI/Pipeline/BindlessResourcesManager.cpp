#include "BindlessResourcesManager.h"
#include "RHI/Device.h"
#include "Core/Assert.h"
#include "RHI/Buffers/Buffer.h"
#include "RHI/Textures/Texture.h"
#include "RHI/Samplers/Sampler.h"

#include <vulkan/vulkan.hpp>

static vk::DescriptorPool sDescriptorPool = {};

static vk::DescriptorSet sDescriptorSet = {};
static vk::DescriptorSetLayout sDescriptorSetLayout = {};

void AkBindlessResourcesManager::Initialize()
{
	const vk::Device& device = AkDevice::GetDevice();

	const std::array<vk::DescriptorPoolSize, 4> kPoolSizes =
	{
		vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageBuffer, .descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources * 2 },
		vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampledImage, .descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources},
		vk::DescriptorPoolSize{.type = vk::DescriptorType::eStorageImage, .descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources},
		vk::DescriptorPoolSize{.type = vk::DescriptorType::eSampler, .descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources }
	};

	const vk::DescriptorPoolCreateInfo poolCreateInfo =
	{
		.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
		.maxSets = 1,
		.poolSizeCount = static_cast<uint32_t>(kPoolSizes.size()),
		.pPoolSizes = kPoolSizes.data()
	};

	sDescriptorPool = device.createDescriptorPool(poolCreateInfo);

	const std::vector<vk::DescriptorSetLayoutBinding> bindings =
	{
		{
			.binding = 0,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources,
			.stageFlags = vk::ShaderStageFlagBits::eAll
		},
		{
			.binding = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources,
			.stageFlags = vk::ShaderStageFlagBits::eAll
		},
		{
			.binding = 2,
			.descriptorType = vk::DescriptorType::eSampledImage,
			.descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources,
			.stageFlags = vk::ShaderStageFlagBits::eAll
		},
		{
			.binding = 3,
			.descriptorType = vk::DescriptorType::eStorageImage,
			.descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources,
			.stageFlags = vk::ShaderStageFlagBits::eAll
		},
		{
			.binding = 4,
			.descriptorType = vk::DescriptorType::eSampler,
			.descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources,
			.stageFlags = vk::ShaderStageFlagBits::eAll
		}
	};

	const std::vector<vk::DescriptorBindingFlags> kBindingFlags(5, vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind);
	vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo =
	{
		.bindingCount = static_cast<uint32_t>(kBindingFlags.size()),
		.pBindingFlags = kBindingFlags.data()
	};

	const vk::DescriptorSetLayoutCreateInfo setLayoutCreateInfo =
	{
		.pNext = &bindingFlagsCreateInfo,
		.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};
	sDescriptorSetLayout = device.createDescriptorSetLayout(setLayoutCreateInfo);

	const vk::DescriptorSetAllocateInfo setAllocateInfo =
	{
		.descriptorPool = sDescriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &sDescriptorSetLayout
	};

	std::vector<vk::DescriptorSet> allocatedSets = device.allocateDescriptorSets(setAllocateInfo);
	if (!allocatedSets.empty())
		sDescriptorSet = allocatedSets[0];
}

void AkBindlessResourcesManager::Deinitialize()
{
	const vk::Device& device = AkDevice::GetDevice();
	device.destroyDescriptorSetLayout(sDescriptorSetLayout);
	device.destroyDescriptorPool(sDescriptorPool);
}

void AkBindlessResourcesManager::AddBuffer(AkBuffer* buffer)
{
	AkAssert(sBuffersCount < kMaxBindlessResources, "Maximum amount of buffers exceded!");

	if (!sBuffersFreeList.empty())
	{
		buffer->m_BindlessIndex = sBuffersFreeList.front();
		sBuffersFreeList.pop();
	}
	else
		buffer->m_BindlessIndex = sBuffersCount.fetch_add(1);

	const vk::DescriptorBufferInfo bufferInfo =
	{
		.buffer = buffer->GetBuffer(),
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};

	const std::vector<vk::WriteDescriptorSet> writeDescriptorSets =
	{
		{
			.dstSet = sDescriptorSet,
			.dstBinding = 0,
			.dstArrayElement = static_cast<uint32_t>(buffer->m_BindlessIndex),
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &bufferInfo
		},
		{
			.dstSet = sDescriptorSet,
			.dstBinding = 1,
			.dstArrayElement = static_cast<uint32_t>(buffer->m_BindlessIndex),
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &bufferInfo
		}
	};

	const vk::Device& device = AkDevice::GetDevice();
	device.updateDescriptorSets(2, writeDescriptorSets.data(), 0, nullptr);
}

void AkBindlessResourcesManager::AddTexture(AkTexture* texture)
{
	AkAssert(sTexturesCount < kMaxBindlessResources, "Maximum amount of textures exceded!");

	if (!sTexturesFreeList.empty())
	{
		texture->m_BindlessIndex = sTexturesFreeList.front();
		sTexturesFreeList.pop();
	}
	else
		texture->m_BindlessIndex = sTexturesCount.fetch_add(1);

	const vk::DescriptorImageInfo readImageInfo =
	{
		.imageView = texture->GetImageView(),
		.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
	};

	const vk::DescriptorImageInfo writeImageInfo =
	{
		.imageView = texture->GetImageView(),
		.imageLayout = vk::ImageLayout::eGeneral
	};

	std::vector<vk::WriteDescriptorSet> writeDescriptorSets =
	{{
		.dstSet = sDescriptorSet,
		.dstBinding = 2,
		.dstArrayElement = static_cast<uint32_t>(texture->m_BindlessIndex),
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eSampledImage,
		.pImageInfo = &readImageInfo
	}};

	const AkTextureDescriptor& descriptor = texture->GetDescriptor();
	if (descriptor.flags & AkTextureFlags_ALLOW_UNORDERED_ACCESS)
	{
		writeDescriptorSets.push_back
		({
			.dstSet = sDescriptorSet,
			.dstBinding = 3,
			.dstArrayElement = static_cast<uint32_t>(texture->m_BindlessIndex),
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageImage,
			.pImageInfo = &writeImageInfo
		});
	}

	const vk::Device& device = AkDevice::GetDevice();
	device.updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void AkBindlessResourcesManager::AddSampler(AkSampler* sampler)
{
	AkAssert(sSamplersCount < kMaxBindlessResources, "Maximum amount of samplers exceded!");

	if (!sSamplersFreeList.empty())
	{
		sampler->m_BindlessIndex = sTexturesFreeList.front();
		sSamplersFreeList.pop();
	}
	else
		sampler->m_BindlessIndex = sSamplersCount.fetch_add(1);

	const vk::DescriptorImageInfo imageInfo = { .sampler = sampler->GetSampler() };
	const vk::WriteDescriptorSet writeDescriptorSet =
	{
		.dstSet = sDescriptorSet,
		.dstBinding = 4,
		.dstArrayElement = static_cast<uint32_t>(sampler->m_BindlessIndex),
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eSampler,
		.pImageInfo = &imageInfo
	};

	const vk::Device& device = AkDevice::GetDevice();
	device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
}

void AkBindlessResourcesManager::RemoveBuffer(AkBuffer* buffer)
{
	if (buffer->m_BindlessIndex != -1)
		sBuffersFreeList.push(buffer->m_BindlessIndex);
}

void AkBindlessResourcesManager::RemoveTexture(AkTexture* texture)
{
	if (texture->m_BindlessIndex != -1)
		sTexturesFreeList.push(texture->m_BindlessIndex);
}

void AkBindlessResourcesManager::RemoveSampler(AkSampler* sampler)
{
	if (sampler->m_BindlessIndex != -1)
		sSamplersFreeList.push(sampler->m_BindlessIndex);
}

const vk::DescriptorSet& AkBindlessResourcesManager::GetDescriptorSet()
{
	return sDescriptorSet;
}

const vk::DescriptorSetLayout& AkBindlessResourcesManager::GetDescriptorSetLayout()
{
	return sDescriptorSetLayout;
}