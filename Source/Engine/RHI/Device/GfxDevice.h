#pragma once

namespace vk { class Device; class Queue; }

class AkGfxDevice
{
public:
	static bool Initialize();
	static void Deinitialize();

	static vk::Device* GetDevice();
	static struct VmaAllocator_T* GetMemoryAllocator();

	static vk::Queue* GetGraphicsQueue();
	static vk::Queue* GetComputeQueue();
	static vk::Queue* GetTransferQueue();

	static bool SupportsAsyncCompute();
	static bool SupportsAsyncTransfer();

private:
	static bool CreateInstance();
	static bool CreateLogicalDevices();
	static bool CreateMemoryAllocator();

	static inline bool m_SupportsAsyncCompute = false;
	static inline bool m_SupportsAsyncTransfer = false;
};