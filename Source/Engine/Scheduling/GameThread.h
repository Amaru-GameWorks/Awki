#pragma once
#include "JobSystem.h"

class AkGameThread
{
public:
	static void Setup();
	static AkJob Execute();
};