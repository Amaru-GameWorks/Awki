#include "StbTextureDecoder.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

AkStbTextureDecoder::AkStbTextureDecoder(const std::filesystem::path& path)
	: AkTextureDecoderInterface(path)
{
	int32_t width = 0;
	int32_t height = 0;
	int32_t channels = 0;
	std::string pathString = m_Path.string();
	const char* texturePath = pathString.c_str();

	if (stbi_info(texturePath, &width, &height, &channels))
	{
		if (channels == 3)
			channels = 4;

		m_Descriptor.width = width;
		m_Descriptor.height = height;

		const bool isHDR = static_cast<bool>(stbi_is_hdr(texturePath));
		const bool is16Bit = static_cast<bool>(stbi_is_16_bit(texturePath));

		if (isHDR)
		{
			if (channels == 1)
				m_Descriptor.format = AkPixelFormat::R32_FLOAT;
			else if (channels == 2)
				m_Descriptor.format = AkPixelFormat::RG32_FLOAT;
			else
				m_Descriptor.format = AkPixelFormat::RGBA32_FLOAT;

			if (float* data = stbi_loadf(texturePath, &width, &height, nullptr, channels))
				m_Data = reinterpret_cast<uint8_t*>(data);
			else
			{
				const std::string errorString = std::format("Failed to decode texture at path '{}' {}", pathString, stbi_failure_reason());
				throw std::runtime_error(errorString);
			}
		}
		else if (is16Bit)
		{
			if (channels == 1)
				m_Descriptor.format = AkPixelFormat::R16_FLOAT;
			else if (channels == 2)
				m_Descriptor.format = AkPixelFormat::RG16_FLOAT;
			else
				m_Descriptor.format = AkPixelFormat::RGBA16_FLOAT;

			if (stbi_us* data = stbi_load_16(texturePath, &width, &height, nullptr, channels))
				m_Data = reinterpret_cast<uint8_t*>(data);
			else
			{
				const std::string errorString = "Failed to decode texture at path '" + pathString + "' : " + std::string(stbi_failure_reason());
				throw std::runtime_error(errorString);
			}
		}
		else
		{
			if (channels == 1)
				m_Descriptor.format = AkPixelFormat::R8_UNORM;
			else if (channels == 2)
				m_Descriptor.format = AkPixelFormat::RG8_UNORM;
			else
				m_Descriptor.format = AkPixelFormat::RGBA8_UNORM;

			if (stbi_uc* data = stbi_load(texturePath, &width, &height, nullptr, channels))
				m_Data = data;
			else
			{
				const std::string errorString = "Failed to decode texture at path '" + pathString + "' : " + std::string(stbi_failure_reason());
				throw std::runtime_error(errorString);
			}
		}
	}
	else
	{
		const std::string errorString = "Failed to decode texture at path '"+ pathString + "' : " + std::string(stbi_failure_reason());
		throw std::runtime_error(errorString);
	}
}

AkStbTextureDecoder::~AkStbTextureDecoder()
{
	stbi_image_free(m_Data);
}
