#include "ComponentsViewSorting.h"
#include "Registry.h"

bool AkComponentsHierarchySorting::Sort(AkArchetype* lhs, AkArchetype* rhs)
{
	return AkRegistry::GetArchetypeHierarchyDepth(*lhs) < AkRegistry::GetArchetypeHierarchyDepth(*rhs);
}
