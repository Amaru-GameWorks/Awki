#pragma once
#include "JobSystemSynchronization.h"

constexpr size_t kMaxFramesInFlight = 2;

struct AkFrameData
{
	size_t frameIndex;
};

inline AkFrameData gFrameData[kMaxFramesInFlight];
inline AkSemaphore gFrameDataAvailable(AkThreadType::Main, kMaxFramesInFlight);