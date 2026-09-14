#pragma once
#include "Utilities/AkId.h"

class AkPipelineResourceId
{
public:
	constexpr AkPipelineResourceId() = default;
	constexpr AkPipelineResourceId(const char* resourceId, void* renderPipeline)
	{
#if DEBUG
		m_Name = resourceId;
#endif

		m_Hash = FNV1aHash(resourceId);
		HashCombine(m_Hash, renderPipeline);
	}

	constexpr operator bool() const
	{
		return m_Hash != 0;
	}

	constexpr operator size_t() const
	{
		return m_Hash;
	}

	const std::string& GetName() const
	{
#if DEBUG
		return m_Name;
#else
		return std::to_string(m_Hash);
#endif
	}

	constexpr auto operator<=>(const AkPipelineResourceId&) const = default;

private:
	size_t m_Hash = 0;

#if DEBUG
	std::string m_Name = {};
#endif
};

namespace std
{
	template <>
	struct hash<AkPipelineResourceId>
	{
		size_t operator()(const AkPipelineResourceId& id) const
		{
			return id;
		}
	};
}