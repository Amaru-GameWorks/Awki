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
	AkBufferFlags_STRUCTURED				= 1 << 9,
	AkBufferFlags_RAW_VIEW					= 1 << 10,
	AkBufferFlags_APPEND_CONSUME			= 1 << 11,
};
using AkBufferFlags = std::underlying_type_t<AkBufferFlagBits>;

struct AkBufferDescriptor
{
	size_t size = 0;
	AkBufferFlags flags = 0;
};

class AkBuffer
{
public:
	AkBuffer(const AkBufferDescriptor& descriptor, uint8_t* data = nullptr);
	~AkBuffer();

	uint8_t* GetMappedDataPointer() const { return m_MappedDataPointer; }
	const AkBufferDescriptor& GetDescriptor() const { return m_Descriptor; }
	vk::DeviceAddress GetDeviceAddress() const;
	const vk::Buffer& GetBuffer() const;

private:
	AkBufferDescriptor m_Descriptor = {};
	uint8_t* m_MappedDataPointer = nullptr;
	ForwardStorage<struct AkBufferStorage, 24> m_Storage;
};

class AkConstantBuffer : public AkBuffer
{
public:
	AkConstantBuffer(const size_t size, uint8_t* data)
		: AkBuffer({ RoundToNextMultiple(size, AkDevice::GetMinConstantBufferAlignment()), AkBufferFlags_CONSTANT | AkBufferFlags_CPU_ACCESS | AkBufferFlags_COPY_DESTINATION }, data)
	{ }

	template<typename T>
	requires (alignof(T) == 16)
	AkConstantBuffer(T& data) 
		: AkConstantBuffer(sizeof(T), reinterpret_cast<uint8_t*>(&data))
	{ }
};