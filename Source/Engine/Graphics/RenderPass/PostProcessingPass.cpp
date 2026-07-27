#include "PostProcessingPass.h"
#include "RHI/Textures/RenderTarget.h"
#include "Graphics/RenderPipeline/DirectAcyclicRenderGraph.h"

AkPostProcessingPass::AkPostProcessingPass(class AkRenderPipeline* renderPipeline)
	: AkRenderPass(renderPipeline)
{
	m_ColorBufferId = CreatePipelineResourceId("ColorBuffer");
	m_RandomTextureId = CreatePipelineResourceId("RandomTexture");
	m_RandomTexture2Id = CreatePipelineResourceId("RandomTexture2");
	m_RandomTexture3Id = CreatePipelineResourceId("RandomTexture3");
}

void AkPostProcessingPass::RequestResources()
{
	Reads(m_ColorBufferId, AkResourceState::SHADER_RESOURCE);
}

void AkPostProcessingPass::Execute(class AkDirectAcyclicRenderGraph& renderGraph)
{
	AkRenderTarget* backBuffer = renderGraph.GetBackBufferRenderTarget();
	m_CommandBuffer->TransitionRenderTargetColorAttachments(backBuffer, AkResourceState::UNDEFINED, AkResourceState::PRESENT);
}
