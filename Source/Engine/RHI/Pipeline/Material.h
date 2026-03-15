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

	void SetConstantBuffer(class AkConstantBuffer* buffer, const uint32_t binding);

	class AkShader* GetShader() const { return m_Shader; }
	const AkRasterizerState& GetRasterizerState() const { return m_RasterizerState; }
	const vk::DescriptorSet& GetDescriptorSet() const;

private:
	ForwardStorage<struct AkMaterialStorage, 16> m_Storage;
	
	class AkShader* m_Shader = nullptr;
	AkRasterizerState m_RasterizerState = {};
};