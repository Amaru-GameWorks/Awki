#pragma once
#include "RenderPass.h"

class AkBaseRenderPass : public AkRenderPass
{
public:
	AkBaseRenderPass(class AkRenderPipeline* renderPipeline);

	void RequestResources() override;
	void Execute(class AkDirectAcyclicRenderGraph& resourceBuilder) override;

private:
	AkPipelineResourceId m_DepthBufferId = {};
	AkPipelineResourceId m_ColorBufferId = {};
};