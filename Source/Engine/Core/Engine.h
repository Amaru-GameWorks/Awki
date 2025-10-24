#pragma once
#include "Version.h"

struct AkInstanceDescriptor
{
	std::string name = {};
	AkVersion version = {};
	uint16_t width = 1280;
	uint16_t height = 720;
};

class Awki
{
public:
	static inline const AkVersion kEngineVersion = {0, 1, 0};

	bool Initialize(const AkInstanceDescriptor& descriptor);
	void Deinitialize();
	void Run();
};