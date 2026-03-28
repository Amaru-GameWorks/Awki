#include "PipelineStateManager.h"
#include "RHI/Device.h"
#include "Utilities/Hash.h"
#include "RHI/Pipeline/Material.h"
#include "RHI/Textures/RenderTarget.h"
#include "RHI/Pipeline/PipelineStateObject.h"

#include <vulkan/vulkan.hpp>

#include <fstream>
#include <filesystem>

static vk::PipelineCache sCache = VK_NULL_HANDLE;
static std::unordered_map<size_t, std::unique_ptr<AkPipelineStateObject>> sPiplineStateObjects = {};

size_t GetPipelineHash(AkMaterial* material, AkRenderTarget* renderTarget, AkPrimitiveType primitiveType)
{
	size_t hash = renderTarget->GetHash();
	HashCombine(hash, material->GetHash());
	HashCombine(hash, primitiveType);
	return hash;
}

void AkPipelineStateManager::Initialize()
{
	const std::filesystem::path cachePath = "Library/Pipeline.cache";
	if (std::filesystem::exists(cachePath))
	{
		const uintmax_t size = std::filesystem::file_size(cachePath);
		std::ifstream file(cachePath, std::ios::binary);

		std::vector<char> buffer(size);
		if (file && file.read(buffer.data(), size))
		{
			const vk::PipelineCacheCreateInfo pipelineCacheCreateInfo =
			{
				.initialDataSize = size,
				.pInitialData = buffer.data()
			};

			const vk::Device& device = AkDevice::GetDevice();
			sCache = device.createPipelineCache(pipelineCacheCreateInfo);
		}
	}
	else
	{
		const vk::Device& device = AkDevice::GetDevice();
		sCache = device.createPipelineCache({});
	}
}

void AkPipelineStateManager::Deinitialize()
{
	const vk::Device& device = AkDevice::GetDevice();

	if (sCache)
	{
		std::vector<uint8_t> pipelineData = device.getPipelineCacheData(sCache);
		if (!pipelineData.empty())
		{
			const std::filesystem::path cachePath = "Library/Pipeline.cache";
			std::ofstream file(cachePath, std::ios::binary | std::ios::trunc);

			if (file)
			{
				file.write(reinterpret_cast<char*>(pipelineData.data()), pipelineData.size());
				file.close();
			}
		}

		device.destroyPipelineCache(sCache);
	}

	sPiplineStateObjects.clear();
}

AkPipelineStateObject* AkPipelineStateManager::GetPipelineStateObject(AkMaterial* material, AkRenderTarget* renderTarget, AkPrimitiveType primitiveType)
{
	const size_t pipelineHash = GetPipelineHash(material, renderTarget, primitiveType);

	if (!sPiplineStateObjects.contains(pipelineHash))
		sPiplineStateObjects[pipelineHash] = std::make_unique<AkPipelineStateObject>(material, primitiveType, renderTarget);

	return sPiplineStateObjects.at(pipelineHash).get();
}