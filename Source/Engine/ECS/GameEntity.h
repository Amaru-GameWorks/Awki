#pragma once
#include "Registry.h"

class AkGameEntity
{
public:
	AkGameEntity() = default;
	AkGameEntity(const AkEntity entity)
		: m_Entity(entity)
	{ }

	operator AkEntity() const { return m_Entity; }

	template <AkComponent T>
	T* AddComponent()
	{
		return AkRegistry::AddComponent<T>(m_Entity);
	}

	template <AkComponent T>
	T* GetComponent()
	{
		return AkRegistry::GetComponent<T>(m_Entity);
	}

	template <AkComponent T>
	void RemoveComponent()
	{
		AkRegistry::RemoveComponent<T>(m_Entity);
	}

	template <AkEntityTag T>
	void AddTag()
	{
		AkRegistry::AddTag<T>(m_Entity);
	}

	template <AkEntityTag T>
	void RemoveTag()
	{
		AkRegistry::RemoveTag<T>(m_Entity);
	}

private:
	AkEntity m_Entity = {};
};