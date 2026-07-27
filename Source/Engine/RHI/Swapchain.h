#pragma once
#include "Utilities/ForwardStorage.h"

#include <memory>
#include <vector>

class AkSwapchain
{
public:
	AkSwapchain(class AkWindow* window);
	~AkSwapchain();

	bool Prepare();
	void Present(const std::vector<class AkCommandBuffer*>& commandBuffers);
	class AkRenderTarget* GetCurrentBackBufferRenderTarget() const;
	uint32_t GetBackBuffersCount() const;
	uint8_t GetCurrentFrameIndex() const;

private:
	bool m_NeedsRecreation = false;
	uint8_t m_CurrentFrameIndex = 0;
	uint32_t m_CurrentBackBufferIndex = 0;

	class AkWindow* m_Window = nullptr;
	AkForwardStorage<struct AkSwapchainStorage, 168> m_Storage;
	std::vector<std::unique_ptr<class AkTexture>> m_BackBufferTextures;
	std::vector<std::unique_ptr<class AkRenderTarget>> m_BackBufferRenderTargets;

	bool CreatePresentationSurface();
	void InitializePersistentData();
	bool CreateSwapchain();
	bool CreateBackBuffersRenderTargets();
	bool CreateSynchronizationPrimitives();

	bool AcquireNextImageIndex();
};