#pragma once
#include "Version.h"

struct AkInstanceDescriptor
{
	std::string name = {};
	AkVersion version = {};
};

class Awki
{
public:
	static inline const AkVersion kEngineVersion = {0, 1, 0};

	bool Initialize(const AkInstanceDescriptor& descriptor);
	void Deinitialize();
	void Run();
};