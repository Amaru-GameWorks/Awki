#pragma once
#include "Utilities/Enum.h"

#include <type_traits>

enum AkPipelineStageFlagBits
{
	AkPipelineStageFlags_RENDER_TARGET		= 1 << 0,
	AkPipelineStageFlags_DEPTH_READ_WRITE	= 1 << 1,
	AkPipelineStageFlags_DRAW_INDIRECT		= 1 << 2,
	AkPipelineStageFlags_VERTEX_INPUT		= 1 << 3,
	AkPipelineStageFlags_VERTEX_SHADER		= 1 << 4,
	AkPipelineStageFlags_PIXEL_SHADER		= 1 << 5,
	AkPipelineStageFlags_NON_PIXEL_SHADING = 1 << 6,
	AkPipelineStageFlags_ALL_GRAPHICS		= AkInclusiveBitOr(6),

	AkPipelineStageFlags_COMPUTE_SHADER	= 1 << 7,
	AkPipelineStageFlags_TRANSFER			= 1 << 8,

	AkPipelineStageFlags_ALL_COMMANDS		= AkInclusiveBitOr(8)
};
using AkPipelineStageFlags = std::underlying_type_t<AkPipelineStageFlagBits>;

enum class AkResourceState
{
	UNDEFINED,
	INDEX_BUFFER,
	VERTEX_BUFFER,
	CONSTANT_BUFFER,
	RENDER_TARGET,
	UNORDERED_ACCESS,
	DEPTH_READ,
	DEPTH_WRITE,
	SHADER_RESOURCE,
	INDIRECT_ARGUMENT,
	COPY_DESTINATION,
	COPY_SOURCE,
	PRESENT
};