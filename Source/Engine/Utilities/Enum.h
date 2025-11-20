#pragma once

template<typename T>
constexpr T AkInclusiveBitOr(const T shift)
{
	T value = 0;
	for (T i = 0; i <= shift; ++i)
		value |= static_cast<T>(1) << i;
	return value;
}

template<typename T>
constexpr T AkExclusiveBitOr(const T shift)
{
	T value = 0;
	for (T i = 0; i < shift; ++i)
		value |= static_cast<T>(1) << i;
	return value;
}