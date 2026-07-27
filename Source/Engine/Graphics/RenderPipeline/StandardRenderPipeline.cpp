#include "StandardRenderPipeline.h"
#include "Graphics/RenderPass/DepthPrePass.h"
#include "Graphics/RenderPass/BaseRenderPass.h"
#include "Graphics/RenderPass/PostProcessingPass.h"

AkStandardRenderPipeline::AkStandardRenderPipeline()
{
	AddRenderPass<AkDepthPrePass>();
	AddRenderPass<AkBaseRenderPass>();
	AddRenderPass<AkPostProcessingPass>();
}