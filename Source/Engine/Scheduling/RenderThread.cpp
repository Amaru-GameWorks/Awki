#include "RenderThread.h"
#include "FrameData.h"

#include "Core/EngineContext.h"
#include "RHI/UploadManager.h"
#include "Graphics/RenderPipeline/RenderPipeline.h"
#include "RHI/CommandBuffers/CommandBufferAllocator.h"
#include "Graphics/RenderPipeline/DirectAcyclicRenderGraph.h"

//Temporary direct ECS access as there is no FrameData yet
#include "ECS/Registry.h"
#include "ECS/Components/Camera.h"

#include <chrono>
using namespace std::chrono_literals;

static AkDirectAcyclicRenderGraph sRenderGraph;
static std::unordered_map<AkEntity, std::unique_ptr<AkRenderPipeline>> sRenderPipelines;

AkJobInitialSuspend AkRenderThread::Execute(size_t frameIndex)
{
	const AkFrameData& frameData = gFrameData[frameIndex % kMaxFramesInFlight];
	AkLogTrace("RenderThread {}", frameData.frameIndex);

	AkSwapchain& mainWindowSwapchain = AkEngineContext::GetMainWindowSwapchain();
	if (mainWindowSwapchain.Prepare())
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
			cameraView.ForEach([&mainWindowSwapchain, &dagNeedsRecompilation, &groupedRenderPipelines](AkEntity entity, AkCamera& camera)
			{
				const size_t renderPipelineHash = camera.GetRenderPipelineHash();
				if (sRenderPipelines.contains(entity))
				{
					std::unique_ptr<AkRenderPipeline>& renderPipeline = sRenderPipelines[entity];
					if (renderPipeline->GetHash() != renderPipelineHash)
					{
						std::unique_ptr<AkRenderPipeline> newRenderPipeline = AkRenderPipelineTypeInfoDatabase::Create(renderPipelineHash);
						renderPipeline.swap(newRenderPipeline);
						dagNeedsRecompilation = true;
					}

					renderPipeline->Initialize(&mainWindowSwapchain, camera.GetRenderTargetResolution());
					groupedRenderPipelines.insert({ camera.GetRenderOrder(), renderPipeline.get() });
				}
				else
				{
					sRenderPipelines[entity] = AkRenderPipelineTypeInfoDatabase::Create(renderPipelineHash);
					sRenderPipelines[entity]->Initialize(&mainWindowSwapchain, camera.GetRenderTargetResolution());
					groupedRenderPipelines.insert({ camera.GetRenderOrder(), sRenderPipelines[entity].get() });
					dagNeedsRecompilation = true;
				}
			});

			const std::vector<AkEntity>& usedEntities = cameraView.GetEntities();
			std::unordered_set<AkEntity> usedEntitiesSet(usedEntities.begin(), usedEntities.end());

			const size_t sizeBeforeErase = sRenderPipelines.size();
			std::erase_if(sRenderPipelines, [&usedEntitiesSet](auto& renderPipeline)
			{
				return !usedEntitiesSet.contains(renderPipeline.first);
			});

			if (sRenderPipelines.size() != sizeBeforeErase)
				dagNeedsRecompilation = true;
		}

		if (groupedRenderPipelines.empty())
		{
			static std::vector<AkCommandBuffer*> sCommandBuffers = AkCommandBufferAllocator::AllocateCommandBuffers(AkDeviceQueue::GRAPHICS, mainWindowSwapchain.GetBackBuffersCount());
			AkCommandBuffer*& commandBuffer = commandBuffersToSubmit.emplace_back(sCommandBuffers[mainWindowSwapchain.GetCurrentFrameIndex()]);

			commandBuffer->Begin();
			AkRenderTarget* backBuffer = mainWindowSwapchain.GetCurrentBackBufferRenderTarget();
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

			sRenderGraph.SetBackBufferRenderTarget(mainWindowSwapchain.GetCurrentBackBufferRenderTarget());
			sRenderGraph.Execute(commandBuffersToSubmit);
		}

		mainWindowSwapchain.Present(commandBuffersToSubmit);
	}

	gFrameDataAvailable.Signal();
	co_return;
}

void AkRenderThread::FreeResources()
{
	sRenderGraph.FreeManagedResources();
}
