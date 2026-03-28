#pragma once
#include <string>

template <class T>
size_t Hash(const T& value)
{
	std::hash<T> hasher;
	return hasher(value);
}

template <class T>
void HashCombine(size_t& hash, const T& value)
{
	hash ^= Hash(value) + 0x9e3779b9ULL + (hash << 6) + (hash >> 2);
}

constexpr size_t FNV1aHash(const char* string)
{
	const size_t FNVoffsetBasis = 14695981039346656037ULL;
	const size_t FNVprime = 1099511628211ULL;

	size_t val = FNVoffsetBasis;
	for (size_t next = 0; next < std::char_traits<char>::length(string); ++next)
	{
		val ^= static_cast<size_t>(string[next]);
		val *= FNVprime;
	}

	return val;
}