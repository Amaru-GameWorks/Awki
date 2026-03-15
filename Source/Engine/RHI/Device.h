#pragma once
#include <cstdint>
#include <cstdlib>

namespace vk 
{ 
	class Instance; 
	class Device; 
	class Queue; 
	class PhysicalDevice; 
}

typedef struct VmaAllocator_T* VmaAllocator;

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

	static const VmaAllocator& GetMemoryAllocator();

	static uint32_t GetGraphicsQueueFamilyIndex();
	static uint32_t GetComputeQueueFamilyIndex();
	static uint32_t GetTransferQueueFamilyIndex();
	static size_t GetMinConstantBufferAlignment();
	static size_t GetMinStructuredBufferAlignment();

	static bool SupportsAsyncCompute();
	static bool SupportsAsyncTransfer();

private:
	static bool CreateInstance();
	static bool CreateLogicalDevices();
	static bool InitializeExtensions();
	static bool InitializeMemoryAllocator();

	static inline bool m_SupportsAsyncCompute = false;
	static inline bool m_SupportsAsyncTransfer = false;
};