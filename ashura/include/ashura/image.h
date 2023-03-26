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

// NOTE: pixels are not stored in endian order but in byte by byte interleaving. i.e. for
// RGBA bytes 0 -> R, 1 -> G, 2 -> B, 3 -> A i.e. [r, g, b, a, r, g, b, a]
enum class ImageFormat : u8
{
  Alpha,               // A8
  Antialiasing,        // A8
  Gray,                // A8
  Rgb,                 // R8G8B8
  Rgba,                // R8G8B8A8
  Bgra                 // B8G8R8A8
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

inline u8 nchannels(ImageFormat fmt)
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

struct ImageView
{
  stx::Span<u8 const> data;
  ash::extent         extent;
  ImageFormat         format = ImageFormat::Rgba;
};

struct ImageBuffer
{
  stx::Memory memory;
  ash::extent extent;
  ImageFormat format = ImageFormat::Rgba;

  stx::Span<u8 const> span() const
  {
    return stx::Span{AS(u8 *, memory.handle), extent.area() * 4};
  }

  operator ImageView() const
  {
    return ImageView{.data = span(), .extent = extent, .format = format};
  }
};

}        // namespace ash
