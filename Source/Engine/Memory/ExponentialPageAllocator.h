#pragma once
#include "Core/Assert.h"

#include <new>
#include <bit>
#include <vector>

struct AkPageAllocation
{
	size_t capacity = 0;
	uint8_t* data = nullptr;
};

class AkExponentialPageAllocator
{
public:
	AkExponentialPageAllocator() = default;

	AkExponentialPageAllocator(const size_t elementSize, const size_t minElementsPerPage, const size_t maxElementsPerPage, const size_t alignment = 1)
		: m_Alignment(alignment)
		, m_ElementSize(elementSize)
		, m_MinElementsPerPage(minElementsPerPage)
		, m_MaxElementsPerPage(maxElementsPerPage)
	{
		AkAssert(std::has_single_bit(m_MinElementsPerPage), "Min Elements is not a power of 2");
		AkAssert(std::has_single_bit(m_MaxElementsPerPage), "Max Elements is not a power of 2");
		AkAssert(m_MinElementsPerPage < m_MaxElementsPerPage, "Min Elements is not smaller than Max Elements");

		const size_t ratio = m_MaxElementsPerPage / m_MinElementsPerPage;
		m_MaxDoublingPages = (ratio > 0) ? static_cast<size_t>(std::bit_width(ratio) - 1) : 0;
		m_DoublingRegionCapacity = m_MinElementsPerPage * ((size_t(1) << m_MaxDoublingPages) - 1);
	}

	void Clear()
	{
		for (AkPageAllocation& allocation : m_Pages)
			:: operator delete[](allocation.data, std::align_val_t(m_Alignment));
		m_Pages.clear();
	}

	void AllocateNewPage()
	{
		size_t numElements = m_MinElementsPerPage;

		if (!m_Pages.empty())
		{
			if (m_Pages.size() < m_MaxDoublingPages)
				numElements = m_Pages.back().capacity * 2;
			else
				numElements = m_MaxElementsPerPage;
		}

		AkPageAllocation& allocation = m_Pages.emplace_back();
		allocation.capacity = numElements;
		allocation.data = static_cast<uint8_t*>(::operator new[](numElements * m_ElementSize, std::align_val_t(m_Alignment)));

		m_TotalCapacity += numElements;
	}

	void Resize(size_t newSize)
	{
		while (m_TotalCapacity < newSize)
			AllocateNewPage();

		while (!m_Pages.empty())
		{
			const size_t capacityWithoutLastPage = m_TotalCapacity - m_Pages.back().capacity;
			if (capacityWithoutLastPage >= newSize)
			{
				::operator delete[](m_Pages.back().data, std::align_val_t(m_Alignment));
				m_TotalCapacity -= m_Pages.back().capacity;
				m_Pages.pop_back();
			}
			else
			{
				break;
			}
		}
	}

	size_t Size() const
	{
		return m_TotalCapacity * m_ElementSize;
	}

	size_t PageCount() const
	{
		return m_Pages.size();
	}

	size_t ElementCount() const
	{
		return m_TotalCapacity;
	}

	size_t GetPageElementCount(size_t pageIndex) const
	{
		return m_Pages[pageIndex].capacity;
	}

	const std::vector<AkPageAllocation>& GetPages()
	{
		return m_Pages;
	}

	uint8_t* Get(size_t index)
	{
		AkAssert(index < m_TotalCapacity, "Element out of range");

		if (index >= m_TotalCapacity)
			return nullptr;

		size_t pageIndex = 0;
		size_t localElementIndex = 0;

		if (index < m_DoublingRegionCapacity)
		{
			const size_t ratio = index / m_MinElementsPerPage;
			pageIndex = static_cast<size_t>(std::bit_width(ratio + 1) - 1);

			const size_t elementsBeforePage = m_MinElementsPerPage * ((size_t(1) << pageIndex) - 1);
			localElementIndex = index - elementsBeforePage;
		}
		else
		{
			const size_t remainingIndex = index - m_DoublingRegionCapacity;
			pageIndex = m_MaxDoublingPages + (remainingIndex / m_MaxElementsPerPage);
			localElementIndex = remainingIndex % m_MaxElementsPerPage;
		}

		return m_Pages[pageIndex].data + (localElementIndex * m_ElementSize);
	}

	template<typename T>
	T& Get(size_t index)
	{
		return *reinterpret_cast<T*>(Get(index));
	}

	void Override(size_t indexA, size_t indexB)
	{
		uint8_t* elementA = Get(indexA);
		uint8_t* elementB = Get(indexB);
		std::memcpy(elementA, elementB, m_ElementSize);
	}

	bool SpaceAvailable(size_t index)
	{
		return m_TotalCapacity > index;
	}

private:
	size_t m_Alignment = 1;
	size_t m_ElementSize = 0;
	size_t m_MinElementsPerPage = 0;
	size_t m_MaxElementsPerPage = 0;
	
	size_t m_TotalCapacity = 0;
	size_t m_MaxDoublingPages = 0;
	size_t m_DoublingRegionCapacity = 0;
	std::vector<AkPageAllocation> m_Pages;
};