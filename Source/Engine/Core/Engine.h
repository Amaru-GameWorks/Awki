#pragma once
#include "Version.h"

#include <string_view>

struct AkInstanceDescriptor
{
	std::string_view gameName = {};
	AkVersion gameVersion = {};
};

class Awki
{
public:
	static constexpr AkVersion kEngineVersion = { 0, 0, 1 };

	Awki(const AkInstanceDescriptor& descriptor);
	~Awki();
	
	void Run();
};