#pragma once
#include "RHI/Pipeline/RasterizerState.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_aligned.hpp>

#include <vector>
#include <filesystem>

struct AkModelNode
{
	std::string name = {};
	std::vector<uint32_t> meshes = {};
	std::vector<uint32_t> children = {};
	glm::mat4 transform = glm::mat4(1.f);
};

struct AkMeshData
{
	std::vector<uint32_t> indices = {};
	std::vector<glm::packed_vec3> positions = {};
	std::vector<glm::packed_vec3> normals = {};
	std::vector<glm::packed_vec2> uvs = {};
	std::vector<glm::packed_vec4> tangents = {};
};

struct AkVertexData
{
	size_t size = 0;
	uint8_t* data = nullptr;
};

struct AkMeshInfo
{
	uint32_t indexCount = 0;
	uint32_t firstIndex = 0;
	uint32_t vertexOffset = 0;
	AkPrimitiveType primitiveType = AkPrimitiveType::TRIANGLES;
};

class AkModelDecoderInterface
{
public:
	AkModelDecoderInterface(const std::filesystem::path& path)
		: m_Path(path) 
	{ }
	virtual ~AkModelDecoderInterface() = default;

	const AkIndexType& GetIndexType() const { return m_IndexType; }
	const AkVertexData& GetIndexData() const { return m_IndexData; }
	const AkVertexData& GetVertexData() const { return m_VertexData; }

	const AkModelNode& GetRootNode() const { return m_RootNode; }
	const std::vector<AkModelNode>& GetModelNodes() const { return m_Nodes; }
	const std::vector<AkMeshInfo>& GetModelMeshes() const { return m_Meshes; }

protected:
	std::filesystem::path m_Path = {};
	
	AkModelNode m_RootNode = {};
	std::vector<AkModelNode> m_Nodes = {};
	std::vector<AkMeshInfo> m_Meshes = {};
	std::vector<AkMeshData> m_MeshData = {};

	AkIndexType m_IndexType = {};
	AkVertexData m_IndexData = {};
	AkVertexData m_VertexData = {};

	void ValidateMesh(const AkMeshInfo& mesh, AkMeshData& data);
	void ProcessMeshes();
};

class AkModelDecoder
{
public:
	static std::unique_ptr<AkModelDecoderInterface> Decode(const std::filesystem::path& path);
};