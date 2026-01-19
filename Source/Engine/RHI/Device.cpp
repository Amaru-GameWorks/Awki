#include "Device.h"
#include "Core/Log.h"
#include "RHI/CommandBuffers/CommandBufferAllocator.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

static vk::Device sDevice = {};
static vk::Instance sInstance = {};
static vk::PhysicalDevice sPhysicalDevice = {};

static vk::Queue sGraphicsQueue = {};
static vk::Queue sComputeQueue = {};
static vk::Queue sTransferQueue = {};
static uint32_t sGraphicsQueueFamilyIndex = {};
static uint32_t sComputeQueueFamilyIndex = {};
static uint32_t sTransferQueueFamilyIndex = {};

#if DEBUG
static vk::DebugUtilsMessengerEXT sDebugMessenger = {};
static constexpr const char* kValidationLayerName = "VK_LAYER_KHRONOS_validation";
static vk::Bool32 VKAPI_PTR ValidationDebugMessages(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT /*messageTypes*/, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* /*pUserData*/)
{
	switch (messageSeverity)
	{
		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
			AkLogInfo("{}", pCallbackData->pMessage);
			break;

		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
			AkLogTrace("{}", pCallbackData->pMessage);
			break;

		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
			AkLogWarning("{}", pCallbackData->pMessage);
			break;

		case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
			AkLogError("{}", pCallbackData->pMessage);
			break;
	}

	return false;
}
#endif

bool AkDevice::Initialize()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		AkLogError("Couldn't initialize SDL Video: {}", SDL_GetError());
		return false;
	}

	if (!SDL_Vulkan_LoadLibrary(NULL))
	{
		AkLogError("Failed to load SDL vulkan entrypoints: {}", SDL_GetError());
		return false;
	}

	if (!CreateInstance())
		return false;

	if (!CreateLogicalDevices())
		return false;

	if (!InitializeExtensions())
		return false;

	if (!AkCommandBufferAllocator::Initialize())
		return false;

	return true;
}

void AkDevice::Deinitialize()
{
	AkCommandBufferAllocator::Deinitialize();

#if DEBUG
	sInstance.destroyDebugUtilsMessengerEXT(sDebugMessenger);
#endif

	sDevice.destroy();
	sInstance.destroy();
}

void AkDevice::WaitIdle()
{
	sDevice.waitIdle();
}

const vk::Instance& AkDevice::GetInstance()
{
	return sInstance;
}

const vk::Device& AkDevice::GetDevice()
{
	return sDevice;
}

const vk::PhysicalDevice& AkDevice::GetPhysicalDevice()
{
	return sPhysicalDevice;
}

const vk::Queue& AkDevice::GetGraphicsQueue()
{
	return sGraphicsQueue;
}

const vk::Queue& AkDevice::GetComputeQueue()
{
	return sComputeQueue;
}

const vk::Queue& AkDevice::GetTransferQueue()
{
	return sTransferQueue;
}

uint32_t AkDevice::GetGraphicsQueueFamilyIndex()
{
	return sGraphicsQueueFamilyIndex;
}

uint32_t AkDevice::GetComputeQueueFamilyIndex()
{
	return sComputeQueueFamilyIndex;
}

uint32_t AkDevice::GetTransferQueueFamilyIndex()
{
	return sTransferQueueFamilyIndex;
}

bool AkDevice::SupportsAsyncCompute()
{
	return m_SupportsAsyncCompute;
}

bool AkDevice::SupportsAsyncTransfer()
{
	return m_SupportsAsyncTransfer;
}

bool AkDevice::CreateInstance()
{
	VULKAN_HPP_DEFAULT_DISPATCHER.init();

	vk::ApplicationInfo applicationInfo =
	{
		.apiVersion = VK_API_VERSION_1_1
	};

	vk::InstanceCreateInfo instanceCreateInfo =
	{
		.pApplicationInfo = &applicationInfo
	};

	Uint32 extensionsCount = 0;
	const char* const* sdlNeededExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionsCount);

	std::vector<const char*> extensions;
	extensions.reserve(extensionsCount);

	for (Uint32 i = 0; i < extensionsCount; ++i)
		extensions.push_back(sdlNeededExtensions[i]);

