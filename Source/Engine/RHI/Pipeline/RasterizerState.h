#pragma once
#include "Utilities/Hash.h"

#include <vector>
#include <cstdint>
#include <type_traits>

enum class AkIndexType : uint8_t
{
	U16,
	U32
};

enum class AkPrimitiveType : uint8_t
{
	POINTS,
	LINES,
	LINE_STRIP,
	TRIANGLES,
	TRIANGLE_STRIP,
	TRIANGLE_FAN,
	PATCH_LIST
};

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

template <>
struct std::hash<AkBlendFactors>
{
	size_t operator()(const AkBlendFactors& blendFactors) const
	{
		size_t hash = Hash(blendFactors.sourceColor);
		HashCombine(hash, blendFactors.destinationColor);
		HashCombine(hash, blendFactors.sourceAlpha);
		HashCombine(hash, blendFactors.destinationAlpha);
		return hash;
	}
};

struct AkBlendOperations
{
	AkBlendingOperation color = AkBlendingOperation::ADD;
	AkBlendingOperation alpha = AkBlendingOperation::ADD;
};

template <>
struct std::hash<AkBlendOperations>
{
	size_t operator()(const AkBlendOperations& blendOperations) const
	{
		size_t hash = Hash(blendOperations.color);
		HashCombine(hash, blendOperations.alpha);
		return hash;
	}
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

template <>
struct std::hash<AkBlendState>
{
	size_t operator()(const AkBlendState& blendState) const
	{
		size_t hash = Hash(blendState.blendEnabled);
		HashCombine(hash, blendState.blendFactors);
		HashCombine(hash, blendState.blendOperations);
		HashCombine(hash, blendState.colorMask);
		return hash;
	}
};

struct AkRasterizerState
{
	float lineWidth = 1.f;
	bool depthWrite = true;

	AkFillMode fillMode = AkFillMode::SOLID;
	AkDepthTest depthTest = AkDepthTest::LEQUAL;
	AkFaceCulling faceCulling = AkFaceCulling::BACK;
	AkWindingOrder windingOrder = AkWindingOrder::COUNTER_CLOCK_WISE;
	
	std::vector<AkBlendState> blendStates = {};

	size_t GetHash() const { return m_Hash; }

private:
	friend class AkMaterial;

	size_t m_Hash = 0;
	void CalculateHash()
	{
		m_Hash = Hash(lineWidth);
		HashCombine(m_Hash, depthWrite);
		HashCombine(m_Hash, fillMode);
		HashCombine(m_Hash, depthTest);
		HashCombine(m_Hash, faceCulling);
		HashCombine(m_Hash, windingOrder);

		for(const AkBlendState& blendState : blendStates)
			HashCombine(m_Hash, blendState);
	}
};

template <>
struct std::hash<AkRasterizerState>
{
	size_t operator()(const AkRasterizerState& state) const
	{
		return state.GetHash();
	}
};