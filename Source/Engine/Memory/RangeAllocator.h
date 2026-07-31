#pragma once
#include "Core/Log.h"

#include <cstdint>

//https://indiegamedev.net/2022/03/27/custom-c20-memory-allocators-for-stl-containers/
template<size_t Size>
class AkRangeAllocator
{
	static constexpr size_t UsableSize = Size < 16 ? 16 : Size;

	struct FreeBlock
	{
		size_t size = 0;
		FreeBlock* next = nullptr;
	};

public:
	AkRangeAllocator()
	{
		m_FreeBlock = reinterpret_cast<FreeBlock*>(&m_Storage);
		m_FreeBlock->size = UsableSize;
		m_FreeBlock->next = nullptr;
	}

	uint8_t* Allocate(size_t allocationSize)
	{
		if (allocationSize < 8)
		{
			AkLogError("Requested allocation size is too small");
			return nullptr;
		}

		FreeBlock* previousFreeBlock = nullptr;
		FreeBlock* currentFreeBlock = m_FreeBlock;
		
		FreeBlock* previousBestFit = nullptr;
		FreeBlock* currentBestFit = nullptr;

		size_t totalAllocationSize = static_cast<size_t>(allocationSize) + sizeof(size_t);
		while (currentFreeBlock)
		{
			if (currentFreeBlock->size >= totalAllocationSize)
			{
				if (!currentBestFit || currentFreeBlock->size < currentBestFit->size)
				{
					previousBestFit = previousFreeBlock;
					currentBestFit = currentFreeBlock;
				}

				if (currentFreeBlock->size == totalAllocationSize)
					break;
			}
		
			previousFreeBlock = currentFreeBlock;
			currentFreeBlock = currentFreeBlock->next;
		}
		
		if (!currentBestFit)
			return nullptr;
		
		if (currentBestFit->size - totalAllocationSize <= sizeof(size_t))
		{
			//Take the whole block since we can't allocate anything else in this one
			totalAllocationSize = currentBestFit->size;
		
			if (previousBestFit)
				previousBestFit->next = currentBestFit->next;
			else
				m_FreeBlock = currentBestFit->next;
		}
		else
		{
			//Split the block
			FreeBlock* newBlock = reinterpret_cast<FreeBlock*>(reinterpret_cast<uint8_t*>(currentBestFit) + totalAllocationSize);
			newBlock->size = currentBestFit->size - totalAllocationSize;
			newBlock->next = currentBestFit->next;

			if (previousBestFit)
				previousBestFit->next = newBlock;
			else
				m_FreeBlock = newBlock;
		}

		size_t* header = reinterpret_cast<size_t*>(currentBestFit);
		*header = totalAllocationSize;

		return reinterpret_cast<uint8_t*>(currentBestFit) + sizeof(size_t);
	}

	template<typename T>
	T* Allocate()
	{
		static_assert(sizeof(T) >= sizeof(void*), "Type size must be big enough to hold a pointer");
		return reinterpret_cast<T*>(Allocate(sizeof(T)));
	}

	template<typename T, typename ...Args>
	T* AllocateNew(Args&& ...arguments)
	{
		T* newAllocation = Allocate<T>();
		return newAllocation ? new (newAllocation) T(arguments...) : nullptr;
	}

	void Deallocate(void* allocation)
	{
		if (!allocation)
			return;

		uint8_t* allocationStart = reinterpret_cast<uint8_t*>(allocation);

		uint8_t* blockStart = allocationStart - sizeof(size_t);
		size_t blockSize = *reinterpret_cast<size_t*>(blockStart);
		uint8_t* blockEnd = blockStart + blockSize;

		FreeBlock* nextFreeBlock = nullptr;
		FreeBlock* previusFreeBlock = nullptr;

		// Find the first free block that starts after or is at the boundary of this block
		FreeBlock* currentFreeBlock = m_FreeBlock;
		while (currentFreeBlock != nullptr)
		{
			uint8_t* currentBlockAddress = reinterpret_cast<uint8_t*>(currentFreeBlock);
			if (!nextFreeBlock && currentBlockAddress >= blockEnd)
			{
				nextFreeBlock = currentFreeBlock;
				break;
			}

			previusFreeBlock = currentFreeBlock;
			currentFreeBlock = currentFreeBlock->next;
		}

		uint8_t* nextFreeBlockAddress = reinterpret_cast<uint8_t*>(nextFreeBlock);
		uint8_t* previusFreeBlockAddress = reinterpret_cast<uint8_t*>(previusFreeBlock);

		FreeBlock* workingBlock = nullptr;
		if (!previusFreeBlock || (previusFreeBlockAddress + previusFreeBlock->size != blockStart))
		{
			workingBlock = reinterpret_cast<FreeBlock*>(blockStart);
			workingBlock->size = blockSize;
			workingBlock->next = nullptr;

			if (previusFreeBlock)
				previusFreeBlock->next = workingBlock;
			else
				m_FreeBlock = workingBlock;
		}
		else
		{
			workingBlock = previusFreeBlock;
			workingBlock->size += blockSize;
		}
		
		if (nextFreeBlock)
		{
			if (blockStart + blockSize == nextFreeBlockAddress)
			{
				workingBlock->size += nextFreeBlock->size;
				workingBlock->next = nextFreeBlock->next;
			}
			else
				workingBlock->next = nextFreeBlock;
		}
	}

	template<typename T>
	void DeallocateDestroy(T* allocation)
	{
		allocation->~T();
		Deallocate(allocation);
	}

private:
	uint8_t m_Storage[UsableSize];
	FreeBlock* m_FreeBlock = nullptr;
};