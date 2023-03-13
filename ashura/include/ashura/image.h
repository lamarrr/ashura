#pragma once
#include "ashura/primitives.h"
#include "stx/memory.h"
#include "stx/span.h"

namespace ash
{
namespace gfx
{

/// NOTE: resource image with index 0 must be a transparent white image
using image = u64;

}        // namespace gfx

enum class ImageFormat : u8
{
  Alpha,
  Antialiasing,
  Gray,
  Rgb,
  Rgba,
  Bgra
};

enum class ColorSpace : u8
{
  Rgb,
  Srgb,
  AdobeRgb,
  Dp3,
  DciP3,
  Yuv
};

inline u8 nsource_channels_for_format(ImageFormat fmt)
{
  switch (fmt)
  {
    case ImageFormat::Alpha:
    case ImageFormat::Antialiasing:
    case ImageFormat::Gray:
      return 1;
    case ImageFormat::Rgb:
      return 3;
    case ImageFormat::Rgba:
      return 4;
    case ImageFormat::Bgra:
      return 4;
    default:
      ASH_UNREACHABLE();
  }
}

struct ImageBuffer
{
  stx::Memory memory;
  ash::extent extent;
  ImageFormat format = ImageFormat::Rgba;

  stx::Span<u8 const> span() const
  {
    return stx::Span{AS(u8 *, memory.handle), extent.area() * 4};
  }
};

}        // namespace ash
