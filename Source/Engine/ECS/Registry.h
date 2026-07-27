#pragma once
#include "Entity.h"
#include "Core/Assert.h"
#include "ComponentPool.h"
#include "ComponentsView.h"
#include "ComponentTypeInfo.h"

#include <queue>
#include <vector>
#include <memory>
#include <unordered_map>

class AkRegistry
{
private:
	friend class Awki;
	static void Initialize();

public:
	static AkEntity CreateEntity();
	static bool IsEntityValid(AkEntity entity);
	static void DestroyEntity(AkEntity entity);

	template <typename T>
	requires(AkIsComponent<T>::value)
	static T* AddComponent(AkEntity entity)
	{
		constexpr size_t typeId = AkComponentTypeInfo<T>::TypeId();
		AkAssert(m_Generations[entity.id] == entity.generation, "Trying to use stale entity [id: {} - generation: {}]", entity.id, entity.generation);
		AkAssert(m_ComponentPools.contains(typeId), "ComponentPool has not been created for {}", AkComponentTypeInfo<T>::Name());

		AkComponentPool<T>* pool = static_cast<AkComponentPool<T>*>(m_ComponentPools[typeId].get());
		if (pool->Contains(entity))
			return &pool->Get(entity);

		return &pool->Add(entity);
	}

	template <typename T>
	requires(AkIsComponent<T>::value)
	static T* GetComponent(AkEntity entity)
	{
		constexpr size_t typeId = AkComponentTypeInfo<T>::TypeId();
		AkAssert(m_Generations[entity.id] == entity.generation, "Trying to use stale entity [id: {}]", entity.id);
		AkAssert(m_ComponentPools.contains(typeId), "ComponentPool has not been created for {}", AkComponentTypeInfo<T>::Name());

		AkComponentPool<T>* pool = static_cast<AkComponentPool<T>*>(m_ComponentPools[typeId].get());
		if (!pool->Contains(entity))
			return nullptr;

		return &pool->Get(entity);
	}

	template <typename T>
	requires(AkIsComponent<T>::value)
	static void RemoveComponent(AkEntity entity)
	{
		constexpr size_t typeId = AkComponentTypeInfo<T>::TypeId();
		AkAssert(m_Generations[entity.id] == entity.generation, "Trying to use stale entity [id: {}]", entity.id);
		AkAssert(m_ComponentPools.contains(typeId), "ComponentPool has not been created for {}", AkComponentTypeInfo<T>::Name());

		AkComponentPool<T>* pool = static_cast<AkComponentPool<T>*>(m_ComponentPools[typeId].get());
		pool->Remove(entity);
	}

	template <typename ...T>
	requires(AkIsComponent<T>::value && ...)
	static AkComponentsView<T...> GetView()
	{
		return AkComponentsView<T...>({ m_ComponentPools[AkComponentTypeInfo<T>::TypeId()].get()... });
	}

private:
	static inline std::queue<uint32_t> m_FreeIndices;
	static inline std::vector<uint32_t> m_Generations;
	static inline std::unordered_map<size_t, std::unique_ptr<AkComponentPoolBase>> m_ComponentPools;
};