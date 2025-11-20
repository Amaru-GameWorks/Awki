#include "Engine.h"
#include "Log.h"
#include "RHI/Device.h"
#include "RHI/Swapchain.h"
#include "Platform/Window.h"
#include "Platform/Events.h"
#include "RHI/CommandBuffer/CommandBufferAllocator.h"

bool Awki::Initialize(const AkInstanceDescriptor& descriptor)
{
	if (!AkLog::Initialize())
		return false;

	AkLogInfo("Awki {} initializing", kEngineVersion);

	if (!AkEvents::Initialize())
		return false;

	if (!AkDevice::Initialize())
		return false;

	if (!AkCommandBufferAllocator::Initialize())
		return false;
	
	try
	{
		m_Window = std::make_shared<AkWindow>(descriptor.window);
		m_Swapchain = std::make_shared<AkSwapchain>(m_Window);
	}
	catch (...) 
	{
		return false;
	}

	AkLogInfo("{} {} initializing", descriptor.name, descriptor.version);
	return true;
}

void Awki::Deinitialize()
{
	AkLogInfo("Awki {} deinitializing", kEngineVersion);
	AkDevice::WaitIdle();

	m_Swapchain = nullptr;
	m_Window = nullptr;

	AkCommandBufferAllocator::Deinitialize();
	AkDevice::Deinitialize();
	AkEvents::Deinitialize();
	AkLog::Deinitialize();
}

void Awki::Run()
{
	while (!AkEvents::ShouldClose())
	{
		AkEvents::PollEvents();

		if (m_Swapchain->Prepare())
		{

			m_Swapchain->Present();
		}
	}
}