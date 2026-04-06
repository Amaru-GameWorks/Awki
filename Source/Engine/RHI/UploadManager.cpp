#include "UploadManager.h"
#include "Utilities/Size.h"
#include "RHI/Buffers/Buffer.h"
#include "RHI/Textures/Texture.h"
#include "RHI/CommandBuffers/CommandBufferAllocator.h"

struct AkAllocation
{
	size_t offset = 0;
	class AkBuffer* stagingBuffer = nullptr;
};

struct AkBufferUploadRequest
{
	size_t size = 0;
	size_t offset = 0;
	AkBuffer* buffer = nullptr;
	AkAllocation allocation = {};
};

struct AkTextureUploadRequest
{
	AkAllocation allocation = {};
	AkTexture* texture = nullptr;
};

struct AkUploadHeap
{
	size_t size = 0;
	size_t used = 0;
	std::unique_ptr<AkBuffer> buffer;
};

static std::vector<AkBufferUploadRequest> sPendingBufferUploads;
static std::vector<AkTextureUploadRequest> sPendingTextureUploads;
static std::vector<AkUploadHeap> sUploadHeaps;

AkAllocation FindFreeUploadSpace(size_t size)
{
	for (AkUploadHeap& uploadHeap : sUploadHeaps)
	{
		const size_t free = uploadHeap.size - uploadHeap.used;
		if (size <= free)
		{
			AkAllocation allocation =
			{
				.offset = uploadHeap.used,
				.stagingBuffer = uploadHeap.buffer.get()
			};

			uploadHeap.used += size;
			return allocation;
		}
	}

	static constexpr size_t kBaseBufferSize = SizeMB(256);
	size_t uploadHeapSize = kBaseBufferSize;
	
	if(size > uploadHeapSize)
		uploadHeapSize = NextPowerOf2(size);

	AkUploadHeap& uploadHeap = sUploadHeaps.emplace_back();
	uploadHeap.size = uploadHeapSize;
	uploadHeap.buffer = std::make_unique<AkStagingBuffer>(uploadHeapSize);

	return FindFreeUploadSpace(size);
}

void AkUploadManager::QueueBufferUpload(AkBuffer* buffer, uint8_t* data, size_t offset)
{
	const AkBufferDescriptor& descriptor = buffer->GetDescriptor();
	AkBufferUploadRequest& uploadRequest = sPendingBufferUploads.emplace_back();
	uploadRequest =
	{
		.size = descriptor.size,
		.offset = offset,
		.buffer = buffer,
		.allocation = FindFreeUploadSpace(descriptor.size)
	};

	memcpy(uploadRequest.allocation.stagingBuffer->GetMappedDataPointer() + uploadRequest.allocation.offset, data, descriptor.size);
}

void AkUploadManager::QueueTextureUpload(AkTexture* texture, uint8_t* data, size_t offset)
{
	const size_t textureSize = texture->GetSize();

	AkTextureUploadRequest& uploadRequest = sPendingTextureUploads.emplace_back();
	uploadRequest.texture = texture;
	uploadRequest.allocation = FindFreeUploadSpace(textureSize);

	memcpy(uploadRequest.allocation.stagingBuffer->GetMappedDataPointer() + uploadRequest.allocation.offset, data, textureSize);
}

bool AkUploadManager::HasPendingUploads()
{
	return !sPendingBufferUploads.empty() || !sPendingTextureUploads.empty();
}

void AkUploadManager::FillCommandBuffer()
{
	static AkCommandBuffer* sUploadCommandBuffer = GetCommandBuffer();
	sUploadCommandBuffer->Begin();

	std::vector<AkBuffer*> buffersToTransition;
	buffersToTransition.reserve(sPendingBufferUploads.size());

	std::vector<AkTexture*> texturesToTransition;
	texturesToTransition.reserve(sPendingTextureUploads.size());

	for (const AkBufferUploadRequest& bufferUpload : sPendingBufferUploads)
		buffersToTransition.push_back(bufferUpload.buffer);

	for (const AkTextureUploadRequest& textureUpload : sPendingTextureUploads)
		texturesToTransition.push_back(textureUpload.texture);

	sUploadCommandBuffer->TransitionResources(buffersToTransition, texturesToTransition, AkResourceState::UNDEFINED, AkResourceState::COPY_DESTINATION);

	for (const AkBufferUploadRequest& bufferUpload : sPendingBufferUploads)
		sUploadCommandBuffer->CopyBufferToBuffer(bufferUpload.allocation.stagingBuffer, bufferUpload.buffer, bufferUpload.size, bufferUpload.allocation.offset, bufferUpload.offset);

	for (const AkTextureUploadRequest& textureUpload : sPendingTextureUploads)
		sUploadCommandBuffer->CopyBufferToTexture(textureUpload.allocation.stagingBuffer, textureUpload.texture, textureUpload.allocation.offset);

	sUploadCommandBuffer->TransitionResources(buffersToTransition, texturesToTransition, AkResourceState::COPY_DESTINATION, AkResourceState::SHADER_RESOURCE);

	sPendingTextureUploads.clear();
	sPendingBufferUploads.clear();
	sUploadCommandBuffer->End();
}

AkCommandBuffer* AkUploadManager::GetCommandBuffer()
{
	static AkCommandBuffer* sUploadCommandBuffer = AkCommandBufferAllocator::AllocateCommandBuffer(AkDeviceQueue::GRAPHICS);
	return sUploadCommandBuffer;
}

void AkUploadManager::ReleaseBuffers()
{
	sUploadHeaps.clear();
}
