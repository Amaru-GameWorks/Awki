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
	std::byte* GetData() { return m_Data; }

protected:
	std::byte* m_Data = nullptr;
	std::filesystem::path m_Path = {};
	AkTextureDescriptor m_Descriptor = {};
};

class AkTextureDecoder
{
public:
	static std::unique_ptr<AkTextureDecoderInterface> Decode(const std::filesystem::path& path);
};