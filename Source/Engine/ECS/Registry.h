#pragma once
#include "Entity.h"
#include "Core/Assert.h"
#include "ComponentPool.h"
#include "ComponentsView.h"
#include "ComponentTypeInfo.h"

#include <queue>
#include <bitset>
#include <vector>
#include <memory>
#include <unordered_map>

class AkRegistry
{
public:
	static AkEntity CreateEntity();
	static bool IsEntityValid(AkEntity entity);
	static void DestroyEntity(AkEntity entity);

	template <AkComponent T>
	static T* AddComponent(AkEntity entity)
	{
		AkAssert(m_Generations[entity.id] == entity.generation, "Trying to use stale entity [id: {} - generation: {}]", entity.id, entity.generation);

		std::apply([entity](auto... type)
		{
			(AddComponentInternal<decltype(type)>(entity), ...);
		}, AkComponentWithDependencies<T>{});

		return GetComponent<T>(entity);
	}

	template <AkComponent T>
	static T* GetComponent(AkEntity entity)
	{
		AkAssert(m_Generations[entity.id] == entity.generation, "Trying to use stale entity [id: {}]", entity.id);
	
		std::shared_ptr<AkComponentPool<T>> pool = GetOrCreatePool<T>();
		if (!pool->Contains(entity))
			return nullptr;
	
		return &pool->Get(entity);
	}

	template <AkComponent T>
	static void RemoveComponent(AkEntity entity)
	{
		AkAssert(m_Generations[entity.id] == entity.generation, "Trying to use stale entity [id: {}]", entity.id);
		std::shared_ptr<AkComponentPool<T>> pool = GetOrCreatePool<T>();
		pool->Remove(entity);
	}

	template <AkComponent ...T>
	static AkComponentsView<T...> GetView()
	{
		return AkComponentsView<T...>({ GetOrCreatePool<T>()... });
	}

private:
	static inline std::queue<uint32_t> m_FreeIndices;
	static inline std::vector<uint32_t> m_Generations;
	static inline std::unordered_map<size_t, std::shared_ptr<AkComponentPoolBase>> m_ComponentPools;

	template <AkComponent T>
	static void AddComponentInternal(AkEntity entity)
	{
		std::shared_ptr<AkComponentPool<T>> pool = GetOrCreatePool<T>();
		if (!pool->Contains(entity))
			pool->Add(entity);
	}

	template <AkComponent T>
	static std::shared_ptr<AkComponentPool<T>> GetOrCreatePool()
	{
		constexpr size_t typeId = AkComponentTypeInfo<T>::TypeId();

		auto it = m_ComponentPools.find(typeId);
		if (it != m_ComponentPools.end() && it->second)
			return std::static_pointer_cast<AkComponentPool<T>>(it->second);

		std::shared_ptr<AkComponentPool<T>> newPool = std::make_shared<AkComponentPool<T>>();
		m_ComponentPools[typeId] = newPool;
		return newPool;
	}
};