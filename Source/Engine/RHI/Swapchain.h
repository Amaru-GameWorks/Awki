#pragma once
#include <memory>

class AkSwapchain
{
public:
	AkSwapchain(const std::shared_ptr<class AkWindow>& window);
	~AkSwapchain();

	bool Prepare();
	void Present();

private:
	bool m_NeedsRecreation = false;
	uint8_t m_CurrentFrameIndex = 0;
	uint32_t m_CurrentBackBufferIndex = 0;

	std::shared_ptr<class AkWindow> m_Window = nullptr;
	std::unique_ptr<struct AkSwapchainStorage> m_Storage = nullptr;
	
	bool CreatePresentationSurface();
	void InitializePersistentData();
	bool CreateSwapchain();
	bool CreateBackbufferViews();
	bool CreateSynchronizationPrimitives();

	bool AcquireNextImageIndex();
};