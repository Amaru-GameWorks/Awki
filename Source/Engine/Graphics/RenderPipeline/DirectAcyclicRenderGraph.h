#pragma once
#include "RHI/ResourceAliaser.h"

#include <map>
#include <vector>

class AkDirectAcyclicRenderGraph
{
public:
	void Compile(const std::multimap<uint8_t, class AkRenderPipeline*>& groupedRenderPipelines);
	const std::vector<class AkRenderPass*>& GetSortedRenderPasses() const { return m_SortedRenderPasses; }
	bool GetManagedResource(AkPipelineResourceId id, AkBuffer*& managedBuffer, AkTexture*& managedTexture);

	void SetBackBufferRenderTarget(class AkRenderTarget* backBuffer) { m_BackBuffer = backBuffer; }
	class AkRenderTarget* GetBackBufferRenderTarget() const { return m_BackBuffer; }

	void FreeManagedResources();

private:
	class AkRenderTarget* m_BackBuffer = nullptr;

	std::vector<AkAliasingGroup> m_AliasingGroups = {};
	std::vector<class AkRenderPass*> m_SortedRenderPasses = {};

	std::unordered_map<AkPipelineResourceId, class AkBuffer*> m_ManagedBuffers;
	std::unordered_map<AkPipelineResourceId, class AkTexture*> m_ManagedTextures;
};