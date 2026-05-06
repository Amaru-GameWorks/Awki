#pragma once
#include "Core/Log.h"
#include "ComponentPool.h"

#include <vector>
#include <algorithm>
#include <functional>

template<typename... Types>
class AkComponentsView
{
public:
	AkComponentsView(std::initializer_list<AkComponentPoolBase*> pools)
		: m_Pools(pools)
	{
		if constexpr (sizeof...(Types) > 1)
		{
			size_t smallestSize = std::numeric_limits<size_t>::max();
			auto minIterator = std::ranges::min_element(m_Pools, {}, [](AkComponentPoolBase* pool) { return pool->Count(); });
			m_SmallesIndex = std::distance(m_Pools.begin(), minIterator);
		}
	}

	void ForEach(const std::function<void(Types&...)>&& function)
	{
		if constexpr (sizeof...(Types) > 1)
		{
			auto indexSequence = std::index_sequence_for<Types...>{};
			ForEach(std::move(function), indexSequence);
		}
		else
		{
			for (auto component : static_cast<AkComponentPool<Types...>*>(m_Pools[m_SmallesIndex])->GetComponents())
				function(component);
		}
	}

private:
	size_t m_SmallesIndex = 0;
	std::vector<AkComponentPoolBase*> m_Pools;

	template<size_t... Is>
	void ForEach(const std::function<void(Types&...)>& function, std::index_sequence<Is...>)
	{
		for (uint32_t entity : m_Pools[m_SmallesIndex]->GetEntities())
		{
			if (std::ranges::all_of(m_Pools, [entity](AkComponentPoolBase* pool) { return pool->Contains(entity); }))
				function(static_cast<AkComponentPool<Types>*>(m_Pools[Is])->Get(entity)...);
		}
	}
};