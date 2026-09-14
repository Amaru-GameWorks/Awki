#pragma once
#include "Utilities/ForwardStorage.h"
#include "RHI/Pipeline/RasterizerState.h"

namespace vk
{
	class Pipeline;
}

class AkPipelineStateObject
{
public:
	AkPipelineStateObject(class AkMaterial* material, const AkPrimitiveType primitiveType, class AkRenderTarget* renderTarget);
	~AkPipelineStateObject();

	const vk::Pipeline& GetPipeline() const;

private:
	AkForwardStorage<struct AkPipelineStorage, 8> m_Storage;
};