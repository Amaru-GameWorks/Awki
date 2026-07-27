#include "RenderPipeline.h"
#include "RHI/Swapchain.h"
#include "RHI/CommandBuffers/CommandBufferAllocator.h"

void AkRenderPipeline::Initialize(AkSwapchain* swapchain, const glm::uvec2& renderTargetResolution)
{
	m_Swapchain = swapchain;
	m_RenderTargetResolution = renderTargetResolution;
	
	const uint32_t backBuffersCount = m_Swapchain->GetBackBuffersCount();
	if(m_CommandBuffers.size() < backBuffersCount)
		m_CommandBuffers.resize(m_Swapchain->GetBackBuffersCount());
}

std::vector<class AkCommandBuffer*>& AkRenderPipeline::GetFrameCommandBuffers()
{
	const uint8_t frameIndex = m_Swapchain->GetCurrentFrameIndex();
	return m_CommandBuffers.at(frameIndex);
}

std::vector<class AkCommandBuffer*> AkRenderPipeline::GetExecutingCommandBuffers()
{
	std::vector<AkCommandBuffer*> commandBuffers;
	commandBuffers.reserve(m_ExecutingRenderPasses.size());

	for (auto renderPass : m_ExecutingRenderPasses)
		commandBuffers.push_back(renderPass->GetCommandBuffer());
		
	return commandBuffers;
}

void AkRenderPipeline::Setup()
{
	m_ExecutingRenderPasses.clear();
	m_ExecutingRenderPasses.reserve(m_RenderPasses.size());

	for (const auto& renderPass : m_RenderPasses)
	{
		if (renderPass->ShouldRun())
			m_ExecutingRenderPasses.push_back(renderPass.get());
	}

	for (size_t i = 0; i < m_ExecutingRenderPasses.size(); ++i)
	{
		AkRenderPass* renderPass = m_ExecutingRenderPasses[i];
		renderPass->ClearResourcesData();
		renderPass->RequestResources();
	}
}

void AkRenderPipeline::SetFrameData()
{
	std::vector<AkCommandBuffer*>& commandBuffers = GetFrameCommandBuffers();
	if (commandBuffers.empty())
	{
		commandBuffers = AkCommandBufferAllocator::AllocateCommandBuffers(AkDeviceQueue::GRAPHICS, m_ExecutingRenderPasses.size());
	}
	else if (commandBuffers.size() < m_ExecutingRenderPasses.size())
	{
		size_t neededCommandBufferCount = m_ExecutingRenderPasses.size() - commandBuffers.size();
		std::vector<AkCommandBuffer*> extraCommandBuffers = AkCommandBufferAllocator::AllocateCommandBuffers(AkDeviceQueue::GRAPHICS, neededCommandBufferCount);
		commandBuffers.insert_range(commandBuffers.end(), extraCommandBuffers);
	}

	for (size_t i = 0; i < m_ExecutingRenderPasses.size(); ++i)
	{
		AkRenderPass* renderPass = m_ExecutingRenderPasses[i];
		renderPass->SetCommandBuffer(commandBuffers[i]);
	}
}
