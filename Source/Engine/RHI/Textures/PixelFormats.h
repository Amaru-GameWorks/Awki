#pragma once
#include "Core/Assert.h"

enum class AkPixelFormat
{
	UNDEFINED,

	R8_UINT,
	R8_SINT,
	R8_UNORM,
	R8_SNORM,

	RG8_UINT,
	RG8_SINT,
	RG8_UNORM,
	RG8_SNORM,

	RGBA8_UINT,
	RGBA8_SINT,
	RGBA8_SNORM,
	RGBA8_UNORM,
	RGBA8_SRGB,

	BGRA8_UNORM,
	BGRA8_SRGB,

	R10G10B10A2_UNORM,

	R16_UINT,
	R16_SINT,
	R16_UNORM,
	R16_SNORM,
	R16_FLOAT,

	RG16_UINT,
	RG16_SINT,
	RG16_UNORM,
	RG16_SNORM,
	RG16_FLOAT,

	RGBA16_UINT,
	RGBA16_SINT,
	RGBA16_UNORM,
	RGBA16_SNORM,
	RGBA16_FLOAT,

	R32_UINT,
	R32_SINT,
	R32_FLOAT,

	RG32_UINT,
	RG32_SINT,
	RG32_FLOAT,

	RGBA32_UINT,
	RGBA32_SINT,
	RGBA32_FLOAT,

	BC1_RGB_UNORM,
	BC1_RGB_SRGB,

	BC1_RGBA_UNORM,
	BC1_RGBA_SRGB,

	BC2_UNORM,
	BC2_SRGB,

	BC3_UNORM,
	BC3_SRGB,

	BC4_UNORM,
	BC4_SNORM,

	BC5_UNORM,
	BC5_SNORM,

	BC6H_UF16,
	BC6H_SF16,

	BC7_UNORM,
	BC7_SRGB,

	D32_SFLOAT_S8_UINT,
	D32_SFLOAT,
	D24_UNORM_S8_UINT,
	D16_UNORM
};

inline constexpr bool IsSRGB(const AkPixelFormat format)
{
	switch (format)
	{
		case AkPixelFormat::RGBA8_SRGB:
		case AkPixelFormat::BGRA8_SRGB:
		case AkPixelFormat::BC1_RGB_SRGB:
		case AkPixelFormat::BC1_RGBA_SRGB:
		case AkPixelFormat::BC2_SRGB:
		case AkPixelFormat::BC3_SRGB:
		case AkPixelFormat::BC7_SRGB:
			return true;

		default:
			return false;
	}
}

inline constexpr bool SupportsSRGB(const AkPixelFormat format)
{
	switch (format)
	{
		case AkPixelFormat::RGBA8_UNORM:
		case AkPixelFormat::BGRA8_UNORM:
		case AkPixelFormat::BC1_RGB_UNORM:
		case AkPixelFormat::BC1_RGBA_UNORM:
		case AkPixelFormat::BC2_UNORM:
		case AkPixelFormat::BC3_UNORM:
		case AkPixelFormat::BC7_UNORM:
			return true;

		default:
			return false;
	}
}

inline constexpr AkPixelFormat GetSRGB(const AkPixelFormat format)
{
	if (!IsSRGB(format) && SupportsSRGB(format))
	{
		AkPixelFormat returnFormat = static_cast<AkPixelFormat>(static_cast<uint32_t>(format) + 1);
		AkAssert(IsSRGB(returnFormat), "Return pixel format is not SRGB.");
		return returnFormat;
	}
	else
		return format;
}

inline constexpr bool IsDepthPixelFormat(const AkPixelFormat format)
{
	return format >= AkPixelFormat::D32_SFLOAT_S8_UINT;
}

inline constexpr bool PixelFormatHasStencil(const AkPixelFormat format)
{
	return format == AkPixelFormat::D32_SFLOAT_S8_UINT
		|| format == AkPixelFormat::D24_UNORM_S8_UINT;
}

