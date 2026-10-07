#pragma once
#include "Entity.h"
#include "Archetype.h"
#include "Core/Assert.h"
#include "ComponentsView.h"
#include "ComponentTypeInfo.h"

#include <queue>
#include <vector>
#include <memory>
#include <unordered_map>

class AkRegistry
{
public:
	static AkEntity CreateEntity()
	{
		uint32_t index = 0;
		if (!m_FreeIndices.empty())
		{
			index = m_FreeIndices.back();
			m_FreeIndices.pop_back();
		}
		else
		{
			index = static_cast<uint32_t>(m_Generations.size());
			m_Generations.push_back(0);
			m_EntityRecords.push_back(nullptr);
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

		AkArchetype*& archetype = m_EntityRecords[entity.id];
		archetype->Remove(entity);
		archetype = nullptr;

		++m_Generations[entity.id];
		m_FreeIndices.push_back(entity.id);
	}

	template <AkComponent T>
	static T* AddComponent(AkEntity entity)
	{
		AkAssert(IsEntityValid(entity), "Trying to use stale entity [id: {} - generation: {}]", entity.id, entity.generation);

		if (!IsEntityValid(entity))
			return nullptr;

		AkComponentTypeHash componentTypeHash = {};
		std::apply([entity, &componentTypeHash](auto... type)
		{
			(componentTypeHash.set(GetBitIndex<decltype(type)>(), true), ...);
		}, AkComponentWithDependencies<T>{});

		AkArchetype*& oldArchetype = m_EntityRecords[entity.id];
		AkArchetypeHash archetypeHash = { oldArchetype ? oldArchetype->GetParentId() : kNullEntityId, componentTypeHash };
		
		if (!m_Archetypes.contains(archetypeHash))
		{
			std::unique_ptr<AkArchetype> archetype = std::make_unique<AkArchetype>(archetypeHash);
			archetype->Initialize(AkComponentWithDependencies<T>{});
			m_Archetypes[archetypeHash] = std::move(archetype);
		}

		std::unique_ptr<AkArchetype>& newArchetype = m_Archetypes[archetypeHash];

		if (oldArchetype)
			newArchetype->Migrate(entity, *oldArchetype);
		else
			newArchetype->Add(entity);

		oldArchetype = newArchetype.get();
		return GetComponent<T>(entity);
	}

	template <AkComponent T>
	static T* GetComponent(AkEntity entity)
	{
		AkAssert(IsEntityValid(entity), "Trying to use stale entity [id: {} - generation: {}]", entity.id, entity.generation);

		if (!IsEntityValid(entity))
			return nullptr;

		if (AkArchetype* entityArchetype = m_EntityRecords[entity.id])
		{
			return &entityArchetype->GetComponent<T>(entity);
		}
		else
		{
			return nullptr;
		}
	}

	template <AkComponent T>
	static void RemoveComponent(AkEntity entity)
	{
		AkAssert(IsEntityValid(entity), "Trying to use stale entity [id: {} - generation: {}]", entity.id, entity.generation);

		if (!IsEntityValid(entity))
			return;

		if (AkArchetype*& entityArchetype = m_EntityRecords[entity.id].archetype)
		{
			AkArchetypeHash newHash = entityArchetype->GetHash();
			newHash.typeHash.set(GetBitIndex<T>(), false);

			if (newHash.typeHash != 0)
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
				entityArchetype = nullptr;
			}
		}
	}

	template<AkEntityTag T>
	static void AddTag(AkEntity entity)
	{
		AkAssert(IsEntityValid(entity), "Trying to use stale entity [id: {} - generation: {}]", entity.id, entity.generation);

		if (!IsEntityValid(entity))
			return;

		AkArchetype*& oldArchetype = m_EntityRecords[entity.id];
		std::unordered_map<size_t, AkComponentDescriptor> componentDescriptors = {};
		
		AkArchetypeHash archetypeHash = {};
		archetypeHash.typeHash = GetArchetypeHash<T>();

		if (oldArchetype)
		{
			archetypeHash.parentId = oldArchetype->GetParentId();
			archetypeHash.typeHash |= oldArchetype->GetTypeHash();
			componentDescriptors = oldArchetype->GetComponentDescriptors();
		}

		if (!m_Archetypes.contains(archetypeHash))
		{
			std::unique_ptr<AkArchetype> archetype = std::make_unique<AkArchetype>(archetypeHash);
			archetype->Initialize(componentDescriptors);
			m_Archetypes[archetypeHash] = std::move(archetype);
		}

		std::unique_ptr<AkArchetype>& newArchetype = m_Archetypes[archetypeHash];

		if (oldArchetype)
			newArchetype->Migrate(entity, *oldArchetype);
		else
			newArchetype->Add(entity);

		oldArchetype = newArchetype.get();
	}

