#include "BaseRenderPass.h"
#include "Graphics/RenderPipeline/RenderPipeline.h"
#include "Graphics/RenderPipeline/DirectAcyclicRenderGraph.h"

AkBaseRenderPass::AkBaseRenderPass(AkRenderPipeline* renderPipeline)
	: AkRenderPass(renderPipeline)
{
	m_DepthBufferId = CreatePipelineResourceId("DepthBuffer");
	m_ColorBufferId = CreatePipelineResourceId("ColorBuffer");
}

void AkBaseRenderPass::RequestResources()
{
	const glm::uvec2& renderTargetResolution = m_RenderPipeline->GetRenderTargetResolution();
	const AkTextureDescriptor colorBufferDescriptor =
	{
		.width = renderTargetResolution.x,
		.height = renderTargetResolution.y,
		.flags = AkTextureFlags_DEFAULT_RT,
		.format = AkPixelFormat::RGBA16_FLOAT
	};

	RequestTexture(m_ColorBufferId, colorBufferDescriptor);
	Writes(m_ColorBufferId, AkResourceState::RENDER_TARGET);
	Reads(m_DepthBufferId, AkResourceState::DEPTH_READ);
}

void AkBaseRenderPass::Execute(AkDirectAcyclicRenderGraph& renderGraph)
{

}
