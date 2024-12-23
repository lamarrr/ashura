/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/gpu/gpu.h"
#include "ashura/std/error.h"
#include "ashura/std/types.h"

namespace ash
{

namespace gpu
{
///
/// Supported Formats:
/// - Undefined
/// - R8_UNORM
/// - R8G8B8_UNORM
/// - B8G8R8_UNORM
/// - R8G8B8A8_UNORM
/// - B8G8R8A8_UNORM
/// - R32G32_SFLOAT
/// - R32G32B32_SFLOAT
/// - R32G32B32A32_SFLOAT
///
inline u8 pixel_pitch(Format fmt)
{
  switch (fmt)
  {
    case Format::R8_UNORM:
      return 1;
    case Format::R8G8B8_UNORM:
      return 2;
    case Format::B8G8R8_UNORM:
      return 3;
    case Format::R8G8B8A8_UNORM:
      return 4;
    case Format::B8G8R8A8_UNORM:
      return 4;
    case Format::R32_SFLOAT:
      return 4;
    case Format::R32G32_SFLOAT:
      return 8;
    case Format::R32G32B32_SFLOAT:
      return 24;
    case Format::R32G32B32A32_SFLOAT:
      return 16;
    default:
      CHECK_UNREACHABLE();
  }
}

inline u64 packed_image_size(u32 width, u32 height, Format format)
{
  return (u64) width * (u64) height * (u64) pixel_pitch(format);
}

}    // namespace gpu

}    // namespace ash
