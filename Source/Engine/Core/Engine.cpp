#include "Engine.h"
#include "Log.h"
#include "Platform/Window.h"

bool Awki::Initialize(const AkInstanceDescriptor& descriptor)
{
	if (!Log::Initialize())
		return false;

	AkLogInfo("Awki {} initializing", kEngineVersion);
	
	try
	{
		m_Window = std::make_shared<AkWindow>(descriptor.window);
	}
	catch (...)
	{
		AkLogError("Failed to initialize Awki Engine");
		return false;
	}

	AkLogInfo("{} {} initializing", descriptor.name, descriptor.version);
	return true;
}

void Awki::Deinitialize()
{
	AkLogInfo("Awki {} deinitializing", kEngineVersion);
	m_Window.reset();

	Log::Deinitialize();
}

void Awki::Run()
{
}