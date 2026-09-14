#pragma once
#include "TextureDecoder.h"

class AkStbTextureDecoder : public AkTextureDecoderInterface
{
public:
	AkStbTextureDecoder(const std::filesystem::path& path);
	~AkStbTextureDecoder();
};