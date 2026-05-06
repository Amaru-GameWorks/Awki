#pragma once
#include "RHI/Buffers/Buffer.h"
#include "RHI/Pipeline/RasterizerState.h"

#include <memory>
#include <vector>

class AkMesh
{
public:
	AkMesh(const struct AkMeshInfo& meshInfo, class AkVertexBuffer* vertexBuffer, class AkIndexBufferBase* indexBuffer);

	uint32_t GetIndexCount() const { return m_IndexCount; }
	uint32_t GetFirstIndex() const { return m_FirstIndex; }
	uint32_t GetVertexOffset() const { return m_VertexOffset; }
	AkPrimitiveType GetPrimitiveType() const { return m_PrimitiveType; }

	class AkVertexBuffer* GetVertexBuffer() const { return m_VertexBuffer; }
	class AkIndexBufferBase* GetIndexBuffer() const { return m_IndexBuffer; }

private:
	class AkVertexBuffer* m_VertexBuffer = nullptr;
	class AkIndexBufferBase* m_IndexBuffer = nullptr;

	uint32_t m_IndexCount = 0;
	uint32_t m_FirstIndex = 0;
	uint32_t m_VertexOffset = 0;
	AkPrimitiveType m_PrimitiveType = AkPrimitiveType::TRIANGLES;
};

class AkModel
{
public:
	AkModel(const std::unique_ptr<class AkModelDecoderInterface>& modelDecoder);
	const std::vector<AkMesh>& GetMeshes() const { return m_Meshes; }

private:
	std::vector<AkMesh> m_Meshes;
	std::unique_ptr<AkVertexBuffer> m_VertexBuffer = nullptr;
	std::unique_ptr<AkIndexBufferBase> m_IndexBuffer = nullptr;
};