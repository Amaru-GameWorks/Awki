#pragma once
#include "Hash.h"

class AkId
{
public:
	constexpr AkId() = default;
	constexpr AkId(size_t hash)
	{
		m_Hash = hash;
	}

	constexpr operator bool() const
	{
		return m_Hash != 0;
	}

	constexpr operator size_t() const
	{
		return m_Hash;
	}

	constexpr auto operator<=>(const AkId&) const = default;

private:
	size_t m_Hash = 0;
};

constexpr AkId operator""_ID(const char* string, size_t)
{
	return { FNV1aHash(string) };
}

namespace std
{
	template <>
	struct hash<AkId>
	{
		size_t operator()(const AkId& id) const
		{
			return id;
		}
	};
}