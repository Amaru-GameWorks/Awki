#include "CommandBuffer.h"
#include "Core/Assert.h"
#include "RHI/Device.h"
#include "RHI/Textures/Texture.h"

#include <vulkan/vulkan.hpp>

vk::ImageAspectFlags GetAspectMask(const AkPixelFormat format)
{
	if (IsDepthPixelFormat(format))
		return vk::ImageAspectFlagBits::eDepth;
	else
		return vk::ImageAspectFlagBits::eColor;
}

constexpr vk::ImageLayout GetImageLayout(const AkResourceState resourceState)
{
	switch (resourceState)
	{
		case AkResourceState::UNDEFINED:
			return vk::ImageLayout::eUndefined;

		case AkResourceState::RENDER_TARGET:
			return vk::ImageLayout::eColorAttachmentOptimal;

		case AkResourceState::UNORDERED_ACCESS:
			return vk::ImageLayout::eGeneral;

		case AkResourceState::DEPTH_READ:
			return vk::ImageLayout::eDepthStencilReadOnlyOptimal;

		case AkResourceState::DEPTH_WRITE:
			return vk::ImageLayout::eDepthStencilAttachmentOptimal;

		case AkResourceState::SHADER_RESOURCE:
			return vk::ImageLayout::eShaderReadOnlyOptimal;

		case AkResourceState::COPY_DESTINATION:
			return vk::ImageLayout::eTransferDstOptimal;

		case AkResourceState::COPY_SOURCE:
			return vk::ImageLayout::eTransferSrcOptimal;

		case AkResourceState::PRESENT:
			return vk::ImageLayout::ePresentSrcKHR;

		case AkResourceState::INDEX_BUFFER:
		case AkResourceState::VERTEX_BUFFER:
		case AkResourceState::CONSTANT_BUFFER:
		case AkResourceState::INDIRECT_ARGUMENT:
			AkLogCritical("Resource state is not a texture compatible state");
			return vk::ImageLayout::eUndefined;

		default:
			AkLogCritical("Resource state not registered in this function");
			return vk::ImageLayout::eUndefined;
	}
}

constexpr vk::AccessFlags GetAccessMask(const AkResourceState resourceState)
{
	switch (resourceState)
	{
		case AkResourceState::UNDEFINED:			return vk::AccessFlagBits::eNone;
		case AkResourceState::INDEX_BUFFER:			return vk::AccessFlagBits::eIndexRead;
		case AkResourceState::VERTEX_BUFFER: 		return vk::AccessFlagBits::eVertexAttributeRead;
		case AkResourceState::CONSTANT_BUFFER:		return vk::AccessFlagBits::eUniformRead;
		case AkResourceState::RENDER_TARGET:		return vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
		case AkResourceState::UNORDERED_ACCESS:		return vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
		case AkResourceState::DEPTH_READ:			return vk::AccessFlagBits::eDepthStencilAttachmentRead;
		case AkResourceState::DEPTH_WRITE:			return vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		case AkResourceState::SHADER_RESOURCE:		return vk::AccessFlagBits::eShaderRead;
		case AkResourceState::INDIRECT_ARGUMENT:	return vk::AccessFlagBits::eIndirectCommandRead;
		case AkResourceState::COPY_DESTINATION:		return vk::AccessFlagBits::eTransferWrite;
		case AkResourceState::COPY_SOURCE:			return vk::AccessFlagBits::eTransferRead;
		case AkResourceState::PRESENT:				return vk::AccessFlagBits::eMemoryRead;

		default:
			AkLogCritical("Resource state not registered in this function");
			return vk::AccessFlagBits::eNone;
	}
}

