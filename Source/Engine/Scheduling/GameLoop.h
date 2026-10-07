#pragma once
#include "FrameData.h"
#include "GameThread.h"
#include "RenderThread.h"

namespace AkGameLoop
{
	static inline void Setup()
	{
		AkGameThread::Setup();
	}

	static inline AkJob Run()
	{
		AkGameThread::Execute();

		while (!AkEvents::ShouldClose())
			AkJobSystem::ProcessMainThreadJobOrWait();

		AkDevice::WaitIdle();
		AkRenderThread::FreeResources();

		co_return;
	}
}