#if DEBUG
	std::vector<vk::LayerProperties> layerProperties = vk::enumerateInstanceLayerProperties();
	auto foundValidationLayer = std::find_if(layerProperties.begin(), layerProperties.end(), [](vk::LayerProperties& layer) { return strcmp(kValidationLayerName, layer.layerName) == 0; });
	if (foundValidationLayer != layerProperties.end())
	{
		instanceCreateInfo.enabledLayerCount = 1;
		instanceCreateInfo.ppEnabledLayerNames = &kValidationLayerName;
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
#endif

	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());

	try
	{
		sInstance = vk::createInstance(instanceCreateInfo);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(sInstance);
	}
	catch (const std::exception& exception)
	{
		AkLogError("Failed to create vulkan instance: {}", exception.what());
		return false;
	}

	return true;
}

bool AkDevice::CreateLogicalDevices()
{
	const std::vector<vk::PhysicalDevice> physicalDevices = sInstance.enumeratePhysicalDevices();

	uint32_t maxDeviceScore = 0;
	uint32_t selectedDeviceGraphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t selectedDeviceComputeQueueFamilyIndex = UINT32_MAX;
	uint32_t selectedDeviceTransferQueueFamilyIndex = UINT32_MAX;

	for (const vk::PhysicalDevice& device : physicalDevices)
	{
		uint32_t currentDeviceScore = 0;
		const vk::PhysicalDeviceProperties properties = device.getProperties();

		AkLogInfo("Found device: {}", properties.deviceName.data());
		switch (properties.deviceType)
		{
			case vk::PhysicalDeviceType::eDiscreteGpu:
			{
				AkLogInfo("\t Type: Discrete GPU");
				currentDeviceScore += 20;
				break;
			}
			case vk::PhysicalDeviceType::eIntegratedGpu:
			{
				AkLogInfo("\t Type: Integrated GPU");
				currentDeviceScore += 10;
				break;
			}
			case vk::PhysicalDeviceType::eVirtualGpu:
			{
				AkLogInfo("\t Type: Virtual GPU");
				currentDeviceScore += 5;
				break;
			}
			case vk::PhysicalDeviceType::eCpu:
			{
				AkLogInfo("\t Type: CPU");
				currentDeviceScore += 1;
				break;
			}
			case vk::PhysicalDeviceType::eOther:
			{
				AkLogInfo("\t Type: Other");
				break;
			}
		}

		bool foundAsyncComputeQueue = false;
		bool foundAsyncTransferQueue = false;
		bool foundGraphicsComputeQueue = false;

		uint32_t graphicsQueueFamilyIndex = 0;
		uint32_t computeQueueFamilyIndex = 0;
		uint32_t transferQueueFamilyIndex = 0;

		const std::vector<vk::QueueFamilyProperties> queueFamilyProperties = device.getQueueFamilyProperties();
		for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
		{
			const vk::QueueFamilyProperties& queueProperties = queueFamilyProperties[i];

			if (queueProperties.queueCount > 0)
			{
				const bool supportsGraphics = static_cast<bool>(queueProperties.queueFlags & vk::QueueFlagBits::eGraphics);
				const bool supportsCompute = static_cast<bool>(queueProperties.queueFlags & vk::QueueFlagBits::eCompute);
				const bool supportsTransfers = static_cast<bool>(queueProperties.queueFlags & vk::QueueFlagBits::eTransfer);

				if (!foundGraphicsComputeQueue && supportsGraphics && supportsCompute)
				{
					foundGraphicsComputeQueue = true;
					currentDeviceScore += 2;

					graphicsQueueFamilyIndex = static_cast<uint32_t>(i);
					computeQueueFamilyIndex = static_cast<uint32_t>(i);
					transferQueueFamilyIndex = static_cast<uint32_t>(i);

					AkLogInfo("\t Supports graphics");
					AkLogInfo("\t Supports compute");
				}

				if (!foundAsyncComputeQueue && supportsCompute && !supportsGraphics)
				{
					foundAsyncComputeQueue = true;
					currentDeviceScore += 1;

					computeQueueFamilyIndex = static_cast<uint32_t>(i);
					AkLogInfo("\t Supports async compute");
				}

				if (!foundAsyncTransferQueue && supportsTransfers && !supportsGraphics && !supportsCompute)
				{
					foundAsyncTransferQueue = true;
					currentDeviceScore += 1;

					transferQueueFamilyIndex = static_cast<uint32_t>(i);
					AkLogInfo("\t Supports async transfers");
				}
			}
		}

		if (currentDeviceScore > maxDeviceScore)
		{
			maxDeviceScore = currentDeviceScore;
			selectedDeviceGraphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
			selectedDeviceComputeQueueFamilyIndex = computeQueueFamilyIndex;
			selectedDeviceTransferQueueFamilyIndex = transferQueueFamilyIndex;

			sPhysicalDevice = device;
		}
	}

	if (selectedDeviceGraphicsQueueFamilyIndex == UINT32_MAX)
	{
		AkLogError("Failed to find a suitable graphics device");
		return false;
	}

	auto IsExtensionAvailable = [](const std::vector<vk::ExtensionProperties>& extensions, const char* extensionName)
	{
		auto foundValidationLayer = std::find_if(extensions.begin(), extensions.end(), [extensionName](const vk::ExtensionProperties& extension)
		{
			return strcmp(extensionName, extension.extensionName) == 0;
		});

		return foundValidationLayer != extensions.end();
	};

	AkLogInfo("Selected device: {}", sPhysicalDevice.getProperties().deviceName.data());

	std::vector<const char*> extensionToEnable = {};
	std::vector<vk::ExtensionProperties> deviceExtensions = sPhysicalDevice.enumerateDeviceExtensionProperties();

	//Required Extensions
	if (!IsExtensionAvailable(deviceExtensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
	{
		AkLogError("Required extension '{}' is not available", VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		return false;
	}

	extensionToEnable.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	//Optional Extensions
#if DEBUG
	if (IsExtensionAvailable(deviceExtensions, VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
		extensionToEnable.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

	if (IsExtensionAvailable(deviceExtensions, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
		extensionToEnable.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
#endif

	//Get Device queues
	std::vector<float> queuePriorities = { 1.0f };
	std::vector<vk::DeviceQueueCreateInfo> deviceQueueInfos;

	vk::DeviceQueueCreateInfo queueCreateInfo =
	{
		.queueFamilyIndex = selectedDeviceGraphicsQueueFamilyIndex,
		.queueCount = 1,
		.pQueuePriorities = queuePriorities.data()
	};

	//Grahics queue
	deviceQueueInfos.push_back(queueCreateInfo);

	//Async Compute Queue
	if (selectedDeviceComputeQueueFamilyIndex != selectedDeviceGraphicsQueueFamilyIndex)
	{
		m_SupportsAsyncCompute = true;
		queueCreateInfo.queueFamilyIndex = selectedDeviceComputeQueueFamilyIndex;
		deviceQueueInfos.push_back(queueCreateInfo);
	}

	//Async Transfer Queue
	if (selectedDeviceTransferQueueFamilyIndex != selectedDeviceGraphicsQueueFamilyIndex)
	{
		m_SupportsAsyncTransfer = true;
		queueCreateInfo.queueFamilyIndex = selectedDeviceTransferQueueFamilyIndex;
		deviceQueueInfos.push_back(queueCreateInfo);
	}

	vk::DeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueInfos.size());
	deviceCreateInfo.pQueueCreateInfos = deviceQueueInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensionToEnable.size());
	deviceCreateInfo.ppEnabledExtensionNames = extensionToEnable.data();

	try
	{
		sDevice = sPhysicalDevice.createDevice(deviceCreateInfo);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(sDevice);
	}
	catch (const std::exception& exception)
	{
		AkLogError("Failed to create vulkan device: {}", exception.what());
		return false;
	}

	sGraphicsQueueFamilyIndex = selectedDeviceGraphicsQueueFamilyIndex;
	sGraphicsQueue = sDevice.getQueue(selectedDeviceGraphicsQueueFamilyIndex, 0);

	sComputeQueueFamilyIndex = selectedDeviceComputeQueueFamilyIndex;
	sComputeQueue = sDevice.getQueue(selectedDeviceComputeQueueFamilyIndex, 0);

	sTransferQueueFamilyIndex = selectedDeviceTransferQueueFamilyIndex;
	sTransferQueue = sDevice.getQueue(selectedDeviceTransferQueueFamilyIndex, 0);
	return true;
}

bool AkDevice::InitializeExtensions()
{
#if DEBUG
	try
	{
		vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo =
		{
			.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
			.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
			.pfnUserCallback = ValidationDebugMessages
		};

		sDebugMessenger = sInstance.createDebugUtilsMessengerEXT(debugMessengerCreateInfo);
	}
	catch (const std::exception& exception)
	{
		AkLogError("Failed to initialize vulkan validation layers: {}", exception.what());
		return false;
	}
#endif

	return true;
}