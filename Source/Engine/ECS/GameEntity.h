#pragma once
#include "Registry.h"

class AkGameEntity
{
public:
	AkGameEntity() = default;
	AkGameEntity(const AkEntity entity)
		: m_Entity(entity)
	{ }

	template <typename T>
	requires(AkIsComponent<T>::value)
	T* AddComponent()
	{
		return AkRegistry::AddComponent<T>(m_Entity);
	}

	template <typename T>
	requires(AkIsComponent<T>::value)
	T* GetComponent()
	{
		return AkRegistry::GetComponent<T>(m_Entity);
	}

	template <typename T>
	requires(AkIsComponent<T>::value)
	void RemoveComponent()
	{
		AkRegistry::RemoveComponent<T>();
	}

private:
	AkEntity m_Entity = {};
};