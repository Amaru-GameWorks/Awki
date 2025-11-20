#pragma once
#include "CommandBuffer.h"

#include <memory>
#include <unordered_map>

enum class AkDeviceQueue
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
	static std::vector<AkCommandBuffer*> AllocateCommandBuffers(const AkDeviceQueue deviceQueue, const uint32_t count);
	static void ReturnCommandBuffer(AkCommandBuffer* commandBuffer);

private:
	static inline std::unordered_map<AkDeviceQueue, std::vector<std::unique_ptr<AkCommandBuffer>>> m_CommandBuffers;
};