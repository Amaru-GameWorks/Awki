#include "Buffer.h"
#include "RHI/Device.h"
#include "RHI/UploadManager.h"
#include "RHI/Pipeline/BindlessResourcesManager.h"

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

constexpr vk::BufferUsageFlags GetUsageFlags(const AkBufferFlags flags)
{
	vk::BufferUsageFlags usageFlags = {};

	if (flags & (AkBufferFlags_STRUCTURED))
		usageFlags |= vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer;

	if (flags & AkBufferFlags_CONSTANT)
		usageFlags |= vk::BufferUsageFlagBits::eUniformBuffer;

	if (flags & AkBufferFlags_COPY_DESTINATION)
		usageFlags |= vk::BufferUsageFlagBits::eTransferDst;

	if (flags & AkBufferFlags_COPY_SOURCE)
		usageFlags |= vk::BufferUsageFlagBits::eTransferSrc;

	if (flags & AkBufferFlags_INDEX)
		usageFlags |= vk::BufferUsageFlagBits::eIndexBuffer;

	if (flags & AkBufferFlags_VERTEX)
		usageFlags |= vk::BufferUsageFlagBits::eVertexBuffer;

	if (flags & AkBufferFlags_INDIRECT)
		usageFlags |= vk::BufferUsageFlagBits::eIndirectBuffer;

	return usageFlags;
}

struct AkBufferStorage
{
	vk::Buffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	vk::DeviceAddress deviceAddress = {};
};

AkBuffer::AkBuffer(const AkBufferDescriptor& descriptor, uint8_t* data)
	: m_Descriptor(descriptor)
{
	const vk::Device& device = AkDevice::GetDevice();
	const VmaAllocator& allocator = AkDevice::GetMemoryAllocator();

	vk::BufferCreateInfo bufferCreateInfo = 
	{
		.size = m_Descriptor.size,
		.usage = GetUsageFlags(m_Descriptor.flags),
		.sharingMode = vk::SharingMode::eExclusive,
	};

	const bool cpuAccess = m_Descriptor.flags & AkBufferFlags_CPU_ACCESS;
	const bool fallbackToDevice = m_Descriptor.flags & AkBufferFlags_NO_SYSTEM_RAM;
	const bool shouldCopyData = data && (m_Descriptor.flags & AkBufferFlags_COPY_DESTINATION);
	
	VmaAllocationInfo allocationInfo = {};
	VmaAllocationCreateInfo allocationCreateInfo = {};
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
	
	if (cpuAccess)
	{
		const bool copySource = m_Descriptor.flags & AkBufferFlags_COPY_SOURCE;

		if(copySource)
			allocationCreateInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		else
			allocationCreateInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

		if (fallbackToDevice)
			allocationCreateInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
	}
	else
		allocationCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	VkBuffer buffer = VK_NULL_HANDLE;
	VkResult result = vmaCreateBuffer(allocator, bufferCreateInfo, &allocationCreateInfo, &buffer, &m_Storage->allocation, &allocationInfo);
	vk::detail::resultCheck(vk::Result(result), VULKAN_HPP_NAMESPACE_STRING "::Device::createBuffer");
	m_Storage->buffer = buffer;

	if (m_Descriptor.flags & AkBufferFlags_STRUCTURED)
	{
		const vk::BufferDeviceAddressInfo bufferAddressInfo = { .buffer = m_Storage->buffer };
		m_Storage->deviceAddress = device.getBufferAddress(bufferAddressInfo);
	}

	if (cpuAccess)
	{
		VkMemoryPropertyFlags memoryPropertyFlags = {};
		vmaGetAllocationMemoryProperties(allocator, m_Storage->allocation, &memoryPropertyFlags);

		if (memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			result = vmaMapMemory(allocator, m_Storage->allocation, reinterpret_cast<void**>(&m_MappedDataPointer));
			vk::detail::resultCheck(vk::Result(result), "vmaMapMemory");
	
			if (shouldCopyData)
				memcpy(m_MappedDataPointer, data, m_Descriptor.size);
		}
		else
		{
			if (shouldCopyData)
				AkUploadManager::QueueBufferUpload(this, data);
		}
	}
	else if (shouldCopyData)
	{
		AkUploadManager::QueueBufferUpload(this, data);
	}

	if (m_Descriptor.flags & (AkBufferFlags_STRUCTURED | AkBufferFlags_VERTEX | AkBufferFlags_INDIRECT))
		AkBindlessResourcesManager::AddBuffer(this);
}

AkBuffer::~AkBuffer()
{
	AkBindlessResourcesManager::RemoveBuffer(this);

	const vk::Device& device = AkDevice::GetDevice();
	const VmaAllocator& allocator = AkDevice::GetMemoryAllocator();

	if (m_Descriptor.flags & AkBufferFlags_CPU_ACCESS)
		vmaUnmapMemory(allocator, m_Storage->allocation);

	device.destroyBuffer(m_Storage->buffer);
	vmaFreeMemory(allocator, m_Storage->allocation);
}

vk::DeviceAddress AkBuffer::GetDeviceAddress() const
{
	return m_Storage->deviceAddress;
}

const vk::Buffer& AkBuffer::GetBuffer() const
{
	return m_Storage->buffer;
}
