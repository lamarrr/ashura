/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/gpu/gpu.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

namespace gpu
{

/// @brief The three-dimensional extent of a texel block.
constexpr u32x3 block_extent(Format format)
{
  switch (format)
  {
    case Format::BC1_RGB_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::BC1_RGB_SRGB_BLOCK:
      return {4, 4, 1};
    case Format::BC1_RGBA_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::BC1_RGBA_SRGB_BLOCK:
      return {4, 4, 1};
    case Format::BC2_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::BC2_SRGB_BLOCK:
      return {4, 4, 1};
    case Format::BC3_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::BC3_SRGB_BLOCK:
      return {4, 4, 1};
    case Format::BC4_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::BC4_SNORM_BLOCK:
      return {4, 4, 1};
    case Format::BC5_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::BC5_SNORM_BLOCK:
      return {4, 4, 1};
    case Format::BC6H_UFLOAT_BLOCK:
      return {4, 4, 1};
    case Format::BC6H_SFLOAT_BLOCK:
      return {4, 4, 1};
    case Format::BC7_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::BC7_SRGB_BLOCK:
      return {4, 4, 1};
    case Format::ETC2_R8G8B8_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::ETC2_R8G8B8_SRGB_BLOCK:
      return {4, 4, 1};
    case Format::ETC2_R8G8B8A1_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::ETC2_R8G8B8A1_SRGB_BLOCK:
      return {4, 4, 1};
    case Format::ETC2_R8G8B8A8_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::ETC2_R8G8B8A8_SRGB_BLOCK:
      return {4, 4, 1};
    case Format::EAC_R11_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::EAC_R11_SNORM_BLOCK:
      return {4, 4, 1};
    case Format::EAC_R11G11_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::EAC_R11G11_SNORM_BLOCK:
      return {4, 4, 1};
    case Format::ASTC_4x4_UNORM_BLOCK:
      return {4, 4, 1};
    case Format::ASTC_4x4_SRGB_BLOCK:
      return {4, 4, 1};
    case Format::ASTC_5x4_UNORM_BLOCK:
      return {5, 4, 1};
    case Format::ASTC_5x4_SRGB_BLOCK:
      return {5, 4, 1};
    case Format::ASTC_5x5_UNORM_BLOCK:
      return {5, 5, 1};
    case Format::ASTC_5x5_SRGB_BLOCK:
      return {5, 5, 1};
    case Format::ASTC_6x5_UNORM_BLOCK:
      return {6, 5, 1};
    case Format::ASTC_6x5_SRGB_BLOCK:
      return {6, 5, 1};
    case Format::ASTC_6x6_UNORM_BLOCK:
      return {6, 6, 1};
    case Format::ASTC_6x6_SRGB_BLOCK:
      return {6, 6, 1};
    case Format::ASTC_8x5_UNORM_BLOCK:
      return {8, 5, 1};
    case Format::ASTC_8x5_SRGB_BLOCK:
      return {8, 5, 1};
    case Format::ASTC_8x6_UNORM_BLOCK:
      return {8, 6, 1};
    case Format::ASTC_8x6_SRGB_BLOCK:
      return {8, 6, 1};
    case Format::ASTC_8x8_UNORM_BLOCK:
      return {8, 8, 1};
    case Format::ASTC_8x8_SRGB_BLOCK:
      return {8, 8, 1};
    case Format::ASTC_10x5_UNORM_BLOCK:
      return {10, 5, 1};
    case Format::ASTC_10x5_SRGB_BLOCK:
      return {10, 5, 1};
    case Format::ASTC_10x6_UNORM_BLOCK:
      return {10, 6, 1};
    case Format::ASTC_10x6_SRGB_BLOCK:
      return {10, 6, 1};
    case Format::ASTC_10x8_UNORM_BLOCK:
      return {10, 8, 1};
    case Format::ASTC_10x8_SRGB_BLOCK:
      return {10, 8, 1};
    case Format::ASTC_10x10_UNORM_BLOCK:
      return {10, 10, 1};
    case Format::ASTC_10x10_SRGB_BLOCK:
      return {10, 10, 1};
    case Format::ASTC_12x10_UNORM_BLOCK:
      return {12, 10, 1};
    case Format::ASTC_12x10_SRGB_BLOCK:
      return {12, 10, 1};
    case Format::ASTC_12x12_UNORM_BLOCK:
      return {12, 12, 1};
    case Format::ASTC_12x12_SRGB_BLOCK:
      return {12, 12, 1};
    case Format::G8B8G8R8_422_UNORM:
      return {2, 1, 1};
    case Format::B8G8R8G8_422_UNORM:
      return {2, 1, 1};
    case Format::G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
      return {2, 1, 1};
    case Format::B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
      return {2, 1, 1};
    case Format::G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
      return {2, 1, 1};
    case Format::B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
      return {2, 1, 1};
    case Format::G16B16G16R16_422_UNORM:
      return {2, 1, 1};
    case Format::B16G16R16G16_422_UNORM:
      return {2, 1, 1};
    case Format::ASTC_4x4_SFLOAT_BLOCK:
      return {4, 4, 1};
    case Format::ASTC_5x4_SFLOAT_BLOCK:
      return {5, 4, 1};
    case Format::ASTC_5x5_SFLOAT_BLOCK:
      return {5, 5, 1};
    case Format::ASTC_6x5_SFLOAT_BLOCK:
      return {6, 5, 1};
    case Format::ASTC_6x6_SFLOAT_BLOCK:
      return {6, 6, 1};
    case Format::ASTC_8x5_SFLOAT_BLOCK:
      return {8, 5, 1};
    case Format::ASTC_8x6_SFLOAT_BLOCK:
      return {8, 6, 1};
    case Format::ASTC_8x8_SFLOAT_BLOCK:
      return {8, 8, 1};
    case Format::ASTC_10x5_SFLOAT_BLOCK:
      return {10, 5, 1};
    case Format::ASTC_10x6_SFLOAT_BLOCK:
      return {10, 6, 1};
    case Format::ASTC_10x8_SFLOAT_BLOCK:
      return {10, 8, 1};
    case Format::ASTC_10x10_SFLOAT_BLOCK:
      return {10, 10, 1};
    case Format::ASTC_12x10_SFLOAT_BLOCK:
      return {12, 10, 1};
    case Format::ASTC_12x12_SFLOAT_BLOCK:
      return {12, 12, 1};
    case Format::PVRTC1_2BPP_UNORM_BLOCK_IMG:
      return {8, 4, 1};
    case Format::PVRTC1_4BPP_UNORM_BLOCK_IMG:
      return {4, 4, 1};
    case Format::PVRTC2_2BPP_UNORM_BLOCK_IMG:
      return {8, 4, 1};
    case Format::PVRTC2_4BPP_UNORM_BLOCK_IMG:
      return {4, 4, 1};
    case Format::PVRTC1_2BPP_SRGB_BLOCK_IMG:
      return {8, 4, 1};
    case Format::PVRTC1_4BPP_SRGB_BLOCK_IMG:
      return {4, 4, 1};
    case Format::PVRTC2_2BPP_SRGB_BLOCK_IMG:
      return {8, 4, 1};
    case Format::PVRTC2_4BPP_SRGB_BLOCK_IMG:
      return {4, 4, 1};

    default:
      return {1, 1, 1};
  }
}

/// @brief The texel block size in bytes.
u8 block_size(Format format)
{
  switch (format)
  {
    case Format::R4G4_UNORM_PACK8:
      return 1;
    case Format::R4G4B4A4_UNORM_PACK16:
      return 2;
    case Format::B4G4R4A4_UNORM_PACK16:
      return 2;
    case Format::R5G6B5_UNORM_PACK16:
      return 2;
    case Format::B5G6R5_UNORM_PACK16:
      return 2;
    case Format::R5G5B5A1_UNORM_PACK16:
      return 2;
    case Format::B5G5R5A1_UNORM_PACK16:
      return 2;
    case Format::A1R5G5B5_UNORM_PACK16:
      return 2;
    case Format::R8_UNORM:
      return 1;
    case Format::R8_SNORM:
      return 1;
    case Format::R8_USCALED:
      return 1;
    case Format::R8_SSCALED:
      return 1;
    case Format::R8_UINT:
      return 1;
    case Format::R8_SINT:
      return 1;
    case Format::R8_SRGB:
      return 1;
    case Format::R8G8_UNORM:
      return 2;
    case Format::R8G8_SNORM:
      return 2;
    case Format::R8G8_USCALED:
      return 2;
    case Format::R8G8_SSCALED:
      return 2;
    case Format::R8G8_UINT:
      return 2;
    case Format::R8G8_SINT:
      return 2;
    case Format::R8G8_SRGB:
      return 2;
    case Format::R8G8B8_UNORM:
      return 3;
    case Format::R8G8B8_SNORM:
      return 3;
    case Format::R8G8B8_USCALED:
      return 3;
    case Format::R8G8B8_SSCALED:
      return 3;
    case Format::R8G8B8_UINT:
      return 3;
    case Format::R8G8B8_SINT:
      return 3;
    case Format::R8G8B8_SRGB:
      return 3;
    case Format::B8G8R8_UNORM:
      return 3;
    case Format::B8G8R8_SNORM:
      return 3;
    case Format::B8G8R8_USCALED:
      return 3;
    case Format::B8G8R8_SSCALED:
      return 3;
    case Format::B8G8R8_UINT:
      return 3;
    case Format::B8G8R8_SINT:
      return 3;
    case Format::B8G8R8_SRGB:
      return 3;
    case Format::R8G8B8A8_UNORM:
      return 4;
    case Format::R8G8B8A8_SNORM:
      return 4;
    case Format::R8G8B8A8_USCALED:
      return 4;
    case Format::R8G8B8A8_SSCALED:
      return 4;
    case Format::R8G8B8A8_UINT:
      return 4;
    case Format::R8G8B8A8_SINT:
      return 4;
    case Format::R8G8B8A8_SRGB:
      return 4;
    case Format::B8G8R8A8_UNORM:
      return 4;
    case Format::B8G8R8A8_SNORM:
      return 4;
    case Format::B8G8R8A8_USCALED:
      return 4;
    case Format::B8G8R8A8_SSCALED:
      return 4;
    case Format::B8G8R8A8_UINT:
      return 4;
    case Format::B8G8R8A8_SINT:
      return 4;
    case Format::B8G8R8A8_SRGB:
      return 4;
    case Format::A8B8G8R8_UNORM_PACK32:
      return 4;
    case Format::A8B8G8R8_SNORM_PACK32:
      return 4;
    case Format::A8B8G8R8_USCALED_PACK32:
      return 4;
    case Format::A8B8G8R8_SSCALED_PACK32:
      return 4;
    case Format::A8B8G8R8_UINT_PACK32:
      return 4;
    case Format::A8B8G8R8_SINT_PACK32:
      return 4;
    case Format::A8B8G8R8_SRGB_PACK32:
      return 4;
    case Format::A2R10G10B10_UNORM_PACK32:
      return 4;
    case Format::A2R10G10B10_SNORM_PACK32:
      return 4;
    case Format::A2R10G10B10_USCALED_PACK32:
      return 4;
    case Format::A2R10G10B10_SSCALED_PACK32:
      return 4;
    case Format::A2R10G10B10_UINT_PACK32:
      return 4;
    case Format::A2R10G10B10_SINT_PACK32:
      return 4;
    case Format::A2B10G10R10_UNORM_PACK32:
      return 4;
    case Format::A2B10G10R10_SNORM_PACK32:
      return 4;
    case Format::A2B10G10R10_USCALED_PACK32:
      return 4;
    case Format::A2B10G10R10_SSCALED_PACK32:
      return 4;
    case Format::A2B10G10R10_UINT_PACK32:
      return 4;
    case Format::A2B10G10R10_SINT_PACK32:
      return 4;
    case Format::R16_UNORM:
      return 2;
    case Format::R16_SNORM:
      return 2;
    case Format::R16_USCALED:
      return 2;
    case Format::R16_SSCALED:
      return 2;
    case Format::R16_UINT:
      return 2;
    case Format::R16_SINT:
      return 2;
    case Format::R16_SFLOAT:
      return 2;
    case Format::R16G16_UNORM:
      return 4;
    case Format::R16G16_SNORM:
      return 4;
    case Format::R16G16_USCALED:
      return 4;
    case Format::R16G16_SSCALED:
      return 4;
    case Format::R16G16_UINT:
      return 4;
    case Format::R16G16_SINT:
      return 4;
    case Format::R16G16_SFLOAT:
      return 4;
    case Format::R16G16B16_UNORM:
      return 6;
    case Format::R16G16B16_SNORM:
      return 6;
    case Format::R16G16B16_USCALED:
      return 6;
    case Format::R16G16B16_SSCALED:
      return 6;
    case Format::R16G16B16_UINT:
      return 6;
    case Format::R16G16B16_SINT:
      return 6;
    case Format::R16G16B16_SFLOAT:
      return 6;
    case Format::R16G16B16A16_UNORM:
      return 8;
    case Format::R16G16B16A16_SNORM:
      return 8;
    case Format::R16G16B16A16_USCALED:
      return 8;
    case Format::R16G16B16A16_SSCALED:
      return 8;
    case Format::R16G16B16A16_UINT:
      return 8;
    case Format::R16G16B16A16_SINT:
      return 8;
    case Format::R16G16B16A16_SFLOAT:
      return 8;
    case Format::R32_UINT:
      return 4;
    case Format::R32_SINT:
      return 4;
    case Format::R32_SFLOAT:
      return 4;
    case Format::R32G32_UINT:
      return 8;
    case Format::R32G32_SINT:
      return 8;
    case Format::R32G32_SFLOAT:
      return 8;
    case Format::R32G32B32_UINT:
      return 12;
    case Format::R32G32B32_SINT:
      return 12;
    case Format::R32G32B32_SFLOAT:
      return 12;
    case Format::R32G32B32A32_UINT:
      return 16;
    case Format::R32G32B32A32_SINT:
      return 16;
    case Format::R32G32B32A32_SFLOAT:
      return 16;
    case Format::R64_UINT:
      return 8;
    case Format::R64_SINT:
      return 8;
    case Format::R64_SFLOAT:
      return 8;
    case Format::R64G64_UINT:
      return 16;
    case Format::R64G64_SINT:
      return 16;
    case Format::R64G64_SFLOAT:
      return 16;
    case Format::R64G64B64_UINT:
      return 24;
    case Format::R64G64B64_SINT:
      return 24;
    case Format::R64G64B64_SFLOAT:
      return 24;
    case Format::R64G64B64A64_UINT:
      return 32;
    case Format::R64G64B64A64_SINT:
      return 32;
    case Format::R64G64B64A64_SFLOAT:
      return 32;
    case Format::B10G11R11_UFLOAT_PACK32:
      return 4;
    case Format::E5B9G9R9_UFLOAT_PACK32:
      return 4;
    case Format::D16_UNORM:
      return 2;
    case Format::X8_D24_UNORM_PACK32:
      return 4;
    case Format::D32_SFLOAT:
      return 4;
    case Format::S8_UINT:
      return 1;
    case Format::D16_UNORM_S8_UINT:
      return 3;
    case Format::D24_UNORM_S8_UINT:
      return 4;
    case Format::D32_SFLOAT_S8_UINT:
      return 5;
    case Format::BC1_RGB_UNORM_BLOCK:
      return 8;
    case Format::BC1_RGB_SRGB_BLOCK:
      return 8;
    case Format::BC1_RGBA_UNORM_BLOCK:
      return 8;
    case Format::BC1_RGBA_SRGB_BLOCK:
      return 8;
    case Format::BC2_UNORM_BLOCK:
      return 16;
    case Format::BC2_SRGB_BLOCK:
      return 16;
    case Format::BC3_UNORM_BLOCK:
      return 16;
    case Format::BC3_SRGB_BLOCK:
      return 16;
    case Format::BC4_UNORM_BLOCK:
      return 8;
    case Format::BC4_SNORM_BLOCK:
      return 8;
    case Format::BC5_UNORM_BLOCK:
      return 16;
    case Format::BC5_SNORM_BLOCK:
      return 16;
    case Format::BC6H_UFLOAT_BLOCK:
      return 16;
    case Format::BC6H_SFLOAT_BLOCK:
      return 16;
    case Format::BC7_UNORM_BLOCK:
      return 16;
    case Format::BC7_SRGB_BLOCK:
      return 16;
    case Format::ETC2_R8G8B8_UNORM_BLOCK:
      return 8;
    case Format::ETC2_R8G8B8_SRGB_BLOCK:
      return 8;
    case Format::ETC2_R8G8B8A1_UNORM_BLOCK:
      return 8;
    case Format::ETC2_R8G8B8A1_SRGB_BLOCK:
      return 8;
    case Format::ETC2_R8G8B8A8_UNORM_BLOCK:
      return 16;
    case Format::ETC2_R8G8B8A8_SRGB_BLOCK:
      return 16;
    case Format::EAC_R11_UNORM_BLOCK:
      return 8;
    case Format::EAC_R11_SNORM_BLOCK:
      return 8;
    case Format::EAC_R11G11_UNORM_BLOCK:
      return 16;
    case Format::EAC_R11G11_SNORM_BLOCK:
      return 16;
    case Format::ASTC_4x4_UNORM_BLOCK:
      return 16;
    case Format::ASTC_4x4_SRGB_BLOCK:
      return 16;
    case Format::ASTC_5x4_UNORM_BLOCK:
      return 16;
    case Format::ASTC_5x4_SRGB_BLOCK:
      return 16;
    case Format::ASTC_5x5_UNORM_BLOCK:
      return 16;
    case Format::ASTC_5x5_SRGB_BLOCK:
      return 16;
    case Format::ASTC_6x5_UNORM_BLOCK:
      return 16;
    case Format::ASTC_6x5_SRGB_BLOCK:
      return 16;
    case Format::ASTC_6x6_UNORM_BLOCK:
      return 16;
    case Format::ASTC_6x6_SRGB_BLOCK:
      return 16;
    case Format::ASTC_8x5_UNORM_BLOCK:
      return 16;
    case Format::ASTC_8x5_SRGB_BLOCK:
      return 16;
    case Format::ASTC_8x6_UNORM_BLOCK:
      return 16;
    case Format::ASTC_8x6_SRGB_BLOCK:
      return 16;
    case Format::ASTC_8x8_UNORM_BLOCK:
      return 16;
    case Format::ASTC_8x8_SRGB_BLOCK:
      return 16;
    case Format::ASTC_10x5_UNORM_BLOCK:
      return 16;
    case Format::ASTC_10x5_SRGB_BLOCK:
      return 16;
    case Format::ASTC_10x6_UNORM_BLOCK:
      return 16;
    case Format::ASTC_10x6_SRGB_BLOCK:
      return 16;
    case Format::ASTC_10x8_UNORM_BLOCK:
      return 16;
    case Format::ASTC_10x8_SRGB_BLOCK:
      return 16;
    case Format::ASTC_10x10_UNORM_BLOCK:
      return 16;
    case Format::ASTC_10x10_SRGB_BLOCK:
      return 16;
    case Format::ASTC_12x10_UNORM_BLOCK:
      return 16;
    case Format::ASTC_12x10_SRGB_BLOCK:
      return 16;
    case Format::ASTC_12x12_UNORM_BLOCK:
      return 16;
    case Format::ASTC_12x12_SRGB_BLOCK:
      return 16;
    case Format::G8B8G8R8_422_UNORM:
      return 4;
    case Format::B8G8R8G8_422_UNORM:
      return 4;
    case Format::G8_B8_R8_3PLANE_420_UNORM:
      return 3;
    case Format::G8_B8R8_2PLANE_420_UNORM:
      return 3;
    case Format::G8_B8_R8_3PLANE_422_UNORM:
      return 3;
    case Format::G8_B8R8_2PLANE_422_UNORM:
      return 3;
    case Format::G8_B8_R8_3PLANE_444_UNORM:
      return 3;
    case Format::R10X6_UNORM_PACK16:
      return 2;
    case Format::R10X6G10X6_UNORM_2PACK16:
      return 4;
    case Format::R10X6G10X6B10X6A10X6_UNORM_4PACK16:
      return 8;
    case Format::G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
      return 8;
    case Format::B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
      return 8;
    case Format::G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
      return 6;
    case Format::G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
      return 6;
    case Format::G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
      return 6;
    case Format::G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
      return 6;
    case Format::G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
      return 6;
    case Format::R12X4_UNORM_PACK16:
      return 2;
    case Format::R12X4G12X4_UNORM_2PACK16:
      return 4;
    case Format::R12X4G12X4B12X4A12X4_UNORM_4PACK16:
      return 8;
    case Format::G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
      return 8;
    case Format::B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
      return 8;
    case Format::G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
      return 6;
    case Format::G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
      return 6;
    case Format::G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
      return 6;
    case Format::G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
      return 6;
    case Format::G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
      return 6;
    case Format::G16B16G16R16_422_UNORM:
      return 8;
    case Format::B16G16R16G16_422_UNORM:
      return 8;
    case Format::G16_B16_R16_3PLANE_420_UNORM:
      return 6;
    case Format::G16_B16R16_2PLANE_420_UNORM:
      return 6;
    case Format::G16_B16_R16_3PLANE_422_UNORM:
      return 6;
    case Format::G16_B16R16_2PLANE_422_UNORM:
      return 6;
    case Format::G16_B16_R16_3PLANE_444_UNORM:
      return 6;
    case Format::G8_B8R8_2PLANE_444_UNORM:
      return 3;
    case Format::G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
      return 6;
    case Format::G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
      return 6;
    case Format::G16_B16R16_2PLANE_444_UNORM:
      return 6;
    case Format::A4R4G4B4_UNORM_PACK16:
      return 2;
    case Format::A4B4G4R4_UNORM_PACK16:
      return 2;
    case Format::ASTC_4x4_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_5x4_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_5x5_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_6x5_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_6x6_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_8x5_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_8x6_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_8x8_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_10x5_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_10x6_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_10x8_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_10x10_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_12x10_SFLOAT_BLOCK:
      return 16;
    case Format::ASTC_12x12_SFLOAT_BLOCK:
      return 16;
    case Format::A1B5G5R5_UNORM_PACK16:
      return 2;
    case Format::A8_UNORM:
      return 1;
    case Format::PVRTC1_2BPP_UNORM_BLOCK_IMG:
      return 8;
    case Format::PVRTC1_4BPP_UNORM_BLOCK_IMG:
      return 8;
    case Format::PVRTC2_2BPP_UNORM_BLOCK_IMG:
      return 8;
    case Format::PVRTC2_4BPP_UNORM_BLOCK_IMG:
      return 8;
    case Format::PVRTC1_2BPP_SRGB_BLOCK_IMG:
      return 8;
    case Format::PVRTC1_4BPP_SRGB_BLOCK_IMG:
      return 8;
    case Format::PVRTC2_2BPP_SRGB_BLOCK_IMG:
      return 8;
    case Format::PVRTC2_4BPP_SRGB_BLOCK_IMG:
      return 8;

    default:
      return 0;
  }
}

/// @brief The number of bits in this component, if not compressed, otherwise empty.
constexpr InplaceVec<u8, 4> component_bits(Format format)
{
  switch (format)
  {
    case Format::R4G4_UNORM_PACK8:
      return {4, 4};
    case Format::R4G4B4A4_UNORM_PACK16:
      return {4, 4, 4};
    case Format::B4G4R4A4_UNORM_PACK16:
      return {4, 4, 4, 4};
    case Format::R5G6B5_UNORM_PACK16:
      return {5, 6, 5};
    case Format::B5G6R5_UNORM_PACK16:
      return {5, 6, 5};
    case Format::R5G5B5A1_UNORM_PACK16:
      return {5, 5, 5, 1};
    case Format::B5G5R5A1_UNORM_PACK16:
      return {5, 5, 5, 1};
    case Format::A1R5G5B5_UNORM_PACK16:
      return {1, 5, 5, 5};
    case Format::R8_UNORM:
      return {8};
    case Format::R8_SNORM:
      return {8};
    case Format::R8_USCALED:
      return {8};
    case Format::R8_SSCALED:
      return {8};
    case Format::R8_UINT:
      return {8};
    case Format::R8_SINT:
      return {8};
    case Format::R8_SRGB:
      return {8};
    case Format::R8G8_UNORM:
      return {8, 8};
    case Format::R8G8_SNORM:
      return {8, 8};
    case Format::R8G8_USCALED:
      return {8, 8};
    case Format::R8G8_SSCALED:
      return {8, 8};
    case Format::R8G8_UINT:
      return {8, 8};
    case Format::R8G8_SINT:
      return {8, 8};
    case Format::R8G8_SRGB:
      return {8, 8};
    case Format::R8G8B8_UNORM:
      return {8, 8, 8};
    case Format::R8G8B8_SNORM:
      return {8, 8, 8};
    case Format::R8G8B8_USCALED:
      return {8, 8, 8};
    case Format::R8G8B8_SSCALED:
      return {8, 8, 8};
    case Format::R8G8B8_UINT:
      return {8, 8, 8};
    case Format::R8G8B8_SINT:
      return {8, 8, 8};
    case Format::R8G8B8_SRGB:
      return {8, 8, 8};
    case Format::B8G8R8_UNORM:
      return {8, 8, 8};
    case Format::B8G8R8_SNORM:
      return {8, 8, 8};
    case Format::B8G8R8_USCALED:
      return {8, 8, 8};
    case Format::B8G8R8_SSCALED:
      return {8, 8, 8};
    case Format::B8G8R8_UINT:
      return {8, 8, 8};
    case Format::B8G8R8_SINT:
      return {8, 8, 8};
    case Format::B8G8R8_SRGB:
      return {8, 8, 8};
    case Format::R8G8B8A8_UNORM:
      return {8, 8, 8, 8};
    case Format::R8G8B8A8_SNORM:
      return {8, 8, 8, 8};
    case Format::R8G8B8A8_USCALED:
      return {8, 8, 8, 8};
    case Format::R8G8B8A8_SSCALED:
      return {8, 8, 8, 8};
    case Format::R8G8B8A8_UINT:
      return {8, 8, 8, 8};
    case Format::R8G8B8A8_SINT:
      return {8, 8, 8, 8};
    case Format::R8G8B8A8_SRGB:
      return {8, 8, 8, 8};
    case Format::B8G8R8A8_UNORM:
      return {8, 8, 8, 8};
    case Format::B8G8R8A8_SNORM:
      return {8, 8, 8, 8};
    case Format::B8G8R8A8_USCALED:
      return {8, 8, 8, 8};
    case Format::B8G8R8A8_SSCALED:
      return {8, 8, 8, 8};
    case Format::B8G8R8A8_UINT:
      return {8, 8, 8, 8};
    case Format::B8G8R8A8_SINT:
      return {8, 8, 8, 8};
    case Format::B8G8R8A8_SRGB:
      return {8, 8, 8, 8};
    case Format::A8B8G8R8_UNORM_PACK32:
      return {8, 8, 8, 8};
    case Format::A8B8G8R8_SNORM_PACK32:
      return {8, 8, 8, 8};
    case Format::A8B8G8R8_USCALED_PACK32:
      return {8, 8, 8, 8};
    case Format::A8B8G8R8_SSCALED_PACK32:
      return {8, 8, 8, 8};
    case Format::A8B8G8R8_UINT_PACK32:
      return {8, 8, 8, 8};
    case Format::A8B8G8R8_SINT_PACK32:
      return {8, 8, 8, 8};
    case Format::A8B8G8R8_SRGB_PACK32:
      return {8, 8, 8, 8};
    case Format::A2R10G10B10_UNORM_PACK32:
      return {2, 10, 10, 10};
    case Format::A2R10G10B10_SNORM_PACK32:
      return {2, 10, 10, 10};
    case Format::A2R10G10B10_USCALED_PACK32:
      return {2, 10, 10, 10};
    case Format::A2R10G10B10_SSCALED_PACK32:
      return {2, 10, 10, 10};
    case Format::A2R10G10B10_UINT_PACK32:
      return {2, 10, 10, 10};
    case Format::A2R10G10B10_SINT_PACK32:
      return {2, 10, 10, 10};
    case Format::A2B10G10R10_UNORM_PACK32:
      return {2, 10, 10, 10};
    case Format::A2B10G10R10_SNORM_PACK32:
      return {2, 10, 10, 10};
    case Format::A2B10G10R10_USCALED_PACK32:
      return {2, 10, 10, 10};
    case Format::A2B10G10R10_SSCALED_PACK32:
      return {2, 10, 10, 10};
    case Format::A2B10G10R10_UINT_PACK32:
      return {2, 10, 10, 10};
    case Format::A2B10G10R10_SINT_PACK32:
      return {2, 10, 10, 10};
    case Format::R16_UNORM:
      return {16};
    case Format::R16_SNORM:
      return {16};
    case Format::R16_USCALED:
      return {16};
    case Format::R16_SSCALED:
      return {16};
    case Format::R16_UINT:
      return {16};
    case Format::R16_SINT:
      return {16};
    case Format::R16_SFLOAT:
      return {16};
    case Format::R16G16_UNORM:
      return {16, 16};
    case Format::R16G16_SNORM:
      return {16, 16};
    case Format::R16G16_USCALED:
      return {16, 16};
    case Format::R16G16_SSCALED:
      return {16, 16};
    case Format::R16G16_UINT:
      return {16, 16};
    case Format::R16G16_SINT:
      return {16, 16};
    case Format::R16G16_SFLOAT:
      return {16, 16};
    case Format::R16G16B16_UNORM:
      return {16, 16, 16};
    case Format::R16G16B16_SNORM:
      return {16, 16, 16};
    case Format::R16G16B16_USCALED:
      return {16, 16, 16};
    case Format::R16G16B16_SSCALED:
      return {16, 16, 16};
    case Format::R16G16B16_UINT:
      return {16, 16, 16};
    case Format::R16G16B16_SINT:
      return {16, 16, 16};
    case Format::R16G16B16_SFLOAT:
      return {16, 16, 16};
    case Format::R16G16B16A16_UNORM:
      return {16, 16, 16, 16};
    case Format::R16G16B16A16_SNORM:
      return {16, 16, 16, 16};
    case Format::R16G16B16A16_USCALED:
      return {16, 16, 16, 16};
    case Format::R16G16B16A16_SSCALED:
      return {16, 16, 16, 16};
    case Format::R16G16B16A16_UINT:
      return {16, 16, 16, 16};
    case Format::R16G16B16A16_SINT:
      return {16, 16, 16, 16};
    case Format::R16G16B16A16_SFLOAT:
      return {16, 16, 16, 16};
    case Format::R32_UINT:
      return {32};
    case Format::R32_SINT:
      return {32};
    case Format::R32_SFLOAT:
      return {32};
    case Format::R32G32_UINT:
      return {32, 32};
    case Format::R32G32_SINT:
      return {32, 32};
    case Format::R32G32_SFLOAT:
      return {32, 32};
    case Format::R32G32B32_UINT:
      return {32, 32, 32};
    case Format::R32G32B32_SINT:
      return {32, 32, 32};
    case Format::R32G32B32_SFLOAT:
      return {32, 32, 32};
    case Format::R32G32B32A32_UINT:
      return {32, 32, 32, 32};
    case Format::R32G32B32A32_SINT:
      return {32, 32, 32, 32};
    case Format::R32G32B32A32_SFLOAT:
      return {32, 32, 32, 32};
    case Format::R64_UINT:
      return {64};
    case Format::R64_SINT:
      return {64};
    case Format::R64_SFLOAT:
      return {64};
    case Format::R64G64_UINT:
      return {64, 64};
    case Format::R64G64_SINT:
      return {64, 64};
    case Format::R64G64_SFLOAT:
      return {64, 64};
    case Format::R64G64B64_UINT:
      return {64, 64, 64};
    case Format::R64G64B64_SINT:
      return {64, 64, 64};
    case Format::R64G64B64_SFLOAT:
      return {64, 64, 64};
    case Format::R64G64B64A64_UINT:
      return {64, 64, 64, 64};
    case Format::R64G64B64A64_SINT:
      return {64, 64, 64, 64};
    case Format::R64G64B64A64_SFLOAT:
      return {64, 64, 64, 64};
    case Format::B10G11R11_UFLOAT_PACK32:
      return {10, 11, 11};
    case Format::E5B9G9R9_UFLOAT_PACK32:
      return {9, 9, 9};
    case Format::D16_UNORM:
      return {16};
    case Format::X8_D24_UNORM_PACK32:
      return {24};
    case Format::D32_SFLOAT:
      return {32};
    case Format::S8_UINT:
      return {8};
    case Format::D16_UNORM_S8_UINT:
      return {16, 8};
    case Format::D24_UNORM_S8_UINT:
      return {24, 8};
    case Format::D32_SFLOAT_S8_UINT:
      return {32, 8};
    case Format::EAC_R11_UNORM_BLOCK:
      return {11};
    case Format::EAC_R11_SNORM_BLOCK:
      return {11};
    case Format::EAC_R11G11_UNORM_BLOCK:
      return {11, 11};
    case Format::EAC_R11G11_SNORM_BLOCK:
      return {11, 11};
    case Format::G8B8G8R8_422_UNORM:
      return {8, 8, 8, 8};
    case Format::B8G8R8G8_422_UNORM:
      return {8, 8, 8, 8};
    case Format::G8_B8_R8_3PLANE_420_UNORM:
      return {8, 8, 8};
    case Format::G8_B8R8_2PLANE_420_UNORM:
      return {8, 8, 8};
    case Format::G8_B8_R8_3PLANE_422_UNORM:
      return {8, 8, 8};
    case Format::G8_B8R8_2PLANE_422_UNORM:
      return {8, 8, 8};
    case Format::G8_B8_R8_3PLANE_444_UNORM:
      return {8, 8, 8};
    case Format::R10X6_UNORM_PACK16:
      return {10};
    case Format::R10X6G10X6_UNORM_2PACK16:
      return {10, 10};
    case Format::R10X6G10X6B10X6A10X6_UNORM_4PACK16:
      return {10, 10, 10, 10};
    case Format::G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
      return {10, 10, 10, 10};
    case Format::B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
      return {10, 10, 10, 10};
    case Format::G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
      return {10, 10, 10};
    case Format::G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
      return {10, 10, 10};
    case Format::G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
      return {10, 10, 10};
    case Format::G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
      return {10, 10, 10};
    case Format::G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
      return {10, 10, 10};
    case Format::R12X4_UNORM_PACK16:
      return {12};
    case Format::R12X4G12X4_UNORM_2PACK16:
      return {12, 12};
    case Format::R12X4G12X4B12X4A12X4_UNORM_4PACK16:
      return {12, 12, 12, 12};
    case Format::G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
      return {12, 12, 12, 12};
    case Format::B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
      return {12, 12, 12};
    case Format::G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
      return {12, 12, 12};
    case Format::G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
      return {12, 12, 12};
    case Format::G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
      return {12, 12, 12};
    case Format::G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
      return {12, 12, 12};
    case Format::G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
      return {12, 12, 12};
    case Format::G16B16G16R16_422_UNORM:
      return {16, 16, 16, 16};
    case Format::B16G16R16G16_422_UNORM:
      return {16, 16, 16, 16};
    case Format::G16_B16_R16_3PLANE_420_UNORM:
      return {16, 16, 16};
    case Format::G16_B16R16_2PLANE_420_UNORM:
      return {16, 16, 16};
    case Format::G16_B16_R16_3PLANE_422_UNORM:
      return {16, 16, 16};
    case Format::G16_B16R16_2PLANE_422_UNORM:
      return {16, 16, 16};
    case Format::G16_B16_R16_3PLANE_444_UNORM:
      return {16, 16, 16};
    case Format::G8_B8R8_2PLANE_444_UNORM:
      return {8, 8, 8};
    case Format::G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
      return {10, 10, 10};
    case Format::G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
      return {12, 12, 12};
    case Format::G16_B16R16_2PLANE_444_UNORM:
      return {16, 16, 16};
    case Format::A4R4G4B4_UNORM_PACK16:
      return {4, 4, 4, 4};
    case Format::A4B4G4R4_UNORM_PACK16:
      return {4, 4, 4, 4};
    case Format::A1B5G5R5_UNORM_PACK16:
      return {1, 5, 5, 5};
    case Format::A8_UNORM:
      return {8};
    default:
      return {};
  }
}

// The number of components of this format.
u8 component_count(Format format)
{
  switch (format)
  {
    case Format::R4G4_UNORM_PACK8:
      return 2;
    case Format::R4G4B4A4_UNORM_PACK16:
      return 4;
    case Format::B4G4R4A4_UNORM_PACK16:
      return 4;
    case Format::R5G6B5_UNORM_PACK16:
      return 3;
    case Format::B5G6R5_UNORM_PACK16:
      return 3;
    case Format::R5G5B5A1_UNORM_PACK16:
      return 4;
    case Format::B5G5R5A1_UNORM_PACK16:
      return 4;
    case Format::A1R5G5B5_UNORM_PACK16:
      return 4;
    case Format::R8_UNORM:
      return 1;
    case Format::R8_SNORM:
      return 1;
    case Format::R8_USCALED:
      return 1;
    case Format::R8_SSCALED:
      return 1;
    case Format::R8_UINT:
      return 1;
    case Format::R8_SINT:
      return 1;
    case Format::R8_SRGB:
      return 1;
    case Format::R8G8_UNORM:
      return 2;
    case Format::R8G8_SNORM:
      return 2;
    case Format::R8G8_USCALED:
      return 2;
    case Format::R8G8_SSCALED:
      return 2;
    case Format::R8G8_UINT:
      return 2;
    case Format::R8G8_SINT:
      return 2;
    case Format::R8G8_SRGB:
      return 2;
    case Format::R8G8B8_UNORM:
      return 3;
    case Format::R8G8B8_SNORM:
      return 3;
    case Format::R8G8B8_USCALED:
      return 3;
    case Format::R8G8B8_SSCALED:
      return 3;
    case Format::R8G8B8_UINT:
      return 3;
    case Format::R8G8B8_SINT:
      return 3;
    case Format::R8G8B8_SRGB:
      return 3;
    case Format::B8G8R8_UNORM:
      return 3;
    case Format::B8G8R8_SNORM:
      return 3;
    case Format::B8G8R8_USCALED:
      return 3;
    case Format::B8G8R8_SSCALED:
      return 3;
    case Format::B8G8R8_UINT:
      return 3;
    case Format::B8G8R8_SINT:
      return 3;
    case Format::B8G8R8_SRGB:
      return 3;
    case Format::R8G8B8A8_UNORM:
      return 4;
    case Format::R8G8B8A8_SNORM:
      return 4;
    case Format::R8G8B8A8_USCALED:
      return 4;
    case Format::R8G8B8A8_SSCALED:
      return 4;
    case Format::R8G8B8A8_UINT:
      return 4;
    case Format::R8G8B8A8_SINT:
      return 4;
    case Format::R8G8B8A8_SRGB:
      return 4;
    case Format::B8G8R8A8_UNORM:
      return 4;
    case Format::B8G8R8A8_SNORM:
      return 4;
    case Format::B8G8R8A8_USCALED:
      return 4;
    case Format::B8G8R8A8_SSCALED:
      return 4;
    case Format::B8G8R8A8_UINT:
      return 4;
    case Format::B8G8R8A8_SINT:
      return 4;
    case Format::B8G8R8A8_SRGB:
      return 4;
    case Format::A8B8G8R8_UNORM_PACK32:
      return 4;
    case Format::A8B8G8R8_SNORM_PACK32:
      return 4;
    case Format::A8B8G8R8_USCALED_PACK32:
      return 4;
    case Format::A8B8G8R8_SSCALED_PACK32:
      return 4;
    case Format::A8B8G8R8_UINT_PACK32:
      return 4;
    case Format::A8B8G8R8_SINT_PACK32:
      return 4;
    case Format::A8B8G8R8_SRGB_PACK32:
      return 4;
    case Format::A2R10G10B10_UNORM_PACK32:
      return 4;
    case Format::A2R10G10B10_SNORM_PACK32:
      return 4;
    case Format::A2R10G10B10_USCALED_PACK32:
      return 4;
    case Format::A2R10G10B10_SSCALED_PACK32:
      return 4;
    case Format::A2R10G10B10_UINT_PACK32:
      return 4;
    case Format::A2R10G10B10_SINT_PACK32:
      return 4;
    case Format::A2B10G10R10_UNORM_PACK32:
      return 4;
    case Format::A2B10G10R10_SNORM_PACK32:
      return 4;
    case Format::A2B10G10R10_USCALED_PACK32:
      return 4;
    case Format::A2B10G10R10_SSCALED_PACK32:
      return 4;
    case Format::A2B10G10R10_UINT_PACK32:
      return 4;
    case Format::A2B10G10R10_SINT_PACK32:
      return 4;
    case Format::R16_UNORM:
      return 1;
    case Format::R16_SNORM:
      return 1;
    case Format::R16_USCALED:
      return 1;
    case Format::R16_SSCALED:
      return 1;
    case Format::R16_UINT:
      return 1;
    case Format::R16_SINT:
      return 1;
    case Format::R16_SFLOAT:
      return 1;
    case Format::R16G16_UNORM:
      return 2;
    case Format::R16G16_SNORM:
      return 2;
    case Format::R16G16_USCALED:
      return 2;
    case Format::R16G16_SSCALED:
      return 2;
    case Format::R16G16_UINT:
      return 2;
    case Format::R16G16_SINT:
      return 2;
    case Format::R16G16_SFLOAT:
      return 2;
    case Format::R16G16B16_UNORM:
      return 3;
    case Format::R16G16B16_SNORM:
      return 3;
    case Format::R16G16B16_USCALED:
      return 3;
    case Format::R16G16B16_SSCALED:
      return 3;
    case Format::R16G16B16_UINT:
      return 3;
    case Format::R16G16B16_SINT:
      return 3;
    case Format::R16G16B16_SFLOAT:
      return 3;
    case Format::R16G16B16A16_UNORM:
      return 4;
    case Format::R16G16B16A16_SNORM:
      return 4;
    case Format::R16G16B16A16_USCALED:
      return 4;
    case Format::R16G16B16A16_SSCALED:
      return 4;
    case Format::R16G16B16A16_UINT:
      return 4;
    case Format::R16G16B16A16_SINT:
      return 4;
    case Format::R16G16B16A16_SFLOAT:
      return 4;
    case Format::R32_UINT:
      return 1;
    case Format::R32_SINT:
      return 1;
    case Format::R32_SFLOAT:
      return 1;
    case Format::R32G32_UINT:
      return 2;
    case Format::R32G32_SINT:
      return 2;
    case Format::R32G32_SFLOAT:
      return 2;
    case Format::R32G32B32_UINT:
      return 3;
    case Format::R32G32B32_SINT:
      return 3;
    case Format::R32G32B32_SFLOAT:
      return 3;
    case Format::R32G32B32A32_UINT:
      return 4;
    case Format::R32G32B32A32_SINT:
      return 4;
    case Format::R32G32B32A32_SFLOAT:
      return 4;
    case Format::R64_UINT:
      return 1;
    case Format::R64_SINT:
      return 1;
    case Format::R64_SFLOAT:
      return 1;
    case Format::R64G64_UINT:
      return 2;
    case Format::R64G64_SINT:
      return 2;
    case Format::R64G64_SFLOAT:
      return 2;
    case Format::R64G64B64_UINT:
      return 3;
    case Format::R64G64B64_SINT:
      return 3;
    case Format::R64G64B64_SFLOAT:
      return 3;
    case Format::R64G64B64A64_UINT:
      return 4;
    case Format::R64G64B64A64_SINT:
      return 4;
    case Format::R64G64B64A64_SFLOAT:
      return 4;
    case Format::B10G11R11_UFLOAT_PACK32:
      return 3;
    case Format::E5B9G9R9_UFLOAT_PACK32:
      return 3;
    case Format::D16_UNORM:
      return 1;
    case Format::X8_D24_UNORM_PACK32:
      return 1;
    case Format::D32_SFLOAT:
      return 1;
    case Format::S8_UINT:
      return 1;
    case Format::D16_UNORM_S8_UINT:
      return 2;
    case Format::D24_UNORM_S8_UINT:
      return 2;
    case Format::D32_SFLOAT_S8_UINT:
      return 2;
    case Format::BC1_RGB_UNORM_BLOCK:
      return 3;
    case Format::BC1_RGB_SRGB_BLOCK:
      return 3;
    case Format::BC1_RGBA_UNORM_BLOCK:
      return 4;
    case Format::BC1_RGBA_SRGB_BLOCK:
      return 4;
    case Format::BC2_UNORM_BLOCK:
      return 4;
    case Format::BC2_SRGB_BLOCK:
      return 4;
    case Format::BC3_UNORM_BLOCK:
      return 4;
    case Format::BC3_SRGB_BLOCK:
      return 4;
    case Format::BC4_UNORM_BLOCK:
      return 1;
    case Format::BC4_SNORM_BLOCK:
      return 1;
    case Format::BC5_UNORM_BLOCK:
      return 2;
    case Format::BC5_SNORM_BLOCK:
      return 2;
    case Format::BC6H_UFLOAT_BLOCK:
      return 3;
    case Format::BC6H_SFLOAT_BLOCK:
      return 3;
    case Format::BC7_UNORM_BLOCK:
      return 4;
    case Format::BC7_SRGB_BLOCK:
      return 4;
    case Format::ETC2_R8G8B8_UNORM_BLOCK:
      return 3;
    case Format::ETC2_R8G8B8_SRGB_BLOCK:
      return 3;
    case Format::ETC2_R8G8B8A1_UNORM_BLOCK:
      return 4;
    case Format::ETC2_R8G8B8A1_SRGB_BLOCK:
      return 4;
    case Format::ETC2_R8G8B8A8_UNORM_BLOCK:
      return 4;
    case Format::ETC2_R8G8B8A8_SRGB_BLOCK:
      return 4;
    case Format::EAC_R11_UNORM_BLOCK:
      return 1;
    case Format::EAC_R11_SNORM_BLOCK:
      return 1;
    case Format::EAC_R11G11_UNORM_BLOCK:
      return 2;
    case Format::EAC_R11G11_SNORM_BLOCK:
      return 2;
    case Format::ASTC_4x4_UNORM_BLOCK:
      return 4;
    case Format::ASTC_4x4_SRGB_BLOCK:
      return 4;
    case Format::ASTC_5x4_UNORM_BLOCK:
      return 4;
    case Format::ASTC_5x4_SRGB_BLOCK:
      return 4;
    case Format::ASTC_5x5_UNORM_BLOCK:
      return 4;
    case Format::ASTC_5x5_SRGB_BLOCK:
      return 4;
    case Format::ASTC_6x5_UNORM_BLOCK:
      return 4;
    case Format::ASTC_6x5_SRGB_BLOCK:
      return 4;
    case Format::ASTC_6x6_UNORM_BLOCK:
      return 4;
    case Format::ASTC_6x6_SRGB_BLOCK:
      return 4;
    case Format::ASTC_8x5_UNORM_BLOCK:
      return 4;
    case Format::ASTC_8x5_SRGB_BLOCK:
      return 4;
    case Format::ASTC_8x6_UNORM_BLOCK:
      return 4;
    case Format::ASTC_8x6_SRGB_BLOCK:
      return 4;
    case Format::ASTC_8x8_UNORM_BLOCK:
      return 4;
    case Format::ASTC_8x8_SRGB_BLOCK:
      return 4;
    case Format::ASTC_10x5_UNORM_BLOCK:
      return 4;
    case Format::ASTC_10x5_SRGB_BLOCK:
      return 4;
    case Format::ASTC_10x6_UNORM_BLOCK:
      return 4;
    case Format::ASTC_10x6_SRGB_BLOCK:
      return 4;
    case Format::ASTC_10x8_UNORM_BLOCK:
      return 4;
    case Format::ASTC_10x8_SRGB_BLOCK:
      return 4;
    case Format::ASTC_10x10_UNORM_BLOCK:
      return 4;
    case Format::ASTC_10x10_SRGB_BLOCK:
      return 4;
    case Format::ASTC_12x10_UNORM_BLOCK:
      return 4;
    case Format::ASTC_12x10_SRGB_BLOCK:
      return 4;
    case Format::ASTC_12x12_UNORM_BLOCK:
      return 4;
    case Format::ASTC_12x12_SRGB_BLOCK:
      return 4;
    case Format::G8B8G8R8_422_UNORM:
      return 4;
    case Format::B8G8R8G8_422_UNORM:
      return 4;
    case Format::G8_B8_R8_3PLANE_420_UNORM:
      return 3;
    case Format::G8_B8R8_2PLANE_420_UNORM:
      return 3;
    case Format::G8_B8_R8_3PLANE_422_UNORM:
      return 3;
    case Format::G8_B8R8_2PLANE_422_UNORM:
      return 3;
    case Format::G8_B8_R8_3PLANE_444_UNORM:
      return 3;
    case Format::R10X6_UNORM_PACK16:
      return 1;
    case Format::R10X6G10X6_UNORM_2PACK16:
      return 2;
    case Format::R10X6G10X6B10X6A10X6_UNORM_4PACK16:
      return 4;
    case Format::G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
      return 4;
    case Format::B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
      return 4;
    case Format::G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
      return 3;
    case Format::G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
      return 3;
    case Format::G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
      return 3;
    case Format::G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
      return 3;
    case Format::G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
      return 3;
    case Format::R12X4_UNORM_PACK16:
      return 1;
    case Format::R12X4G12X4_UNORM_2PACK16:
      return 2;
    case Format::R12X4G12X4B12X4A12X4_UNORM_4PACK16:
      return 4;
    case Format::G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
      return 4;
    case Format::B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
      return 4;
    case Format::G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
      return 3;
    case Format::G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
      return 3;
    case Format::G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
      return 3;
    case Format::G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
      return 3;
    case Format::G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
      return 3;
    case Format::G16B16G16R16_422_UNORM:
      return 4;
    case Format::B16G16R16G16_422_UNORM:
      return 4;
    case Format::G16_B16_R16_3PLANE_420_UNORM:
      return 3;
    case Format::G16_B16R16_2PLANE_420_UNORM:
      return 3;
    case Format::G16_B16_R16_3PLANE_422_UNORM:
      return 3;
    case Format::G16_B16R16_2PLANE_422_UNORM:
      return 3;
    case Format::G16_B16_R16_3PLANE_444_UNORM:
      return 3;
    case Format::G8_B8R8_2PLANE_444_UNORM:
      return 3;
    case Format::G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
      return 3;
    case Format::G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
      return 3;
    case Format::G16_B16R16_2PLANE_444_UNORM:
      return 3;
    case Format::A4R4G4B4_UNORM_PACK16:
      return 4;
    case Format::A4B4G4R4_UNORM_PACK16:
      return 4;
    case Format::ASTC_4x4_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_5x4_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_5x5_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_6x5_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_6x6_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_8x5_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_8x6_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_8x8_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_10x5_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_10x6_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_10x8_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_10x10_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_12x10_SFLOAT_BLOCK:
      return 4;
    case Format::ASTC_12x12_SFLOAT_BLOCK:
      return 4;
    case Format::A1B5G5R5_UNORM_PACK16:
      return 4;
    case Format::A8_UNORM:
      return 1;
    case Format::PVRTC1_2BPP_UNORM_BLOCK_IMG:
      return 4;
    case Format::PVRTC1_4BPP_UNORM_BLOCK_IMG:
      return 4;
    case Format::PVRTC2_2BPP_UNORM_BLOCK_IMG:
      return 4;
    case Format::PVRTC2_4BPP_UNORM_BLOCK_IMG:
      return 4;
    case Format::PVRTC1_2BPP_SRGB_BLOCK_IMG:
      return 4;
    case Format::PVRTC1_4BPP_SRGB_BLOCK_IMG:
      return 4;
    case Format::PVRTC2_2BPP_SRGB_BLOCK_IMG:
      return 4;
    case Format::PVRTC2_4BPP_SRGB_BLOCK_IMG:
      return 4;
    default:
      return 0;
  }
}

enum class NumericFormat : u8
{
  None    = 0,
  UNORM   = 1,
  SNORM   = 2,
  USCALED = 3,
  SSCALED = 4,
  SINT    = 5,
  SFLOAT  = 6,
  UFLOAT  = 7,
  UINT    = 8,
  SRGB    = 9
};

// The numeric format of the component
constexpr InplaceVec<NumericFormat, 4> component_numeric_format(Format format)
{
  switch (format)
  {
    case Format::R4G4_UNORM_PACK8:
      return {NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::R4G4B4A4_UNORM_PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::B4G4R4A4_UNORM_PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::R5G6B5_UNORM_PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::B5G6R5_UNORM_PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::R5G5B5A1_UNORM_PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::B5G5R5A1_UNORM_PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::A1R5G5B5_UNORM_PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::R8_UNORM:
      return {NumericFormat::UNORM};
    case Format::R8_SNORM:
      return {NumericFormat::SNORM};
    case Format::R8_USCALED:
      return {NumericFormat::USCALED};
    case Format::R8_SSCALED:
      return {NumericFormat::SSCALED};
    case Format::R8_UINT:
      return {NumericFormat::UINT};
    case Format::R8_SINT:
      return {NumericFormat::SINT};
    case Format::R8_SRGB:
      return {NumericFormat::SRGB};
    case Format::R8G8_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::R8G8_SNORM:
      return {NumericFormat::SNORM, NumericFormat::SNORM};
    case Format::R8G8_USCALED:
      return {NumericFormat::USCALED, NumericFormat::USCALED};
    case Format::R8G8_SSCALED:
      return {NumericFormat::SSCALED, NumericFormat::SSCALED};
    case Format::R8G8_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT};
    case Format::R8G8_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT};
    case Format::R8G8_SRGB:
      return {NumericFormat::SRGB, NumericFormat::SRGB};
    case Format::R8G8B8_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::R8G8B8_SNORM:
      return {NumericFormat::SNORM, NumericFormat::SNORM, NumericFormat::SNORM};
    case Format::R8G8B8_USCALED:
      return {NumericFormat::USCALED, NumericFormat::USCALED,
              NumericFormat::USCALED};
    case Format::R8G8B8_SSCALED:
      return {NumericFormat::SSCALED, NumericFormat::SSCALED,
              NumericFormat::SSCALED};
    case Format::R8G8B8_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT};
    case Format::R8G8B8_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT};
    case Format::R8G8B8_SRGB:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB};
    case Format::B8G8R8_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::B8G8R8_SNORM:
      return {NumericFormat::SNORM, NumericFormat::SNORM, NumericFormat::SNORM};
    case Format::B8G8R8_USCALED:
      return {NumericFormat::USCALED, NumericFormat::USCALED,
              NumericFormat::USCALED};
    case Format::B8G8R8_SSCALED:
      return {NumericFormat::SSCALED, NumericFormat::SSCALED,
              NumericFormat::SSCALED};
    case Format::B8G8R8_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT};
    case Format::B8G8R8_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT};
    case Format::B8G8R8_SRGB:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB};
    case Format::R8G8B8A8_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::R8G8B8A8_SNORM:
      return {NumericFormat::SNORM, NumericFormat::SNORM, NumericFormat::SNORM,
              NumericFormat::SNORM};
    case Format::R8G8B8A8_USCALED:
      return {NumericFormat::USCALED, NumericFormat::USCALED,
              NumericFormat::USCALED, NumericFormat::USCALED};
    case Format::R8G8B8A8_SSCALED:
      return {NumericFormat::SSCALED, NumericFormat::SSCALED,
              NumericFormat::SSCALED, NumericFormat::SSCALED};
    case Format::R8G8B8A8_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT,
              NumericFormat::UINT};
    case Format::R8G8B8A8_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT,
              NumericFormat::SINT};
    case Format::R8G8B8A8_SRGB:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::B8G8R8A8_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::B8G8R8A8_SNORM:
      return {NumericFormat::SNORM, NumericFormat::SNORM, NumericFormat::SNORM,
              NumericFormat::SNORM};
    case Format::B8G8R8A8_USCALED:
      return {NumericFormat::USCALED, NumericFormat::USCALED,
              NumericFormat::USCALED, NumericFormat::USCALED};
    case Format::B8G8R8A8_SSCALED:
      return {NumericFormat::SSCALED, NumericFormat::SSCALED,
              NumericFormat::SSCALED, NumericFormat::SSCALED};
    case Format::B8G8R8A8_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT,
              NumericFormat::UINT};
    case Format::B8G8R8A8_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT,
              NumericFormat::SINT};
    case Format::B8G8R8A8_SRGB:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::A8B8G8R8_UNORM_PACK32:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::A8B8G8R8_SNORM_PACK32:
      return {NumericFormat::SNORM, NumericFormat::SNORM, NumericFormat::SNORM,
              NumericFormat::SNORM};
    case Format::A8B8G8R8_USCALED_PACK32:
      return {NumericFormat::USCALED, NumericFormat::USCALED,
              NumericFormat::USCALED, NumericFormat::USCALED};
    case Format::A8B8G8R8_SSCALED_PACK32:
      return {NumericFormat::SSCALED, NumericFormat::SSCALED,
              NumericFormat::SSCALED, NumericFormat::SSCALED};
    case Format::A8B8G8R8_UINT_PACK32:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT,
              NumericFormat::UINT};
    case Format::A8B8G8R8_SINT_PACK32:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT,
              NumericFormat::SINT};
    case Format::A8B8G8R8_SRGB_PACK32:
      return {NumericFormat::UNORM, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::SRGB};
    case Format::A2R10G10B10_UNORM_PACK32:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::A2R10G10B10_SNORM_PACK32:
      return {NumericFormat::SNORM, NumericFormat::SNORM, NumericFormat::SNORM,
              NumericFormat::SNORM};
    case Format::A2R10G10B10_USCALED_PACK32:
      return {NumericFormat::USCALED, NumericFormat::USCALED,
              NumericFormat::USCALED, NumericFormat::USCALED};
    case Format::A2R10G10B10_SSCALED_PACK32:
      return {NumericFormat::SSCALED, NumericFormat::SSCALED,
              NumericFormat::SSCALED, NumericFormat::SSCALED};
    case Format::A2R10G10B10_UINT_PACK32:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT,
              NumericFormat::UINT};
    case Format::A2R10G10B10_SINT_PACK32:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT,
              NumericFormat::SINT};
    case Format::A2B10G10R10_UNORM_PACK32:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::A2B10G10R10_SNORM_PACK32:
      return {NumericFormat::SNORM, NumericFormat::SNORM, NumericFormat::SNORM,
              NumericFormat::SNORM};
    case Format::A2B10G10R10_USCALED_PACK32:
      return {NumericFormat::USCALED, NumericFormat::USCALED,
              NumericFormat::USCALED, NumericFormat::USCALED};
    case Format::A2B10G10R10_SSCALED_PACK32:
      return {NumericFormat::SSCALED, NumericFormat::SSCALED,
              NumericFormat::SSCALED, NumericFormat::SSCALED};
    case Format::A2B10G10R10_UINT_PACK32:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT,
              NumericFormat::UINT};
    case Format::A2B10G10R10_SINT_PACK32:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT,
              NumericFormat::SINT};
    case Format::R16_UNORM:
      return {NumericFormat::UNORM};
    case Format::R16_SNORM:
      return {NumericFormat::SNORM};
    case Format::R16_USCALED:
      return {NumericFormat::USCALED};
    case Format::R16_SSCALED:
      return {NumericFormat::SSCALED};
    case Format::R16_UINT:
      return {NumericFormat::UINT};
    case Format::R16_SINT:
      return {NumericFormat::SINT};
    case Format::R16_SFLOAT:
      return {NumericFormat::SFLOAT};
    case Format::R16G16_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::R16G16_SNORM:
      return {NumericFormat::SNORM, NumericFormat::SNORM};
    case Format::R16G16_USCALED:
      return {NumericFormat::USCALED, NumericFormat::USCALED};
    case Format::R16G16_SSCALED:
      return {NumericFormat::SSCALED, NumericFormat::SSCALED};
    case Format::R16G16_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT};
    case Format::R16G16_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT};
    case Format::R16G16_SFLOAT:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::R16G16B16_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::R16G16B16_SNORM:
      return {NumericFormat::SNORM, NumericFormat::SNORM, NumericFormat::SNORM};
    case Format::R16G16B16_USCALED:
      return {NumericFormat::USCALED, NumericFormat::USCALED,
              NumericFormat::USCALED};
    case Format::R16G16B16_SSCALED:
      return {NumericFormat::SSCALED, NumericFormat::SSCALED,
              NumericFormat::SSCALED};
    case Format::R16G16B16_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT};
    case Format::R16G16B16_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT};
    case Format::R16G16B16_SFLOAT:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT};
    case Format::R16G16B16A16_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::R16G16B16A16_SNORM:
      return {NumericFormat::SNORM, NumericFormat::SNORM, NumericFormat::SNORM,
              NumericFormat::SNORM};
    case Format::R16G16B16A16_USCALED:
      return {NumericFormat::USCALED, NumericFormat::USCALED,
              NumericFormat::USCALED, NumericFormat::USCALED};
    case Format::R16G16B16A16_SSCALED:
      return {NumericFormat::SSCALED, NumericFormat::SSCALED,
              NumericFormat::SSCALED, NumericFormat::SSCALED};
    case Format::R16G16B16A16_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT,
              NumericFormat::UINT};
    case Format::R16G16B16A16_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT,
              NumericFormat::SINT};
    case Format::R16G16B16A16_SFLOAT:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::R32_UINT:
      return {NumericFormat::UINT};
    case Format::R32_SINT:
      return {NumericFormat::SINT};
    case Format::R32_SFLOAT:
      return {NumericFormat::SFLOAT};
    case Format::R32G32_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT};
    case Format::R32G32_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT};
    case Format::R32G32_SFLOAT:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::R32G32B32_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT};
    case Format::R32G32B32_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT};
    case Format::R32G32B32_SFLOAT:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT};
    case Format::R32G32B32A32_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT,
              NumericFormat::UINT};
    case Format::R32G32B32A32_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT,
              NumericFormat::SINT};
    case Format::R32G32B32A32_SFLOAT:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::R64_UINT:
      return {NumericFormat::UINT};
    case Format::R64_SINT:
      return {NumericFormat::SINT};
    case Format::R64_SFLOAT:
      return {NumericFormat::SFLOAT};
    case Format::R64G64_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT};
    case Format::R64G64_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT};
    case Format::R64G64_SFLOAT:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::R64G64B64_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT};
    case Format::R64G64B64_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT};
    case Format::R64G64B64_SFLOAT:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT};
    case Format::R64G64B64A64_UINT:
      return {NumericFormat::UINT, NumericFormat::UINT, NumericFormat::UINT,
              NumericFormat::UINT};
    case Format::R64G64B64A64_SINT:
      return {NumericFormat::SINT, NumericFormat::SINT, NumericFormat::SINT,
              NumericFormat::SINT};
    case Format::R64G64B64A64_SFLOAT:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::B10G11R11_UFLOAT_PACK32:
      return {NumericFormat::UFLOAT, NumericFormat::UFLOAT,
              NumericFormat::UFLOAT};
    case Format::E5B9G9R9_UFLOAT_PACK32:
      return {NumericFormat::UFLOAT, NumericFormat::UFLOAT,
              NumericFormat::UFLOAT};
    case Format::D16_UNORM:
      return {NumericFormat::UNORM};
    case Format::X8_D24_UNORM_PACK32:
      return {NumericFormat::UNORM};
    case Format::D32_SFLOAT:
      return {NumericFormat::SFLOAT};
    case Format::S8_UINT:
      return {NumericFormat::UINT};
    case Format::D16_UNORM_S8_UINT:
      return {NumericFormat::UNORM, NumericFormat::UINT};
    case Format::D24_UNORM_S8_UINT:
      return {NumericFormat::UNORM, NumericFormat::UINT};
    case Format::D32_SFLOAT_S8_UINT:
      return {NumericFormat::SFLOAT, NumericFormat::UINT};
    case Format::BC1_RGB_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::BC1_RGB_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB};
    case Format::BC1_RGBA_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::BC1_RGBA_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::BC2_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::BC2_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::BC3_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::BC3_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::BC4_UNORM_BLOCK:
      return {NumericFormat::UNORM};
    case Format::BC4_SNORM_BLOCK:
      return {NumericFormat::SNORM};
    case Format::BC5_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::BC5_SNORM_BLOCK:
      return {NumericFormat::SNORM, NumericFormat::SNORM};
    case Format::BC6H_UFLOAT_BLOCK:
      return {NumericFormat::UFLOAT, NumericFormat::UFLOAT,
              NumericFormat::UFLOAT};
    case Format::BC6H_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT};
    case Format::BC7_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::BC7_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ETC2_R8G8B8_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::ETC2_R8G8B8_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB};
    case Format::ETC2_R8G8B8A1_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ETC2_R8G8B8A1_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ETC2_R8G8B8A8_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ETC2_R8G8B8A8_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::EAC_R11_UNORM_BLOCK:
      return {NumericFormat::UNORM};
    case Format::EAC_R11_SNORM_BLOCK:
      return {NumericFormat::SNORM};
    case Format::EAC_R11G11_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::EAC_R11G11_SNORM_BLOCK:
      return {NumericFormat::SNORM, NumericFormat::SNORM};
    case Format::ASTC_4x4_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_4x4_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_5x4_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_5x4_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_5x5_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_5x5_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_6x5_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_6x5_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_6x6_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_6x6_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_8x5_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_8x5_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_8x6_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_8x6_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_8x8_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_8x8_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_10x5_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_10x5_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_10x6_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_10x6_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_10x8_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_10x8_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_10x10_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_10x10_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_12x10_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_12x10_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::ASTC_12x12_UNORM_BLOCK:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_12x12_SRGB_BLOCK:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::G8B8G8R8_422_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::B8G8R8G8_422_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::G8_B8_R8_3PLANE_420_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G8_B8R8_2PLANE_420_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G8_B8_R8_3PLANE_422_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G8_B8R8_2PLANE_422_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G8_B8_R8_3PLANE_444_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::R10X6_UNORM_PACK16:
      return {NumericFormat::UNORM};
    case Format::R10X6G10X6_UNORM_2PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::R10X6G10X6B10X6A10X6_UNORM_4PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::R12X4_UNORM_PACK16:
      return {NumericFormat::UNORM};
    case Format::R12X4G12X4_UNORM_2PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::R12X4G12X4B12X4A12X4_UNORM_4PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G16B16G16R16_422_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::B16G16R16G16_422_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::G16_B16_R16_3PLANE_420_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G16_B16R16_2PLANE_420_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G16_B16_R16_3PLANE_422_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G16_B16R16_2PLANE_422_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G16_B16_R16_3PLANE_444_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G8_B8R8_2PLANE_444_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::G16_B16R16_2PLANE_444_UNORM:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM};
    case Format::A4R4G4B4_UNORM_PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::A4B4G4R4_UNORM_PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::ASTC_4x4_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_5x4_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_5x5_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_6x5_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_6x6_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_8x5_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_8x6_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_8x8_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_10x5_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_10x6_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_10x8_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_10x10_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_12x10_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::ASTC_12x12_SFLOAT_BLOCK:
      return {NumericFormat::SFLOAT, NumericFormat::SFLOAT,
              NumericFormat::SFLOAT, NumericFormat::SFLOAT};
    case Format::A1B5G5R5_UNORM_PACK16:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::A8_UNORM:
      return {NumericFormat::UNORM};
    case Format::PVRTC1_2BPP_UNORM_BLOCK_IMG:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::PVRTC1_4BPP_UNORM_BLOCK_IMG:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::PVRTC2_2BPP_UNORM_BLOCK_IMG:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::PVRTC2_4BPP_UNORM_BLOCK_IMG:
      return {NumericFormat::UNORM, NumericFormat::UNORM, NumericFormat::UNORM,
              NumericFormat::UNORM};
    case Format::PVRTC1_2BPP_SRGB_BLOCK_IMG:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::PVRTC1_4BPP_SRGB_BLOCK_IMG:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::PVRTC2_2BPP_SRGB_BLOCK_IMG:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    case Format::PVRTC2_4BPP_SRGB_BLOCK_IMG:
      return {NumericFormat::SRGB, NumericFormat::SRGB, NumericFormat::SRGB,
              NumericFormat::UNORM};
    default:
      return {};
  }
}

