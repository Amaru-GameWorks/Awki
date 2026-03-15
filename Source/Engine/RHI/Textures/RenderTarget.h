#pragma once
#include "RHI/ResourceStates.h"
#include "Utilities/ForwardStorage.h"

#include <glm/vec4.hpp>

#include <vector>
#include <memory>
#include <optional>

namespace vk
{
	struct RenderingAttachmentInfo;
};

union AkClearColorValue
{
	glm::vec4 fColor = { 0.f, 0.f, 0.f, 0.f };
	glm::ivec4 iColor;
	glm::uvec4 uColor;
};

struct AkClearDepthStencilValue
{
	float depth = 0.f;
	uint32_t stencil = 0;
};

union AkClearValue
{
	AkClearColorValue color = {};
	AkClearDepthStencilValue depthStencil;
};

enum class AkLoadOperation
{
	LOAD,
	CLEAR,
	DONT_CARE
};

enum class AkStoreOperation
{
	STORE,
	DONT_CARE
};

struct AkRenderTargetAttachmentInfo
{
	uint32_t mip = 0;
	uint32_t slice = 0;
	AkClearValue clearColor = {};
	class AkTexture* texture = nullptr;

	AkResourceState state = AkResourceState::UNDEFINED;
	AkLoadOperation loadOperation = AkLoadOperation::DONT_CARE;
	AkStoreOperation storeOperation = AkStoreOperation::STORE;
};

class AkRenderTarget
{
public:
	AkRenderTarget(std::optional<std::vector<AkRenderTargetAttachmentInfo>> colorAttachments, std::optional<AkRenderTargetAttachmentInfo> depthStencilAttachment);
	~AkRenderTarget();

	class AkTexture* GetColorTexture(const uint32_t index = 0) const;
	class AkTexture* GetDepthStencilTexture() const;
	const struct AkTextureDescriptor& GetValidTextureDescriptor() const;

	const std::vector<AkRenderTargetAttachmentInfo>& GetColorAttachmentInfos() const { return m_ColorAttachments; }
	const std::optional<AkRenderTargetAttachmentInfo>& GetDepthStencilAttachmentInfo() const { return m_DepthStencilAttachment; }

	const std::vector<struct vk::RenderingAttachmentInfo>& GetColorAttachments() const;
	const std::optional<struct vk::RenderingAttachmentInfo>& GetDepthStencilAttachment() const;

private:
	ForwardStorage<struct AkRenderTargetStorage, 112> m_Storage;
	
	std::vector<AkRenderTargetAttachmentInfo> m_ColorAttachments = {};
	std::optional<AkRenderTargetAttachmentInfo> m_DepthStencilAttachment = std::nullopt;
};