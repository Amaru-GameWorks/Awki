#include "Engine.h"
#include "Log.h"
#include "RHI/Device.h"
#include "RHI/Swapchain.h"
#include "Platform/Window.h"
#include "Platform/Events.h"

Awki::Awki(const AkInstanceDescriptor& descriptor)
{
	if (!AkLog::Initialize())
		throw std::runtime_error("Failed to initialize Logging System!");

	AkLogInfo("Awki {} initializing", kEngineVersion);

	if (!AkEvents::Initialize())
		throw std::runtime_error("Failed to initialize Events System!");

	if (!AkDevice::Initialize())
		throw std::runtime_error("Failed to initialize RHI Device!");

	m_Window = std::make_shared<AkWindow>(descriptor.windowDescriptor);
	m_Swapchain = std::make_shared<AkSwapchain>(m_Window);

	AkLogInfo("{} {} initializing", descriptor.gameName, descriptor.gameVersion);
}

Awki::~Awki()
{
	AkLogInfo("Awki {} deinitializing", kEngineVersion);
	AkDevice::WaitIdle();

	m_Swapchain.reset();
	m_Window.reset();

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