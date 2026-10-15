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

	std::byte* Allocate(size_t size, size_t alignment = 1)
	{
		uintptr_t raw = reinterpret_cast<uintptr_t>(m_Head);
		uintptr_t aligned = (raw + alignment - 1) & ~(static_cast<uintptr_t>(alignment) - 1);
		std::byte* candidate = reinterpret_cast<std::byte*>(aligned);

		if (candidate + size > m_End)
			return nullptr;

		m_Head = candidate + size;
		return candidate;
	}

	void Reset() { m_Head = m_Storage; }
	size_t Used() const { return m_Head - m_Storage; }
	size_t Capacity() const { return m_End - m_Storage; }

private:
	std::byte* m_Head = nullptr;
	std::byte* m_End = nullptr;
	uint8_t m_Storage[Size];
};