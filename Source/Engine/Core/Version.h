#pragma once
#include <format>

class AkVersion
{
public:
	constexpr AkVersion() = default;
	constexpr AkVersion(uint8_t major, uint8_t minor, uint8_t patch)
		: m_Major(major)
		, m_Minor(minor)
		, m_Patch(patch)
	{ }

	uint8_t GetMajor() const { return m_Major; }
	uint8_t GetMinor() const { return m_Minor; }
	uint8_t GetPatch() const { return m_Patch; }

private:
	uint8_t m_Major = 0;
	uint8_t m_Minor = 0;
	uint8_t m_Patch = 0;
};

template <>
struct std::formatter<AkVersion> : std::formatter<std::string> 
{
	auto format(AkVersion version, std::format_context& ctx) const
	{
		return std::formatter<string>::format(std::format("v{}.{}.{}", version.GetMajor(), version.GetMinor(), version.GetPatch()), ctx);
	}
};