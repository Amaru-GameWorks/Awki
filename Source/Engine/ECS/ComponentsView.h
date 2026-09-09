#pragma once
#include "Archetype.h"
#include "ComponentTypeInfo.h"

#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_set>

template<AkComponent... Types>
class AkComponentsView
{
public:
	AkComponentsView(const std::vector<AkArchetype*>& archetypes)
		: m_Archetypes(archetypes)
	{ }

	const std::vector<AkEntity>& GetEntities()
	{
		if (m_Archetypes.empty())
		{
			static std::vector<AkEntity> sDummy;
			return sDummy;
		}
		
		if constexpr (sizeof...(Types) > 1)
		{
			if (m_Entities.empty())
			{
				std::unordered_set<AkEntity> uniqueEntities;
				for (const std::shared_ptr<AkArchetype>& archetype : m_Archetypes)
					uniqueEntities.insert_range(archetype->GetEntities());

				m_Entities.append_range(uniqueEntities);
			}

			return m_Entities;
		}
		else
		{
			return m_Archetypes[0]->GetEntities();
		}
	}

	size_t Count() const
	{
		if (m_Archetypes.empty())
			return 0;

		return m_Archetypes[0]->Count();
	}

	template<typename Function>
	void ForEach(Function&& function)
	{
		if (m_Archetypes.empty())
			return;

		auto indexSequence = std::index_sequence_for<Types...>{};
		ForEach(function, indexSequence);
	}

	template <AkEntityTag ...Tags>
	AkComponentsView<Types...> WithTags()
	{
		AkComponentTypeHash tagsHash = {};
		(tagsHash.set(GetBitIndex<Tags>(), true), ...);

		std::vector<AkArchetype*> compatibleArchetypes = {};
		compatibleArchetypes.reserve(m_Archetypes.size());

		for (auto& archetype : m_Archetypes)
		{
			const AkComponentTypeHash testHash = archetype->GetHash() & tagsHash;
			if (testHash == tagsHash)
			{
				compatibleArchetypes.push_back(archetype);
			}
		}

		return AkComponentsView<Types...>(compatibleArchetypes);
	}

	template<typename SortFunction>
	AkComponentsView<Types...>& Sort(SortFunction&& function)
	{
		std::sort(m_Archetypes.begin(), m_Archetypes.end(), function);
		return *this;
	}

	AkComponentsView<Types...>& HierarchySort();

private:
	std::vector<AkEntity> m_Entities = {};
	std::vector<AkArchetype*> m_Archetypes;

	template<typename Function, size_t... Is>
	void ForEach(Function&& function, std::index_sequence<Is...>)
	{
		for (AkArchetype* archetype : m_Archetypes)
		{
			const std::vector<AkEntity>& entities = archetype->GetEntities();
			std::vector<AkExponentialPageAllocator*> componentAllocators = archetype->GetComponentAllocators<Types...>();
			
			AkExponentialPageAllocator* firstAllocator = componentAllocators[0];
			const size_t pageCount = firstAllocator->PageCount();
			
			size_t entityOffset = 0;
			size_t remaining = archetype->Count();

			std::vector<std::vector<AkPageAllocation>> pages = { componentAllocators[Is]->GetPages()... };
			for (size_t page = 0; page < pageCount; ++page)
			{
				if (remaining == 0)
					break;

				const size_t countInPage = std::min(remaining, firstAllocator->GetPageElementCount(page));

				if constexpr (std::is_invocable_v<Function, AkEntity, Types&...>)
				{

					for (size_t i = 0; i < countInPage; ++i)
						function(entities[entityOffset++], reinterpret_cast<Types*>(pages[Is][page].data)[i]...);
				}
				else
				{
					for (size_t i = 0; i < countInPage; ++i)
						function(reinterpret_cast<Types*>(pages[Is][page].data)[i]...);
				}

				remaining -= countInPage;
			}
		}
	}
};