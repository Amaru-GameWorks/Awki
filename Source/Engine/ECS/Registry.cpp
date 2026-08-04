#include "Registry.h"

AkEntity AkRegistry::CreateEntity()
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
	}

	return AkEntity{ .id = index, .generation = m_Generations[index] };
}

bool AkRegistry::IsEntityValid(AkEntity entity)
{
	if (entity.id < m_Generations.size())
		return m_Generations[entity.id] == entity.generation;
	return false;
}

void AkRegistry::DestroyEntity(AkEntity entity)
{
	if (!IsEntityValid(entity))
		return;

	++m_Generations[entity.id];
	m_FreeIndices.push(entity.id);

	for (auto& [typeId, pool] : m_ComponentPools)
		pool->Remove(entity);
}