enum class FormatCompression : u8
{
  None     = 0,
  BC       = 1,
  ETC2     = 2,
  EAC      = 3,
  ASTC_LDR = 4,
  ASTC_HDR = 5,
  PVRTC    = 6
};

FormatCompression compression_scheme(Format format)
{
  switch (format)
  {
    case Format::BC1_RGB_UNORM_BLOCK:
      return FormatCompression::BC;
    case Format::BC1_RGB_SRGB_BLOCK:
      return FormatCompression::BC;
    case Format::BC1_RGBA_UNORM_BLOCK:
      return FormatCompression::BC;
    case Format::BC1_RGBA_SRGB_BLOCK:
      return FormatCompression::BC;
    case Format::BC2_UNORM_BLOCK:
      return FormatCompression::BC;
    case Format::BC2_SRGB_BLOCK:
      return FormatCompression::BC;
    case Format::BC3_UNORM_BLOCK:
      return FormatCompression::BC;
    case Format::BC3_SRGB_BLOCK:
      return FormatCompression::BC;
    case Format::BC4_UNORM_BLOCK:
      return FormatCompression::BC;
    case Format::BC4_SNORM_BLOCK:
      return FormatCompression::BC;
    case Format::BC5_UNORM_BLOCK:
      return FormatCompression::BC;
    case Format::BC5_SNORM_BLOCK:
      return FormatCompression::BC;
    case Format::BC6H_UFLOAT_BLOCK:
      return FormatCompression::BC;
    case Format::BC6H_SFLOAT_BLOCK:
      return FormatCompression::BC;
    case Format::BC7_UNORM_BLOCK:
      return FormatCompression::BC;
    case Format::BC7_SRGB_BLOCK:
      return FormatCompression::BC;
    case Format::ETC2_R8G8B8_UNORM_BLOCK:
      return FormatCompression::ETC2;
    case Format::ETC2_R8G8B8_SRGB_BLOCK:
      return FormatCompression::ETC2;
    case Format::ETC2_R8G8B8A1_UNORM_BLOCK:
      return FormatCompression::ETC2;
    case Format::ETC2_R8G8B8A1_SRGB_BLOCK:
      return FormatCompression::ETC2;
    case Format::ETC2_R8G8B8A8_UNORM_BLOCK:
      return FormatCompression::ETC2;
    case Format::ETC2_R8G8B8A8_SRGB_BLOCK:
      return FormatCompression::ETC2;
    case Format::EAC_R11_UNORM_BLOCK:
      return FormatCompression::EAC;
    case Format::EAC_R11_SNORM_BLOCK:
      return FormatCompression::EAC;
    case Format::EAC_R11G11_UNORM_BLOCK:
      return FormatCompression::EAC;
    case Format::EAC_R11G11_SNORM_BLOCK:
      return FormatCompression::EAC;
    case Format::ASTC_4x4_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_4x4_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_5x4_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_5x4_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_5x5_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_5x5_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_6x5_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_6x5_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_6x6_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_6x6_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_8x5_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_8x5_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_8x6_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_8x6_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_8x8_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_8x8_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_10x5_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_10x5_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_10x6_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_10x6_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_10x8_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_10x8_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_10x10_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_10x10_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_12x10_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_12x10_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_12x12_UNORM_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_12x12_SRGB_BLOCK:
      return FormatCompression::ASTC_LDR;
    case Format::ASTC_4x4_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_5x4_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_5x5_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_6x5_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_6x6_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_8x5_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_8x6_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_8x8_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_10x5_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_10x6_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_10x8_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_10x10_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_12x10_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::ASTC_12x12_SFLOAT_BLOCK:
      return FormatCompression::ASTC_HDR;
    case Format::PVRTC1_2BPP_UNORM_BLOCK_IMG:
      return FormatCompression::PVRTC;
    case Format::PVRTC1_4BPP_UNORM_BLOCK_IMG:
      return FormatCompression::PVRTC;
    case Format::PVRTC2_2BPP_UNORM_BLOCK_IMG:
      return FormatCompression::PVRTC;
    case Format::PVRTC2_4BPP_UNORM_BLOCK_IMG:
      return FormatCompression::PVRTC;
    case Format::PVRTC1_2BPP_SRGB_BLOCK_IMG:
      return FormatCompression::PVRTC;
    case Format::PVRTC1_4BPP_SRGB_BLOCK_IMG:
      return FormatCompression::PVRTC;
    case Format::PVRTC2_2BPP_SRGB_BLOCK_IMG:
      return FormatCompression::PVRTC;
    case Format::PVRTC2_4BPP_SRGB_BLOCK_IMG:
      return FormatCompression::PVRTC;

    default:
      return FormatCompression::None;
  }
}

