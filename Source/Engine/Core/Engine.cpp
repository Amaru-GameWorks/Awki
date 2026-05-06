#include "Engine.h"
#include "Log.h"
#include "RHI/Device.h"
#include "RHI/Swapchain.h"
#include "Platform/Window.h"
#include "Platform/Events.h"

#include "ECS/Registry.h"
#include "RHI/UploadManager.h"
#include "RHI/Samplers/Sampler.h"
#include "RHI/Pipeline/Material.h"
#include "RHI/Pipeline/PipelineStateManager.h"
#include "RHI/Pipeline/BindlessResourcesManager.h"
#include "RHI/CommandBuffers/CommandBufferAllocator.h"

std::unique_ptr<AkSampler> gPointClampSampler;
std::unique_ptr<AkSampler> gLinearClampSampler;

void InitializeEngineResources()
{
	AkMaterial::InitializeGlobalBuffer();

	AkSamplerDescriptor samplerDescriptor = {};
	samplerDescriptor.filterMode = AkFilterMode::NEAREST;
	samplerDescriptor.SetWrapMode(AkWrapMode::CLAMP_TO_EDGE);
	gPointClampSampler = std::make_unique<AkSampler>(samplerDescriptor);

	samplerDescriptor.filterMode = AkFilterMode::LINEAR;
	samplerDescriptor.SetWrapMode(AkWrapMode::CLAMP_TO_EDGE);
	gLinearClampSampler = std::make_unique<AkSampler>(samplerDescriptor);
}

void FreeEngineResources()
{
	gPointClampSampler = nullptr;
	gLinearClampSampler = nullptr;

	AkMaterial::DeinitializeGlobalBuffer();
}

Awki::Awki(const AkInstanceDescriptor& descriptor)
{
	if (!AkLog::Initialize())
		throw std::runtime_error("Failed to initialize Logging System!");

	AkLogInfo("Awki {} initializing", kEngineVersion);

	if (!AkEvents::Initialize())
		throw std::runtime_error("Failed to initialize Events System!");

	if (!AkDevice::Initialize())
		throw std::runtime_error("Failed to initialize RHI Device!");

	AkRegistry::Initialize();
	AkPipelineStateManager::Initialize();
	AkBindlessResourcesManager::Initialize();

	InitializeEngineResources();

	m_Window = std::make_unique<AkWindow>(descriptor.windowDescriptor);
	m_Swapchain = std::make_unique<AkSwapchain>(m_Window.get());

	AkLogInfo("{} {} initializing", descriptor.gameName, descriptor.gameVersion);
}

Awki::~Awki()
{
	AkLogInfo("Awki {} deinitializing", kEngineVersion);
	AkDevice::WaitIdle();

	m_Swapchain = nullptr;
	m_Window = nullptr;

	FreeEngineResources();
	AkUploadManager::ReleaseBuffers();

	AkBindlessResourcesManager::Deinitialize();
	AkPipelineStateManager::Deinitialize();
	AkDevice::Deinitialize();
	AkEvents::Deinitialize();
	AkLog::Deinitialize();
}

void Awki::Run()
{
	m_OnEngineStart.Broadcast();
	const std::vector<AkCommandBuffer*> commandBuffers = AkCommandBufferAllocator::AllocateCommandBuffers(AkDeviceQueue::GRAPHICS, m_Swapchain->GetBackBuffersCount());

	while (!AkEvents::ShouldClose())
	{
		//GameThread
		AkEvents::PollEvents();

		// RenderThread
		if (m_Swapchain->Prepare())
		{
			AkCommandBuffer* currentCommandBuffer = commandBuffers[m_Swapchain->GetCurrentFrameIndex()];
			AkRenderTarget* currentBackBuffer = m_Swapchain->GetCurrentBackBufferRenderTarget();

			currentCommandBuffer->Begin();
			m_OnFrameRender.Broadcast(currentCommandBuffer, currentBackBuffer);
			currentCommandBuffer->End();

			if (AkUploadManager::HasPendingUploads())
			{
				AkUploadManager::FillCommandBuffer();
				m_Swapchain->Present({ AkUploadManager::GetCommandBuffer(), currentCommandBuffer });
			}
			else
				m_Swapchain->Present({ currentCommandBuffer });
		}
	}

	AkDevice::WaitIdle();
	m_OnEngineShutdown.Broadcast();
}