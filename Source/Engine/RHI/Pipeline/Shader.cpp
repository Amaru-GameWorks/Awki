#include "Shader.h"
#include "RHI/Device.h"

#include <vulkan/vulkan.hpp>

struct AkShaderStorage
{
	vk::ShaderModule shaderModule;
	vk::PipelineLayout pipelineLayout;
	vk::DescriptorSetLayout descriptorLayout;
};

AkShader::AkShader(const uint8_t* byteCode, size_t size)
{
	const vk::Device& device = AkDevice::GetDevice();

	vk::ShaderModuleCreateInfo moduleCreateInfo =
	{
		.codeSize = size,
		.pCode = reinterpret_cast<const uint32_t*>(byteCode),
	};

	m_Storage->shaderModule = device.createShaderModule(moduleCreateInfo);

	vk::DescriptorSetLayoutBinding descriptorLayoutBinding =
	{
		.descriptorType = vk::DescriptorType::eUniformBuffer,
		.descriptorCount = 1,
		.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
	};

	const vk::DescriptorSetLayoutCreateInfo descriptorLayoutCreateInfo =
	{
		.bindingCount = 1,
		.pBindings = &descriptorLayoutBinding
	};

	m_Storage->descriptorLayout = device.createDescriptorSetLayout(descriptorLayoutCreateInfo);

	const vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo =
	{
		.setLayoutCount = 1,
		.pSetLayouts = &m_Storage->descriptorLayout
	};
	m_Storage->pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
}

AkShader::~AkShader()
{
	const vk::Device& device = AkDevice::GetDevice();
	device.destroyShaderModule(m_Storage->shaderModule); 
	device.destroyPipelineLayout(m_Storage->pipelineLayout);
	device.destroyDescriptorSetLayout(m_Storage->descriptorLayout);
}

const vk::ShaderModule& AkShader::GetModule() const
{
	return m_Storage->shaderModule;
}

const vk::PipelineLayout& AkShader::GetPipelineLayout() const
{
	return m_Storage->pipelineLayout;
}

const vk::DescriptorSetLayout& AkShader::GetDescriptorSetLayout() const
{
	return m_Storage->descriptorLayout;
}