	template<AkEntityTag T>
	static void RemoveTag(AkEntity entity)
	{
		AkAssert(IsEntityValid(entity), "Trying to use stale entity [id: {} - generation: {}]", entity.id, entity.generation);

		if (!IsEntityValid(entity))
			return;

		if (AkArchetype*& entityArchetype = m_EntityRecords[entity.id])
		{
			AkArchetypeHash newHash = entityArchetype->GetHash();
			newHash.typeHash.set(GetBitIndex<T>(), false);

			if (newHash.typeHash != 0)
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
				entityArchetype = nullptr;
			}
		}
	}

	static void SetParent(AkEntity entity, AkEntity parent)
	{
		AkAssert(IsEntityValid(entity), "Trying to use stale entity [id: {} - generation: {}]", entity.id, entity.generation);
		AkAssert(parent == kNullEntity || IsEntityValid(parent), "Trying to use stale parent entity [id: {} - generation: {}]", parent.id, parent.generation);

		if (!IsEntityValid(entity) || (parent != kNullEntity && !IsEntityValid(parent)) || entity == parent)
			return;

		AkArchetypeHash archetypeHash = {};
		std::unordered_map<size_t, AkComponentDescriptor> componentDescriptors = {};

		AkArchetype*& oldArchetype = m_EntityRecords[entity.id];
		if (oldArchetype)
		{
			if (oldArchetype->GetParentId() == kNullEntityId && parent == kNullEntity)
				return;

			archetypeHash = oldArchetype->GetHash();
				
			if (parent == kNullEntity)
				archetypeHash.parentId = kNullEntityId;
			else
				archetypeHash.parentId = parent.id;

			componentDescriptors = oldArchetype->GetComponentDescriptors();
		}

		if (!m_Archetypes.contains(archetypeHash))
		{
			std::unique_ptr<AkArchetype> archetype = std::make_unique<AkArchetype>(archetypeHash);
			archetype->Initialize(componentDescriptors);
			m_Archetypes[archetypeHash] = std::move(archetype);
		}

		std::unique_ptr<AkArchetype>& newArchetype = m_Archetypes[archetypeHash];

		if (oldArchetype)
			newArchetype->Migrate(entity, *oldArchetype);
		else
			newArchetype->Add(entity);

		oldArchetype = newArchetype.get();
	}

	template <AkComponent ...T>
	static AkComponentsView<T...> GetView()
	{
		AkComponentTypeHash viewHash = {};
		(viewHash.set(GetBitIndex<T>(), true), ...);

		std::vector<AkArchetype*> compatibleArchetypes = {};
		compatibleArchetypes.reserve(sizeof...(T));

		for (auto& [hash, archetype] : m_Archetypes)
		{
			const AkComponentTypeHash testHash = hash.typeHash & viewHash;
			if (testHash == viewHash)
			{
				compatibleArchetypes.push_back(archetype.get());
			}
		}

		return AkComponentsView<T...>(compatibleArchetypes);
	}

	static size_t GetArchetypeHierarchyDepth(AkArchetype& archetype)
	{
		size_t depth = 0;
		uint32_t currentParent = archetype.GetParentId();

		while (currentParent != kNullEntityId)
		{
			++depth;
			currentParent = m_EntityRecords[currentParent]->GetParentId();
		}

		return depth;
	}

private:
	static inline std::vector<uint32_t> m_FreeIndices;
	static inline std::vector<uint32_t> m_Generations;

	static inline std::vector<AkArchetype*> m_EntityRecords;
	static inline std::unordered_map<AkArchetypeHash, std::unique_ptr<AkArchetype>> m_Archetypes;
};