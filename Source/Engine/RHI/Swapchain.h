#pragma once
#include "Utilities/ForwardStorage.h"

#include <memory>
#include <vector>

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
	ForwardStorage<struct AkSwapchainStorage, 168> m_Storage;
	std::vector<std::unique_ptr<class AkTexture>> m_BackBufferTextures = {};

	bool CreatePresentationSurface();
	void InitializePersistentData();
	bool CreateSwapchain();
	bool CreateBackBuffersRenderTargets();
	bool CreateSynchronizationPrimitives();

	bool AcquireNextImageIndex();
};