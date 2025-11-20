#pragma once
#include "RHI/PipelineStates.h"

#include <glm/vec4.hpp>
#include <memory>

namespace vk { class CommandPool; class CommandBuffer; }

class AkCommandBuffer
{
	friend class AkCommandBufferAllocator;

public:
	AkCommandBuffer(const vk::CommandPool& commandPool, const vk::CommandBuffer& commandBuffer);
	~AkCommandBuffer();
	
	void Begin();
	void End();

	void TransitionTexture(class AkTexture* texture, const AkResourceState sourceState, const AkResourceState destinationState);
	void ClearColor(class AkTexture* texture, const AkResourceState sourceState, const glm::vec4& color);

	vk::CommandBuffer& GetBuffer();

private:
	std::unique_ptr<struct AkCommandBufferStorage> m_Storage = nullptr;
};