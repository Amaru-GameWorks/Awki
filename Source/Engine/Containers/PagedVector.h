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

		T* newElement = m_WriteHead++;
		new (newElement) T();

		return *newElement;
	}

	T& Get(size_t index)
	{
		return m_Allocator.Get<T>(index);
	}

	void Reserve(size_t newCapacity)
	{
		if (newCapacity <= m_Capacity)
			return;

		m_Allocator.Resize(newCapacity * sizeof(T));
		m_Capacity = m_Allocator.ElementCount();
	}

	void Resize(size_t newSize)
	{
		if (m_Size == newSize)
			return;

		if (m_Size > newSize)
		{
			for (size_t i = m_Size; i > newSize; --i)
				m_Allocator.Get<T>(i - 1).~T();

			m_Allocator.Resize(newSize * sizeof(T));
		}
		else
		{
			m_Allocator.Resize(newSize * sizeof(T));

			for (size_t i = m_Size; i < newSize; ++i)
				new (&m_Allocator.Get<T>(i)) T();
		}

		m_Size = newSize;
		m_Capacity = m_Allocator.ElementCount();

		if (m_Size != 0)
		{
			m_WriteHead = reinterpret_cast<T*>(m_Allocator.GetPages().back());
			m_WriteHead += (m_Size % PageSize);
		}
		else
		{
			m_WriteHead = nullptr;
		}
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
	size_t m_Capacity = 0;
	T* m_WriteHead = nullptr;
	AkPageAllocator m_Allocator;

	void ResizeInternal()
	{
		const size_t availableSpace = m_Capacity - m_Size;
		if (availableSpace == 0)
		{
			m_Allocator.AllocateNewPage();
			m_Capacity = m_Allocator.ElementCount();

			m_WriteHead = reinterpret_cast<T*>(m_Allocator.GetPages().back());
		}
	}
};