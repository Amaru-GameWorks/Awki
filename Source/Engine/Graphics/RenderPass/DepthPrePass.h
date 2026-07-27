#pragma once
#include "RenderPass.h"

class AkDepthPrePass : public AkRenderPass
{
public:
	AkDepthPrePass(class AkRenderPipeline* renderPipeline, const bool exportRenderTarget = false, const AkId exportTag = {});

	void RequestResources() override;
	void Execute(class AkDirectAcyclicRenderGraph& resourceBuilder) override;

private:
	AkId m_ExportTag = {};
	bool m_ExportRenderTarget = false;
	AkPipelineResourceId m_DepthBufferId = {};
};