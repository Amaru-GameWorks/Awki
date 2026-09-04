#pragma once
#include "Entity.h"
#include "Core/Assert.h"
#include "ComponentTypeInfo.h"

#include <queue>
#include <bitset>
#include <vector>
#include <memory>
#include <unordered_map>

#define AK_USE_ARCHETYPES 1

#if AK_USE_ARCHETYPES
#include "Archetype.h"
#include "ComponentsArchetypeView.h"
#else
#include "ComponentPool.h"
#include "ComponentsPoolView.h"
#endif

class AkRegistry
{
public:
	static AkEntity CreateEntity()
	{
		uint32_t index = 0;
		if (!m_FreeIndices.empty())
		{
			index = m_FreeIndices.front();
			m_FreeIndices.pop();
		}
		else
		{
			index = static_cast<uint32_t>(m_Generations.size());
			m_Generations.push_back(0);
			m_EntityToArchetype.push_back(nullptr);
		}

		return AkEntity{ .id = index, .generation = m_Generations[index] };
	}

	static bool IsEntityValid(AkEntity entity)
	{
		if (entity.id < m_Generations.size())
			return m_Generations[entity.id] == entity.generation;
		return false;
	}

	static void DestroyEntity(AkEntity entity)
	{
		AkAssert(m_Generations[entity.id] == entity.generation, "Trying to use stale entity [id: {} - generation: {}]", entity.id, entity.generation);

		if (!IsEntityValid(entity))
			return;

#if AK_USE_ARCHETYPES
		m_EntityToArchetype[entity.id]->Remove(entity);
		m_EntityToArchetype[entity.id] = nullptr;
#else
		for (auto& [typeId, pool] : m_ComponentPools)
			pool->Remove(entity);
#endif

		++m_Generations[entity.id];
		m_FreeIndices.push(entity.id);
	}

	template <AkComponent T>
	static T* AddComponent(AkEntity entity)
	{
		AkAssert(m_Generations[entity.id] == entity.generation, "Trying to use stale entity [id: {} - generation: {}]", entity.id, entity.generation);

		if (!IsEntityValid(entity))
			return nullptr;

#if AK_USE_ARCHETYPES
		AkArchetypeHash archetypeHash = {};
		std::apply([entity, &archetypeHash](auto... type)
#else
		std::apply([entity](auto... type)
#endif
		{
#if AK_USE_ARCHETYPES
			(archetypeHash.set(AkComponentTypeInfo<decltype(type)>::BitIndex(), true), ...);
#else
			(AddComponentInternal<decltype(type)>(entity), ...);
#endif
		}, AkComponentWithDependencies<T>{});

#if AK_USE_ARCHETYPES
		if (!m_Archetypes.contains(archetypeHash))
		{
			std::shared_ptr<AkArchetype> archetype = std::make_shared<AkArchetype>(archetypeHash);
			archetype->Initialize(AkComponentWithDependencies<T>{});
			m_Archetypes[archetypeHash] = archetype;
		}

		std::shared_ptr<AkArchetype>& newArchetype = m_Archetypes[archetypeHash];
		std::shared_ptr<AkArchetype>& oldArchetype = m_EntityToArchetype[entity.id];

		if (oldArchetype)
			newArchetype->Migrate(entity, *oldArchetype);
		else
			newArchetype->Add(entity);

		oldArchetype = newArchetype;
#endif

		return GetComponent<T>(entity);
	}

	template <AkComponent T>
	static T* GetComponent(AkEntity entity)
	{
		AkAssert(m_Generations[entity.id] == entity.generation, "Trying to use stale entity [id: {} - generation: {}]", entity.id, entity.generation);

		if (!IsEntityValid(entity))
			return nullptr;

#if AK_USE_ARCHETYPES
		if (std::shared_ptr<AkArchetype>& entityArchetype = m_EntityToArchetype[entity.id])
		{
			return &entityArchetype->GetComponent<T>(entity);
		}
		else
		{
			return nullptr;
		}
#else
		std::shared_ptr<AkComponentPool<T>> pool = GetOrCreatePool<T>();
		if (!pool->Contains(entity))
			return nullptr;
	
		return &pool->Get(entity);
#endif

	}

	template <AkComponent T>
	static void RemoveComponent(AkEntity entity)
	{
		AkAssert(m_Generations[entity.id] == entity.generation, "Trying to use stale entity [id: {} - generation: {}]", entity.id, entity.generation);
		
		if (!IsEntityValid(entity))
			return;

#if AK_USE_ARCHETYPES
		if (std::shared_ptr<AkArchetype>& entityArchetype = m_EntityToArchetype[entity.id])
		{
			AkArchetypeHash newHash = entityArchetype->GetHash();
			newHash.set(AkComponentTypeInfo<T>::BitIndex(), false);

			if (newHash != 0)
			{
				std::shared_ptr<AkArchetype> oldArchetype = entityArchetype;

				auto newArchetype = m_Archetypes.find(newHash);
				if (newArchetype != m_Archetypes.end())
				{
					entityArchetype = newArchetype->second;
				}
				else
				{
					std::unordered_map<size_t, AkComponentDescriptor> descriptors = oldArchetype->GetComponentDescriptors();
					descriptors.erase(AkComponentTypeInfo<T>::TypeId());

					std::shared_ptr<AkArchetype> newArchetype = std::make_shared<AkArchetype>(newHash);
					newArchetype->Initialize(descriptors);
					m_Archetypes[newHash] = newArchetype;

					entityArchetype = newArchetype;
				}

				entityArchetype->Migrate(entity, *oldArchetype);
			}
			else
			{
				entityArchetype->Remove(entity);
			}
		}
#else
		std::shared_ptr<AkComponentPool<T>> pool = GetOrCreatePool<T>();
		pool->Remove(entity);
#endif
	}

	template <AkComponent ...T>
	static AkComponentsView<T...> GetView()
	{
#if AK_USE_ARCHETYPES
		AkArchetypeHash viewHash = {};
		(viewHash.set(AkComponentTypeInfo<T>::BitIndex(), true), ...);

		std::vector<std::shared_ptr<AkArchetype>> compatibleArchetypes = {};
		compatibleArchetypes.reserve(sizeof...(T));

		for (auto& [hash, archetype] : m_Archetypes)
		{
			const AkArchetypeHash testHash = hash & viewHash;
			if (testHash == viewHash)
			{
				compatibleArchetypes.push_back(archetype);
			}
		}

		return AkComponentsView<T...>(compatibleArchetypes);
#else
		return AkComponentsView<T...>({ GetOrCreatePool<T>()... });
#endif
	}

private:
	static inline std::queue<uint32_t> m_FreeIndices;
	static inline std::vector<uint32_t> m_Generations;

#if AK_USE_ARCHETYPES
	static inline std::unordered_map<AkArchetypeHash, std::shared_ptr<AkArchetype>> m_Archetypes;
	static inline std::vector<std::shared_ptr<AkArchetype>> m_EntityToArchetype;
#else
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
#endif
};