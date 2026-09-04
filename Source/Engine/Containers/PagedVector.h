#pragma once
#include "Memory/PageAllocator.h"

#include <functional>

template<typename T, size_t PageSize>
class AkPagedVector
{
public:
	AkPagedVector()
	{
		m_Allocator = AkPageAllocator(sizeof(T), PageSize, alignof(T));
	}

	void PushBack(const T& element)
	{
		ResizeInternal();

		++m_Size;
		*(m_WriteHead++) = element;
	}

	void PushBack(T&& element)
	{
		ResizeInternal();

		++m_Size;
		std::construct_at(m_WriteHead++, std::move(element));
	}

	T& EmplaceBack()
	{
		ResizeInternal();
		++m_Size;

		return *(m_WriteHead++);
	}

	void Resize(size_t newSize)
	{
		m_Size = newSize;
		m_Allocator.Resize(m_Size * sizeof(T));

		m_WriteHead = reinterpret_cast<T*>(m_Allocator.GetPages().back());
		m_WriteHead += (m_Size % PageSize);
	}

	size_t Size() const
	{
		return m_Size;
	}

	size_t Capacity() const
	{
		return m_Allocator.Size();
	}

	template<typename Function>
	void ForEach(Function&& function)
	{
		std::vector<uint8_t*>& pages = m_Allocator.GetPages();

		size_t remaining = m_Size;
		for (uint8_t* page : pages)
		{
			if (remaining == 0)
				break;

			T* typedPage = reinterpret_cast<T*>(page);
			const size_t countInPage = std::min(remaining, PageSize);

			for (size_t i = 0; i < countInPage; ++i)
				function(typedPage[i]);

			remaining -= countInPage;
		}
	}

private:
	size_t m_Size = 0;
	T* m_WriteHead = nullptr;
	AkPageAllocator m_Allocator;

	void ResizeInternal()
	{
		if (m_Size % PageSize == 0)
		{
			m_Allocator.AllocateNewPage();
			m_WriteHead = reinterpret_cast<T*>(m_Allocator.GetPages().back());
		}
	}
};