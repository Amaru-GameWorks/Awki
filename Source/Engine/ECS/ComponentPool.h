#pragma once
#include "Entity.h"

#include <vector>

class AkComponentPoolBase
{
public:
	virtual bool Contains(AkEntity entity) const = 0;
	virtual void Remove(AkEntity entity) = 0;
	virtual size_t Count() const = 0;
	virtual const std::vector<AkEntity>& GetEntities() = 0;
};

template<typename T>
class AkComponentPool : public AkComponentPoolBase
{
public:
	T& Add(AkEntity entity)
	{
		if (entity.id >= m_Sparse.size())
		{
			size_t newSize = std::max(static_cast<size_t>(entity.id + 1), m_Sparse.size() * 3 / 2);
			m_Sparse.resize(newSize, kNullEntityId);
		}

		m_Sparse[entity.id] = static_cast<uint32_t>(m_Dense.size());
		m_Dense.push_back(entity);
		return m_Components.emplace_back();
	}

	T& Get(AkEntity entity)
	{
		return m_Components[m_Sparse[entity.id]];
	}

	bool Contains(AkEntity entity) const override
	{
		if (entity.id >= m_Sparse.size())
			return false;

		return m_Sparse[entity.id] != kNullEntityId;
	}

	void Remove(AkEntity entity) override
	{
		if (entity.id >= m_Sparse.size() || m_Sparse[entity.id] == kNullEntityId)
			return;

		const uint32_t index = m_Sparse[entity.id];
		const AkEntity lastEntity = m_Dense.back();

		m_Components[index] = std::move(m_Components.back());
		m_Dense[index] = lastEntity;
		m_Sparse[lastEntity.id] = index;

		m_Sparse[entity.id] = kNullEntityId;
		m_Components.pop_back();
		m_Dense.pop_back();
	}

	size_t Count() const override
	{
		return m_Components.size();
	}

	const std::vector<AkEntity>& GetEntities() override
	{
		return m_Dense;
	}

	const std::vector<T>& GetComponents() const
	{
		return m_Components;
	}

private:
	std::vector<T> m_Components;
	std::vector<AkEntity> m_Dense;
	std::vector<uint32_t> m_Sparse;
};