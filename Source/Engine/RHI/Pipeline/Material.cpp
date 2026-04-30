#include "Material.h"

#include "RHI/Device.h"
#include "RHI/Buffers/Buffer.h"
#include "RHI/Pipeline/Shader.h"
#include "Utilities/Hash.h"
#include "Utilities/Size.h"
#include "Utilities/Math.h"

#include <vulkan/vulkan.hpp>

#include <map>
#include <queue>

struct AkMaterialStorage
{
	vk::DescriptorPool pool;
	std::vector<vk::DescriptorSet> descriptorSets;
};

static constexpr uint32_t kAllocationPageSize = 16;
static std::unique_ptr<AkStructuredBuffer> sMaterialsBuffer;

static int32_t sAllocationTail = 0;
static std::unordered_map<uint32_t, uint8_t> sAllocations = {};
static std::map<uint8_t, std::queue<int32_t>> sFreeAllocations = {};

uint32_t AllocateMaterial(uint8_t pagesToAllocate)
{
	const uint32_t allocationSize = pagesToAllocate * kAllocationPageSize;

	if (!sFreeAllocations.empty())
	{
		for (auto [pages, offsets] : sFreeAllocations)
		{
			if (!offsets.empty() && pages >= pagesToAllocate)
			{
				uint32_t offset = offsets.front();
				offsets.pop();

				sAllocations[offset] = pagesToAllocate;

				if (pages != pagesToAllocate)
				{
					uint8_t pagesLeft = pages - pagesToAllocate;
					uint32_t newOffset = offset + allocationSize;
					sFreeAllocations[pagesLeft].push(newOffset);
				}

				return offset;
			}
		}
	}
	
	const uint32_t allocationOffset = sAllocationTail;
	sAllocationTail += allocationSize;
	sAllocations[allocationOffset] = pagesToAllocate;
	return allocationOffset;
}

void AkMaterial::InitializeGlobalBuffer()
{
	size_t initialSize = SizeMB(4);
	sMaterialsBuffer = std::make_unique<AkStructuredBuffer>(initialSize, nullptr, AkBufferFlags_CPU_ACCESS);
}

void AkMaterial::DeinitializeGlobalBuffer()
{
	sMaterialsBuffer = nullptr;
}

AkMaterial::AkMaterial(class AkShader* shader, const std::optional<AkRasterizerState>& rasterizerState)
	: m_Shader(shader)
{
	if (rasterizerState.has_value())
		m_RasterizerState = rasterizerState.value();
	m_RasterizerState.CalculateHash();

	const vk::Device& device = AkDevice::GetDevice();
	const AkShaderReflection& reflection = m_Shader->GetReflection();
	const std::vector<vk::DescriptorSetLayout>& descriptorLayouts = m_Shader->GetDescriptorSetLayouts();

	if (!reflection.descriptorSets.empty())
	{
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
	}

	if (reflection.materialDataSize)
	{
		uint32_t roundedSize = RoundToNextMultiple(reflection.materialDataSize, kAllocationPageSize);
		uint8_t neededPages = static_cast<uint8_t>(roundedSize / kAllocationPageSize);
		m_BindlessOffset = AllocateMaterial(neededPages);
	}

	m_Hash = Hash(m_RasterizerState);
	HashCombine(m_Hash, m_Shader);
}

AkMaterial::~AkMaterial()
{
	if (m_Storage->pool)
	{
		const vk::Device& device = AkDevice::GetDevice();
		device.destroyDescriptorPool(m_Storage->pool);
	}

	if (m_BindlessOffset != -1)
	{
		uint8_t pages = sAllocations[m_BindlessOffset];
		sFreeAllocations[pages].push(m_BindlessOffset);
		sAllocations.erase(m_BindlessOffset);
	}
}

void AkMaterial::SetData(uint8_t* data, size_t size)
{
	if (m_BindlessOffset != -1)
		if (uint8_t* bufferPointer = sMaterialsBuffer->GetMappedDataPointer())
			memcpy(bufferPointer + m_BindlessOffset, data, size);
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