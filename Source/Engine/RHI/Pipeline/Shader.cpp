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
	std::vector<vk::DescriptorSetLayout> descriptorLayouts;
};

AkShader::AkShader(const AkShaderData& shaderData)
	: m_Reflection(shaderData.GetReflection())
{
	const vk::Device& device = AkDevice::GetDevice();

	vk::ShaderModuleCreateInfo moduleCreateInfo =
	{
		.codeSize = shaderData.GetByteCodeSize(),
		.pCode = reinterpret_cast<const uint32_t*>(shaderData.GetByteCode()),
	};
	m_Storage->shaderModule = device.createShaderModule(moduleCreateInfo);

	for (const auto& [set, descriptor] : m_Reflection.descriptorSets)
	{
		for (const auto& [binding, constantBuffer] : descriptor.constantBuffers)
		{
			const vk::DescriptorSetLayoutBinding descriptorLayoutBinding =
			{
				.binding = binding,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eAll
			};

			const vk::DescriptorSetLayoutCreateInfo descriptorLayoutCreateInfo =
			{
				.bindingCount = 1,
				.pBindings = &descriptorLayoutBinding
			};

			m_Storage->descriptorLayouts.push_back(device.createDescriptorSetLayout(descriptorLayoutCreateInfo));
		}
	}

	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = { AkBindlessResourcesManager::GetDescriptorSetLayout() };
	descriptorSetLayouts.insert(descriptorSetLayouts.end(), m_Storage->descriptorLayouts.begin(), m_Storage->descriptorLayouts.end());

	const vk::PushConstantRange pushConstantsRange =
	{
		.stageFlags = vk::ShaderStageFlagBits::eAll,
		.offset = 0,
		.size = m_Reflection.pushConstantSize
	};

	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo =
	{
		.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
		.pSetLayouts = descriptorSetLayouts.data()
	};

	if (m_Reflection.pushConstantSize > 0)
	{
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantsRange;
	}

	m_Storage->pipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
}

AkShader::~AkShader()
{
	const vk::Device& device = AkDevice::GetDevice();
	device.destroyShaderModule(m_Storage->shaderModule); 
	device.destroyPipelineLayout(m_Storage->pipelineLayout);

	for(auto& descriptorLayout : m_Storage->descriptorLayouts)
		device.destroyDescriptorSetLayout(descriptorLayout);
}

const vk::ShaderModule& AkShader::GetModule() const
{
	return m_Storage->shaderModule;
}

const vk::PipelineLayout& AkShader::GetPipelineLayout() const
{
	return m_Storage->pipelineLayout;
}

const std::vector<vk::DescriptorSetLayout>& AkShader::GetDescriptorSetLayouts() const
{
	return m_Storage->descriptorLayouts;
}
