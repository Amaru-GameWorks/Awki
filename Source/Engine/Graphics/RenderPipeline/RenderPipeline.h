#pragma once
#include "RHI/Swapchain.h"
#include "Graphics/RenderPass/RenderPass.h"

#include <vector>
#include <memory>
#include <glm/vec2.hpp>

class AkRenderPipeline
{
public:
	AkRenderPipeline() = default;
	virtual ~AkRenderPipeline() = default;

	void Initialize(AkSwapchain* swapchain, const glm::uvec2& renderTargetResolution);
	std::vector<class AkCommandBuffer*>& GetFrameCommandBuffers();
	std::vector<class AkCommandBuffer*> GetExecutingCommandBuffers();
	
	const glm::uvec2& GetRenderTargetResolution() const { return m_RenderTargetResolution; }
	const std::vector<AkRenderPass*>& GetExecutingRenderPasses() const { return m_ExecutingRenderPasses; }

	template<typename T, typename ...Args>
	requires (std::derived_from<T, AkRenderPass>)
	void AddRenderPass(Args&&... Arguments)
	{
		m_RenderPasses.push_back(std::make_unique<T>(this, Arguments...));
	}

	virtual void Setup();
	virtual void SetFrameData();
	virtual size_t GetHash() = 0;

protected:
	uint8_t m_FrameIndex = 0;
	glm::uvec2 m_RenderTargetResolution = {};
	class AkSwapchain* m_Swapchain = nullptr;
	std::vector<AkRenderPass*> m_ExecutingRenderPasses;
	std::vector<std::unique_ptr<AkRenderPass>> m_RenderPasses;
	std::vector<std::vector<class AkCommandBuffer*>> m_CommandBuffers;
};