#include "PipelineStateObject.h"

#include "RHI/Device.h"
#include "RHI/Pipeline/Shader.h"
#include "RHI/Textures/Texture.h"
#include "RHI/Pipeline/Material.h"
#include "RHI/Textures/RenderTarget.h"
#include "RHI/Pipeline/RasterizerState.h"

#include <vulkan/vulkan.hpp>

extern vk::Format GetFormat(const AkPixelFormat format);
extern vk::SampleCountFlagBits GetMSAA(const AkMSAA msaa);

constexpr vk::PrimitiveTopology GetPrimitiveTopology(const AkPrimitiveType primitiveType)
{
	switch (primitiveType)
	{
		case AkPrimitiveType::LINES:
			return vk::PrimitiveTopology::eLineList;

		case AkPrimitiveType::LINE_STRIP:
			return vk::PrimitiveTopology::eLineStrip;
		
		case AkPrimitiveType::PATCH_LIST:
			return vk::PrimitiveTopology::ePatchList;
		
		case AkPrimitiveType::POINTS:
			return vk::PrimitiveTopology::ePointList;
		
		case AkPrimitiveType::TRIANGLES:
			return vk::PrimitiveTopology::eTriangleList;
		
		case AkPrimitiveType::TRIANGLE_FAN:
			return vk::PrimitiveTopology::eTriangleFan;
		
		case AkPrimitiveType::TRIANGLE_STRIP:
			return vk::PrimitiveTopology::eTriangleStrip;

		default:
			AkLogCritical("Primitive topology not registered in this function");
			return vk::PrimitiveTopology::eTriangleList;
	}
}

constexpr vk::BlendOp GetBlendOperation(const AkBlendingOperation blendOperation)
{
	switch (blendOperation)
	{
		case AkBlendingOperation::ADD:
			return vk::BlendOp::eAdd;

		case AkBlendingOperation::SUBSTRACT:
			return vk::BlendOp::eSubtract;

		case AkBlendingOperation::REVERSE_SUBSTRACT:
			return vk::BlendOp::eReverseSubtract;

		case AkBlendingOperation::MIN:
			return vk::BlendOp::eMin;

		case AkBlendingOperation::MAX:
			return vk::BlendOp::eMax;

		default:
			AkLogCritical("Blend Operation not registered in this function");
			return vk::BlendOp::eAdd;
	}
}

constexpr vk::BlendFactor GetBlendFactor(const AkBlendingFactor blendFactor)
{
	switch (blendFactor)
	{
		case AkBlendingFactor::ONE:
			return vk::BlendFactor::eOne;

		case AkBlendingFactor::ZERO:
			return vk::BlendFactor::eZero;

		case AkBlendingFactor::SRC_COLOR:
			return vk::BlendFactor::eSrcColor;

		case AkBlendingFactor::ONE_MINUS_SRC_COLOR:
			return vk::BlendFactor::eOneMinusSrcColor;

		case AkBlendingFactor::DST_COLOR:
			return vk::BlendFactor::eDstColor;

		case AkBlendingFactor::ONE_MINUS_DST_COLOR:
			return vk::BlendFactor::eOneMinusDstColor;

		case AkBlendingFactor::SRC_ALPHA:
			return vk::BlendFactor::eSrcAlpha;

		case AkBlendingFactor::ONE_MINUS_SRC_ALPHA:
			return vk::BlendFactor::eOneMinusSrcAlpha;

		case AkBlendingFactor::DST_ALPHA:
			return vk::BlendFactor::eDstAlpha;

		case AkBlendingFactor::ONE_MINUS_DST_ALPHA:
			return vk::BlendFactor::eOneMinusDstAlpha;
		
		default:
			AkLogCritical("Blend Factor not registered in this function");
			return vk::BlendFactor::eOne;
	}
}

constexpr vk::CullModeFlags GetCullMode(const AkFaceCulling cullMode)
{
	switch (cullMode)
	{
		case AkFaceCulling::OFF:
			return vk::CullModeFlagBits::eNone;

		case AkFaceCulling::BACK:
			return vk::CullModeFlagBits::eBack;

		case AkFaceCulling::FRONT:
			return vk::CullModeFlagBits::eFront;

		case AkFaceCulling::FRONT_BACK:
			return vk::CullModeFlagBits::eFrontAndBack;

		default:
			AkLogCritical("Face Culling not registered in this function");
			return vk::CullModeFlagBits::eNone;
	}
}

