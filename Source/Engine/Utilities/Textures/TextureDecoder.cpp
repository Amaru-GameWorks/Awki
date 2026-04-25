#include "TextureDecoder.h"
#include "StbTextureDecoder.h"
#include "Utilities/Hash.h"

std::unique_ptr<AkTextureDecoderInterface> AkTextureDecoder::Decode(const std::filesystem::path& path)
{
	std::string extension = path.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	switch (FNV1aHash(extension))
	{
		case FNV1aHash(".jpg"):
		case FNV1aHash(".jpeg"):
		case FNV1aHash(".png"):
		case FNV1aHash(".bmp"):
		case FNV1aHash(".psd"):
		case FNV1aHash(".tga"):
		case FNV1aHash(".gif"):
		case FNV1aHash(".hdr"):
		case FNV1aHash(".pic"):
		case FNV1aHash(".ppm"):
		case FNV1aHash(".pgm"):
			return std::make_unique<AkStbTextureDecoder>(path);
	}

	throw std::runtime_error("Texture format not supported!");
}
