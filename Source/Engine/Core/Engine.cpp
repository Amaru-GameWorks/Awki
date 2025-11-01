#include "Engine.h"
#include "Log.h"
#include "Platform/Window.h"
#include "Platform/Events.h"
#include "RHI/Device/GfxDevice.h"

bool Awki::Initialize(const AkInstanceDescriptor& descriptor)
{
	if (!AkLog::Initialize())
		return false;

	AkLogInfo("Awki {} initializing", kEngineVersion);

	if (!AkEvents::Initialize())
		return false;

	if (!AkGfxDevice::Initialize())
		return false;
	
	try
	{
		m_Window = std::make_shared<AkWindow>(descriptor.window);
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
	m_Window = nullptr;

	AkGfxDevice::Deinitialize();
	AkEvents::Deinitialize();
	AkLog::Deinitialize();
}

void Awki::Run()
{
	while (!AkEvents::ShouldClose())
	{
		AkEvents::PollEvents();
	}
}