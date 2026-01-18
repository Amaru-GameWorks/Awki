#include "Swapchain.h"
#include "Core/Log.h"
#include "Platform/Window.h"
#include "RHI/Device.h"
#include "RHI/Textures/Texture.h"

#include <glm/vec2.hpp>
#include <vulkan/vulkan.hpp>
#include <SDL3/SDL_vulkan.h>

AkPixelFormat GetAkPixelFormat(const vk::Format format)
{
	switch (format)
	{
		case vk::Format::eR8G8B8A8Unorm:			return AkPixelFormat::RGBA8_UNORM;
		case vk::Format::eR8G8B8A8Srgb:				return AkPixelFormat::RGBA8_SRGB;
		case vk::Format::eA2B10G10R10UnormPack32:	return AkPixelFormat::R10G10B10A2_UNORM;

		default:
			AkLogCritical("Vulkan format not registered on this function");
			return AkPixelFormat::UNDEFINED;
	}
}

struct AkSwapchainStorage
{
	uint32_t backBuffersCount = 1;
	vk::PresentModeKHR presentationMode = vk::PresentModeKHR::eFifo;

	std::vector<vk::Image> backBufferImages;

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

	if (!CreatePresentationSurface())
		throw std::runtime_error("Failed to create AkSwapchain");

	InitializePersistentData();

	if (!CreateSwapchain())
		throw std::runtime_error("Failed to create AkSwapchain");

	if (!CreateBackBuffersRenderTargets())
		throw std::runtime_error("Failed to create AkSwapchain");

	if (!CreateSynchronizationPrimitives())
		throw std::runtime_error("Failed to create AkSwapchain");
}

AkSwapchain::~AkSwapchain()
{
	m_BackBufferTextures.clear();

	const vk::Device& device = AkDevice::GetDevice();
	for (uint32_t i = 0; i < m_Storage->backBuffersCount; ++i)
	{
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

		if (!CreateBackBuffersRenderTargets())
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

#include "CommandBuffers/CommandBufferAllocator.h"
void AkSwapchain::Present()
{
	static constexpr vk::PipelineStageFlags kDefaultWaitStage = vk::PipelineStageFlagBits::eBottomOfPipe;
	const vk::Queue& graphicsQueue = AkDevice::GetGraphicsQueue();

	// -- Testing it Works
	static const std::vector<AkCommandBuffer*> commandBuffers = AkCommandBufferAllocator::AllocateCommandBuffers(AkDeviceQueue::GRAPHICS, 3);
	AkTexture* currentBackBufferTexture = m_BackBufferTextures[m_CurrentBackBufferIndex].get();

	commandBuffers[m_CurrentFrameIndex]->Begin();
	commandBuffers[m_CurrentFrameIndex]->TransitionTexture(currentBackBufferTexture, AkResourceState::UNDEFINED, AkResourceState::COPY_DESTINATION);
	commandBuffers[m_CurrentFrameIndex]->ClearColor(currentBackBufferTexture, AkResourceState::COPY_DESTINATION, glm::vec4(0.1f, 0.2f, 0.3f, 1.f));
	commandBuffers[m_CurrentFrameIndex]->TransitionTexture(currentBackBufferTexture, AkResourceState::COPY_DESTINATION, AkResourceState::PRESENT);
	commandBuffers[m_CurrentFrameIndex]->End();
	// -- Testing it Works

	const vk::SubmitInfo submitInfo =
	{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_Storage->imageAcquireSemaphores[m_CurrentFrameIndex],
		.pWaitDstStageMask = &kDefaultWaitStage,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffers[m_CurrentFrameIndex]->GetBuffer(),
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
			default:
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

	auto SelectBestVSyncMode = [surfacePresentModes]()
	{
		if (std::find(surfacePresentModes.begin(), surfacePresentModes.end(), vk::PresentModeKHR::eMailbox) != surfacePresentModes.end())
			return vk::PresentModeKHR::eMailbox;

		if (std::find(surfacePresentModes.begin(), surfacePresentModes.end(), vk::PresentModeKHR::eFifo) != surfacePresentModes.end())
			return vk::PresentModeKHR::eFifo;

		return surfacePresentModes[0];
	};

	//Select presentation mode
	m_Storage->presentationMode = SelectBestVSyncMode();

	//Resize persistent arrays
	m_Storage->fences.resize(m_Storage->backBuffersCount);
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
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
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

bool AkSwapchain::CreateBackBuffersRenderTargets()
{
	m_BackBufferTextures.clear();
	const AkTextureDescriptor descriptor =
	{
		.width = m_Storage->swapchainExtents.x,
		.height = m_Storage->swapchainExtents.y,
		.flags = AkTextureFlags_DEFAULT_RT,
		.format = GetAkPixelFormat(m_Storage->presentationSurfaceFormat.format)
	};

	for (uint32_t i = 0; i < m_Storage->backBuffersCount; ++i)
	{
		try
		{
			m_BackBufferTextures.push_back(std::make_unique<AkTexture>(descriptor, m_Storage->backBufferImages[i]));
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

			if (!CreateBackBuffersRenderTargets())
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