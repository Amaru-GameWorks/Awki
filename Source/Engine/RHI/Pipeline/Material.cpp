#include "Material.h"

#include "RHI/Device.h"
#include "RHI/Buffers/Buffer.h"
#include "RHI/Pipeline/Shader.h"
#include "Utilities/Hash.h"

#include <vulkan/vulkan.hpp>

struct AkMaterialStorage
{
	vk::DescriptorPool pool;
	std::vector<vk::DescriptorSet> descriptorSets;
};

AkMaterial::AkMaterial(class AkShader* shader, const std::optional<AkRasterizerState>& rasterizerState)
	: m_Shader(shader)
{
	if (rasterizerState.has_value())
		m_RasterizerState = rasterizerState.value();
	m_RasterizerState.CalculateHash();

	const vk::Device& device = AkDevice::GetDevice();
	const AkShaderReflection& reflection = m_Shader->GetReflection();
	const std::vector<vk::DescriptorSetLayout>& descriptorLayouts = m_Shader->GetDescriptorSetLayouts();

	const vk::DescriptorPoolSize poolSize = { .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = reflection.constantBuffersCount };
	const vk::DescriptorPoolCreateInfo descPoolCreateInfo = { .maxSets = static_cast<uint32_t>(reflection.descriptorSets.size()), .poolSizeCount = 1, .pPoolSizes = &poolSize};
	m_Storage->pool = device.createDescriptorPool(descPoolCreateInfo);

	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo =
	{
		.descriptorPool = m_Storage->pool,
		.descriptorSetCount = descPoolCreateInfo.maxSets,
		.pSetLayouts = descriptorLayouts.data()
	};

	m_Storage->descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);
	m_Hash = Hash(m_RasterizerState);
	HashCombine(m_Hash, m_Shader);
}

AkMaterial::~AkMaterial()
{
	const vk::Device& device = AkDevice::GetDevice();
	device.destroyDescriptorPool(m_Storage->pool);
}

void AkMaterial::SetConstantBuffer(AkConstantBuffer* buffer, const uint32_t binding, const uint32_t set)
{
	const vk::Device& device = AkDevice::GetDevice();

	const vk::DescriptorBufferInfo bufferUpdateInfo =
	{
		.buffer = buffer->GetBuffer(),
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};

	const vk::WriteDescriptorSet writeDescriptorSet =
	{
		.dstSet = m_Storage->descriptorSets[set],
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eUniformBuffer,
		.pBufferInfo = &bufferUpdateInfo
	};

	device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
}

const std::vector<vk::DescriptorSet>& AkMaterial::GetDescriptorSets() const
{
	return m_Storage->descriptorSets;
}