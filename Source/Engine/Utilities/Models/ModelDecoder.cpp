#include "ModelDecoder.h"
#include "GltfModelDecoder.h"
#include "Utilities/Hash.h"

#include <mikktspace.c>
#include <meshoptimizer.h>

namespace MikkTSpace
{
	static int GetNumFaces(const SMikkTSpaceContext* context)
	{
		AkMeshData* meshData = static_cast<AkMeshData*> (context->m_pUserData);
		return static_cast<int>(meshData->indices.size() / 3);
	}

	static int GetNumVerticesOfFace(const SMikkTSpaceContext*, const int)
	{
		return 3;
	}

	static int GetVertexIndex(const AkMeshData& meshtData, int face, int vert)
	{
		const int index = (face * 3) + vert;
		return meshtData.indices.empty() ? index : meshtData.indices[index];
	}

	static void GetPosition(const SMikkTSpaceContext* context, float* outpos, const int face, const int vert)
	{
		AkMeshData* meshData = static_cast<AkMeshData*> (context->m_pUserData);

		auto index = GetVertexIndex(*meshData, face, vert);
		const glm::packed_vec3& position = meshData->positions[index];

		outpos[0] = position.x;
		outpos[1] = position.y;
		outpos[2] = position.z;
	}

	static void GetNormal(const SMikkTSpaceContext* context, float* outnormal, const int face, const int vert)
	{
		AkMeshData* meshData = static_cast<AkMeshData*> (context->m_pUserData);

		auto index = GetVertexIndex(*meshData, face, vert);
		const glm::packed_vec3& normal = meshData->normals[index];

		outnormal[0] = normal.x;
		outnormal[1] = normal.y;
		outnormal[2] = normal.z;
	}

	static void GetTexCoords(const SMikkTSpaceContext* context, float* outuv, const int face, const int vert)
	{
		AkMeshData* meshData = static_cast<AkMeshData*> (context->m_pUserData);

		auto index = GetVertexIndex(*meshData, face, vert);
		const glm::packed_vec2& uv = meshData->uvs[index];

		outuv[0] = uv.x;
		outuv[1] = uv.y;
	}

	static void SetTspaceBasic(const SMikkTSpaceContext* context, const float* tangentu, const float fSign, const int face, const int vert)
	{
		AkMeshData* meshData = static_cast<AkMeshData*> (context->m_pUserData);

		auto index = GetVertexIndex(*meshData, face, vert);
		glm::packed_vec4& tangent = meshData->tangents[index];

		tangent.x = tangentu[0];
		tangent.y = tangentu[1];
		tangent.z = tangentu[2];
		tangent.w = fSign;
	}
};

void CalculateTangents(AkMeshData& data)
{
	SMikkTSpaceInterface sMikkInterface = {};
	sMikkInterface.m_getNumFaces = MikkTSpace::GetNumFaces;
	sMikkInterface.m_getNumVerticesOfFace = MikkTSpace::GetNumVerticesOfFace;
	sMikkInterface.m_getPosition = MikkTSpace::GetPosition;
	sMikkInterface.m_getNormal = MikkTSpace::GetNormal;
	sMikkInterface.m_getTexCoord = MikkTSpace::GetTexCoords;
	sMikkInterface.m_setTSpaceBasic = MikkTSpace::SetTspaceBasic;

	SMikkTSpaceContext mikkContext = {};
	mikkContext.m_pInterface = &sMikkInterface;
	mikkContext.m_pUserData = &data;

	data.tangents.resize(data.positions.size());
	genTangSpaceDefault(&mikkContext);
}

