#include "Sampler.h"
#include "Core/Log.h"
#include "RHI/Device.h"
#include "RHI/Pipeline/BindlessResourcesManager.h"

#include <vulkan/vulkan.hpp>

struct AkSamplerStorage
{
	vk::Sampler sampler = VK_NULL_HANDLE;
};

vk::SamplerAddressMode GetWrapMode(const AkWrapMode& wrapMode)
{
	switch (wrapMode)
	{
		case AkWrapMode::CLAMP_TO_BORDER:		return vk::SamplerAddressMode::eClampToBorder;
		case AkWrapMode::CLAMP_TO_EDGE:			return vk::SamplerAddressMode::eClampToEdge;
		case AkWrapMode::MIRRORED_REPEAT:		return vk::SamplerAddressMode::eMirroredRepeat;
		case AkWrapMode::MIRROR_CLAMP_TO_EDGE:	return vk::SamplerAddressMode::eMirrorClampToEdge;
		case AkWrapMode::REPEAT:				return vk::SamplerAddressMode::eRepeat;

		default:
			AkLogCritical("Wrap mode not registered in this function");
			return vk::SamplerAddressMode::eClampToEdge;
	}
}

vk::Filter GetFilterMode(const AkFilterMode& filterMode)
{
	switch (filterMode)
	{
		case AkFilterMode::LINEAR:	return vk::Filter::eLinear;
		case AkFilterMode::NEAREST:	return vk::Filter::eNearest;

		default:
			AkLogCritical("Filter mode not registered in this function");
			return vk::Filter::eLinear;
	}
}

vk::SamplerMipmapMode GetMipMapFilterMode(const AkFilterMode& filterMode)
{
	switch (filterMode)
	{
		case AkFilterMode::LINEAR:	return vk::SamplerMipmapMode::eLinear;
		case AkFilterMode::NEAREST:	return vk::SamplerMipmapMode::eNearest;

		default:
			AkLogCritical("Filter mode not registered on this function");
			return vk::SamplerMipmapMode::eLinear;
	}
}

vk::CompareOp GetCompareOperation(const AkCompareOperation& compareOperation)
{
	switch (compareOperation)
	{
		case AkCompareOperation::OFF:
		case AkCompareOperation::ALWAYS:			return vk::CompareOp::eAlways;
		case AkCompareOperation::EQUAL:				return vk::CompareOp::eEqual;
		case AkCompareOperation::GREATER:			return vk::CompareOp::eGreater;
		case AkCompareOperation::GREATER_OR_EQUAL:	return vk::CompareOp::eGreaterOrEqual;
		case AkCompareOperation::LESS:				return vk::CompareOp::eLess;
		case AkCompareOperation::LESS_OR_EQUAL:		return vk::CompareOp::eLessOrEqual;
		case AkCompareOperation::NEVER:				return vk::CompareOp::eNever;
		case AkCompareOperation::NOT_EQUAL:			return vk::CompareOp::eNotEqual;

		default:
			AkLogCritical("Compare operation not registered on this function");
			return vk::CompareOp::eAlways;
	}
}

vk::BorderColor GetBorderColor(const AkBorderColor& borderColor)
{
	switch (borderColor)
	{
		case AkBorderColor::FLOAT_OPAQUE_BLACK:			return vk::BorderColor::eFloatOpaqueBlack;
		case AkBorderColor::FLOAT_OPAQUE_WHITE:			return vk::BorderColor::eFloatOpaqueWhite;
		case AkBorderColor::FLOAT_TRANSPARENT_BLACK:	return vk::BorderColor::eFloatTransparentBlack;
		case AkBorderColor::INT_OPAQUE_BLACK:			return vk::BorderColor::eIntOpaqueBlack;
		case AkBorderColor::INT_OPAQUE_WHITE:			return vk::BorderColor::eIntOpaqueWhite;
		case AkBorderColor::INT_TRANSPARENT_BLACK:		return vk::BorderColor::eIntTransparentBlack;

		default:
			AkLogCritical("Border color not registered on this function");
			return vk::BorderColor::eFloatOpaqueBlack;
	}
}

AkSampler::AkSampler(const AkSamplerDescriptor& descriptor)
	: m_Descriptor(descriptor)
{
	const vk::SamplerCreateInfo samplerCreateInfo =
	{
		.magFilter = GetFilterMode(m_Descriptor.filterMode),
		.minFilter = GetFilterMode(m_Descriptor.filterMode),
		.mipmapMode = GetMipMapFilterMode(m_Descriptor.mipsFilterMode),
		.addressModeU = GetWrapMode(m_Descriptor.wrapModeU),
		.addressModeV = GetWrapMode(m_Descriptor.wrapModeV),
		.addressModeW = GetWrapMode(m_Descriptor.wrapModeW),
		.mipLodBias = m_Descriptor.lodBias,
		.anisotropyEnable = m_Descriptor.anisotropy > 0.f,
		.maxAnisotropy = m_Descriptor.anisotropy,
		.compareEnable = m_Descriptor.compareOperation != AkCompareOperation::OFF,
		.compareOp = GetCompareOperation(m_Descriptor.compareOperation),
		.minLod = m_Descriptor.minLod,
		.maxLod = m_Descriptor.maxLod,
		.borderColor = GetBorderColor(m_Descriptor.borderColor),
	};

	const vk::Device& device = AkDevice::GetDevice();
	m_Storage->sampler = device.createSampler(samplerCreateInfo);

	AkBindlessResourcesManager::AddSampler(this);
}

AkSampler::~AkSampler()
{
	AkBindlessResourcesManager::RemoveSampler(this);

	const vk::Device& device = AkDevice::GetDevice();
	device.destroySampler(m_Storage->sampler);
}

const vk::Sampler& AkSampler::GetSampler() const
{
	return m_Storage->sampler;
}
