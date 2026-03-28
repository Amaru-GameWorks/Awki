#pragma once
#include "RHI/Device.h"
#include "Utilities/Math.h"
#include "Utilities/ForwardStorage.h"

#include <type_traits>

namespace vk
{
	using DeviceAddress = uint64_t;
	class Buffer;
}

enum AkBufferFlagBits : uint16_t
{
	//Access
	AkBufferFlags_COPY_DESTINATION			= 1 << 0,
	AkBufferFlags_COPY_SOURCE				= 1 << 1,
	AkBufferFlags_CPU_ACCESS				= 1 << 2,
	AkBufferFlags_ALLOW_UNORDERED_ACCESS	= 1 << 3,
	AkBufferFlags_NO_SYSTEM_RAM				= 1 << 4,

	//Usage
	AkBufferFlags_INDEX						= 1 << 5,
	AkBufferFlags_VERTEX					= 1 << 6,
	AkBufferFlags_CONSTANT					= 1 << 7,
	AkBufferFlags_INDIRECT					= 1 << 8,
	AkBufferFlags_STRUCTURED				= 1 << 9
};
using AkBufferFlags = std::underlying_type_t<AkBufferFlagBits>;

struct AkBufferDescriptor
{
	size_t size = 0;
	AkBufferFlags flags = 0;
};

class AkBuffer
{
	friend class AkBindlessResourcesManager;

public:
	AkBuffer(const AkBufferDescriptor& descriptor, uint8_t* data = nullptr);
	~AkBuffer();

	uint8_t* GetMappedDataPointer() const { return m_MappedDataPointer; }
	const AkBufferDescriptor& GetDescriptor() const { return m_Descriptor; }
	int32_t GetBindlessIndex() const { return m_BindlessIndex; }
	
	vk::DeviceAddress GetDeviceAddress() const;
	const vk::Buffer& GetBuffer() const;

private:

	int32_t m_BindlessIndex = -1;
	AkBufferDescriptor m_Descriptor = {};
	uint8_t* m_MappedDataPointer = nullptr;
	ForwardStorage<struct AkBufferStorage, 24> m_Storage;
};

class AkConstantBuffer : public AkBuffer
{
public:
	AkConstantBuffer(const size_t size, uint8_t* data, AkBufferFlagBits extraFlags = {})
		: AkBuffer({ RoundToNextMultiple(size, AkDevice::GetMinConstantBufferAlignment()), static_cast<AkBufferFlags>(AkBufferFlags_CONSTANT | AkBufferFlags_CPU_ACCESS | AkBufferFlags_COPY_DESTINATION | extraFlags) }, data)
	{ }

	template<typename T>
	AkConstantBuffer(T& data, AkBufferFlagBits extraFlags = {})
		: AkConstantBuffer(RoundToNextMultiple(sizeof(T), AkDevice::GetMinConstantBufferAlignment()), reinterpret_cast<uint8_t*>(&data), extraFlags)
	{ }
};

class AkStructuredBuffer : public AkBuffer
{
public:
	AkStructuredBuffer(const size_t size, uint8_t* data, AkBufferFlagBits extraFlags = {})
		: AkBuffer({ RoundToNextMultiple(size, AkDevice::GetMinStructuredBufferAlignment()), static_cast<AkBufferFlags>(AkBufferFlags_STRUCTURED | AkBufferFlags_COPY_DESTINATION | AkBufferFlags_NO_SYSTEM_RAM | extraFlags) }, data)
	{ }

	template<typename T>
	AkStructuredBuffer(T& data, AkBufferFlagBits extraFlags = {})
		: AkStructuredBuffer(RoundToNextMultiple(sizeof(T), AkDevice::GetMinStructuredBufferAlignment()), reinterpret_cast<uint8_t*>(&data), extraFlags)
	{ }
};

class AkRWStructuredBuffer : public AkBuffer
{
public:
	AkRWStructuredBuffer(const size_t size, uint8_t* data, AkBufferFlagBits extraFlags = {})
		: AkBuffer({ RoundToNextMultiple(size, AkDevice::GetMinStructuredBufferAlignment()), static_cast<AkBufferFlags>(AkBufferFlags_STRUCTURED | AkBufferFlags_ALLOW_UNORDERED_ACCESS | AkBufferFlags_COPY_DESTINATION | AkBufferFlags_NO_SYSTEM_RAM | extraFlags) }, data)
	{ }

	template<typename T>
	AkRWStructuredBuffer(T& data, AkBufferFlagBits extraFlags = {})
		: AkRWStructuredBuffer(RoundToNextMultiple(sizeof(T), AkDevice::GetMinStructuredBufferAlignment()), reinterpret_cast<uint8_t*>(&data), extraFlags)
	{ }
};