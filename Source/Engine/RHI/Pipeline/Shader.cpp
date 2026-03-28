#include "Shader.h"
#include "RHI/Device.h"
#include "Utilities/Shaders/ShaderCompiler.h"
#include "RHI/Pipeline/BindlessResourcesManager.h"

#include <vector>
#include <vulkan/vulkan.hpp>

struct AkShaderStorage
{
	vk::ShaderModule shaderModule;
	vk::PipelineLayout pipelineLayout;
	vk::DescriptorSetLayout descriptorLayout;
};

AkShader::AkShader(const AkShaderByteCode& byteCode)
{
	const vk::Device& device = AkDevice::GetDevice();

	vk::ShaderModuleCreateInfo moduleCreateInfo =
	{
		.codeSize = byteCode.GetSize(),
		.pCode = reinterpret_cast<const uint32_t*>(byteCode.GetByteCode()),
	};
	m_Storage->shaderModule = device.createShaderModule(moduleCreateInfo);

	const vk::DescriptorSetLayoutBinding descriptorLayoutBinding =
	{
		.descriptorType = vk::DescriptorType::eUniformBuffer,
		.descriptorCount = 1,
		.stageFlags = vk::ShaderStageFlagBits::eAll
	};
	const vk::DescriptorSetLayoutCreateInfo descriptorLayoutCreateInfo =
	{
		.bindingCount = 1,
		.pBindings = &descriptorLayoutBinding
	};
	m_Storage->descriptorLayout = device.createDescriptorSetLayout(descriptorLayoutCreateInfo);

	const std::vector<vk::DescriptorSetLayout> descriptorSetLayouts =
	{
		AkBindlessResourcesManager::GetBuffersDescriptorSetLayout(),
		AkBindlessResourcesManager::GetTexturesDescriptorSetLayout(),
		AkBindlessResourcesManager::GetSamplersDescriptorSetLayout(),
		m_Storage->descriptorLayout
	};

	const vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo =
	{
		.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
		.pSetLayouts = descriptorSetLayouts.data()
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