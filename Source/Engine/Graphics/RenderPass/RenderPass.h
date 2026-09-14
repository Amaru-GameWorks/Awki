#pragma once
#include "RHI/Buffers/Buffer.h"
#include "RHI/Textures/Texture.h"
#include "RHI/Textures/RenderTarget.h"
#include "RHI/Pipeline/ResourceState.h"
#include "RHI/CommandBuffers/CommandBuffer.h"
#include "Graphics/RenderPipeline/PipelineResourceId.h"

struct AkResourceRequest
{
	enum class Type : uint8_t
	{
		TEXTURE,
		BUFFER
	};

	Type type = {};
	size_t size = 0;
	bool persistent = {};
	uint8_t renderGroup = {};
	AkPipelineResourceId id = {};

	union Descriptor
	{
		AkTextureDescriptor texture = {};
		AkBufferDescriptor buffer;
	} descriptor;

	bool operator==(const AkResourceRequest& rhs) const
	{
		return id == rhs.id;
	}
};

namespace std
{
	template <>
	struct hash<AkResourceRequest>
	{
		size_t operator()(const AkResourceRequest& request) const
		{
			return request.id;
		}
	};
}

struct AkResourceUsage
{
	enum class AccessType : uint8_t
	{
		READ,
		WRITE
	};

	AkPipelineResourceId id = {};
	AkResourceState state = {};
	AccessType access = {};
};

class AkRenderPass
{
public:
	AkRenderPass(class AkRenderPipeline* renderPipeline);

	virtual bool ShouldRun();
	virtual void RequestResources();
	virtual void Execute(class AkDirectAcyclicRenderGraph& resourceBuilder) = 0;

	void ClearResourcesData();
	void SetCommandBuffer(class AkCommandBuffer* commandBuffer);
	class AkCommandBuffer* GetCommandBuffer() const;

	AkPipelineResourceId CreatePipelineResourceId(const char* resourceName);
	void Reads(AkPipelineResourceId resourceId, AkResourceState resourceState);
	void ReadsRenderPipelineDependencies(AkId resourceTag, AkResourceState resourceState);

	void Writes(AkPipelineResourceId resourceId, AkResourceState resourceState);
	void WritesRenderPipelineDependency(AkId resourceTag, AkPipelineResourceId resourceId);

	const std::vector<AkResourceUsage>& GetResourcesUsage() const { return m_ResourcesUsage; }
	const std::unordered_map<AkId, AkResourceState>& GetImportedResources() const { return m_ImportedResources; }
	const std::unordered_map<AkId, AkPipelineResourceId>& GetExportedResources() const { return m_ExportedResources; }

	void RequestPersistentTexture(const AkPipelineResourceId resourceId, const AkTextureDescriptor& descriptor);
	void RequestTexture(const AkPipelineResourceId resourceId, const AkTextureDescriptor& descriptor);

	void RequestPersistentBuffer(const AkPipelineResourceId resourceId, const AkBufferDescriptor& descriptor);
	void RequestBuffer(const AkPipelineResourceId resourceId, const AkBufferDescriptor& descriptor);

	class AkRenderPipeline* GetRenderPipeline() const { return m_RenderPipeline; }
	std::vector<AkResourceRequest>& GetResourceRequests() { return m_ResourceRequests; }

protected:

	AkPipelineResourceId m_RenderPassId;
	class AkCommandBuffer* m_CommandBuffer = nullptr;
	class AkRenderPipeline* m_RenderPipeline = nullptr;
	std::vector<AkResourceUsage> m_ResourcesUsage = {};
	std::vector<AkResourceRequest> m_ResourceRequests = {};

	std::unordered_map<AkId, AkResourceState> m_ImportedResources = {};
	std::unordered_map<AkId, AkPipelineResourceId> m_ExportedResources = {};
};