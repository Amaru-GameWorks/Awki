#include "CommandBuffer.h"
#include "Core/Assert.h"
#include "RHI/Device.h"
#include "RHI/Pipeline/Shader.h"
#include "RHI/Textures/Texture.h"
#include "RHI/Pipeline/Material.h"
#include "RHI/Textures/RenderTarget.h"
#include "RHI/Pipeline/PipelineStateObject.h"
#include "RHI/Pipeline/PipelineStateManager.h"
#include "RHI/Pipeline/BindlessResourcesManager.h"

#include <vulkan/vulkan.hpp>

extern vk::ImageLayout GetImageLayout(const AkResourceState resourceState)
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

constexpr vk::ImageAspectFlags GetAspectMask(const AkPixelFormat format)
{
	if (IsDepthPixelFormat(format))
		return vk::ImageAspectFlagBits::eDepth;
	else
		return vk::ImageAspectFlagBits::eColor;
}

constexpr vk::AccessFlags2 GetAccessMask(const AkResourceState resourceState)
{
	switch (resourceState)
	{
		case AkResourceState::UNDEFINED:			return vk::AccessFlagBits2::eNone;
		case AkResourceState::INDEX_BUFFER:			return vk::AccessFlagBits2::eIndexRead;
		case AkResourceState::VERTEX_BUFFER: 		return vk::AccessFlagBits2::eVertexAttributeRead;
		case AkResourceState::CONSTANT_BUFFER:		return vk::AccessFlagBits2::eUniformRead;
		case AkResourceState::RENDER_TARGET:		return vk::AccessFlagBits2::eColorAttachmentRead | vk::AccessFlagBits2::eColorAttachmentWrite;
		case AkResourceState::UNORDERED_ACCESS:		return vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite;
		case AkResourceState::DEPTH_READ:			return vk::AccessFlagBits2::eDepthStencilAttachmentRead;
		case AkResourceState::DEPTH_WRITE:			return vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
		case AkResourceState::SHADER_RESOURCE:		return vk::AccessFlagBits2::eShaderRead;
		case AkResourceState::INDIRECT_ARGUMENT:	return vk::AccessFlagBits2::eIndirectCommandRead;
		case AkResourceState::COPY_DESTINATION:		return vk::AccessFlagBits2::eTransferWrite;
		case AkResourceState::COPY_SOURCE:			return vk::AccessFlagBits2::eTransferRead;
		case AkResourceState::PRESENT:				return vk::AccessFlagBits2::eMemoryRead;

		default:
			AkLogCritical("Resource state not registered in this function");
			return vk::AccessFlagBits2::eNone;
	}
}

