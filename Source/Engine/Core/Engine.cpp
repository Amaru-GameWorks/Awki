#include "Engine.h"
#include "Log.h"

bool Awki::Initialize(const AkInstanceDescriptor& descriptor)
{
	if (!Log::Initialize())
		return false;

	AkLogInfo("Awki {} initializing", kEngineVersion);
	
	//Init Engine

	AkLogInfo("{} {} initializing", descriptor.name, descriptor.version);
	return true;
}

void Awki::Deinitialize()
{
}

void Awki::Run()
{
}