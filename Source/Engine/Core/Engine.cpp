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

		if (AkEvents::GetKeyDown(AkKeyCode::SPACE))
			m_Window->SetFullScreen(true, AkFullscreenMode::EXCLUSIVE);

		if (AkEvents::GetKeyUp(AkKeyCode::SPACE))
		{
			uint32_t w, h = 0;
			m_Window->GetSize(w, h);
			AkLogTrace("Window Size {} : {}", w, h);
		}

		if (AkEvents::GetMouseButtonDown(AkMouseButton::X2))
			m_Window->SetFullScreen(false);

		if (AkEvents::GetMouseButtonUp(AkMouseButton::X2))
		{
			uint32_t w, h = 0;
			m_Window->GetSize(w, h);
			AkLogTrace("Window Size {} : {}", w, h);
		}

		if (AkEvents::GetKeyDown(AkKeyCode::ENTER))
			m_Window->SetSize(100, 100);

		if (AkEvents::GetKeyUp(AkKeyCode::ENTER))
		{
			uint32_t w, h = 0;
			m_Window->GetSize(w, h);
			AkLogTrace("Window Size {} : {}", w, h);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(8));
	}
}