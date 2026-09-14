#pragma once
#include <cstdint>
#include <cstdarg>
//https://mightyprofessionalgaming.com/tutorials/memory-allocators-from-scratch.html#linear

template<size_t Size>
class AkLinearAllocator
{
public:
	AkLinearAllocator()
	{
		m_Head = m_Storage;
		m_End = m_Head + Size;
	}

	uint8_t* Allocate(size_t size, size_t alignment = 1) 
	{
		uintptr_t raw = reinterpret_cast<uintptr_t>(m_Head);
		uintptr_t aligned = (raw + alignment - 1) & ~(static_cast<uintptr_t>(alignment) - 1);
		uint8_t* candidate = reinterpret_cast<uint8_t*>(aligned);

		if (candidate + size > m_End)
			return nullptr;

		m_Head = candidate + size;
		return candidate;
	}

	void Reset() { m_Head = m_Storage; }
	size_t Used() const { return m_Head - m_Storage; }
	size_t Capacity() const { return m_End - m_Storage; }

private:
	uint8_t* m_Head = nullptr;
	uint8_t* m_End = nullptr;
	uint8_t m_Storage[Size];
};