constexpr vk::PipelineStageFlags GetPipelineStageFlags(const AkResourceState resourceState)
{
	switch (resourceState)
	{
		case AkResourceState::UNDEFINED:
			return vk::PipelineStageFlagBits::eTopOfPipe;

		case AkResourceState::INDEX_BUFFER:
		case AkResourceState::VERTEX_BUFFER:
			return vk::PipelineStageFlagBits::eVertexInput;

		case AkResourceState::SHADER_RESOURCE:
		case AkResourceState::CONSTANT_BUFFER:
		case AkResourceState::UNORDERED_ACCESS:
			return	vk::PipelineStageFlagBits::eVertexShader |
				vk::PipelineStageFlagBits::eFragmentShader |
				vk::PipelineStageFlagBits::eGeometryShader |
				vk::PipelineStageFlagBits::eComputeShader |
				vk::PipelineStageFlagBits::eTessellationControlShader |
				vk::PipelineStageFlagBits::eTessellationEvaluationShader;

		case AkResourceState::RENDER_TARGET:
			return vk::PipelineStageFlagBits::eColorAttachmentOutput;

		case AkResourceState::DEPTH_READ:
		case AkResourceState::DEPTH_WRITE:
			return	vk::PipelineStageFlagBits::eEarlyFragmentTests |
				vk::PipelineStageFlagBits::eLateFragmentTests;

		case AkResourceState::INDIRECT_ARGUMENT:
			return vk::PipelineStageFlagBits::eDrawIndirect;

		case AkResourceState::COPY_SOURCE:
		case AkResourceState::COPY_DESTINATION:
			return vk::PipelineStageFlagBits::eTransfer;

		case AkResourceState::PRESENT:
			return vk::PipelineStageFlagBits::eBottomOfPipe;

		default:
			AkLogCritical("Resource state not registered in this function");
			return vk::PipelineStageFlagBits::eNone;
	}
}

struct AkCommandBufferStorage
{
	vk::CommandPool commandPool = {};
	vk::CommandBuffer commandBuffer = {};
};

AkCommandBuffer::AkCommandBuffer(const vk::CommandPool& commandPool, const vk::CommandBuffer& commandBuffer)
{
	m_Storage->commandPool = commandPool;
	m_Storage->commandBuffer = commandBuffer;
}

AkCommandBuffer::~AkCommandBuffer()
{
	const vk::Device& device = AkDevice::GetDevice();

	if (m_Storage->commandPool && m_Storage->commandBuffer)
		device.freeCommandBuffers(m_Storage->commandPool, m_Storage->commandBuffer);
}

void AkCommandBuffer::Begin()
{
	static const vk::CommandBufferBeginInfo kCommandBufferBeginInfo =
	{
		.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
	};

	m_Storage->commandBuffer.begin(kCommandBufferBeginInfo);
}

void AkCommandBuffer::End()
{
	m_Storage->commandBuffer.end();
}

void AkCommandBuffer::TransitionTexture(AkTexture* texture, const AkResourceState sourceState, const AkResourceState destinationState)
{
	const AkTextureDescriptor& descriptor = texture->GetDescriptor();
	const vk::ImageSubresourceRange subResourceRange =
	{
		.aspectMask = GetAspectMask(descriptor.format),
		.levelCount = descriptor.mips,
		.layerCount = descriptor.slices,
	};

	const vk::ImageMemoryBarrier imageMemoryBarrier =
	{
		.srcAccessMask = GetAccessMask(sourceState),
		.dstAccessMask = GetAccessMask(destinationState),
		.oldLayout = GetImageLayout(sourceState),
		.newLayout = GetImageLayout(destinationState),
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = texture->GetImage(),
		.subresourceRange = subResourceRange
	};

	const vk::PipelineStageFlags sourceStage = GetPipelineStageFlags(sourceState);
	const vk::PipelineStageFlags destinationStage = GetPipelineStageFlags(destinationState);
	m_Storage->commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void AkCommandBuffer::ClearColor(AkTexture* texture, const AkResourceState sourceState, const glm::vec4& color)
{
	AkSoftAssert(sourceState == AkResourceState::COPY_DESTINATION || sourceState == AkResourceState::UNORDERED_ACCESS, "Texture is in an invalid resource state\nOnly 'COPY_DESTINATION' and 'UNORDERED_ACCESS' are supported.");

	const AkTextureDescriptor& descriptor = texture->GetDescriptor();
	const vk::ImageSubresourceRange subResourceRange =
	{
		.aspectMask = GetAspectMask(descriptor.format),
		.levelCount = descriptor.mips,
		.layerCount = descriptor.slices
	};

	const vk::ImageLayout currentLayout = GetImageLayout(sourceState);
	const vk::ClearColorValue clearColor = { {{ color.x, color.y, color.z, color.w}} };
	m_Storage->commandBuffer.clearColorImage(texture->GetImage(), currentLayout, clearColor, subResourceRange);
}

vk::CommandBuffer& AkCommandBuffer::GetBuffer()
{
	return m_Storage->commandBuffer;
}