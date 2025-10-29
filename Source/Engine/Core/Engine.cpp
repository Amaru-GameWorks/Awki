#include "Engine.h"
#include "Log.h"
#include "Platform/Window.h"
#include "Platform/Events.h"

#include <thread>

bool Awki::Initialize(const AkInstanceDescriptor& descriptor)
{
	if (!AkLog::Initialize())
		return false;

	if (!AkEvents::Initialize())
		return false;

	AkLogInfo("Awki {} initializing", kEngineVersion);
	
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
	m_Window.reset();

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