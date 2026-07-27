#pragma once

#include <limits>
#include <cstdint>
#include <compare>

inline constexpr uint32_t kNullEntityId = { std::numeric_limits<uint32_t>::max() };

struct AkEntity
{
	uint32_t id = kNullEntityId;
	uint32_t generation = 0;
	auto operator<=>(const AkEntity&) const = default;
};

namespace std 
{
	template <>
	struct hash<AkEntity> 
	{
		size_t operator()(AkEntity entity) const noexcept 
		{
			uint64_t* data = reinterpret_cast<uint64_t*>(&entity);
			return std::hash<uint64_t>{}(*data);
		}
	};
}