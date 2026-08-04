#pragma once
#include "Utilities/Hash.h"

#include <type_traits>

template <typename T>
struct AkComponentTypeInfo
{
	using kRequires = std::tuple<>;
	static constexpr bool kIsRegistered = false;
};

struct AkComponentCounter
{
	template <typename T>
	friend struct AkComponentTypeInfo;

private:
	static inline size_t sCount = 0;
};

template <typename T>
concept AkComponent = AkComponentTypeInfo<T>::kIsRegistered;

#define REGISTER_COMPONENT(component, ...)	class component; template<> struct AkComponentTypeInfo<component>																			\
											{																																			\
												using kRequires = std::tuple<__VA_ARGS__>;																								\
												static constexpr bool kIsRegistered = true;																								\
												static constexpr std::string_view Name() { return #component; }																			\
												static constexpr size_t TypeId() { return FNV1aHash(#component); }																		\
												static size_t GetBitIndex() { static size_t sIndex = AkComponentCounter::sCount++; return sIndex; }										\
											};

namespace AkTypeTraits
{
	template <typename T, typename Tuple>
	struct AkContains;

	template <typename T, typename... Types>
	struct AkContains<T, std::tuple<Types...>> : std::disjunction<std::is_same<T, Types>...> { };

	template <typename TargetTuple, typename RemainingTuple>
	struct AkDependencySolver;

	template <typename TargetTuple>
	struct AkDependencySolver<TargetTuple, std::tuple<>>
	{ using type = TargetTuple; };

	template <typename TargetTuple, typename Head, typename... Tail>
	struct AkDependencySolver<TargetTuple, std::tuple<Head, Tail...>>
	{
		using HeadDeps = typename AkComponentTypeInfo<Head>::kRequires;
		using TargetWithHeadDeps = typename AkDependencySolver<TargetTuple, HeadDeps>::type;
		using NextTarget = std::conditional_t<
			AkContains<Head, TargetWithHeadDeps>::value,
			TargetWithHeadDeps,
			decltype(std::tuple_cat(std::declval<TargetWithHeadDeps>(), std::declval<std::tuple<Head>>()))
		>;

		using type = typename AkDependencySolver<NextTarget, std::tuple<Tail...>>::type;
	};
}

template <typename T>
using AkComponentWithDependencies = typename AkTypeTraits::AkDependencySolver<std::tuple<>, std::tuple<T>>::type;