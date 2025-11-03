#include "Swapchain.h"
#include "Core/Log.h"
#include "RHI/Device.h"
#include "Platform/Window.h"

#include <glm/vec2.hpp>
#include <vulkan/vulkan.hpp>
#include <SDL3/SDL_vulkan.h>

struct AkSwapchainStorage
{
	uint32_t backBuffersCount = 1;
	vk::PresentModeKHR presentationMode = vk::PresentModeKHR::eMailbox;

	std::vector<vk::Image> backBufferImages;
	std::vector<vk::ImageView> backBufferImageViews;

	vk::SurfaceKHR presentationSurface = {};
	vk::SurfaceFormatKHR presentationSurfaceFormat = {};

	vk::SwapchainKHR swapchain = {};
	glm::uvec2 swapchainExtents = {};

	std::vector<vk::Fence> fences = {};
	std::vector<vk::Semaphore> imageAcquireSemaphores = {};
	std::vector<vk::Semaphore> finishedRenderingSemaphores = {};
};

AkSwapchain::AkSwapchain(const std::shared_ptr<AkWindow>& window)
{
	m_Window = window;
	m_Storage = std::make_unique<AkSwapchainStorage>();

	if (!CreatePresentationSurface())
		throw std::exception("Failed to create AkSwapchain");

	InitializePersistentData();

	if (!CreateSwapchain())
		throw std::exception("Failed to create AkSwapchain");

	if (!CreateBackbufferViews())
		throw std::exception("Failed to create AkSwapchain");

	if (!CreateSynchronizationPrimitives())
		throw std::exception("Failed to create AkSwapchain");
}

AkSwapchain::~AkSwapchain()
{
	const vk::Device& device = AkDevice::GetDevice();
	for (uint32_t i = 0; i < m_Storage->backBuffersCount; ++i)
	{
		device.destroyImageView(m_Storage->backBufferImageViews[i]);
		device.destroyFence(m_Storage->fences[i]);
		device.destroySemaphore(m_Storage->imageAcquireSemaphores[i]);
		device.destroySemaphore(m_Storage->finishedRenderingSemaphores[i]);
	}

	device.destroySwapchainKHR(m_Storage->swapchain);

	const vk::Instance& instance = AkDevice::GetInstance();
	instance.destroySurfaceKHR(m_Storage->presentationSurface);
}

bool AkSwapchain::Prepare()
{
	const glm::uvec2& windowSize = m_Window->GetSize();
	bool windowStateChanged = m_Window->IsMinimized();
	windowStateChanged |= windowSize.x != m_Storage->swapchainExtents.x || windowSize.y != m_Storage->swapchainExtents.y;

	if (m_NeedsRecreation || windowStateChanged)
	{
		const vk::PhysicalDevice& physicalDevice = AkDevice::GetPhysicalDevice();
		const vk::SurfaceCapabilitiesKHR& surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_Storage->presentationSurface);
		if (surfaceCapabilities.maxImageExtent.width == 0 || surfaceCapabilities.maxImageExtent.height == 0)
			return false;

		if (!CreateSwapchain())
			return false;

		if (!CreateBackbufferViews())
			return false;

		m_NeedsRecreation = false;
	}

	try
	{
		const vk::Device& device = AkDevice::GetDevice();
		device.waitForFences(m_Storage->fences[m_CurrentFrameIndex], true, UINT64_MAX);
		device.resetFences(m_Storage->fences[m_CurrentFrameIndex]);
	}
	catch (const std::exception& exception)
	{
		AkLogError("Error while processing frame fence: {}", exception.what());
		return false;
	}

	if (!AcquireNextImageIndex())
		return false;

	return true;
}

