#pragma once 
#include <vector>
#include <type_traits>

enum class AkFillMode : uint8_t
{
	SOLID,
	LINE,
	POINT
};

enum class AkWindingOrder : uint8_t
{
	CLOCK_WISE,
	COUNTER_CLOCK_WISE
};

enum class AkFaceCulling : uint8_t
{
	OFF,
	FRONT,
	BACK,
	FRONT_BACK
};

enum class AkDepthTest : uint8_t
{
	OFF,
	NEVER,
	LESS,
	GREATER,
	EQUAL,
	ALWAYS,
	LEQUAL,
	GEQUAL,
	NOT_EQUAL
};

enum class AkBlendingFactor : uint8_t
{
	ONE,
	ZERO,
	SRC_COLOR,
	ONE_MINUS_SRC_COLOR,
	DST_COLOR,
	ONE_MINUS_DST_COLOR,
	SRC_ALPHA,
	ONE_MINUS_SRC_ALPHA,
	DST_ALPHA,
	ONE_MINUS_DST_ALPHA
};

enum class AkBlendingOperation : uint8_t
{
	ADD,
	SUBSTRACT,
	REVERSE_SUBSTRACT,
	MIN,
	MAX
};

enum AkColorMaskBits : uint8_t
{
	AkColorMask_R = 1 << 0,
	AkColorMask_G = 1 << 1,
	AkColorMask_B = 1 << 2,
	AkColorMask_A = 1 << 3
};
using AkColorMask = std::underlying_type_t<AkColorMaskBits>;

struct AkBlendFactors
{
	AkBlendingFactor sourceColor = AkBlendingFactor::SRC_ALPHA;
	AkBlendingFactor destinationColor = AkBlendingFactor::ONE_MINUS_SRC_ALPHA;

	AkBlendingFactor sourceAlpha = AkBlendingFactor::SRC_ALPHA;
	AkBlendingFactor destinationAlpha = AkBlendingFactor::ONE_MINUS_SRC_ALPHA;
};

struct AkBlendOperations
{
	AkBlendingOperation color = AkBlendingOperation::ADD;
	AkBlendingOperation alpha = AkBlendingOperation::ADD;
};

struct AkBlendState
{
	bool blendEnabled = true;
	AkBlendFactors blendFactors = {};
	AkBlendOperations blendOperations = {};
	AkColorMask colorMask = AkColorMask_R | AkColorMask_G | AkColorMask_B | AkColorMask_A;

	static const AkBlendState& GetDefault()
	{
		static AkBlendState kDefault;
		return kDefault;
	};
};

struct AkRasterizerState
{
	float lineWidth = 1.f;
	bool depthWrite = true;
	std::vector<AkBlendState> blendStates = {};

	AkFillMode fillMode = AkFillMode::SOLID;
	AkDepthTest depthTest = AkDepthTest::LEQUAL;
	AkFaceCulling faceCulling = AkFaceCulling::BACK;
	AkWindingOrder windingOrder = AkWindingOrder::COUNTER_CLOCK_WISE;
};