constexpr vk::FrontFace GetFrontFace(const AkWindingOrder windingOrder)
{
	switch (windingOrder)
	{
		case AkWindingOrder::CLOCK_WISE:
			return vk::FrontFace::eClockwise;

		case AkWindingOrder::COUNTER_CLOCK_WISE:
			return vk::FrontFace::eCounterClockwise;

		default:
			AkLogCritical("Winding Order not registered in this function");
			return vk::FrontFace::eClockwise;
	}
}

constexpr vk::PolygonMode GetPolygonMode(const AkFillMode fillMode)
{
	switch (fillMode)
	{
		case AkFillMode::SOLID:
			return vk::PolygonMode::eFill;

		case AkFillMode::LINE:
			return vk::PolygonMode::eLine;

		case AkFillMode::POINT:
			return vk::PolygonMode::ePoint;
		
		default:
			AkLogCritical("Fill Mode not registered in this function");
			return vk::PolygonMode::eFill;
	}
}

constexpr vk::CompareOp GetDepthTestOperation(const AkDepthTest depthTest)
{
	switch (depthTest)
	{
		case AkDepthTest::OFF:
		case AkDepthTest::NEVER:
			return vk::CompareOp::eNever;

		case AkDepthTest::ALWAYS:
			return vk::CompareOp::eAlways;

		case AkDepthTest::LESS:
			return vk::CompareOp::eLess;

		case AkDepthTest::GREATER:
			return vk::CompareOp::eGreater;

		case AkDepthTest::EQUAL:
			return vk::CompareOp::eEqual;

		case AkDepthTest::NOT_EQUAL:
			return vk::CompareOp::eNotEqual;

		case AkDepthTest::LEQUAL:
			return vk::CompareOp::eLessOrEqual;

		case AkDepthTest::GEQUAL:
			return vk::CompareOp::eGreaterOrEqual;
		
		default:
			AkLogCritical("Depth Test not registered in this function");
			return vk::CompareOp::eNever;
	}
}

struct AkPipelineStorage
{
	vk::Pipeline pipeline;
};

