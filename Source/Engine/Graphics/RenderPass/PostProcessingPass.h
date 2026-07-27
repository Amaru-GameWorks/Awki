#pragma once
#include "RenderPass.h"

class AkPostProcessingPass : public AkRenderPass
{
public:
	AkPostProcessingPass(class AkRenderPipeline* renderPipeline);

	void RequestResources() override;
	void Execute(class AkDirectAcyclicRenderGraph& resourceBuilder) override;

private:
	AkPipelineResourceId m_ColorBufferId = {};
	AkPipelineResourceId m_RandomTextureId = {};
	AkPipelineResourceId m_RandomTexture2Id = {};
	AkPipelineResourceId m_RandomTexture3Id = {};
};