#include "RenderPass.h"

extern size_t CalculateTextureSize(const AkTextureDescriptor& descriptor);

AkRenderPass::AkRenderPass(AkRenderPipeline* renderPipeline)
	: m_RenderPipeline(renderPipeline)
{ }

bool AkRenderPass::ShouldRun()
{
	return true;
}

void AkRenderPass::RequestResources()
{
}

void AkRenderPass::ClearResourcesData()
{
	m_ResourcesUsage.clear();
	m_ResourceRequests.clear();
	m_ExportedResources.clear();
	m_ImportedResources.clear();
}

void AkRenderPass::SetCommandBuffer(AkCommandBuffer* commandBuffer)
{
	m_CommandBuffer = commandBuffer;
}

AkCommandBuffer* AkRenderPass::GetCommandBuffer() const
{
	return m_CommandBuffer;
}

AkPipelineResourceId AkRenderPass::CreatePipelineResourceId(const char* resourceName)
{
	return { resourceName, m_RenderPipeline };
}

void AkRenderPass::Reads(AkPipelineResourceId resourceId, AkResourceState resourceState)
{
	m_ResourcesUsage.push_back({ resourceId, resourceState, AkResourceUsage::AccessType::READ });
}

void AkRenderPass::ReadsRenderPipelineDependencies(AkId resourceTag, AkResourceState resourceState)
{
	m_ImportedResources[resourceTag] = resourceState;
}

void AkRenderPass::Writes(AkPipelineResourceId resourceId, AkResourceState resourceState)
{
	m_ResourcesUsage.push_back({ resourceId, resourceState, AkResourceUsage::AccessType::WRITE });
}

void AkRenderPass::WritesRenderPipelineDependency(AkId resourceTag, AkPipelineResourceId resourceId)
{
	m_ExportedResources[resourceTag] = resourceId;
}

void AkRenderPass::RequestPersistentTexture(const AkPipelineResourceId resourceId, const AkTextureDescriptor& descriptor)
{
	AkResourceRequest& request = m_ResourceRequests.emplace_back();
	request.id = resourceId;
	request.persistent = true;
	request.descriptor.texture = descriptor;
	request.size = CalculateTextureSize(descriptor);
	request.type = AkResourceRequest::Type::TEXTURE;
}

void AkRenderPass::RequestTexture(const AkPipelineResourceId resourceId, const AkTextureDescriptor& descriptor)
{
	AkResourceRequest& request = m_ResourceRequests.emplace_back();
	request.id = resourceId;
	request.persistent = false;
	request.descriptor.texture = descriptor;
	request.size = CalculateTextureSize(descriptor);
	request.type = AkResourceRequest::Type::TEXTURE;
}

void AkRenderPass::RequestPersistentBuffer(const AkPipelineResourceId resourceId, const AkBufferDescriptor& descriptor)
{
	AkResourceRequest& request = m_ResourceRequests.emplace_back();
	request.id = resourceId;
	request.persistent = true;
	request.descriptor.buffer = descriptor;
	request.type = AkResourceRequest::Type::BUFFER;
	request.size = RoundToNextMultiple(descriptor.size, AkDevice::GetMinStructuredBufferAlignment());
}

void AkRenderPass::RequestBuffer(const AkPipelineResourceId resourceId, const AkBufferDescriptor& descriptor)
{
	AkResourceRequest& request = m_ResourceRequests.emplace_back();
	request.id = resourceId;
	request.persistent = false;
	request.descriptor.buffer = descriptor;
	request.type = AkResourceRequest::Type::BUFFER;
	request.size = RoundToNextMultiple(descriptor.size, AkDevice::GetMinStructuredBufferAlignment());
}
