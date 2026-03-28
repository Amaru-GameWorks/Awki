#include "Engine.h"
#include "Log.h"
#include "RHI/Device.h"
#include "RHI/Swapchain.h"
#include "Platform/Window.h"
#include "Platform/Events.h"

#include "RHI/Pipeline/PipelineStateManager.h"
#include "RHI/Pipeline/BindlessResourcesManager.h"
#include "RHI/CommandBuffers/CommandBufferAllocator.h"

Awki::Awki(const AkInstanceDescriptor& descriptor)
{
	if (!AkLog::Initialize())
		throw std::runtime_error("Failed to initialize Logging System!");

	AkLogInfo("Awki {} initializing", kEngineVersion);

	if (!AkEvents::Initialize())
		throw std::runtime_error("Failed to initialize Events System!");

	if (!AkDevice::Initialize())
		throw std::runtime_error("Failed to initialize RHI Device!");

	AkPipelineStateManager::Initialize();
	AkBindlessResourcesManager::Initialize();

	m_Window = std::make_shared<AkWindow>(descriptor.windowDescriptor);
	m_Swapchain = std::make_shared<AkSwapchain>(m_Window);

	AkLogInfo("{} {} initializing", descriptor.gameName, descriptor.gameVersion);
}

Awki::~Awki()
{
	AkLogInfo("Awki {} deinitializing", kEngineVersion);

	m_Swapchain.reset();
	m_Window.reset();

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
		AkEvents::PollEvents();
		if (m_Swapchain->Prepare())
		{
			AkCommandBuffer* currentCommandBuffer = commandBuffers[m_Swapchain->GetCurrentFrameIndex()];
			AkRenderTarget* currentBackBuffer = m_Swapchain->GetCurrentBackBufferRenderTarget();
			
			currentCommandBuffer->Begin();
			m_OnFrameRender.Broadcast(currentCommandBuffer, currentBackBuffer);
			currentCommandBuffer->End();
			
			m_Swapchain->Present({ currentCommandBuffer });
		}
	}

	AkDevice::WaitIdle();
	m_OnEngineShutdown.Broadcast();
}