void CalculateFaceNormals(AkMeshData& data)
{
	const bool useIndices = !data.indices.empty();
	data.normals.resize(data.positions.size());

	for (size_t i = 0; i < data.positions.size(); i += 3)
	{
		size_t i0 = i + 0;
		size_t i1 = i + 1;
		size_t i2 = i + 2;

		if (useIndices)
		{
			i0 = data.indices[i0];
			i1 = data.indices[i1];
			i2 = data.indices[i2];
		}

		const glm::vec3 v0 = data.positions[i0];
		const glm::vec3 v1 = data.positions[i1];
		const glm::vec3 v2 = data.positions[i2];

		const glm::vec3 v0v1 = v1 - v0;
		const glm::vec3 v0v2 = v2 - v0;
		const glm::vec3 normal = glm::cross(v0v1, v0v2);

		data.normals[i0] = normal;
		data.normals[i1] = normal;
		data.normals[i2] = normal;
	}
}

std::unique_ptr<AkModelDecoderInterface> AkModelDecoder::Decode(const std::filesystem::path& path)
{
	std::string extension = path.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	switch (FNV1aHash(extension))
	{
		case FNV1aHash(".gltf"):
			return std::make_unique<AkGltfModelDecoder>(path);
	}

	throw std::runtime_error("Model format not supported!");
}

void AkModelDecoderInterface::ValidateMesh(const AkMeshInfo& mesh, AkMeshData& data)
{
	if (mesh.primitiveType != AkPrimitiveType::TRIANGLES)
		throw std::runtime_error("We currently only support importing triangle list models!");

	if (data.positions.empty())
		throw std::runtime_error("No position data found!");

	if (data.uvs.empty())
		throw std::runtime_error("No uvs found!");

	if (data.normals.empty())
		CalculateFaceNormals(data);

	if (data.tangents.empty())
		CalculateTangents(data);
}

