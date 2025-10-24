#pragma once
#include <string>
#include <cstdint>

#include <fmt/format.h>

class AkVersion
{
public:
	AkVersion() = default;
	AkVersion(uint8_t major, uint8_t minor, uint8_t patch)
		: m_Major(major)
		, m_Minor(minor)
		, m_Patch(patch)
	{ }

	uint8_t GetMajor() const { return m_Major; }
	uint8_t GetMinor() const { return m_Minor; }
	uint8_t GetPatch() const { return m_Patch; }

private:
	uint8_t m_Major;
	uint8_t m_Minor;
	uint8_t m_Patch;
};

template <>
class fmt::formatter<AkVersion> 
{
public:
	constexpr auto parse(format_parse_context& ctx) { return ctx.end(); }
	
	template <typename FmtContext>
	constexpr auto format(const AkVersion& version, FmtContext& ctx) const
	{
		return format_to(ctx.out(), "v{}.{}.{}", version.GetMajor(), version.GetMinor(), version.GetPatch());
	}
};