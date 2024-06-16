#pragma once
#include "ashura/gfx/gfx.h"
#include "ashura/std/error.h"
#include "ashura/std/types.h"

namespace ash
{

namespace gfx
{

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
      UNREACHABLE();
  }
}

inline u64 packed_image_size(u32 width, u32 height, Format format)
{
  return (u64) width * (u64) height * (u64) pixel_pitch(format);
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
///
/// @param offset offset where the first row of the image begins from. this
/// enables using the correct offset for slicing the image along with the
/// correct pitch.
/// @param pitch number of bytes to skip to get to the next row. a.k.a. row
/// stride
///
template <typename B>
struct ImageSpan
{
  static_assert(std::is_same_v<B, u8> || std::is_same_v<B, u8 const>);

  Span<B> span   = {};
  Format  format = Format::Undefined;
  u32     pitch  = 0;
  u32     width  = 0;
  u32     height = 0;

  constexpr bool is_packed() const
  {
    return ((u64) width * (u64) pixel_pitch(format)) == pitch;
  }

  constexpr u64 row_bytes() const
  {
    return (u64) width * (u64) pixel_pitch(format);
  }

  constexpr bool is_empty() const
  {
    return width == 0 || height == 0 || pitch == 0 ||
           format == Format::Undefined || span.is_empty();
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
    u64 const data_offset = offset.y * pitch + offset.x * pixel_pitch(format);
    u64 const data_span   = extent.y * pitch;

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
void copy_image(ImageSpan<T const> const &src, ImageSpan<U> const &dst)
{
  CHECK(src.format == dst.format);
  CHECK(src.width <= dst.width);
  CHECK(src.height <= dst.height);

  u8       *out       = dst.span.data();
  u8 const *in        = src.span.data();
  u64 const row_bytes = src.row_bytes();

  for (u32 i = 0; i < src.height; i++, out += dst.pitch, in += src.pitch)
  {
    memcpy(out, in, row_bytes);
  }
}

template <typename T, typename U>
void copy_alpha_image_to_BGRA(ImageSpan<T const> const &src,
                              ImageSpan<U> const &dst, u8 B, u8 G, u8 R)
{
  CHECK(src.format == gfx::Format::R8_UNORM);
  CHECK(dst.format == gfx::Format::B8G8R8A8_UNORM);
  CHECK(src.width <= dst.width);
  CHECK(src.height <= dst.height);

  u8       *out = dst.span.data();
  u8 const *in  = src.span.data();

  for (u32 i = 0; i < src.height; i++, out += dst.pitch, in += src.pitch)
  {
    for (u32 j = 0; j < src.width; j++)
    {
      u32 pixel      = j << 2;
      out[pixel]     = B;
      out[pixel + 1] = G;
      out[pixel + 2] = R;
      out[pixel + 3] = in[j];
    }
  }
}

}        // namespace gfx

}        // namespace ash
