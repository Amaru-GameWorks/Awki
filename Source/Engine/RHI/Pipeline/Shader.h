#pragma once
#include "Utilities/ForwardStorage.h"

#include <vector>
#include <unordered_map>

namespace vk
{
	class ShaderModule;
	class PipelineLayout;
	class DescriptorSetLayout;
}

struct AkConstantBufferReflection
{
	size_t size = 0;
};

struct AkSetReflection
{
	std::unordered_map<uint32_t, AkConstantBufferReflection> constantBuffers = {};
};

struct AkShaderReflection
{
	uint32_t pushConstantSize = 0;
	uint32_t materialDataSize = 0;
	uint32_t constantBuffersCount = 0;
	std::unordered_map<uint32_t, AkSetReflection> descriptorSets = {};
};

class AkShader
{
public:
	AkShader(const class AkShaderData& byteCode);
	~AkShader();

	const AkShaderReflection& GetReflection() const { return m_Reflection; }

	const vk::ShaderModule& GetModule() const;
	const vk::PipelineLayout& GetPipelineLayout() const;
	const std::vector<vk::DescriptorSetLayout>& GetDescriptorSetLayouts() const;

private:
	AkShaderReflection m_Reflection;
	AkForwardStorage<struct AkShaderStorage, 48> m_Storage;
};