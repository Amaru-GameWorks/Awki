#pragma once
#include <cmath>
#include <array>
#include <vector>

template<typename T, size_t PageSize>
class AkPageAllocator
{
public:
	void Clear()
	{
		m_Pages.clear();
		m_TotalElements = 0;
	}

	void Resize(size_t elements)
	{
		const size_t newPageCount = static_cast<size_t>(std::ceil(static_cast<float>(elements) / static_cast<float>(PageSize)));
		if (newPageCount != m_Pages.size())
		{
			m_Pages.resize(newPageCount);
			m_TotalElements = m_Pages.size() * PageSize;
		}
	}

	size_t Count() const
	{
		return m_TotalElements;
	}

	std::vector<std::array<T, PageSize>>& GetPages()
	{
		return m_Pages;
	}

	T& operator[](size_t index) 
	{
		const size_t pageIndex = index / PageSize;
		const size_t pageElement = index % PageSize;
		return m_Pages[pageIndex][pageElement];
	}

private:
	size_t m_TotalElements = 0;
	std::vector<std::array<T, PageSize>> m_Pages;
};