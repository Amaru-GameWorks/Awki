#pragma once
#include <functional>

#if _MSC_VER
#define __Placeholder std::_Ph
#elif __GNUC__
#define __Placeholder std::_Placeholder
#endif

template<typename OwnerClass, typename... Types, int... Index>
auto AutomaticPlaceholderExpansionBind(void* owner, void(OwnerClass::* callback)(Types... Args), std::integer_sequence<int, Index...>)
{
	return std::bind(callback, static_cast<OwnerClass*>(owner), __Placeholder<1 + Index>{}...);
}

template<typename OwnerClass, typename... Types>
auto AutomaticPlaceholderExpansionBind(void* owner, void(OwnerClass::* callback)(Types... Args))
{
	return AutomaticPlaceholderExpansionBind(owner, callback, std::make_integer_sequence<int, sizeof...(Types)>{});
}