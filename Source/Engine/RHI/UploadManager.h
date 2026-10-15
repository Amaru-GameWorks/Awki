#pragma once
#include "RHI/CommandBuffers/CommandBuffer.h"

#include <memory>

class AkUploadManager
{
public:
	static void QueueBufferUpload(class AkBuffer* buffer, uint8_t* data, size_t offset = 0);
	static void QueueTextureUpload(class AkTexture* texture, uint8_t* data);

	static bool HasPendingUploads();
	static void FillCommandBuffer();
	static class AkCommandBuffer* GetCommandBuffer();

	static void ReleaseBuffers();

private:
	static inline std::unique_ptr<AkCommandBuffer> m_CommandBuffer;
};