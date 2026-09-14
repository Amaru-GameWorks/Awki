#pragma once
#include "RHI/Textures/Texture.h"

#include <filesystem>

class AkTextureDecoderInterface
{
public:
	AkTextureDecoderInterface(const std::filesystem::path& path)
		: m_Path(path) 
	{ }
	virtual ~AkTextureDecoderInterface() = default;

	AkTextureDescriptor GetDescriptor() { return m_Descriptor; }
	uint8_t* GetData() { return m_Data; }

protected:
	uint8_t* m_Data = nullptr;
	std::filesystem::path m_Path = {};
	AkTextureDescriptor m_Descriptor = {};
};

class AkTextureDecoder
{
public:
	static std::unique_ptr<AkTextureDecoderInterface> Decode(const std::filesystem::path& path);
};