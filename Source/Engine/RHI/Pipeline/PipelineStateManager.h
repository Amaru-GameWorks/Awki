#pragma once
#include "RHI/Pipeline/RasterizerState.h"

class AkPipelineStateManager
{
public:
	static void Initialize();
	static void Deinitialize();

	static class AkPipelineStateObject* GetPipelineStateObject(class AkMaterial* material, class AkRenderTarget* renderTarget, const AkPrimitiveType primitiveType);
};