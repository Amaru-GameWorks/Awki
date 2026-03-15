#pragma once
#include <cstdint>

template<typename T>
constexpr inline T RoundToNextMultiple(const T value, const T multiple)
{
	if (value == 0) return 0;
	return ((value + multiple - 1) / multiple) * multiple;
}

constexpr inline uint8_t NextPowerOf2(uint8_t value)
{
	--value;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	return ++value;
}

constexpr inline uint16_t NextPowerOf2(uint16_t value)
{
	--value;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	return ++value;
}

constexpr inline uint32_t NextPowerOf2(uint32_t value)
{
	--value;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	return ++value;
}

constexpr inline uint64_t NextPowerOf2(uint64_t value)
{
	--value;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value |= value >> 32;
	return ++value;
}