inline constexpr bool IsHDRPixelFormat(const AkPixelFormat format)
{
	return format >= AkPixelFormat::R10G10B10A2_UNORM && format <= AkPixelFormat::RGBA32_FLOAT;
}

inline constexpr bool IsBlockCompressedPixelFormat(const AkPixelFormat format)
{
	return format >= AkPixelFormat::BC1_RGB_UNORM && format <= AkPixelFormat::BC7_SRGB;
}

inline constexpr size_t GetPixelSize(const AkPixelFormat format)
{
	switch (format)
	{
		default:
			AkRaise("Pixel format not registered on this function");
			return 0;

		case AkPixelFormat::R8_UINT:
		case AkPixelFormat::R8_SINT:
		case AkPixelFormat::R8_UNORM:
		case AkPixelFormat::R8_SNORM:
			return 1;

		case AkPixelFormat::RG8_UINT:
		case AkPixelFormat::RG8_SINT:
		case AkPixelFormat::RG8_UNORM:
		case AkPixelFormat::RG8_SNORM:

		case AkPixelFormat::R16_UINT:
		case AkPixelFormat::R16_SINT:
		case AkPixelFormat::R16_UNORM:
		case AkPixelFormat::R16_SNORM:
		case AkPixelFormat::R16_FLOAT:

		case AkPixelFormat::D16_UNORM:
			return 2;

		case AkPixelFormat::RGBA8_UINT:
		case AkPixelFormat::RGBA8_SINT:
		case AkPixelFormat::RGBA8_UNORM:
		case AkPixelFormat::RGBA8_SNORM:
		case AkPixelFormat::RGBA8_SRGB:

		case AkPixelFormat::BGRA8_UNORM:
		case AkPixelFormat::BGRA8_SRGB:

		case AkPixelFormat::R10G10B10A2_UNORM:

		case AkPixelFormat::RG16_UINT:
		case AkPixelFormat::RG16_SINT:
		case AkPixelFormat::RG16_UNORM:
		case AkPixelFormat::RG16_SNORM:
		case AkPixelFormat::RG16_FLOAT:

		case AkPixelFormat::R32_UINT:
		case AkPixelFormat::R32_SINT:
		case AkPixelFormat::R32_FLOAT:

		case AkPixelFormat::D32_SFLOAT:
		case AkPixelFormat::D24_UNORM_S8_UINT:
			return 4;

		case AkPixelFormat::D32_SFLOAT_S8_UINT:
			return 5;

		case AkPixelFormat::RGBA16_UINT:
		case AkPixelFormat::RGBA16_SINT:
		case AkPixelFormat::RGBA16_UNORM:
		case AkPixelFormat::RGBA16_SNORM:
		case AkPixelFormat::RGBA16_FLOAT:

		case AkPixelFormat::RG32_UINT:
		case AkPixelFormat::RG32_SINT:
		case AkPixelFormat::RG32_FLOAT:
			return 8;

		case AkPixelFormat::RGBA32_UINT:
		case AkPixelFormat::RGBA32_SINT:
		case AkPixelFormat::RGBA32_FLOAT:
			return 16;
	}
}

inline constexpr size_t GetCompressedBlockSize(const AkPixelFormat format)
{
	switch (format)
	{
		default:
			AkRaise("Pixel format not registered on this function");
			return 0;

		case AkPixelFormat::BC1_RGB_UNORM:
		case AkPixelFormat::BC1_RGB_SRGB:

		case AkPixelFormat::BC1_RGBA_UNORM:
		case AkPixelFormat::BC1_RGBA_SRGB:

		case AkPixelFormat::BC4_UNORM:
		case AkPixelFormat::BC4_SNORM:
			return 8;

		case AkPixelFormat::BC2_UNORM:
		case AkPixelFormat::BC2_SRGB:

		case AkPixelFormat::BC3_UNORM:
		case AkPixelFormat::BC3_SRGB:

		case AkPixelFormat::BC5_UNORM:
		case AkPixelFormat::BC5_SNORM:

		case AkPixelFormat::BC6H_UF16:
		case AkPixelFormat::BC6H_SF16:

		case AkPixelFormat::BC7_UNORM:
		case AkPixelFormat::BC7_SRGB:
			return 16;
	}
}

