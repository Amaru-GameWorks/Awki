#pragma once
#include "ComponentPool.h"
#include "ComponentTypeInfo.h"

#include <vector>
#include <algorithm>

template<AkComponent... Types>
class AkComponentsView
{
public:
	AkComponentsView(std::initializer_list<std::shared_ptr<AkComponentPoolBase>> pools)
		: m_Pools(pools)
	{
		if constexpr (sizeof...(Types) > 1)
		{
			auto minIterator = std::ranges::min_element(m_Pools, {}, [](std::shared_ptr<AkComponentPoolBase> pool) { return pool->Count(); });
			m_SmallesIndex = std::distance(m_Pools.begin(), minIterator);
		}
	}

	const std::vector<AkEntity>& GetEntities()
	{
		return m_Pools[m_SmallesIndex]->GetEntities();
	}

	size_t Count() const
	{
		return m_Pools[m_SmallesIndex]->Count();
	}

	template<typename Function>
	void ForEach(Function&& function)
	{
		if constexpr (sizeof...(Types) > 1)
		{
			auto indexSequence = std::index_sequence_for<Types...>{};
			ForEach(std::forward<Function>(function), indexSequence);
		}
		else
		{
			if constexpr (std::is_invocable_v<Function, AkEntity, Types&...>)
			{
				for (AkEntity entity : m_Pools[m_SmallesIndex]->GetEntities())
					function(entity, std::static_pointer_cast<AkComponentPool<Types>>(m_Pools[m_SmallesIndex])->Get(entity)...);
			}
			else
			{
				for (auto component : std::static_pointer_cast<AkComponentPool<Types...>>(m_Pools[m_SmallesIndex])->GetComponents())
					function(component);
			}
		}
	}

private:
	size_t m_SmallesIndex = 0;
	std::vector<std::shared_ptr<AkComponentPoolBase>> m_Pools;

	template<typename Function, size_t... Is>
	void ForEach(Function&& function, std::index_sequence<Is...>)
	{
		std::vector< std::shared_ptr<AkComponentPoolBase>> poolsToCheckEntityValidity;
		poolsToCheckEntityValidity.reserve(m_Pools.size() - 1);

		for (size_t i = 0; i < m_Pools.size(); ++i)
		{
			if(i != m_SmallesIndex)
				poolsToCheckEntityValidity.push_back(m_Pools[i]);
		}

		if constexpr (std::is_invocable_v<Function, AkEntity, Types&...>)
		{
			for (AkEntity entity : m_Pools[m_SmallesIndex]->GetEntities())
			{
				if (std::ranges::all_of(poolsToCheckEntityValidity, [entity](std::shared_ptr<AkComponentPoolBase> pool) { return pool->Contains(entity); }))
					function(entity, std::static_pointer_cast<AkComponentPool<Types>>(m_Pools[Is])->Get(entity)...);
			}
		}
		else
		{
			for (AkEntity entity : m_Pools[m_SmallesIndex]->GetEntities())
			{
				if (std::ranges::all_of(poolsToCheckEntityValidity, [entity](std::shared_ptr<AkComponentPoolBase> pool) { return pool->Contains(entity); }))
					function(std::static_pointer_cast<AkComponentPool<Types>>(m_Pools[Is])->Get(entity)...);
			}
		}
	}
};