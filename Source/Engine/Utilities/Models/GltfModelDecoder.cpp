#include "GltfModelDecoder.h"

#define TINYGLTF3_IMPLEMENTATION
#define TINYGLTF3_ENABLE_FS
#include <tiny_gltf_v3.h>

#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

struct AkGltfModelStorage
{
	tg3_model model;
};

AkGltfModelDecoder::AkGltfModelDecoder(const std::filesystem::path& path)
	: AkModelDecoderInterface(path)
{
	std::string pathString = m_Path.string();
	const char* modelPath = pathString.c_str();

	tg3_parse_options parseOption = {};
	tg3_parse_options_init(&parseOption);
	parseOption.parse_float32 = 1;

	tg3_error_stack errors = { 0 };
	if (tg3_parse_file(&m_Storage->model, &errors, modelPath, static_cast<uint32_t>(strlen(modelPath)), &parseOption) != TG3_OK)
	{
		std::string errorMessage = std::format("Failed to parse Gltf file at path '{}': ", modelPath);
		for (uint32_t i = 0; i < errors.count; ++i)
		{
			const tg3_error_entry& errorEntry = errors.entries[i];
			errorMessage += std::format("{}\n", errorEntry.message);
		}

		throw std::runtime_error(errorMessage);
	}

	if (m_Storage->model.scenes_count > 1)
		throw std::runtime_error("Multi scene gltf files are not supported!");

	const tg3_scene& scene = m_Storage->model.scenes[0];

	m_Nodes.resize(scene.nodes_count);
	m_RootNode.name = m_Path.stem().string();
	m_RootNode.children.reserve(scene.nodes_count);

	for (uint32_t i = 0; i < scene.nodes_count; ++i)
		m_RootNode.children.push_back(scene.nodes[i]);

	for (const uint32_t& nodeIndex : m_RootNode.children)
		ProcessNode(m_Storage->model.nodes[nodeIndex], m_Nodes[nodeIndex]);

	ProcessMeshes();
}

AkGltfModelDecoder::~AkGltfModelDecoder()
{
	tg3_model_free(&m_Storage->model);
}

void AkGltfModelDecoder::ProcessNode(const tg3_node& nodeInfo, AkModelNode& node)
{
	if (nodeInfo.camera != -1 || nodeInfo.light != -1 || nodeInfo.emitter != -1)
		throw std::runtime_error("GLTF files only support meshes");

	if (nodeInfo.name.data)
		node.name = nodeInfo.name.data;
	else
		node.name = "Node " + std::to_string(m_Nodes.size());

	if (nodeInfo.mesh != -1)
		ProcessMesh(m_Storage->model.meshes[nodeInfo.mesh], node);

	if (nodeInfo.has_matrix)
	{
		node.transform = glm::mat4
		(
			static_cast<float>(nodeInfo.matrix[0]),
			static_cast<float>(nodeInfo.matrix[1]),
			static_cast<float>(nodeInfo.matrix[2]),
			static_cast<float>(nodeInfo.matrix[3]),
			static_cast<float>(nodeInfo.matrix[4]),
			static_cast<float>(nodeInfo.matrix[5]),
			static_cast<float>(nodeInfo.matrix[6]),
			static_cast<float>(nodeInfo.matrix[7]),
			static_cast<float>(nodeInfo.matrix[8]),
			static_cast<float>(nodeInfo.matrix[9]),
			static_cast<float>(nodeInfo.matrix[10]),
			static_cast<float>(nodeInfo.matrix[11]),
			static_cast<float>(nodeInfo.matrix[12]),
			static_cast<float>(nodeInfo.matrix[13]),
			static_cast<float>(nodeInfo.matrix[14]),
			static_cast<float>(nodeInfo.matrix[15])
		);
	}
	else
	{
		glm::vec3 translation = glm::vec3(
			static_cast<float>(nodeInfo.translation[0]),
			static_cast<float>(nodeInfo.translation[1]),
			static_cast<float>(nodeInfo.translation[2])
		);

		glm::quat rotation = glm::normalize(glm::quat(
			static_cast<float>(nodeInfo.rotation[3]),
			static_cast<float>(nodeInfo.rotation[0]),
			static_cast<float>(nodeInfo.rotation[1]),
			static_cast<float>(nodeInfo.rotation[2])
		));

		glm::vec3 scale = glm::vec3(
			static_cast<float>(nodeInfo.scale[0]),
			static_cast<float>(nodeInfo.scale[1]),
			static_cast<float>(nodeInfo.scale[2])
		);

		node.transform = glm::translate(translation);
		node.transform *= glm::toMat4(rotation);
		node.transform *= glm::scale(scale);
	}

	if (nodeInfo.children_count)
	{
		node.children.reserve(nodeInfo.children_count);
		
		for (uint32_t i = 0; i < nodeInfo.children_count; ++i)
			node.children.push_back(nodeInfo.children[i]);

		for (int const& childIndex : node.children)
			ProcessNode(m_Storage->model.nodes[childIndex], m_Nodes[childIndex]);
	}
}

