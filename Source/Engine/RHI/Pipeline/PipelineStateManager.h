#pragma once
#include "RHI/Pipeline/RasterizerState.h"

class AkPipelineStateManager
{
private:
	friend class Awki;
	static void Initialize();
	static void Deinitialize();

public:
	static class AkPipelineStateObject* GetPipelineStateObject(class AkMaterial* material, class AkRenderTarget* renderTarget, const AkPrimitiveType primitiveType);
};