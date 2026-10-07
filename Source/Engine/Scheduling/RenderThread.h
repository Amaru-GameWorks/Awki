#pragma once
#include "JobSystem.h"


class AkRenderThread
{
public:
	static AkJobInitialSuspend Execute(size_t frameIndex);
	static void FreeResources();
};