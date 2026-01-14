#include "Engine.h"
#include "Log.h"
#include "Platform/Window.h"
#include "Platform/Events.h"

Awki::Awki(const AkInstanceDescriptor& descriptor)
{
	if (!AkLog::Initialize())
		throw std::runtime_error("Failed to initialize Logging System!");

	AkLogInfo("Awki {} initializing", kEngineVersion);

	if (!AkEvents::Initialize())
		throw std::runtime_error("Failed to initialize Events System!");

	m_Window = std::make_shared<AkWindow>(descriptor.windowDescriptor);

	AkLogInfo("{} {} initializing", descriptor.gameName, descriptor.gameVersion);
}

Awki::~Awki()
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