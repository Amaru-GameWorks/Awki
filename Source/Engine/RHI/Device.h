#pragma once
#include <cstdint>

namespace vk 
{ 
	class Instance; 
	class Device; 
	class Queue; 
	class PhysicalDevice; 
}

class AkDevice
{
public:
	static bool Initialize();
	static void Deinitialize();
	static void WaitIdle();

	static const vk::Instance& GetInstance();
	static const vk::Device& GetDevice();
	static const vk::PhysicalDevice& GetPhysicalDevice();

	static const vk::Queue& GetGraphicsQueue();
	static const vk::Queue& GetComputeQueue();
	static const vk::Queue& GetTransferQueue();

	static uint32_t GetGraphicsQueueFamilyIndex();
	static uint32_t GetComputeQueueFamilyIndex();
	static uint32_t GetTransferQueueFamilyIndex();

	static bool SupportsAsyncCompute();
	static bool SupportsAsyncTransfer();

private:
	static bool CreateInstance();
	static bool CreateLogicalDevices();
	static bool InitializeExtensions();

	static inline bool m_SupportsAsyncCompute = false;
	static inline bool m_SupportsAsyncTransfer = false;
};