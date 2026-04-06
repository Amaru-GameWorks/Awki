#pragma once
#include "RHI/ResourceStates.h"
#include "Utilities/ForwardStorage.h"
#include "RHI/Pipeline/RasterizerState.h"

#include <glm/vec4.hpp>

namespace vk 
{ 
	class CommandPool; 
	class CommandBuffer; 
}

class AkCommandBuffer
{
	friend class AkCommandBufferAllocator;

public:
	AkCommandBuffer(const vk::CommandPool& commandPool, const vk::CommandBuffer& commandBuffer);
	~AkCommandBuffer();

	void Begin();
	void End();

	void BeginRendering(class AkRenderTarget* renderTarget);
	void EndRendering();

	void DrawPrimitive(class AkMaterial* material, const AkPrimitiveType primitiveType, const uint32_t vertexCount);

	void TransitionRenderTargetColorAttachments(class AkRenderTarget* renderTarget, const AkResourceState sourceState, const AkResourceState destinationState);
	void TransitionRenderTargetDepthAttachment(class AkRenderTarget* renderTarget, const AkResourceState sourceState, const AkResourceState destinationState);
	void TransitionTexture(class AkTexture* texture, const AkResourceState sourceState, const AkResourceState destinationState);
	void TransitionBuffer(class AkBuffer* buffer, const AkResourceState sourceState, const AkResourceState destinationState);
	void TransitionResources(const std::vector<class AkBuffer*>& buffers, const std::vector<class AkTexture*>& textures, const AkResourceState sourceState, const AkResourceState destinationState);
	
	void CopyBufferToBuffer(class AkBuffer* source, class AkBuffer* destination, const size_t size, const size_t sourceOffset = 0, const size_t destinationOffset = 0);
	void CopyBufferToTexture(class AkBuffer* source, class AkTexture* destination, const size_t sourceOffset = 0);
	
	void ClearColor(class AkTexture* texture, const AkResourceState sourceState, const glm::vec4& color);

	vk::CommandBuffer& GetBuffer();

private:
	class AkRenderTarget* m_CurrentRenderTarget = nullptr;
	ForwardStorage<struct AkCommandBufferStorage, 16> m_Storage;
};