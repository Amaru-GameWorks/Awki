#include "Texture.h"

#include <vulkan/vulkan.hpp>

struct AkTextureStorage
{
	vk::Image image = nullptr;
};

AkTexture::AkTexture(const AkTextureDescriptor& descriptor, const vk::Image& image)
	: m_Descriptor(descriptor)
{
	m_Storage->image = image;
}

AkTexture::~AkTexture()
{
}

const vk::Image& AkTexture::GetImage()
{
	return m_Storage->image;
}