#pragma once
#include "Version.h"
#include "Platform/Window.h"

#include <memory>
#include <string_view>

struct AkInstanceDescriptor
{
	std::string_view gameName = {};
	AkVersion gameVersion = {};
	AkWindowDescriptor windowDescriptor = {};
};

class Awki
{
public:
	static constexpr AkVersion kEngineVersion = { 0, 0, 1 };

	Awki(const AkInstanceDescriptor& descriptor);
	~Awki();
	
	void Run();

private:
	std::shared_ptr<AkWindow> m_Window = nullptr;
	std::shared_ptr<class AkSwapchain> m_Swapchain = nullptr;
};