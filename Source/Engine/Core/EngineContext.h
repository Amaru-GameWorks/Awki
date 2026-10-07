#pragma once
#include "RHI/Swapchain.h"
#include "Platform/Window.h"

class AkEngineContext
{
public:
	static void ProvideMainWindow(AkWindow* window) { m_MainWindow = window; }
	static AkWindow& GetMainWindow() { return *m_MainWindow; }
	
	static void ProvideMainWindowSwapchain(AkSwapchain* swapchain) { m_MainWindowSwapchain = swapchain; }
	static AkSwapchain& GetMainWindowSwapchain() { return *m_MainWindowSwapchain; }

private:
	inline static AkWindow* m_MainWindow = nullptr;
	inline static AkSwapchain* m_MainWindowSwapchain = nullptr;
};