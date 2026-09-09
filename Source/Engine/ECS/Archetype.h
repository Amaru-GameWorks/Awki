#pragma once
#include "Entity.h"
#include "ECS/ComponentTypeInfo.h"
#include "Memory/ExponentialPageAllocator.h"

#include <vector>
#include <unordered_map>

struct AkComponentDescriptor
{
	size_t size = 0;
	size_t alignment = 1;
	std::function<void(uint8_t* componentAddress)> initializeComponent = nullptr;
};

struct AkArchetypeHash
{
	uint32_t parentId = kNullEntityId;
	AkComponentTypeHash typeHash = {};

	friend bool operator== (const AkArchetypeHash& rhs, const AkArchetypeHash& lhs)
	{
		return rhs.parentId == lhs.parentId && rhs.typeHash == lhs.typeHash;
	}
};

namespace std
{
	template <>
	struct hash<AkArchetypeHash>
	{
		size_t operator()(AkArchetypeHash archetype) const noexcept
		{
			size_t hash = Hash(archetype.parentId);
			HashCombine(hash, archetype.typeHash);
			return hash;
		}
	};
}


class AkArchetype
{
public:
	static inline constexpr size_t kMaxComponentPerPage = 256;

	AkArchetype(const AkArchetypeHash& hash)
		: m_Hash(hash)
	{ }

	template <typename... Ts>
	void Initialize(const std::tuple<Ts...>& tuple) 
	{
		m_Descriptors.clear();
		std::apply([this](auto... type)
		{
			(this->AddComponentDescriptor<decltype(type)>(), ...);
		}, tuple);

		InitializeInternal();
	}

	void Initialize(const std::unordered_map<size_t, AkComponentDescriptor>& descriptors)
	{
		m_Descriptors = descriptors;
		InitializeInternal();
	}

	void Add(AkEntity entity)
	{
		if (entity.id >= m_Sparse.size())
		{
			size_t newSize = std::max(static_cast<size_t>(entity.id + 1), m_Sparse.size() * 3 / 2);
			m_Sparse.resize(newSize, kNullEntityId);
		}

		m_Sparse[entity.id] = static_cast<uint32_t>(m_Dense.size());
		m_Dense.push_back(entity);

		EnsureComponentsSize();
		InitializeComponents(entity);
	}

	template<typename T>
	T& GetComponent(AkEntity entity)
	{
		AkAssert(m_ComponentAllocators.contains(AkComponentTypeInfo<T>::TypeId()), "Trying to get a component from an archetype which does not contain it");
		
		const uint32_t index = m_Sparse[entity.id];
		const size_t typeId = AkComponentTypeInfo<T>::TypeId();
		return m_ComponentAllocators[typeId].Get<T>(index);
	}

	bool Contains(AkEntity entity) const
	{
		if (entity.id >= m_Sparse.size())
			return false;

		return m_Sparse[entity.id] != kNullEntityId;
	}

	void Remove(AkEntity entity)
	{
		AkAssert(entity.id < m_Sparse.size(), "Invalid entity");

		if (entity.id >= m_Sparse.size() || m_Sparse[entity.id] == kNullEntityId)
			return;

		const uint32_t index = m_Sparse[entity.id];
		const AkEntity lastEntity = m_Dense.back();
		const uint32_t lastIndex = m_Sparse[lastEntity.id];

		for (auto& [id, allocator] : m_ComponentAllocators)
			allocator.Override(index, lastIndex);

		m_Dense[index] = lastEntity;
		m_Sparse[lastEntity.id] = index;

		m_Sparse[entity.id] = kNullEntityId;
		m_Dense.pop_back();
	}

	uint32_t GetEntityIndex(AkEntity entity)
	{
		AkAssert(entity.id < m_Sparse.size(), "Invalid entity");
		return m_Sparse[entity.id];
	}

	void Migrate(AkEntity entity, AkArchetype& sourceArchetype)
	{
		Add(entity);

		const uint32_t destinationIndex = m_Sparse[entity.id];
		const uint32_t sourceIndex = sourceArchetype.GetEntityIndex(entity);

		for (auto& [typeId, allocator] : m_ComponentAllocators)
		{
			if (sourceArchetype.m_ComponentAllocators.contains(typeId))
			{
				AkExponentialPageAllocator& sourceAllocator = sourceArchetype.m_ComponentAllocators[typeId];
				uint8_t* destination = allocator.Get(destinationIndex);
				uint8_t* source = sourceAllocator.Get(sourceIndex);
				std::memcpy(destination, source, m_Descriptors[typeId].size);
			}
		}

		sourceArchetype.Remove(entity);
	}

	size_t Count() const
	{
		return m_Dense.size();
	}

	const AkArchetypeHash& GetHash() const
	{
		return m_Hash;
	}

	const uint32_t& GetParentId() const
	{
		return m_Hash.parentId;
	}

	const AkComponentTypeHash& GetTypeHash() const
	{
		return m_Hash.typeHash;
	}

	const std::vector<AkEntity>& GetEntities()
	{
		return m_Dense;
	}

	const std::unordered_map<size_t, AkComponentDescriptor>& GetComponentDescriptors() const
	{
		return m_Descriptors;
	}

	template<typename ...T>
	std::vector<AkExponentialPageAllocator*> GetComponentAllocators()
	{
		std::vector<AkExponentialPageAllocator*> allocators = {};
		allocators.reserve(sizeof...(T));

		([&]{
			auto it = m_ComponentAllocators.find(AkComponentTypeInfo<T>::TypeId());
			if (it != m_ComponentAllocators.end())
				allocators.push_back(&it->second);
		}(), ...);

		return allocators;
	}

private:
	AkArchetypeHash m_Hash;
	std::vector<AkEntity> m_Dense;
	std::vector<uint32_t> m_Sparse;
	std::unordered_map<size_t, AkComponentDescriptor> m_Descriptors;
	std::unordered_map<size_t, AkExponentialPageAllocator> m_ComponentAllocators;

	void InitializeInternal()
	{
		for (auto& [typeId, descriptor] : m_Descriptors)
			m_ComponentAllocators[typeId] = AkExponentialPageAllocator(descriptor.size, 8ull, kMaxComponentPerPage, descriptor.alignment);
	}

	template<typename T>
	void AddComponentDescriptor()
	{
		AkComponentDescriptor& descriptor = m_Descriptors[AkComponentTypeInfo<T>::TypeId()];
		descriptor.size = sizeof(T);
		descriptor.alignment = alignof(T);
		descriptor.initializeComponent = [](uint8_t* address) 
		{
			new (address) T();
		};
	}

	void EnsureComponentsSize()
	{
		const size_t componentsCount = m_Dense.size();
		auto iterator = m_ComponentAllocators.begin();

		if (iterator != m_ComponentAllocators.end() && !iterator->second.SpaceAvailable(componentsCount))
		{
			for (auto& [id, allocator] : m_ComponentAllocators)
				allocator.AllocateNewPage();
		}
	}

	void InitializeComponents(AkEntity entity)
	{
		const uint32_t index = m_Sparse[entity.id];
		for (auto& [id, allocator] : m_ComponentAllocators)
		{
			AkComponentDescriptor& descriptor = m_Descriptors[id];
			descriptor.initializeComponent(allocator.Get(index));
		}
	}
};