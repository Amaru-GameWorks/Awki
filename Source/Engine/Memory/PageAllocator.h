#pragma once
#include <new>
#include <vector>

class AkPageAllocator
{
public:
	AkPageAllocator() = default;

	AkPageAllocator(const size_t elementSize, const size_t elementsPerPage, const size_t alignment = 1)
		: m_Alignment(alignment)
		, m_ElementSize(elementSize)
		, m_ElementsPerPage(elementsPerPage)
	{
		m_PageSize = m_ElementSize * m_ElementsPerPage;
	}

	void Clear()
	{
		for (uint8_t* page : m_Pages)
			:: operator delete[](page, std::align_val_t(m_Alignment));
		m_Pages.clear();
	}

	void AllocateNewPage()
	{
		uint8_t* page = static_cast<uint8_t*>(::operator new[](m_PageSize, std::align_val_t(m_Alignment)));
		m_Pages.push_back(page);
	}

	void Resize(size_t newSize)
	{
		const size_t newPageCount = static_cast<size_t>(std::ceil(static_cast<float>(newSize) / static_cast<float>(m_PageSize)));
		if (newPageCount > m_Pages.size())
		{
			const size_t missingPages = newPageCount - m_Pages.size();
			m_Pages.reserve(m_Pages.size() + newPageCount);
		
			for (size_t i = 0; i < missingPages; ++i)
			{
				uint8_t* page = static_cast<uint8_t*>(::operator new[](m_PageSize, std::align_val_t(m_Alignment)));
				m_Pages.push_back(page);
			}
		}
		else if(newPageCount < m_Pages.size())
		{
			for (size_t i = m_Pages.size() - 1; i >= newPageCount; --i)
				:: operator delete[](m_Pages[i], std::align_val_t(m_Alignment));
			m_Pages.resize(newPageCount);
		}
	}

	size_t Size() const
	{
		return m_Pages.size() * m_PageSize;
	}

	size_t PageCount() const
	{
		return m_Pages.size();
	}

	std::vector<uint8_t*>& GetPages()
	{
		return m_Pages;
	}

	uint8_t* Get(size_t index)
	{
		const size_t pageIndex = index / m_ElementsPerPage;
		const size_t pageElement = index % m_ElementsPerPage;
		return &m_Pages[pageIndex][pageElement * m_ElementSize];
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
		const size_t pageIndex = index / m_ElementsPerPage;
		return pageIndex < m_Pages.size();
	}

private:
	size_t m_PageSize = 0;
	size_t m_Alignment = 1;
	size_t m_ElementSize = 0;
	size_t m_ElementsPerPage = 0;
	std::vector<uint8_t*> m_Pages;
};