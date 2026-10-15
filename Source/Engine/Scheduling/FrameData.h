#pragma once
#include "JobSystemSynchronization.h"

struct AkFrameData
{
	size_t frameIndex;
};

constexpr size_t kMaxFramesInFlight = 2;
inline AkFrameData gFrameData[kMaxFramesInFlight];
inline AkSemaphore gFrameDataAvailable(AkThreadType::Main, kMaxFramesInFlight);