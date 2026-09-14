#pragma once
#include "Utilities/ForwardStorage.h"

namespace vk
{
	class Sampler;
}

enum class AkFilterMode
{
	NEAREST,
	LINEAR
};

enum class AkWrapMode
{
	REPEAT,
	MIRRORED_REPEAT,
	CLAMP_TO_EDGE,
	CLAMP_TO_BORDER,
	MIRROR_CLAMP_TO_EDGE
};

enum class AkBorderColor
{
	FLOAT_TRANSPARENT_BLACK,
	FLOAT_OPAQUE_BLACK,
	FLOAT_OPAQUE_WHITE,

	INT_TRANSPARENT_BLACK,
	INT_OPAQUE_BLACK,
	INT_OPAQUE_WHITE,
};

enum class AkCompareOperation
{
	OFF,
	NEVER,
	LESS,
	EQUAL,
	LESS_OR_EQUAL,
	GREATER,
	NOT_EQUAL,
	GREATER_OR_EQUAL,
	ALWAYS
};

struct AkSamplerDescriptor
{
	AkFilterMode filterMode = AkFilterMode::LINEAR;
	AkFilterMode mipsFilterMode = AkFilterMode::LINEAR;

	AkWrapMode wrapModeU = AkWrapMode::REPEAT;
	AkWrapMode wrapModeV = AkWrapMode::REPEAT;
	AkWrapMode wrapModeW = AkWrapMode::REPEAT;

	AkCompareOperation compareOperation = AkCompareOperation::OFF;
	AkBorderColor borderColor = AkBorderColor::FLOAT_OPAQUE_WHITE;

	float anisotropy = 16.f;
	float maxLod = 1000.0f;
	float minLod = 0.f;
	float lodBias = 0.f;

	void SetWrapMode(const AkWrapMode& wrapMode)
	{
		wrapModeU = wrapMode;
		wrapModeV = wrapMode;
		wrapModeW = wrapMode;
	}
};

class AkSampler
{
	friend class AkBindlessResourcesManager;

public:
	AkSampler(const AkSamplerDescriptor& descriptor);
	~AkSampler();

	int32_t GetBindlessIndex() const { return m_BindlessIndex; }
	const AkSamplerDescriptor& GetDescriptor() const { return m_Descriptor; }

	const vk::Sampler& GetSampler() const;

private:
	int32_t m_BindlessIndex = -1;
	AkSamplerDescriptor m_Descriptor;
	AkForwardStorage<struct AkSamplerStorage, 8> m_Storage;
};