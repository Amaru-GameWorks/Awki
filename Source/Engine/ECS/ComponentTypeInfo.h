#pragma once
#include "ComponentPool.h"
#include "Utilities/Hash.h"

#include <memory>
#include <functional>
#include <unordered_map>

class AkComponentPoolInitializers
{
public:
	static const std::unordered_map<size_t, std::function<std::unique_ptr<AkComponentPoolBase>()>>& Get()
	{
		return sPoolInitializers;
	}

	static void Set(size_t typeId, const std::function<std::unique_ptr<AkComponentPoolBase>()>& initializer)
	{
		sPoolInitializers[typeId] = initializer;
	}

private:
	static inline std::unordered_map<size_t, std::function<std::unique_ptr<AkComponentPoolBase>()>> sPoolInitializers;
};

template<typename T>
struct AkComponentTypeInfo
{
	static const constexpr char* Name()
	{
		return "";
	}

	static constexpr size_t TypeId()
	{
		return 0;
	}
};

template<typename T>
struct AkIsComponent : std::false_type {};

#define REGISTER_COMPONENT(component)	class component; template<> struct AkComponentTypeInfo<component>																			\
										{																																			\
											AkComponentTypeInfo() { AkComponentPoolInitializers::Set(TypeId(), []() { return std::make_unique<AkComponentPool<component>>(); }); }	\
											static constexpr const char* Name() { return #component; }																				\
											static constexpr size_t TypeId() { return FNV1aHash(#component); }																		\
										};																																			\
										static inline AkComponentTypeInfo<component> s##component##StaticInitializer;																\
										template<> struct AkIsComponent<component> : std::true_type { };