#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/std/types.h"

namespace ash
{

constexpr u32 pixel_byte_size(gfx::Format fmt)
{
  switch (fmt)
  {
    case gfx::Format::R8_UNORM:
      return 1;
    case gfx::Format::R8G8B8_UNORM:
      return 2;
    case gfx::Format::B8G8R8_UNORM:
      return 3;
    case gfx::Format::R8G8B8A8_UNORM:
      return 4;
    case gfx::Format::B8G8R8A8_UNORM:
      return 4;
    case gfx::Format::R32_SFLOAT:
      return 4;
    case gfx::Format::R32G32_SFLOAT:
      return 8;
    case gfx::Format::R32G32B32_SFLOAT:
      return 24;
    case gfx::Format::R32G32B32A32_SFLOAT:
      return 16;
    case gfx::Format::A8_UNORM:
      return 1;
    default:
    case gfx::Format::Undefined:
      return 0;
  }
}

inline u64 packed_image_size(u32 width, u32 height, gfx::Format format)
{
  return (u64) width * (u64) height * (u64) pixel_byte_size(format);
}

/// B: must be u8 or u8 const
///
/// This is a linear-tiled image with channels
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
/// - A8_UNORM
///
///
/// @offset: offset where the first row of the image begins from. this enables
/// using the correct offset for slicing the image along with the correct pitch.
/// @pitch: number of bytes to skip to get to the next row. a.k.a. row stride
///
template <typename B>
struct ImageSpan
{
  static_assert(std::is_same_v<B, u8> || std::is_same_v<B, u8 const>);

  Span<B>     span   = {};
  gfx::Format format = gfx::Format::Undefined;
  u64         pitch  = 0;
  u32         width  = 0;
  u32         height = 0;

  constexpr bool is_packed() const
  {
    return ((u64) width * (u64) pixel_byte_size(format)) == pitch;
  }

  constexpr u64 row_bytes() const
  {
    return (u64) width * (u64) pixel_byte_size(format);
  }

  constexpr bool is_empty() const
  {
    return width == 0 || height == 0 || pitch == 0 ||
           format == gfx::Format::Undefined || span.is_empty();
  }

  constexpr operator ImageSpan<B const>() const
  {
    return ImageSpan<B const>{.span   = Span<B const>{span},
                              .format = format,
                              .pitch  = pitch,
                              .width  = width,
                              .height = height};
  }

  constexpr ImageSpan slice(Vec2U offset, Vec2U extent) const
  {
    offset.x = offset.x > width ? width : offset.x;
    offset.y = offset.y > height ? height : offset.y;
    extent.x = (width - offset.x) > extent.x ? extent.x : (width - offset.x);
    extent.y = (height - offset.y) > extent.y ? extent.y : (height - offset.y);

    // trim down the span
    u64 const data_offset =
        offset.y * pitch + offset.x * pixel_byte_size(format);
    u64 const data_span = extent.y * pitch;

    return ImageSpan{.span   = span.slice(data_offset, data_span),
                     .format = format,
                     .pitch  = pitch,
                     .width  = extent.x,
                     .height = extent.y};
  }

  constexpr ImageSpan slice(Vec2U offset) const
  {
    return slice(offset, {U32_MAX, U32_MAX});
  }
};

template <typename T, typename U>
bool copy_image(ImageSpan<T const> const &src, ImageSpan<U> const &dst)
{
  if (src.format != dst.format || src.width > dst.width ||
      src.height > dst.height)
  {
    return false;
  }

  u8       *out       = dst.span.data;
  u8 const *in        = src.span.data;
  u64 const row_bytes = src.row_bytes();

  for (u64 irow = 0; irow < src.height;
       irow++, out += dst.pitch, in += src.pitch)
  {
    memcpy(out, in, row_bytes);
  }
}
}        // namespace ash
