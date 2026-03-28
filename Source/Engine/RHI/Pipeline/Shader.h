#pragma once
#include "Utilities/ForwardStorage.h"

namespace vk
{
	class ShaderModule;
	class PipelineLayout;
	class DescriptorSetLayout;
}

class AkShader
{
public:
	AkShader(const class AkShaderByteCode& byteCode);
	~AkShader();

	const vk::ShaderModule& GetModule() const;
	const vk::PipelineLayout& GetPipelineLayout() const;
	const vk::DescriptorSetLayout& GetDescriptorSetLayout() const;

private:
	ForwardStorage<struct AkShaderStorage, 24> m_Storage;
};