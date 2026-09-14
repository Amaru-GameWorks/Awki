#pragma once
#include <cstdint>
//https://mightyprofessionalgaming.com/tutorials/memory-allocators-from-scratch.html#pool

template<typename T, size_t Count>
requires(sizeof(T) >= sizeof(void*))
class AkPoolAllocator
{
public:
	AkPoolAllocator()
	{
		for (size_t i = 0; i < Count - 1; ++i)
			*reinterpret_cast<size_t*>(SlotHeader(i)) = i + 1;
		*reinterpret_cast<size_t*>(SlotHeader(Count - 1)) = -1;
	}

	T* Allocate()
	{
		if (m_Head < 0)
			return nullptr;

		uint8_t* slot = SlotHeader(m_Head);
		size_t nextHead = *reinterpret_cast<size_t*>(slot);
		m_Head = nextHead;

		return reinterpret_cast<T*>(slot);
	}

	T* AllocateNew()
	{
		T* newAllocation = Allocate();
		return newAllocation ? new (newAllocation) T() : nullptr;
	}

	void Deallocate(T* allocation)
	{
		size_t index = (reinterpret_cast<uint8_t*>(allocation) - m_Storage) / sizeof(T);
		*reinterpret_cast<size_t*>(SlotHeader(index)) = m_Head;
		m_Head = index;
	}

	void DeallocateDestroy(T* allocation)
	{
		allocation->~T();
		Deallocate(allocation);
	}

private:

	size_t m_Head = 0;
	alignas(T) uint8_t m_Storage[Count * sizeof(T)];

	uint8_t* SlotHeader(size_t index) { return m_Storage + index * sizeof(T); }
};