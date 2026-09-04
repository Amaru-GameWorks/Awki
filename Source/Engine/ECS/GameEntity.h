#pragma once
#include "Registry.h"

class AkGameEntity
{
public:
	AkGameEntity() = default;
	AkGameEntity(const AkEntity entity)
		: m_Entity(entity)
	{ }

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

private:
	AkEntity m_Entity = {};
};