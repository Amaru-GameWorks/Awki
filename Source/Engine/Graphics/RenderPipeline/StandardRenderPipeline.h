#pragma once
#include "RenderPipeline.h"
#include "RenderPipelineTypeInfo.h"

REGISTER_RENDERPIPELINE(AkStandardRenderPipeline);

class AkStandardRenderPipeline : public AkRenderPipeline
{
public:
	AkStandardRenderPipeline();
	size_t GetHash() override { return AkRenderPipelineTypeInfo<AkStandardRenderPipeline>::TypeId(); }
};