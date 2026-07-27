#pragma once
#include "ECS/Entity.h"
#include "Graphics/RenderPipeline/RenderPipeline.h"

#include <memory>
#include <unordered_map>

class AkScheduler
{
public:
	void Run();
	void SetWindow(class AkWindow* window);
	void SetSwapchain(class AkSwapchain* swapchain);

private:
	void ProcessGameThread();
	void ProcessRenderThread();

	class AkWindow* m_Window = nullptr;
	class AkSwapchain* m_Swapchain = nullptr;
	std::unordered_map<AkEntity, std::unique_ptr<AkRenderPipeline>> m_RenderPipelines;
};