/// @brief The number of bits into which the format is packed. A single image element in this format
/// can be stored in the same space as a scalar type of this bit width.
constexpr u8 packed_size(Format format)
{
  switch (format)
  {
    case Format::R4G4_UNORM_PACK8:
      return 8;
    case Format::R4G4B4A4_UNORM_PACK16:
      return 16;
    case Format::B4G4R4A4_UNORM_PACK16:
      return 16;
    case Format::R5G6B5_UNORM_PACK16:
      return 16;
    case Format::B5G6R5_UNORM_PACK16:
      return 16;
    case Format::R5G5B5A1_UNORM_PACK16:
      return 16;
    case Format::B5G5R5A1_UNORM_PACK16:
      return 16;
    case Format::A1R5G5B5_UNORM_PACK16:
      return 16;
    case Format::A8B8G8R8_UNORM_PACK32:
      return 32;
    case Format::A8B8G8R8_SNORM_PACK32:
      return 32;
    case Format::A8B8G8R8_USCALED_PACK32:
      return 32;
    case Format::A8B8G8R8_SSCALED_PACK32:
      return 32;
    case Format::A8B8G8R8_UINT_PACK32:
      return 32;
    case Format::A8B8G8R8_SINT_PACK32:
      return 32;
    case Format::A8B8G8R8_SRGB_PACK32:
      return 32;
    case Format::A2R10G10B10_UNORM_PACK32:
      return 32;
    case Format::A2R10G10B10_SNORM_PACK32:
      return 32;
    case Format::A2R10G10B10_USCALED_PACK32:
      return 32;
    case Format::A2R10G10B10_SSCALED_PACK32:
      return 32;
    case Format::A2R10G10B10_UINT_PACK32:
      return 32;
    case Format::A2R10G10B10_SINT_PACK32:
      return 32;
    case Format::A2B10G10R10_UNORM_PACK32:
      return 32;
    case Format::A2B10G10R10_SNORM_PACK32:
      return 32;
    case Format::A2B10G10R10_USCALED_PACK32:
      return 32;
    case Format::A2B10G10R10_SSCALED_PACK32:
      return 32;
    case Format::A2B10G10R10_UINT_PACK32:
      return 32;
    case Format::A2B10G10R10_SINT_PACK32:
      return 32;
    case Format::B10G11R11_UFLOAT_PACK32:
      return 32;
    case Format::E5B9G9R9_UFLOAT_PACK32:
      return 32;
    case Format::X8_D24_UNORM_PACK32:
      return 32;
    case Format::R10X6_UNORM_PACK16:
      return 16;
    case Format::R10X6G10X6_UNORM_2PACK16:
      return 16;
    case Format::R10X6G10X6B10X6A10X6_UNORM_4PACK16:
      return 16;
    case Format::G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
      return 16;
    case Format::B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
      return 16;
    case Format::G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
      return 16;
    case Format::G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
      return 16;
    case Format::G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
      return 16;
    case Format::G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
      return 16;
    case Format::G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
      return 16;
    case Format::R12X4_UNORM_PACK16:
      return 16;
    case Format::R12X4G12X4_UNORM_2PACK16:
      return 16;
    case Format::R12X4G12X4B12X4A12X4_UNORM_4PACK16:
      return 16;
    case Format::G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
      return 16;
    case Format::B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
      return 16;
    case Format::G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
      return 16;
    case Format::G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
      return 16;
    case Format::G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
      return 16;
    case Format::G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
      return 16;
    case Format::G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
      return 16;
    case Format::G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
      return 16;
    case Format::G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
      return 16;
    case Format::A4R4G4B4_UNORM_PACK16:
      return 16;
    case Format::A4B4G4R4_UNORM_PACK16:
      return 16;
    case Format::A1B5G5R5_UNORM_PACK16:
      return 16;

    default:
      return 0;
  }
}

}    // namespace gpu
}    // namespace ash
