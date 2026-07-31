#include "Scheduler.h"

#include "Platform/Window.h"
#include "Platform/Events.h"

#include "RHI/Swapchain.h"
#include "RHI/UploadManager.h"
#include "RHI/Textures/RenderTarget.h"
#include "RHI/CommandBuffers/CommandBufferAllocator.h"

#include "Graphics/RenderPipeline/DirectAcyclicRenderGraph.h"

#include "ECS/Registry.h"
#include "ECS/Components/Camera.h"
#include "ECS/Components/Transform.h"

#include <map>
#include <unordered_set>

static AkDirectAcyclicRenderGraph sRenderGraph;

void AkScheduler::Run()
{
	while (true)
	{
		if (AkEvents::ShouldClose())
			break;

		ProcessGameThread();
		ProcessRenderThread();
	}

	AkDevice::WaitIdle();
	sRenderGraph.FreeManagedResources();
}

void AkScheduler::SetWindow(AkWindow* window)
{
	m_Window = window;
}

void AkScheduler::SetSwapchain(AkSwapchain* swapchain)
{
	m_Swapchain = swapchain;
}

void AkScheduler::ProcessGameThread()
{
	AkEvents::PollEvents();

	AkRegistry::GetView<AkTransform>().ForEach([](AkTransform& transform)
	{
		transform.Update();
	});

	const glm::uvec2 windowResolution = m_Window->GetSize();
	AkRegistry::GetView<AkTransform, AkCamera>().ForEach([windowResolution](AkTransform& transform, AkCamera& camera)
	{
		AkRenderTarget* renderTarget = camera.GetRenderTarget();
		const glm::uvec2 targetResolution = renderTarget ? renderTarget->GetSize() : windowResolution;
		camera.Update(transform, targetResolution);
	});
}

void AkScheduler::ProcessRenderThread()
{
	if (m_Swapchain->Prepare())
	{
		bool dagNeedsRecompilation = false;
		std::vector<AkCommandBuffer*> commandBuffersToSubmit = {};
		std::multimap<uint8_t, AkRenderPipeline*> groupedRenderPipelines = {};
		AkComponentsView<AkCamera> cameraView = AkRegistry::GetView<AkCamera>();
		
		if (AkUploadManager::HasPendingUploads())
		{
			AkUploadManager::FillCommandBuffer();
			commandBuffersToSubmit.push_back(AkUploadManager::GetCommandBuffer());
		}

		if (cameraView.Count())
		{
			cameraView.ForEach([this, &dagNeedsRecompilation, &groupedRenderPipelines](AkEntity entity, AkCamera& camera)
			{
				const size_t renderPipelineHash = camera.GetRenderPipelineHash();
				if (m_RenderPipelines.contains(entity))
				{
					std::unique_ptr<AkRenderPipeline>& renderPipeline = m_RenderPipelines[entity];
					if (renderPipeline->GetHash() != renderPipelineHash)
					{
						std::unique_ptr<AkRenderPipeline> newRenderPipeline = AkRenderPipelineTypeInfoDatabase::Create(renderPipelineHash);
						renderPipeline.swap(newRenderPipeline);
						dagNeedsRecompilation = true;
					}

					renderPipeline->Initialize(m_Swapchain, camera.GetRenderTargetResolution());
					groupedRenderPipelines.insert({ camera.GetRenderOrder(), renderPipeline.get() });
				}
				else
				{
					m_RenderPipelines[entity] = AkRenderPipelineTypeInfoDatabase::Create(renderPipelineHash);
					m_RenderPipelines[entity]->Initialize(m_Swapchain, camera.GetRenderTargetResolution());
					groupedRenderPipelines.insert({ camera.GetRenderOrder(), m_RenderPipelines[entity].get() });
					dagNeedsRecompilation = true;
				}
			});

			const std::vector<AkEntity>& usedEntities = cameraView.GetEntities();
			std::unordered_set<AkEntity> usedEntitiesSet(usedEntities.begin(), usedEntities.end());
			
			const size_t sizeBeforeErase = m_RenderPipelines.size();
			std::erase_if(m_RenderPipelines, [&usedEntitiesSet](auto& renderPipeline)
			{
				return !usedEntitiesSet.contains(renderPipeline.first);
			});

			if (m_RenderPipelines.size() != sizeBeforeErase)
				dagNeedsRecompilation = true;
		}

		if (groupedRenderPipelines.empty())
		{
			static std::vector<AkCommandBuffer*> sCommandBuffers = AkCommandBufferAllocator::AllocateCommandBuffers(AkDeviceQueue::GRAPHICS, m_Swapchain->GetBackBuffersCount());
			AkCommandBuffer*& commandBuffer = commandBuffersToSubmit.emplace_back(sCommandBuffers[m_Swapchain->GetCurrentFrameIndex()]);

			commandBuffer->Begin();
			AkRenderTarget* backBuffer = m_Swapchain->GetCurrentBackBufferRenderTarget();
			commandBuffer->TransitionRenderTargetColorAttachments(backBuffer, AkResourceState::UNDEFINED, AkResourceState::PRESENT);
			commandBuffer->End();
		}
		else
		{
			if (dagNeedsRecompilation)
			{
				for (auto& [renderOrder, renderPipeline] : groupedRenderPipelines)
					renderPipeline->Setup();

				sRenderGraph.Compile(groupedRenderPipelines);
			}

			for (auto& [renderOrder, renderPipeline] : groupedRenderPipelines)
				renderPipeline->SetFrameData();

			sRenderGraph.SetBackBufferRenderTarget(m_Swapchain->GetCurrentBackBufferRenderTarget());
			sRenderGraph.Execute(commandBuffersToSubmit);
		}

		m_Swapchain->Present(commandBuffersToSubmit);
	}
}
