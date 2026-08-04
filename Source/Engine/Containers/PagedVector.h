#pragma once
#include "Memory/PageAllocator.h"

#include <functional>

template<typename T, size_t PageSize>
class AkPagedVector
{
public:
	void PushBack(const T& element)
	{
		const size_t index = m_Size++;
		m_Allocator.Resize(m_Size);

		m_Allocator[index] = element;
	}

	void PushBack(T&& element)
	{
		const size_t index = m_Size++;
		m_Allocator.Resize(m_Size);

		new (&m_Allocator[index]) T(std::move(element));
	}

	T& EmplaceBack()
	{
		const size_t index = m_Size++;
		m_Allocator.Resize(m_Size);
		return m_Allocator[index];
	}

	void Resize(size_t newSize)
	{
		m_Size = newSize;
		m_Allocator.Resize(m_Size);
	}

	size_t Size() const
	{
		return m_Size;
	}

	size_t Capacity() const
	{
		return m_Allocator.Count();
	}

	template<typename Function>
	void ForEach(Function&& function)
	{
		std::vector<std::array<T, PageSize>>& pages = m_Allocator.GetPages();
		
		size_t remaining = m_Size;
		for (std::array<T, PageSize>& page : pages)
		{
			const size_t countInPage = std::min(remaining, PageSize);
			for (size_t i = 0; i < PageSize; ++i)
			{
				function(page[i]);
			}

			remaining -= countInPage;
		}
	}

private:
	size_t m_Size = 0;
	AkPageAllocator<T, PageSize> m_Allocator;
};