constexpr vk::PipelineStageFlags2 GetPipelineStageFlags(const AkResourceState resourceState)
{
	switch (resourceState)
	{
		case AkResourceState::UNDEFINED:
			return vk::PipelineStageFlagBits2::eTopOfPipe;

		case AkResourceState::INDEX_BUFFER:
		case AkResourceState::VERTEX_BUFFER:
			return vk::PipelineStageFlagBits2::eVertexInput;

		case AkResourceState::SHADER_RESOURCE:
		case AkResourceState::CONSTANT_BUFFER:
		case AkResourceState::UNORDERED_ACCESS:
			return	vk::PipelineStageFlagBits2::eVertexShader |
				vk::PipelineStageFlagBits2::eFragmentShader |
				vk::PipelineStageFlagBits2::eGeometryShader |
				vk::PipelineStageFlagBits2::eComputeShader |
				vk::PipelineStageFlagBits2::eTessellationControlShader |
				vk::PipelineStageFlagBits2::eTessellationEvaluationShader;

		case AkResourceState::RENDER_TARGET:
			return vk::PipelineStageFlagBits2::eColorAttachmentOutput;

		case AkResourceState::DEPTH_READ:
		case AkResourceState::DEPTH_WRITE:
			return	vk::PipelineStageFlagBits2::eEarlyFragmentTests |
				vk::PipelineStageFlagBits2::eLateFragmentTests;

		case AkResourceState::INDIRECT_ARGUMENT:
			return vk::PipelineStageFlagBits2::eDrawIndirect;

		case AkResourceState::COPY_SOURCE:
		case AkResourceState::COPY_DESTINATION:
			return vk::PipelineStageFlagBits2::eTransfer;

		case AkResourceState::PRESENT:
			return vk::PipelineStageFlagBits2::eBottomOfPipe;

		default:
			AkLogCritical("Resource state not registered in this function");
			return vk::PipelineStageFlagBits2::eNone;
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

void AkCommandBuffer::BeginRendering(AkRenderTarget* renderTarget)
{
	m_CurrentRenderTarget = renderTarget;
	const AkTextureDescriptor& descriptor = renderTarget->GetValidTextureDescriptor();
	const std::vector<vk::RenderingAttachmentInfo> colorAttachments = renderTarget->GetColorAttachments();
	const std::optional<vk::RenderingAttachmentInfo> depthAttachment = renderTarget->GetDepthStencilAttachment();

	vk::RenderingInfo renderingInfo = 
	{
		.renderArea = 
		{
			.extent = 
			{
				.width = descriptor.width, 
				.height = descriptor.height 
			}
		},
		.layerCount = descriptor.depth,
		.colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
		.pColorAttachments = renderingInfo.colorAttachmentCount ? colorAttachments.data() : nullptr,
		.pDepthAttachment = depthAttachment.has_value() ? &depthAttachment.value() : nullptr
	};


	const vk::Viewport viewport = 
	{ 
		.width = static_cast<float>(descriptor.width),
		.height = static_cast<float>(descriptor.height),
		.minDepth = 0.0f, 
		.maxDepth = 1.0f 
	};

	const vk::Rect2D scissor = 
	{ 
		.extent = 
		{
			.width = static_cast<uint32_t>(descriptor.width),
			.height = static_cast<uint32_t>(descriptor.height)
		}
	};
	
	m_Storage->commandBuffer.setViewport(0, 1, &viewport);
	m_Storage->commandBuffer.setScissor(0, 1, &scissor);
	m_Storage->commandBuffer.beginRendering(renderingInfo);
}

void AkCommandBuffer::EndRendering()
{
	m_CurrentRenderTarget = nullptr;
	m_Storage->commandBuffer.endRendering();
}

void AkCommandBuffer::DrawPrimitive(AkMaterial* material, const AkPrimitiveType primitiveType, const uint32_t vertexCount)
{
	AkShader* shader = material->GetShader();
	AkPipelineStateObject* pso = AkPipelineStateManager::GetPipelineStateObject(material, m_CurrentRenderTarget, primitiveType);

	const std::vector<vk::DescriptorSet> descriptorSets =
	{
		AkBindlessResourcesManager::GetBuffersDescriptorSet(),
		AkBindlessResourcesManager::GetTexturesDescriptorSet(),
		AkBindlessResourcesManager::GetSamplersDescriptorSet(),
		material->GetDescriptorSet()
	};

	m_Storage->commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pso->GetPipeline());
	m_Storage->commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, shader->GetPipelineLayout(), 0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
	m_Storage->commandBuffer.draw(vertexCount, 1, 0, 0);
}

void AkCommandBuffer::TransitionRenderTargetColorAttachments(AkRenderTarget* renderTarget, const AkResourceState sourceState, const AkResourceState destinationState)
{
	const std::vector<AkRenderTargetAttachmentInfo>& colorAttachmentsInfo = renderTarget->GetColorAttachmentInfos();
	if (colorAttachmentsInfo.empty())
		return;

	std::vector<vk::ImageMemoryBarrier2> imageMemoryBarriers = {};
	imageMemoryBarriers.reserve(colorAttachmentsInfo.size());

	for (auto const& attachmentInfo : colorAttachmentsInfo)
	{
		const AkTextureDescriptor& descriptor = attachmentInfo.texture->GetDescriptor();
		const vk::ImageMemoryBarrier2 imageMemoryBarrier =
		{
			.srcStageMask = GetPipelineStageFlags(sourceState),
			.srcAccessMask = GetAccessMask(sourceState),
			.dstStageMask = GetPipelineStageFlags(destinationState),
			.dstAccessMask = GetAccessMask(destinationState),
			.oldLayout = GetImageLayout(sourceState),
			.newLayout = GetImageLayout(destinationState),
			.image = attachmentInfo.texture->GetImage(),
			.subresourceRange =
			{
				.aspectMask = GetAspectMask(descriptor.format),
				.levelCount = descriptor.mips,
				.layerCount = descriptor.slices
			}
		};
		imageMemoryBarriers.push_back(imageMemoryBarrier);
	}

	const vk::DependencyInfo barrierDependencyInfo =
	{
		.imageMemoryBarrierCount = static_cast<uint32_t>(imageMemoryBarriers.size()),
		.pImageMemoryBarriers = imageMemoryBarriers.data()
	};

	m_Storage->commandBuffer.pipelineBarrier2(barrierDependencyInfo);
}

void AkCommandBuffer::TransitionRenderTargetDepthAttachment(AkRenderTarget* renderTarget, const AkResourceState sourceState, const AkResourceState destinationState)
{
	const std::optional<AkRenderTargetAttachmentInfo>& depthStencilAttachmentInfo = renderTarget->GetDepthStencilAttachmentInfo();
	if(depthStencilAttachmentInfo.has_value())
		TransitionTexture(depthStencilAttachmentInfo->texture, sourceState, destinationState);
}

void AkCommandBuffer::TransitionTexture(AkTexture* texture, const AkResourceState sourceState, const AkResourceState destinationState)
{
	const AkTextureDescriptor& descriptor = texture->GetDescriptor();

	const vk::ImageMemoryBarrier2 imageMemoryBarrier =
	{
		.srcStageMask = GetPipelineStageFlags(sourceState),
		.srcAccessMask = GetAccessMask(sourceState),
		.dstStageMask = GetPipelineStageFlags(destinationState),
		.dstAccessMask = GetAccessMask(destinationState),
		.oldLayout = GetImageLayout(sourceState),
		.newLayout = GetImageLayout(destinationState),
		.image = texture->GetImage(),
		.subresourceRange =
		{
			.aspectMask = GetAspectMask(descriptor.format),
			.levelCount = descriptor.mips,
			.layerCount = descriptor.slices
		}
	};

	const vk::DependencyInfo barrierDependencyInfo =
	{
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &imageMemoryBarrier
	};

	m_Storage->commandBuffer.pipelineBarrier2(barrierDependencyInfo);
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
	const vk::ClearColorValue clearColor = { color.x, color.y, color.z, color.w };
	m_Storage->commandBuffer.clearColorImage(texture->GetImage(), currentLayout, clearColor, subResourceRange);
}

vk::CommandBuffer& AkCommandBuffer::GetBuffer()
{
	return m_Storage->commandBuffer;
}