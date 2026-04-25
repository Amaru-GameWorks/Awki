#pragma once
#include "Utilities/ForwardStorage.h"
#include "RHI/Pipeline/RasterizerState.h"

#include <optional>

namespace vk
{
	class DescriptorSet;
}

class AkMaterial
{
public:
	AkMaterial(class AkShader* shader, const std::optional<AkRasterizerState>& rasterizerState = std::nullopt);
	~AkMaterial();

	void SetConstantBuffer(class AkConstantBuffer* buffer, const uint32_t binding, const uint32_t set = 0);

	class AkShader* GetShader() const { return m_Shader; }
	const AkRasterizerState& GetRasterizerState() const { return m_RasterizerState; }
	const std::vector<vk::DescriptorSet>& GetDescriptorSets() const;

	size_t GetHash() const { return m_Hash; }

private:
	size_t m_Hash = 0;
	ForwardStorage<struct AkMaterialStorage, 40> m_Storage;
	
	class AkShader* m_Shader = nullptr;
	AkRasterizerState m_RasterizerState = {};
};