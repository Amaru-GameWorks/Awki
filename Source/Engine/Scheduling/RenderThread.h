#pragma once
#include "JobSystem.h"

class AkRenderThread
{
public:
	static AkJobInitiallySuspended Execute(size_t frameIndex);
	static void FreeResources();
};