void AkSwapchain::Present()
{
	static constexpr vk::PipelineStageFlags kDefaultWaitStage = vk::PipelineStageFlagBits::eBottomOfPipe;
	const vk::Queue& graphicsQueue = AkDevice::GetGraphicsQueue();

	const vk::SubmitInfo submitInfo =
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_Storage->imageAcquireSemaphores[m_CurrentFrameIndex],
		.pWaitDstStageMask = &kDefaultWaitStage,
		.commandBufferCount = 0,
		.pCommandBuffers = nullptr,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_Storage->finishedRenderingSemaphores[m_CurrentFrameIndex],
	};

	try
	{
		graphicsQueue.submit(submitInfo, m_Storage->fences[m_CurrentFrameIndex]);
	}
	catch (const std::exception& exception)
	{
		AkLogError("Failed to submit workload: {}", exception.what());
		return;
	}

	const vk::PresentInfoKHR presentInfo =
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_Storage->finishedRenderingSemaphores[m_CurrentFrameIndex],
		.swapchainCount = 1,
		.pSwapchains = &m_Storage->swapchain,
		.pImageIndices = &m_CurrentBackBufferIndex
	};

	try
	{
		switch (graphicsQueue.presentKHR(presentInfo))
		{
			case vk::Result::eSuccess:
				break;
			case vk::Result::eSuboptimalKHR:
			case vk::Result::eErrorOutOfDateKHR:
			{
				const vk::PhysicalDevice& physicalDevice = AkDevice::GetPhysicalDevice();
				const vk::SurfaceCapabilitiesKHR& surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_Storage->presentationSurface);
				if (surfaceCapabilities.maxImageExtent.width == 0 || surfaceCapabilities.maxImageExtent.height == 0)
					break;

				m_NeedsRecreation = true;
				break;
			}
		}
	}
	catch (const std::exception& exception)
	{
		AkLogError("Failed to present image to screen: {}", exception.what());
	}

	m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_Storage->backBuffersCount;
}

bool AkSwapchain::CreatePresentationSurface()
{
	VkSurfaceKHR presentationSurface = VK_NULL_HANDLE;
	if (!SDL_Vulkan_CreateSurface(m_Window->GetHandle(), AkDevice::GetInstance(), nullptr, &presentationSurface))
	{
		AkLogError("Failed to crete vulkan presentation surface: {}", SDL_GetError());
		return false;
	}

	m_Storage->presentationSurface = presentationSurface;
	return true;
}

void AkSwapchain::InitializePersistentData()
{
	const vk::PhysicalDevice& physicalDevice = AkDevice::GetPhysicalDevice();
	const vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_Storage->presentationSurface);
	const std::vector<vk::SurfaceFormatKHR> surfaceFormats = physicalDevice.getSurfaceFormatsKHR(m_Storage->presentationSurface);
	const std::vector<vk::PresentModeKHR> surfacePresentModes = physicalDevice.getSurfacePresentModesKHR(m_Storage->presentationSurface);

	//Get the number of backbuffer images to create
	m_Storage->backBuffersCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && m_Storage->backBuffersCount > surfaceCapabilities.maxImageCount)
		m_Storage->backBuffersCount = surfaceCapabilities.maxImageCount;

	//There is no preference for surface format
	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
		m_Storage->presentationSurfaceFormat = { VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
	else
	{
		m_Storage->presentationSurfaceFormat = surfaceFormats[0];
		for (size_t i = 0; i < surfaceFormats.size(); ++i)
		{
			if (surfaceFormats[i].format == vk::Format::eR8G8B8A8Unorm)
			{
				m_Storage->presentationSurfaceFormat = surfaceFormats[i];
				break;
			}
		}
	}

	//Select presentation mode
	m_Storage->presentationMode = surfacePresentModes[0];
	for (const vk::PresentModeKHR& mode : surfacePresentModes)
	{
		if (mode == vk::PresentModeKHR::eFifo)
		{
			m_Storage->presentationMode = mode;
			break;
		}
	}

	//Resize persistent arrays
	m_Storage->fences.resize(m_Storage->backBuffersCount);
	m_Storage->backBufferImageViews.resize(m_Storage->backBuffersCount);
	m_Storage->imageAcquireSemaphores.resize(m_Storage->backBuffersCount);
	m_Storage->finishedRenderingSemaphores.resize(m_Storage->backBuffersCount);
}

