#include "Material.h"

#include "RHI/Device.h"
#include "RHI/Buffers/Buffer.h"
#include "RHI/Pipeline/Shader.h"
#include "Utilities/Hash.h"

#include <vulkan/vulkan.hpp>

struct AkMaterialStorage
{
	vk::DescriptorPool pool;
	vk::DescriptorSet descriptorSet;
};

AkMaterial::AkMaterial(class AkShader* shader, const std::optional<AkRasterizerState>& rasterizerState)
	: m_Shader(shader)
{
	if (rasterizerState.has_value())
		m_RasterizerState = rasterizerState.value();
	m_RasterizerState.CalculateHash();

	const vk::Device& device = AkDevice::GetDevice();

	const vk::DescriptorPoolSize poolSize = { .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1 };
	const vk::DescriptorPoolCreateInfo descPoolCreateInfo = { .maxSets = 1, .poolSizeCount = 1, .pPoolSizes = &poolSize };
	m_Storage->pool = device.createDescriptorPool(descPoolCreateInfo);

	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo =
	{
		.descriptorPool = m_Storage->pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &m_Shader->GetDescriptorSetLayout()
	};

	std::vector<vk::DescriptorSet> allocatedSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);
	if(!allocatedSets.empty())
		m_Storage->descriptorSet = allocatedSets[0];

	m_Hash = Hash(m_RasterizerState);
	HashCombine(m_Hash, m_Shader);
}

AkMaterial::~AkMaterial()
{
	const vk::Device& device = AkDevice::GetDevice();
	device.destroyDescriptorPool(m_Storage->pool);
}

void AkMaterial::SetConstantBuffer(AkConstantBuffer* buffer, const uint32_t binding)
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
		.dstSet = m_Storage->descriptorSet,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eUniformBuffer,
		.pBufferInfo = &bufferUpdateInfo
	};

	device.updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
}

const vk::DescriptorSet& AkMaterial::GetDescriptorSet() const
{
	return m_Storage->descriptorSet;
}