void AkModelDecoderInterface::ProcessMeshes()
{
	struct AwkiVertex
	{
		glm::packed_vec3 position;
		glm::packed_vec3 normal;
		glm::packed_vec2 uv;
		glm::packed_vec4 tangent;
		glm::packed_vec4 _padding;
	};

	size_t totalIndices = 0;
	uint32_t maxIndexValue = 0;
	size_t totalVertexSize = 0;

	for (size_t meshIndex = 0; meshIndex < m_Meshes.size(); ++meshIndex)
	{
		AkMeshData& data = m_MeshData[meshIndex];
		AkMeshData originalData = data;

		const std::vector<meshopt_Stream> streams =
		{
			{ data.positions.data(), sizeof(glm::packed_vec3), sizeof(glm::packed_vec3) },
			{ data.normals.data(), sizeof(glm::packed_vec3), sizeof(glm::packed_vec3) },
			{ data.uvs.data(), sizeof(glm::packed_vec2), sizeof(glm::packed_vec2) },
			{ data.tangents.data(), sizeof(glm::packed_vec4), sizeof(glm::packed_vec4) }
		};

		size_t vertexCount = 0;
		std::vector<unsigned int> remapTable(data.positions.size());

		if (data.indices.empty())
		{
			data.indices.resize(data.positions.size());
			vertexCount = meshopt_generateVertexRemapMulti(&remapTable[0], NULL, data.positions.size(), data.positions.size(), streams.data(), streams.size());
			meshopt_remapIndexBuffer(data.indices.data(), NULL, data.indices.size(), &remapTable[0]);
		}
		else
		{
			std::vector<uint32_t> newIndices;
			newIndices.resize(data.indices.size());

			vertexCount = meshopt_generateVertexRemapMulti(&remapTable[0], data.indices.data(), data.indices.size(), data.positions.size(), streams.data(), streams.size());
			meshopt_remapIndexBuffer(newIndices.data(), data.indices.data(), data.indices.size(), &remapTable[0]);

			std::swap(newIndices, data.indices);
		}

		meshopt_optimizeVertexCache(data.indices.data(), data.indices.data(), data.indices.size(), vertexCount);
		meshopt_optimizeOverdraw(data.indices.data(), data.indices.data(), data.indices.size(), &data.positions[0].x, vertexCount, sizeof(glm::packed_vec3), 1.05f);

		data.positions.resize(vertexCount);
		data.normals.resize(vertexCount);
		data.tangents.resize(vertexCount);
		data.uvs.resize(vertexCount);

		meshopt_remapVertexBuffer(data.positions.data(), originalData.positions.data(), originalData.positions.size(), sizeof(glm::packed_vec3), &remapTable[0]);
		meshopt_remapVertexBuffer(data.normals.data(), originalData.normals.data(), originalData.normals.size(), sizeof(glm::packed_vec3), &remapTable[0]);
		meshopt_remapVertexBuffer(data.uvs.data(), originalData.uvs.data(), originalData.uvs.size(), sizeof(glm::packed_vec2), &remapTable[0]);
		meshopt_remapVertexBuffer(data.tangents.data(), originalData.tangents.data(), originalData.tangents.size(), sizeof(glm::packed_vec4), &remapTable[0]);

		totalIndices += data.indices.size();
		maxIndexValue = std::max(maxIndexValue, *std::max_element(data.indices.begin(), data.indices.end()));
		totalVertexSize += data.positions.size() * sizeof(AwkiVertex);
	}

	if (maxIndexValue <= USHRT_MAX)
		m_IndexType = AkIndexType::U16;

	const size_t indexTypeSize = (m_IndexType == AkIndexType::U32 ? sizeof(uint32_t) : sizeof(uint16_t));

	m_IndexData.size = totalIndices * indexTypeSize;
	m_IndexData.data = new uint8_t[m_IndexData.size];

	m_VertexData.size = totalVertexSize;
	m_VertexData.data = new uint8_t[m_VertexData.size];

	size_t indexCount = 0;
	size_t vertexOffset = 0;

	for (size_t meshIndex = 0; meshIndex < m_Meshes.size(); ++meshIndex)
	{
		AkMeshInfo& mesh = m_Meshes[meshIndex];
		AkMeshData& data = m_MeshData[meshIndex];
		mesh.indexCount = static_cast<uint32_t>(data.indices.size());

		// Indices
		if (m_IndexType == AkIndexType::U16)
		{
			std::vector<uint16_t> convertedIndices;
			convertedIndices.reserve(data.indices.size());

			std::transform(data.indices.begin(), data.indices.end(), std::back_inserter(convertedIndices),
				[](uint32_t& x) -> uint16_t { return static_cast<uint16_t>(x); });

			memcpy(m_IndexData.data + (indexCount * indexTypeSize), convertedIndices.data(), mesh.indexCount * indexTypeSize);
		}
		else
			memcpy(m_IndexData.data + (indexCount * indexTypeSize), data.indices.data(), mesh.indexCount * indexTypeSize);

		size_t dataOffset = vertexOffset * sizeof(AwkiVertex);
		for (size_t i = 0; i < data.positions.size(); ++i)
		{
			memcpy(m_VertexData.data + dataOffset, &data.positions[i], sizeof(glm::packed_vec3));
			dataOffset += sizeof(glm::packed_vec3);

			memcpy(m_VertexData.data + dataOffset, &data.normals[i], sizeof(glm::packed_vec3));
			dataOffset += sizeof(glm::packed_vec3);

			memcpy(m_VertexData.data + dataOffset, &data.uvs[i], sizeof(glm::packed_vec2));
			dataOffset += sizeof(glm::packed_vec2);

			memcpy(m_VertexData.data + dataOffset, &data.tangents[i], sizeof(glm::packed_vec4));
			dataOffset += sizeof(glm::packed_vec4);

			//Padding
			dataOffset += sizeof(glm::packed_vec4);
		}

		mesh.firstIndex = static_cast<uint32_t>(indexCount);
		mesh.vertexOffset = static_cast<uint32_t>(vertexOffset);

		vertexOffset += data.positions.size();
		indexCount += mesh.indexCount;
	}
}
