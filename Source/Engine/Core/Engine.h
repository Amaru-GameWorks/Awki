#pragma once
#include "Version.h"
#include "RHI/Swapchain.h"
#include "Platform/Window.h"
#include "Utilities/Delegates.h"

#include <memory>
#include <string_view>

AK_DECLARE_CUSTOM_DELEGATE(AkOnFrameRender, class AkCommandBuffer*, class AkRenderTarget*);

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
	AkSimpleDelegate& GetOnEngineStart() { return m_OnEngineStart; }
	AkSimpleDelegate& GetOnEngineShutdown() { return m_OnEngineShutdown; }
	AkOnFrameRender& GetOnFrameRender() { return m_OnFrameRender; }

	const std::shared_ptr<AkWindow>& GetMainWindow() const { return m_Window; }
	const std::shared_ptr<AkSwapchain>& GetMainSwapchain() const { return m_Swapchain; }

private:
	AkSimpleDelegate m_OnEngineStart = {};
	AkSimpleDelegate m_OnEngineShutdown = {};
	AkOnFrameRender m_OnFrameRender = {};

	std::shared_ptr<AkWindow> m_Window = nullptr;
	std::shared_ptr<AkSwapchain> m_Swapchain = nullptr;
};