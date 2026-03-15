#pragma once
#include "Utilities/ForwardStorage.h"

namespace vk
{
	class Pipeline;
}

class AkPipelineStateObject
{
public:
	AkPipelineStateObject(class AkMaterial* material, class AkRenderTarget* renderTarget);
	~AkPipelineStateObject();

	const vk::Pipeline& GetPipeline() const;

private:
	ForwardStorage<struct AkPipelineStorage, 8> m_Storage;
};