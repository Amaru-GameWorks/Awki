#pragma once
#include <limits>
#include <cstdint>

struct AkEntity
{
	uint32_t id;
	uint32_t generation;
};

inline constexpr uint32_t kNullEntityId = { std::numeric_limits<uint32_t>::max() };