AkPipelineStateObject::AkPipelineStateObject(AkMaterial* material, const AkPrimitiveType primitiveType, AkRenderTarget* renderTarget)
{
	AkShader* shader = material->GetShader();
	const AkRasterizerState& rasterizerState = material->GetRasterizerState();

	const vk::Device& device = AkDevice::GetDevice();
	const vk::ShaderModule& shaderModule = shader->GetModule();
	const vk::PipelineLayout& pipelineLayout = shader->GetPipelineLayout();

	const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages =
	{
		{.stage = vk::ShaderStageFlagBits::eVertex, .module = shaderModule, .pName = "main" },
		{.stage = vk::ShaderStageFlagBits::eFragment, .module = shaderModule, .pName = "main" }
	};

	const vk::PipelineVertexInputStateCreateInfo vertexInputState = {};
	const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState = { .topology = GetPrimitiveTopology(primitiveType) };
	const std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	const vk::PipelineDynamicStateCreateInfo dynamicState = { .dynamicStateCount = 2, .pDynamicStates = dynamicStates.data() };

	const vk::PipelineViewportStateCreateInfo viewportState = { .viewportCount = 1, .scissorCount = 1 };
	const vk::PipelineRasterizationStateCreateInfo rasterizationState =
	{
		.polygonMode = GetPolygonMode(rasterizerState.fillMode),
		.cullMode = GetCullMode(rasterizerState.faceCulling),
		.frontFace = GetFrontFace(rasterizerState.windingOrder),
		.lineWidth = rasterizerState.lineWidth,
	};

	const vk::PipelineDepthStencilStateCreateInfo depthStencilState =
	{
		.depthTestEnable = rasterizerState.depthTest != AkDepthTest::OFF,
		.depthWriteEnable = rasterizerState.depthWrite,
		.depthCompareOp = GetDepthTestOperation(rasterizerState.depthTest)
	};

	vk::PipelineMultisampleStateCreateInfo multisampleState = { .rasterizationSamples = vk::SampleCountFlagBits::e1 };
	const std::vector<AkRenderTargetAttachmentInfo>& colorAttachmentsInfo = renderTarget->GetColorAttachmentInfos();
	const std::optional<AkRenderTargetAttachmentInfo>& depthStencilAttachmentInfo = renderTarget->GetDepthStencilAttachmentInfo();

	vk::PipelineColorBlendStateCreateInfo colorBlendState = {};
	vk::PipelineRenderingCreateInfo piepelineRenderingCreateInfo = {};

	std::vector<vk::Format> colorFormats = {};
	std::vector<vk::PipelineColorBlendAttachmentState> colorAttachmentBlendModes = {};

	if (!colorAttachmentsInfo.empty())
	{
		multisampleState.rasterizationSamples = GetMSAA(colorAttachmentsInfo[0].texture->GetDescriptor().msaa);

		colorFormats.reserve(colorAttachmentsInfo.size());
		colorAttachmentBlendModes.reserve(colorAttachmentsInfo.size());

		for (size_t i = 0; i < colorAttachmentsInfo.size(); ++i)
		{
			const AkRenderTargetAttachmentInfo& attachmentInfo = colorAttachmentsInfo[i];
			const AkTextureDescriptor& textureDescriptor = attachmentInfo.texture->GetDescriptor();
			colorFormats.emplace_back(GetFormat(textureDescriptor.format));

			vk::PipelineColorBlendAttachmentState& colorBlendAttachmentState = colorAttachmentBlendModes.emplace_back();
			const AkBlendState& blendState = i < rasterizerState.blendStates.size() ? rasterizerState.blendStates[i] : AkBlendState::GetDefault();

			colorBlendAttachmentState.blendEnable = blendState.blendEnabled;
			colorBlendAttachmentState.colorWriteMask = static_cast<vk::ColorComponentFlagBits>(blendState.colorMask);

			if (colorBlendAttachmentState.blendEnable)
			{
				colorBlendAttachmentState.srcColorBlendFactor = GetBlendFactor(blendState.blendFactors.sourceColor);
				colorBlendAttachmentState.srcAlphaBlendFactor = GetBlendFactor(blendState.blendFactors.sourceAlpha);
				colorBlendAttachmentState.dstColorBlendFactor = GetBlendFactor(blendState.blendFactors.destinationColor);
				colorBlendAttachmentState.dstAlphaBlendFactor = GetBlendFactor(blendState.blendFactors.destinationAlpha);

				colorBlendAttachmentState.colorBlendOp = GetBlendOperation(blendState.blendOperations.color);
				colorBlendAttachmentState.alphaBlendOp = GetBlendOperation(blendState.blendOperations.alpha);
			}
		}

		piepelineRenderingCreateInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentsInfo.size());
		piepelineRenderingCreateInfo.pColorAttachmentFormats = colorFormats.data();

		colorBlendState.attachmentCount = static_cast<uint32_t>(colorAttachmentBlendModes.size());
		colorBlendState.pAttachments = colorAttachmentBlendModes.data();
	};

	if (depthStencilAttachmentInfo.has_value())
	{
		const AkTextureDescriptor& depthTextureDescriptor = depthStencilAttachmentInfo->texture->GetDescriptor();
		piepelineRenderingCreateInfo.depthAttachmentFormat = GetFormat(depthTextureDescriptor.format);
	}

	vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo =
	{
		.pNext = &piepelineRenderingCreateInfo,
		.stageCount = 2,
		.pStages = shaderStages.data(),
		.pVertexInputState = &vertexInputState,
		.pInputAssemblyState = &inputAssemblyState,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizationState,
		.pMultisampleState = &multisampleState,
		.pDepthStencilState = &depthStencilState,
		.pColorBlendState = &colorBlendState,
		.pDynamicState = &dynamicState,
		.layout = pipelineLayout
	};

	const vk::ResultValue result = device.createGraphicsPipeline(VK_NULL_HANDLE, graphicsPipelineCreateInfo);
	if (result.has_value())
		m_Storage->pipeline = result.value;
}

AkPipelineStateObject::~AkPipelineStateObject()
{
	const vk::Device& device = AkDevice::GetDevice();
	device.destroyPipeline(m_Storage->pipeline);
}

const vk::Pipeline& AkPipelineStateObject::GetPipeline() const
{
	return m_Storage->pipeline;
}
