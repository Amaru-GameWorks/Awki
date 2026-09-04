#pragma once
#include "Archetype.h"
#include "ComponentTypeInfo.h"

#include <vector>
#include <memory>
#include <unordered_set>

template<AkComponent... Types>
class AkComponentsView
{
public:
	AkComponentsView(const std::vector<std::shared_ptr<AkArchetype>>& archetypes)
	{
		for (auto& archetype : archetypes)
		{
			if (archetype)
				m_Archetypes.push_back(archetype);
		}
	}

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

private:
	std::vector<AkEntity> m_Entities = {};
	std::vector<std::shared_ptr<AkArchetype>> m_Archetypes;

	template<typename Function, size_t... Is>
	void ForEach(Function&& function, std::index_sequence<Is...>)
	{
		for (std::shared_ptr<AkArchetype>& archetype : m_Archetypes)
		{
			std::vector<AkPageAllocator*> componentAllocators = archetype->GetComponentAllocators<Types...>();
			std::vector<std::vector<uint8_t*>> pages = { componentAllocators[Is]->GetPages()... };
			const size_t pageCount = componentAllocators[0]->PageCount();
			
			size_t remaining = archetype->Count();
			for (size_t page = 0; page < pageCount; ++page)
			{
				if (remaining == 0)
					break;

				const size_t countInPage = std::min(remaining, AkArchetype::kMaxComponentPerPage);

				if constexpr (std::is_invocable_v<Function, AkEntity, Types&...>)
				{
					const std::vector<AkEntity>& entities = archetype->GetEntities();
					const size_t entitiesOffset = page * AkArchetype::kMaxComponentPerPage;

					for (size_t i = 0; i < countInPage; ++i)
						function(entities[entitiesOffset + i], reinterpret_cast<Types*>(pages[Is][page])[i]...);
				}
				else
				{
					for (size_t i = 0; i < countInPage; ++i)
						function(reinterpret_cast<Types*>(pages[Is][page])[i]...);
				}
				

				remaining -= countInPage;
			}
		}
	}
};