inline constexpr size_t GetChannelCount(const AkPixelFormat format)
{
	switch (format)
	{
		default:
			AkRaise("Pixel format not registered on this function");
			return 0;

		case AkPixelFormat::BC4_UNORM:
		case AkPixelFormat::BC4_SNORM:

		case AkPixelFormat::R8_UINT:
		case AkPixelFormat::R8_SINT:
		case AkPixelFormat::R8_UNORM:
		case AkPixelFormat::R8_SNORM:

		case AkPixelFormat::R16_UINT:
		case AkPixelFormat::R16_SINT:
		case AkPixelFormat::R16_UNORM:
		case AkPixelFormat::R16_SNORM:
		case AkPixelFormat::R16_FLOAT:

		case AkPixelFormat::R32_UINT:
		case AkPixelFormat::R32_SINT:
		case AkPixelFormat::R32_FLOAT:

		case AkPixelFormat::D16_UNORM:
		case AkPixelFormat::D32_SFLOAT:
			return 1;

		case AkPixelFormat::BC5_UNORM:
		case AkPixelFormat::BC5_SNORM:

		case AkPixelFormat::RG8_UINT:
		case AkPixelFormat::RG8_SINT:
		case AkPixelFormat::RG8_UNORM:
		case AkPixelFormat::RG8_SNORM:

		case AkPixelFormat::RG16_UINT:
		case AkPixelFormat::RG16_SINT:
		case AkPixelFormat::RG16_UNORM:
		case AkPixelFormat::RG16_SNORM:
		case AkPixelFormat::RG16_FLOAT:

		case AkPixelFormat::RG32_UINT:
		case AkPixelFormat::RG32_SINT:
		case AkPixelFormat::RG32_FLOAT:

		case AkPixelFormat::D24_UNORM_S8_UINT:
		case AkPixelFormat::D32_SFLOAT_S8_UINT:
			return 2;

		case AkPixelFormat::BC1_RGB_UNORM:
		case AkPixelFormat::BC1_RGB_SRGB:
			return 3;

		case AkPixelFormat::BC1_RGBA_UNORM:
		case AkPixelFormat::BC1_RGBA_SRGB:
		case AkPixelFormat::BC2_UNORM:
		case AkPixelFormat::BC2_SRGB:
		case AkPixelFormat::BC3_UNORM:
		case AkPixelFormat::BC3_SRGB:
		case AkPixelFormat::BC6H_UF16:
		case AkPixelFormat::BC6H_SF16:
		case AkPixelFormat::BC7_UNORM:
		case AkPixelFormat::BC7_SRGB:

		case AkPixelFormat::RGBA8_UINT:
		case AkPixelFormat::RGBA8_SINT:
		case AkPixelFormat::RGBA8_UNORM:
		case AkPixelFormat::RGBA8_SNORM:
		case AkPixelFormat::RGBA8_SRGB:

		case AkPixelFormat::BGRA8_UNORM:
		case AkPixelFormat::BGRA8_SRGB:

		case AkPixelFormat::RGBA16_UINT:
		case AkPixelFormat::RGBA16_SINT:
		case AkPixelFormat::RGBA16_UNORM:
		case AkPixelFormat::RGBA16_SNORM:
		case AkPixelFormat::RGBA16_FLOAT:

		case AkPixelFormat::RGBA32_UINT:
		case AkPixelFormat::RGBA32_SINT:
		case AkPixelFormat::RGBA32_FLOAT:

		case AkPixelFormat::R10G10B10A2_UNORM:
			return 4;
	}
}