void AkGltfModelDecoder::ProcessMesh(const tg3_mesh& meshInfo, AkModelNode& node)
{
	for (uint32_t i = 0; i < meshInfo.primitives_count; ++i)
	{
		const tg3_primitive& primitive = meshInfo.primitives[i];

		AkMeshInfo mesh = {};
		AkMeshData data = {};

		if (primitive.indices != -1)
		{
			const tg3_accessor& accessor = m_Storage->model.accessors[primitive.indices];
			if (accessor.component_type == TG3_COMPONENT_TYPE_UNSIGNED_BYTE)
			{
				std::vector<uint8_t> tempIndices(accessor.count);
				GetVertexData(accessor, sizeof(uint8_t), tempIndices.data());
			
				data.indices.reserve(accessor.count);
				std::transform(tempIndices.begin(), tempIndices.end(), std::back_inserter(data.indices), [](uint8_t& x) -> uint32_t { return static_cast<uint32_t>(x); });
			}
			else if (accessor.component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT)
			{
				std::vector<uint16_t> tempIndices(accessor.count);
				GetVertexData(accessor, sizeof(uint16_t), tempIndices.data());
			
				data.indices.reserve(accessor.count);
				std::transform(tempIndices.begin(), tempIndices.end(), std::back_inserter(data.indices), [](uint16_t& x) -> uint32_t { return static_cast<uint32_t>(x); });
			}
			else if (accessor.component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT)
			{
				data.indices.resize(accessor.count);
				GetVertexData(accessor, sizeof(uint16_t), data.indices.data());
			}
		}

		for(uint32_t j = 0; j < primitive.attributes_count; ++j)
		{
			const tg3_str_int_pair& attribute = primitive.attributes[j];
			const tg3_accessor& accessor = m_Storage->model.accessors[attribute.value];

			const bool isPositions	= strcmp(attribute.key.data, "POSITION") == 0;
			const bool isNormals	= strcmp(attribute.key.data, "NORMAL") == 0;
			const bool isTangents	= strcmp(attribute.key.data, "TANGENT") == 0;
			const bool isUVs		= strcmp(attribute.key.data, "TEXCOORD_0") == 0;

			if (isPositions)
			{
				data.positions.resize(accessor.count);
				GetVertexData(accessor, sizeof(glm::packed_vec3), data.positions.data());
			}
			else if (isNormals)
			{
				data.normals.resize(accessor.count);
				GetVertexData(accessor, sizeof(glm::packed_vec3), data.normals.data());
			}
			else if (isTangents)
			{
				data.tangents.resize(accessor.count);
				GetVertexData(accessor, sizeof(glm::packed_vec4), data.tangents.data());
			}
			else if (isUVs)
			{
				data.uvs.resize(accessor.count);
				GetVertexData(accessor, sizeof(glm::packed_vec2), data.uvs.data());
			}
		}

		switch (primitive.mode)
		{
			case TG3_MODE_TRIANGLES:
				mesh.primitiveType = AkPrimitiveType::TRIANGLES;
				break;
			case TG3_MODE_POINTS:
				mesh.primitiveType = AkPrimitiveType::POINTS;
				break;
			case TG3_MODE_LINE:
				mesh.primitiveType = AkPrimitiveType::LINES;
				break;
			case TG3_MODE_LINE_LOOP:
				mesh.primitiveType = AkPrimitiveType::LINE_STRIP;
				break;
			case TG3_MODE_LINE_STRIP:
				mesh.primitiveType = AkPrimitiveType::LINE_STRIP;
				break;
			case TG3_MODE_TRIANGLE_STRIP:
				mesh.primitiveType = AkPrimitiveType::TRIANGLE_STRIP;
				break;
			case TG3_MODE_TRIANGLE_FAN:
				mesh.primitiveType = AkPrimitiveType::TRIANGLE_FAN;
				break;
		}

		ValidateMesh(mesh, data);
		node.meshes.push_back(static_cast<uint32_t>(m_Meshes.size()));

		m_Meshes.push_back(mesh);
		m_MeshData.push_back(data);
	}
}

void AkGltfModelDecoder::GetVertexData(const tg3_accessor& accessor, const size_t elementSize, void* destination)
{
	const tg3_buffer_view& bufferView = m_Storage->model.buffer_views[accessor.buffer_view];
	const size_t byteStride = static_cast<size_t>(tg3_accessor_byte_stride(&accessor, &bufferView));
	const size_t fileOffset = bufferView.byte_offset + accessor.byte_offset;
	const size_t bytesToRead = byteStride * accessor.count;

	if (byteStride == elementSize)
		memcpy(destination, m_Storage->model.buffers[bufferView.buffer].data.data + fileOffset, bytesToRead);
	else
	{
		size_t writeOffset = 0;
		size_t readOffset = fileOffset;
		const size_t readCapacity = fileOffset + bytesToRead;
		while (readOffset < readCapacity)
		{
			memcpy(reinterpret_cast<uint8_t*>(destination) + writeOffset, m_Storage->model.buffers[bufferView.buffer].data.data + readOffset, elementSize);
			
			readOffset += byteStride;
			writeOffset += 1;
		}
	}
}