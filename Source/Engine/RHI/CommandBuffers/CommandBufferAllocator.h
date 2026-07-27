#pragma once
#include "CommandBuffer.h"

#include <memory>
#include <unordered_map>

enum class AkDeviceQueue : uint8_t
{
	GRAPHICS,
	COMPUTE,
	TRANSFER
};

class AkCommandBufferAllocator
{
public:
	static bool Initialize();
	static void Deinitialize();

	static AkCommandBuffer* AllocateCommandBuffer(const AkDeviceQueue deviceQueue);
	static std::vector<AkCommandBuffer*> AllocateCommandBuffers(const AkDeviceQueue deviceQueue, const size_t count);
	static void ReturnCommandBuffer(AkCommandBuffer* commandBuffer);
	static void ReturnCommandBuffers(std::vector<AkCommandBuffer*>& commandBuffers);

private:
	static inline std::unordered_map<AkDeviceQueue, std::vector<std::unique_ptr<AkCommandBuffer>>> m_CommandBuffers;
};