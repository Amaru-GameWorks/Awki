#include "Model.h"
#include "RHI/Buffers/Buffer.h"
#include "Utilities/Models/ModelDecoder.h"

AkMesh::AkMesh(const AkMeshInfo& meshInfo, AkVertexBuffer* vertexBuffer, AkIndexBufferBase* indexBuffer)
	: m_VertexBuffer(vertexBuffer)
	, m_IndexBuffer(indexBuffer)
	, m_IndexCount(meshInfo.indexCount)
	, m_FirstIndex(meshInfo.firstIndex)
	, m_VertexOffset(meshInfo.vertexOffset)
	, m_PrimitiveType(meshInfo.primitiveType)
{ }

AkModel::AkModel(const std::unique_ptr<class AkModelDecoderInterface>& modelDecoder)
{
	const AkIndexType& indexType = modelDecoder->GetIndexType();
	const AkVertexData& vertexData = modelDecoder->GetVertexData();
	m_VertexBuffer = std::make_unique<AkVertexBuffer>(vertexData.size, vertexData.data);

	const AkVertexData& indexData = modelDecoder->GetIndexData();
	if (indexType == AkIndexType::U16)
		m_IndexBuffer = std::make_unique<AkIndexBuffer<AkIndexType::U16>>(indexData.size, indexData.data);
	else
		m_IndexBuffer = std::make_unique<AkIndexBuffer<AkIndexType::U32>>(indexData.size, indexData.data);

	const std::vector<AkMeshInfo>& meshes = modelDecoder->GetModelMeshes();
	m_Meshes.reserve(meshes.size());

	for (const AkMeshInfo& meshInfo : modelDecoder->GetModelMeshes())
		m_Meshes.emplace_back(meshInfo, m_VertexBuffer.get(), m_IndexBuffer.get());
}