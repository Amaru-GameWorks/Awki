#include "BindlessResourcesManager.h"
#include "RHI/Device.h"
#include "Core/Assert.h"
#include "RHI/Buffers/Buffer.h"

#include <vulkan/vulkan.hpp>

static vk::DescriptorPool sDescriptorPool = {};

static vk::DescriptorSet sBuffersDescriptorSet = {};
static vk::DescriptorSetLayout sBuffersDescriptorSetLayout = {};

static vk::DescriptorSet sTexturesDescriptorSet = {};
static vk::DescriptorSetLayout sTexturesDescriptorSetLayout = {};

static vk::DescriptorSet sSamplersDescriptorSet = {};
static vk::DescriptorSetLayout sSamplersDescriptorSetLayout = {};

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
		.maxSets = 3,
		.poolSizeCount = static_cast<uint32_t>(kPoolSizes.size()),
		.pPoolSizes = kPoolSizes.data()
	};

	sDescriptorPool = device.createDescriptorPool(poolCreateInfo);

	const std::vector<vk::DescriptorSetLayoutBinding> buffersBindings =
	{
		{
			.binding = 32,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources,
			.stageFlags = vk::ShaderStageFlagBits::eAll
		},
		{
			.binding = 64,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources,
			.stageFlags = vk::ShaderStageFlagBits::eAll
		}
	};
	
	const std::vector<vk::DescriptorSetLayoutBinding> texturesBindings =
	{
		{
			.binding = 32,
			.descriptorType = vk::DescriptorType::eSampledImage,
			.descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources,
			.stageFlags = vk::ShaderStageFlagBits::eAll
		},
		{
			.binding = 64,
			.descriptorType = vk::DescriptorType::eStorageImage,
			.descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources,
			.stageFlags = vk::ShaderStageFlagBits::eAll
		}
	};

	const vk::DescriptorSetLayoutBinding samplersBinding =
	{
		.binding = 96,
		.descriptorType = vk::DescriptorType::eSampler,
		.descriptorCount = AkBindlessResourcesManager::kMaxBindlessResources,
		.stageFlags = vk::ShaderStageFlagBits::eAll
	};

	const std::array<vk::DescriptorBindingFlags, 2> kBindingFlags = { vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind, vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind };
	vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo =
	{
		.bindingCount = 2,
		.pBindingFlags = kBindingFlags.data()
	};

	const vk::DescriptorSetLayoutCreateInfo buffersSetLayoutCreateInfo = 
	{
		.pNext = &bindingFlagsCreateInfo,
		.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
		.bindingCount = 2,
		.pBindings = buffersBindings.data()
	};
	sBuffersDescriptorSetLayout = device.createDescriptorSetLayout(buffersSetLayoutCreateInfo);

	const vk::DescriptorSetLayoutCreateInfo texturesSetLayoutCreateInfo =
	{
		.pNext = &bindingFlagsCreateInfo,
		.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
		.bindingCount = 2,
		.pBindings = texturesBindings.data()
	};
	sTexturesDescriptorSetLayout = device.createDescriptorSetLayout(texturesSetLayoutCreateInfo);

	bindingFlagsCreateInfo.bindingCount = 1;
	const vk::DescriptorSetLayoutCreateInfo samplersSetLayoutCreateInfo =
	{
		.pNext = &bindingFlagsCreateInfo,
		.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
		.bindingCount = 1,
		.pBindings = &samplersBinding
	};
	sSamplersDescriptorSetLayout = device.createDescriptorSetLayout(samplersSetLayoutCreateInfo);

	const vk::DescriptorSetAllocateInfo buffersSetAllocateInfo = 
	{
		.descriptorPool = sDescriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &sBuffersDescriptorSetLayout
	};
	
	std::vector<vk::DescriptorSet> allocatedSets = device.allocateDescriptorSets(buffersSetAllocateInfo);
	if (!allocatedSets.empty())
		sBuffersDescriptorSet = allocatedSets[0];

	const vk::DescriptorSetAllocateInfo texturesSetAllocateInfo =
	{
		.descriptorPool = sDescriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &sTexturesDescriptorSetLayout
	};
	
	allocatedSets = device.allocateDescriptorSets(texturesSetAllocateInfo);
	if (!allocatedSets.empty())
		sTexturesDescriptorSet = allocatedSets[0];

	const vk::DescriptorSetAllocateInfo samplersSetAllocateInfo =
	{
		.descriptorPool = sDescriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &sSamplersDescriptorSetLayout
	};

	allocatedSets = device.allocateDescriptorSets(samplersSetAllocateInfo);
	if (!allocatedSets.empty())
		sSamplersDescriptorSet = allocatedSets[0];
}

void AkBindlessResourcesManager::Deinitialize()
{
	const vk::Device& device = AkDevice::GetDevice();
	device.destroyDescriptorSetLayout(sBuffersDescriptorSetLayout);
	device.destroyDescriptorSetLayout(sSamplersDescriptorSetLayout);
	device.destroyDescriptorSetLayout(sTexturesDescriptorSetLayout);
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
			.dstSet = sBuffersDescriptorSet,
			.dstBinding = 32,
			.dstArrayElement = static_cast<uint32_t>(buffer->m_BindlessIndex),
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &bufferInfo
		},
		{
			.dstSet = sBuffersDescriptorSet,
			.dstBinding = 64,
			.dstArrayElement = static_cast<uint32_t>(buffer->m_BindlessIndex),
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eStorageBuffer,
			.pBufferInfo = &bufferInfo
		}
	};

	const vk::Device& device = AkDevice::GetDevice();
	device.updateDescriptorSets(2, writeDescriptorSets.data(), 0, nullptr);
}

const vk::DescriptorSet& AkBindlessResourcesManager::GetBuffersDescriptorSet()
{
	return sBuffersDescriptorSet;
}

const vk::DescriptorSet& AkBindlessResourcesManager::GetTexturesDescriptorSet()
{
	return sTexturesDescriptorSet;
}

const vk::DescriptorSet& AkBindlessResourcesManager::GetSamplersDescriptorSet()
{
	return sSamplersDescriptorSet;
}

const vk::DescriptorSetLayout& AkBindlessResourcesManager::GetBuffersDescriptorSetLayout()
{
	return sBuffersDescriptorSetLayout;
}

const vk::DescriptorSetLayout& AkBindlessResourcesManager::GetTexturesDescriptorSetLayout()
{
	return sTexturesDescriptorSetLayout;
}

const vk::DescriptorSetLayout& AkBindlessResourcesManager::GetSamplersDescriptorSetLayout()
{
	return sSamplersDescriptorSetLayout;
}
