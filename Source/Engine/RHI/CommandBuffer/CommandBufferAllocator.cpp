#include "CommandBufferAllocator.h"
#include "RHI/Device.h"
#include "Core/Log.h"

#include <vulkan/vulkan.hpp>

static std::unordered_map<AkDeviceQueue, vk::CommandPool> sCommandPools;

bool AkCommandBufferAllocator::Initialize()
{
	const vk::Device& device = AkDevice::GetDevice();

	try
	{
		vk::CommandPoolCreateInfo poolCreateInfo =
		{
			.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			.queueFamilyIndex = AkDevice::GetGraphicsQueueFamilyIndex()
		};
		sCommandPools[AkDeviceQueue::GRAPHICS] = device.createCommandPool(poolCreateInfo);

		poolCreateInfo.queueFamilyIndex = AkDevice::GetComputeQueueFamilyIndex();
		sCommandPools[AkDeviceQueue::COMPUTE] = device.createCommandPool(poolCreateInfo);

		poolCreateInfo.queueFamilyIndex = AkDevice::GetTransferQueueFamilyIndex();
		sCommandPools[AkDeviceQueue::TRANSFER] = device.createCommandPool(poolCreateInfo);
	}
	catch (const std::exception& exception)
	{
		AkLogError("Failed to create command pool: {}", exception.what());
		return false;
	}

	return true;
}

void AkCommandBufferAllocator::Deinitialize()
{
	m_CommandBuffers.clear();

	const vk::Device& device = AkDevice::GetDevice();
	device.destroyCommandPool(sCommandPools[AkDeviceQueue::GRAPHICS]);
	device.destroyCommandPool(sCommandPools[AkDeviceQueue::COMPUTE]);
	device.destroyCommandPool(sCommandPools[AkDeviceQueue::TRANSFER]);
}

AkCommandBuffer* AkCommandBufferAllocator::AllocateCommandBuffer(const AkDeviceQueue deviceQueue)
{
	const vk::CommandBufferAllocateInfo bufferAllocateInfo =
	{
		.commandPool = sCommandPools[deviceQueue],
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};

	try
	{
		const vk::Device& device = AkDevice::GetDevice();
		std::vector<vk::CommandBuffer> vkCommandBuffer = device.allocateCommandBuffers(bufferAllocateInfo);
		std::unique_ptr<AkCommandBuffer> commandBuffer = std::make_unique<AkCommandBuffer>(sCommandPools[deviceQueue], vkCommandBuffer[0]);
		
		m_CommandBuffers[deviceQueue].push_back(std::move(commandBuffer));
		return m_CommandBuffers[deviceQueue].back().get();
	}
	catch (const std::exception& exception)
	{
		AkLogError("Failed to allocate command buffer: {}", exception.what());
		return nullptr;
	}
}

std::vector<AkCommandBuffer*> AkCommandBufferAllocator::AllocateCommandBuffers(const AkDeviceQueue deviceQueue, const uint32_t count)
{
	const vk::CommandBufferAllocateInfo bufferAllocateInfo =
	{
		.commandPool = sCommandPools[deviceQueue],
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = count
	};

	try
	{
		const vk::Device& device = AkDevice::GetDevice();
		std::vector<vk::CommandBuffer> vkCommandBuffers = device.allocateCommandBuffers(bufferAllocateInfo);

		std::vector<AkCommandBuffer*> commandBuffers;
		commandBuffers.resize(count);

		for (uint32_t i = 0; i < count; ++i)
		{
			std::unique_ptr<AkCommandBuffer> commandBuffer = std::make_unique<AkCommandBuffer>(sCommandPools[deviceQueue], vkCommandBuffers[i]);
			m_CommandBuffers[deviceQueue].push_back(std::move(commandBuffer));
			commandBuffers[i] = m_CommandBuffers[deviceQueue].back().get();
		}

		return commandBuffers;
	}
	catch (const std::exception& exception)
	{
		AkLogError("Failed to allocate command buffer: {}", exception.what());
		return {};
	}
}

void AkCommandBufferAllocator::ReturnCommandBuffer(AkCommandBuffer* commandBuffer)
{
	for (auto& [deviceQueue, commandBuffers] : m_CommandBuffers)
	{
		auto found = std::find_if(commandBuffers.begin(), commandBuffers.end(), [commandBuffer](const std::unique_ptr<AkCommandBuffer>& other) { return other.get() == commandBuffer; });
		if (found != commandBuffers.end())
		{
			commandBuffers.erase(found);
			return;
		}
	}
}