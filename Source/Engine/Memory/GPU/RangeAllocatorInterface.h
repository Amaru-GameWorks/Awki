#pragma once
#include <list>
#include <unordered_map>

class AkRangeAllocatorInterface
{
	struct FreeBlock
	{
		size_t size = 0;
		size_t offset = 0;
		std::list<FreeBlock>::iterator next = {};
	};

public:
	AkRangeAllocatorInterface(size_t size)
		: m_Size(size) 
	{
		FreeBlock& freeBlock = m_FreeBlocks.emplace_back();
		freeBlock.offset = 0;
		freeBlock.size = m_Size;
		freeBlock.next = m_FreeBlocks.end();
		m_Root = m_FreeBlocks.begin();
	}

	size_t Allocate(size_t allocationSize)
	{
		std::list<FreeBlock>::iterator previousFreeBlock = m_FreeBlocks.end();
		std::list<FreeBlock>::iterator currentFreeBlock = m_Root;

		std::list<FreeBlock>::iterator previousBestFit = m_FreeBlocks.end();
		std::list<FreeBlock>::iterator currentBestFit = m_FreeBlocks.end();

		while (currentFreeBlock != m_FreeBlocks.end())
		{
			if (currentFreeBlock->size >= allocationSize)
			{
				if (currentBestFit == m_FreeBlocks.end() || currentFreeBlock->size < currentBestFit->size)
				{
					previousBestFit = previousFreeBlock;
					currentBestFit = currentFreeBlock;
				}

				if (currentFreeBlock->size == allocationSize)
					break;
			}

			previousFreeBlock = currentFreeBlock;
			currentFreeBlock = currentFreeBlock->next;
		}

		if (currentBestFit == m_FreeBlocks.end())
			return SIZE_MAX;

		const size_t allocationOffset = currentBestFit->offset;

		//Split the block
		FreeBlock& newBlock = m_FreeBlocks.emplace_back();
		newBlock.offset = currentBestFit->offset + allocationSize;
		newBlock.size = currentBestFit->size - allocationSize;
		newBlock.next = currentBestFit->next;


		std::list<FreeBlock>::iterator newBlockIterator = --m_FreeBlocks.end();
		m_FreeBlocks.erase(currentBestFit);

		if (previousBestFit != m_FreeBlocks.end())
			previousBestFit->next = newBlockIterator;
		else
			m_Root = newBlockIterator;

		m_OffsetToSize[allocationOffset] = allocationSize;
		return allocationOffset;
	}

	void Deallocate(size_t allocationOffset)
	{
		size_t blockStart = allocationOffset;
		size_t blockSize = m_OffsetToSize[blockStart];
		size_t blockEnd = blockStart + blockSize;
		
		std::list<FreeBlock>::iterator nextFreeBlock = m_FreeBlocks.end();
		std::list<FreeBlock>::iterator previusFreeBlock = m_FreeBlocks.end();
		
		// Find the first free block that starts after or is at the boundary of this block
		std::list<FreeBlock>::iterator currentFreeBlock = m_Root;
		while (currentFreeBlock != m_FreeBlocks.end())
		{
			if (nextFreeBlock == m_FreeBlocks.end() && currentFreeBlock->offset >= blockEnd)
			{
				nextFreeBlock = currentFreeBlock;
				break;
			}
		
			previusFreeBlock = currentFreeBlock;
			currentFreeBlock = currentFreeBlock->next;
		}

		std::list<FreeBlock>::iterator workingBlock = m_FreeBlocks.end();
		if (previusFreeBlock == m_FreeBlocks.end() || (previusFreeBlock->offset + previusFreeBlock->size != blockStart))
		{
			FreeBlock& newBlock = m_FreeBlocks.emplace_back();
			newBlock.offset = blockStart;
			newBlock.size = blockSize;
			newBlock.next = m_FreeBlocks.end();
			workingBlock = --m_FreeBlocks.end();

			if (previusFreeBlock != m_FreeBlocks.end())
				previusFreeBlock->next = workingBlock;
			else
				m_Root = workingBlock;
		}
		else
		{
			workingBlock = previusFreeBlock;
			workingBlock->size += blockSize;
		}
		
		if (nextFreeBlock != m_FreeBlocks.end())
		{
			if (blockStart + blockSize == nextFreeBlock->offset)
			{
				workingBlock->size += nextFreeBlock->size;
				workingBlock->next = nextFreeBlock->next;
				m_FreeBlocks.erase(nextFreeBlock);
			}
			else
				workingBlock->next = nextFreeBlock;
		}
	}

private:
	size_t m_Size = 0;
	std::list<FreeBlock>::iterator m_Root;
	std::list<FreeBlock> m_FreeBlocks = {};
	std::unordered_map<size_t, size_t> m_OffsetToSize = {};
};