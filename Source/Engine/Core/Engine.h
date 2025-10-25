#pragma once
#include "Version.h"
#include "Platform/Window.h"

#include <memory>

struct AkInstanceDescriptor
{
	std::string name = {};
	AkVersion version = {};
	AkWindowDescriptor window = {};
};

class Awki
{
public:
	static inline const AkVersion kEngineVersion = {0, 1, 0};

	bool Initialize(const AkInstanceDescriptor& descriptor);
	void Deinitialize();
	void Run();

private:
	std::shared_ptr<class AkWindow> m_Window = nullptr;
};