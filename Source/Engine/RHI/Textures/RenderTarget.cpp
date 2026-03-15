#include "RenderTarget.h"
#include "Texture.h"

#include <vulkan/vulkan.hpp>

extern vk::ImageLayout GetImageLayout(const AkResourceState resourceState);

constexpr vk::AttachmentLoadOp GetLoadOperation(const AkLoadOperation& loadOp)
{
	switch (loadOp)
	{
		case AkLoadOperation::LOAD:			return vk::AttachmentLoadOp::eLoad;
		case AkLoadOperation::CLEAR:		return vk::AttachmentLoadOp::eClear;
		case AkLoadOperation::DONT_CARE:	return vk::AttachmentLoadOp::eDontCare;
		default:
			AkLogCritical("Load Operation not registered in this function");
			return vk::AttachmentLoadOp::eDontCare;
	}
}

constexpr vk::AttachmentStoreOp GetStoreOperation(const AkStoreOperation& storeOp)
{
	switch (storeOp)
	{
		case AkStoreOperation::STORE:		return vk::AttachmentStoreOp::eStore;
		case AkStoreOperation::DONT_CARE:	return vk::AttachmentStoreOp::eDontCare;
		default:
			AkLogCritical("Store Operation not registered in this function");
			return vk::AttachmentStoreOp::eDontCare;
	}
}

struct AkRenderTargetStorage
{
	std::vector<vk::RenderingAttachmentInfo> colorAttachments;
	std::optional<vk::RenderingAttachmentInfo> depthAttachment;
};

AkRenderTarget::AkRenderTarget(const AkRenderTargetAttachmentInfo& depthStencilAttachment)
	: AkRenderTarget(std::nullopt, depthStencilAttachment)
{ }

AkRenderTarget::AkRenderTarget(const std::vector<AkRenderTargetAttachmentInfo>& colorAttachments)
	: AkRenderTarget(colorAttachments, std::nullopt)
{ }

AkRenderTarget::AkRenderTarget(std::optional<std::vector<AkRenderTargetAttachmentInfo>> colorAttachments, std::optional<AkRenderTargetAttachmentInfo> depthStencilAttachment)
{
	if (colorAttachments.has_value())
	{
		m_ColorAttachments = std::move(colorAttachments.value());
		m_Storage->colorAttachments.reserve(m_ColorAttachments.size());

		for (const auto& colorAttachment : m_ColorAttachments)
		{
			m_Storage->colorAttachments.push_back({
				.imageView = colorAttachment.texture->GetImageView(colorAttachment.mip, colorAttachment.slice),
				.imageLayout = GetImageLayout(colorAttachment.state),
				.loadOp = GetLoadOperation(colorAttachment.loadOperation),
				.storeOp = GetStoreOperation(colorAttachment.storeOperation),
				.clearValue = {{colorAttachment.clearColor.color.fColor.x, colorAttachment.clearColor.color.fColor.y, colorAttachment.clearColor.color.fColor.z, colorAttachment.clearColor.color.fColor.w}}
			});
		}
	}

	if (depthStencilAttachment.has_value())
	{
		m_DepthStencilAttachment = std::move(depthStencilAttachment);
		m_Storage->depthAttachment =
		{
			.imageView = m_DepthStencilAttachment->texture->GetImageView(0, 0),
			.imageLayout = GetImageLayout(m_DepthStencilAttachment->state),
			.loadOp = GetLoadOperation(m_DepthStencilAttachment->loadOperation),
			.storeOp = GetStoreOperation(m_DepthStencilAttachment->storeOperation),
			.clearValue = {{m_DepthStencilAttachment->clearColor.depthStencil.depth, m_DepthStencilAttachment->clearColor.depthStencil.stencil}}
		};
	}
}

AkRenderTarget::~AkRenderTarget()
{
}

AkTexture* AkRenderTarget::GetColorTexture(const uint32_t index) const
{
	if (m_ColorAttachments.empty())
		return nullptr;

	AkAssert(index < m_ColorAttachments.size(), "Color attachment index out of range");
	return m_ColorAttachments[index].texture;
}

AkTexture* AkRenderTarget::GetDepthStencilTexture() const
{
	if (m_DepthStencilAttachment.has_value())
		return m_DepthStencilAttachment->texture;
	return nullptr;
}

const AkTextureDescriptor& AkRenderTarget::GetValidTextureDescriptor() const
{
	if (!m_ColorAttachments.empty())
		return m_ColorAttachments.begin()->texture->GetDescriptor();
	else if(m_DepthStencilAttachment.has_value())
		return m_DepthStencilAttachment->texture->GetDescriptor();

	AkRaise("Failed to get a valid texture descriptor");
	static AkTextureDescriptor failedFunctionResult = {};
	return failedFunctionResult;
}

const std::vector<struct vk::RenderingAttachmentInfo>& AkRenderTarget::GetColorAttachments() const
{
	return m_Storage->colorAttachments;
}

const std::optional<struct vk::RenderingAttachmentInfo>& AkRenderTarget::GetDepthStencilAttachment() const
{
	return m_Storage->depthAttachment;
}