bool AkSwapchain::CreateSwapchain()
{
	const vk::Device& device = AkDevice::GetDevice();
	const vk::PhysicalDevice& physicalDevice = AkDevice::GetPhysicalDevice();
	const vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_Storage->presentationSurface);

	const vk::SwapchainCreateInfoKHR swapchainCreateInfo =
	{
		.surface = m_Storage->presentationSurface,
		.minImageCount = m_Storage->backBuffersCount,
		.imageFormat = m_Storage->presentationSurfaceFormat.format,
		.imageColorSpace = m_Storage->presentationSurfaceFormat.colorSpace,
		.imageExtent = { surfaceCapabilities.maxImageExtent.width, surfaceCapabilities.maxImageExtent.height },
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
		.presentMode = m_Storage->presentationMode,
		.oldSwapchain = m_Storage->swapchain
	};

	try
	{
		vk::SwapchainKHR oldSwapchain = m_Storage->swapchain;
		
		m_Storage->swapchain = device.createSwapchainKHR(swapchainCreateInfo);
		m_Storage->backBufferImages = device.getSwapchainImagesKHR(m_Storage->swapchain);
		m_Storage->swapchainExtents = { surfaceCapabilities.maxImageExtent.width, surfaceCapabilities.maxImageExtent.height };

		device.destroySwapchainKHR(oldSwapchain);
	}
	catch (const std::exception& exception)
	{
		AkLogError("Failed to create swapchain: {}", exception.what());
		return false;
	}

	return true;
}

bool AkSwapchain::CreateBackbufferViews()
{
	const vk::Device& device = AkDevice::GetDevice();
	for (vk::ImageView& view : m_Storage->backBufferImageViews)
		device.destroyImageView(view);

	vk::ImageViewCreateInfo imageViewCreateInfo =
	{
		.viewType = vk::ImageViewType::e2D,
		.format = m_Storage->presentationSurfaceFormat.format,
		.components = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA },
		.subresourceRange =
		{
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	for (uint32_t i = 0; i < m_Storage->backBuffersCount; ++i)
	{
		try
		{
			imageViewCreateInfo.image = m_Storage->backBufferImages[i];
			m_Storage->backBufferImageViews[i] = device.createImageView(imageViewCreateInfo);
		}
		catch (const std::exception& exception)
		{
			AkLogError("Failed to create back buffer image view: {}", exception.what());
			return false;
		}
	}

	return true;
}

bool AkSwapchain::CreateSynchronizationPrimitives()
{
	const vk::Device& device = AkDevice::GetDevice();
	const vk::SemaphoreCreateInfo semaphoreCreateInfo = { };
	const vk::FenceCreateInfo fenceCreateInfo = { .flags = vk::FenceCreateFlagBits::eSignaled };

	for (uint32_t i = 0; i < m_Storage->backBuffersCount; ++i)
	{
		try
		{
			m_Storage->fences[i] = device.createFence(fenceCreateInfo);
			m_Storage->imageAcquireSemaphores[i] = device.createSemaphore(semaphoreCreateInfo);
			m_Storage->finishedRenderingSemaphores[i] = device.createSemaphore(semaphoreCreateInfo);
		}
		catch (const std::exception& exception)
		{
			AkLogError("Failed to create synchronization primitive: {}", exception.what());
			return false;
		}
	}

	return true;
}

bool AkSwapchain::AcquireNextImageIndex()
{
	const vk::Device& device = AkDevice::GetDevice();

	vk::ResultValue result = device.acquireNextImageKHR(m_Storage->swapchain, UINT64_MAX, m_Storage->imageAcquireSemaphores[m_CurrentFrameIndex]);
	m_CurrentBackBufferIndex = result.has_value() ? result.value : UINT32_MAX;

	switch (result.result)
	{
		case vk::Result::eSuccess:
			break;
		case vk::Result::eSuboptimalKHR:
		case vk::Result::eErrorOutOfDateKHR:
		{
			const vk::PhysicalDevice& physicalDevice = AkDevice::GetPhysicalDevice();
			const vk::SurfaceCapabilitiesKHR& surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_Storage->presentationSurface);
			if (surfaceCapabilities.maxImageExtent.width == 0 || surfaceCapabilities.maxImageExtent.height == 0)
				return false;

			if (!CreateSwapchain())
				return false;

			if (!CreateBackbufferViews())
				return false;

			return AcquireNextImageIndex();
		}
		default:
		{
			m_CurrentBackBufferIndex = UINT32_MAX;
			AkLogError("Failed to get next swapchain image");
			return false;
		}
	}

	return true;
}
