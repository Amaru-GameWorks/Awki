#include "DepthPrePass.h"
#include "Graphics/RenderPipeline/RenderPipeline.h"
#include "Graphics/RenderPipeline/DirectAcyclicRenderGraph.h"

AkDepthPrePass::AkDepthPrePass(class AkRenderPipeline* renderPipeline, const bool exportRenderTarget, const AkId exportTag)
	: AkRenderPass(renderPipeline)
	, m_ExportRenderTarget(exportRenderTarget)
	, m_ExportTag(exportTag)
{
	m_DepthBufferId = CreatePipelineResourceId("DepthBuffer");
}

void AkDepthPrePass::RequestResources()
{
	const glm::uvec2& renderTargetResolution = m_RenderPipeline->GetRenderTargetResolution();
	const AkTextureDescriptor depthBufferDescriptor =
	{
		.width = renderTargetResolution.x,
		.height = renderTargetResolution.y,
		.flags = AkTextureFlags_DEFAULT_DEPTH,
		.format = AkPixelFormat::D32_SFLOAT
	};

	RequestTexture(m_DepthBufferId, depthBufferDescriptor);
	Writes(m_DepthBufferId, AkResourceState::DEPTH_WRITE);

	if (m_ExportRenderTarget)
		WritesRenderPipelineDependency(m_ExportTag, m_DepthBufferId);
}

void AkDepthPrePass::Execute(class AkDirectAcyclicRenderGraph& renderGraph)
{

}
