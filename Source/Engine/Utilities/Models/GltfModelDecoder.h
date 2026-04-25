#pragma once
#include "ModelDecoder.h"
#include "Utilities/ForwardStorage.h"

class AkGltfModelDecoder : public AkModelDecoderInterface
{
public:
	AkGltfModelDecoder(const std::filesystem::path& path);
	~AkGltfModelDecoder();

private:
	ForwardStorage<struct AkGltfModelStorage, 472> m_Storage;

	void ProcessNode(const struct tg3_node& nodeInfo, AkModelNode& node);
	void ProcessMesh(const struct tg3_mesh& meshInfo, AkModelNode& node);
	void GetVertexData(const struct tg3_accessor& accessor, const size_t elementSize, void* destination);
};