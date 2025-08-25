/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/math.h"
#include "ashura/std/mem.h"
#include "ashura/std/types.h"

namespace ash
{

constexpr u64 pixel_size_bytes(u32x2 extent, u32 bytes_per_pixel)
{
  return (u64) extent.x() * (u64) extent.y() * (u64) bytes_per_pixel;
}

/// @brief A dense, multi-channel, and row-major image span, format insensitive.
/// @param stride number of pixels to skip to move from row i to row i+1
/// i+1
///
/// @tparam R pixel element type, one of [f32, u32, u8]
/// @tparam C number of channels in the image, range [1, 4]
template <typename R, u32 C>
struct ImageSpan
{
  using Element                     = R;
  static constexpr u32 NUM_CHANNELS = C;

  Span<R> channels = {};
  u32x2   extent   = {0, 0};
  u64     stride   = 0;

  constexpr bool is_empty() const
  {
    return extent.x() == 0 | extent.y() == 0;
  }

  /// @brief number of pixel elements to skip to move from row i to row i+1
  /// i+1
  constexpr u64 pitch() const
  {
    return stride * C;
  }

  constexpr ImageSpan slice(u32x2 offset, u32x2 extent) const
  {
    offset = offset.min(this->extent);
    extent = extent.min(this->extent - offset);

    u64 const data_offset = (offset.y() * stride + offset.x()) * C;
    u64 const data_span   = (extent.y() * stride) * C;

    return ImageSpan{.channels = channels.slice(data_offset, data_span),
                     .extent   = extent,
                     .stride   = stride};
  }

  constexpr ImageSpan slice(u32x2 offset) const
  {
    return slice(offset, u32x2::splat(U32_MAX));
  }

  constexpr operator ImageSpan<R const, C>() const
  {
    return ImageSpan<R const, C>{
      .channels = channels.as_const(), .extent = extent, .stride = stride};
  }

  constexpr ImageSpan<R const, C> as_const() const
  {
    return ImageSpan<R const, C>{
      .channels = channels.as_const(), .extent = extent, .stride = stride};
  }
};

/// @brief similar to ImageSpan but expresses the layers of a multi-layered
/// image
/// @tparam R pixel element type, one of [f32, u32, u8]
/// @tparam C number of channels in the image, range [1, 4]
template <typename R, u32 C>
struct ImageLayerSpan
{
  Span<R> channels = {};
  u32x2   extent   = {};
  u32     layers   = 0;

  constexpr operator ImageLayerSpan<R const, C>() const
  {
    return ImageLayerSpan<R const, C>{
      .channels = channels.as_const(), .extent = extent, .layers = layers};
  }

  constexpr ImageSpan<R, C> layer(u32 layer) const
  {
    u64 data_offset = (u64) layer * (u64) extent.x() * (u64) extent.y() * C;
    u64 data_span   = (u64) extent.x() * (u64) extent.y() * C;
    return ImageSpan<R, C>{.channels = channels.slice(data_offset, data_span),
                           .extent   = extent,
                           .stride   = extent.x()};
  }
};

template <typename T, u32 C>
void copy_image(ImageSpan<T const, C> src, ImageSpan<T, C> dst)
{
  src.extent = src.extent.min(dst.extent);

  auto const * ASH_RESTRICT in_row  = src.channels.data();
  auto * ASH_RESTRICT       out_row = dst.channels.data();

  for (isize i = 0; i < src.extent.y();
       i++, in_row += src.pitch(), out_row += dst.pitch())
  {
    mem::copy(Span{in_row, src.extent.x() * C}, out_row);
  }
}

template <typename T>
void copy_alpha_image_to_BGRA(ImageSpan<T const, 1> src, ImageSpan<T, 4> dst,
                              T B, T G, T R)
{
  src.extent = src.extent.min(dst.extent);

  auto const * ASH_RESTRICT in_row  = src.channels.data();
  auto * ASH_RESTRICT       out_row = dst.channels.data();

  for (isize i = 0; i < src.extent.y();
       i++, in_row += src.pitch(), out_row += dst.pitch())
  {
    auto const * ASH_RESTRICT in  = in_row;
    auto * ASH_RESTRICT       out = out_row;
    for (isize j = 0; j < src.extent.x(); j++, in += 1, out += 4)
    {
      out[0] = B;
      out[1] = G;
      out[2] = R;
      out[3] = in[0];
    }
  }
}

template <typename T>
void copy_RGBA_to_BGRA(ImageSpan<T const, 4> src, ImageSpan<T, 4> dst)
{
  src.extent = src.extent.min(dst.extent);

  auto const * ASH_RESTRICT in_row  = src.channels.data();
  auto * ASH_RESTRICT       out_row = dst.channels.data();

  for (isize i = 0; i < src.extent.y();
       i++, in_row += src.pitch(), out_row += dst.pitch())
  {
    auto const * ASH_RESTRICT in  = in_row;
    auto * ASH_RESTRICT       out = out_row;
    for (isize j = 0; j < src.extent.x(); j++, in += 4, out += 4)
    {
      out[0] = in[2];
      out[1] = in[1];
      out[2] = in[0];
      out[3] = in[3];
    }
  }
}

template <typename T>
void copy_RGB_to_BGRA(ImageSpan<T const, 3> src, ImageSpan<T, 4> dst, T A)
{
  src.extent = src.extent.min(dst.extent);

  auto const * in_row  = src.channels.data();
  auto *       out_row = dst.channels.data();

  for (isize i = 0; i < src.extent.y();
       i++, in_row += src.pitch(), out_row += dst.pitch())
  {
    auto const * in  = in_row;
    auto *       out = out_row;
    for (isize j = 0; j < src.extent.x(); j++, in += 3, out += 4)
    {
      out[0] = in[2];
      out[1] = in[1];
      out[2] = in[0];
      out[3] = A;
    }
  }
}

}    // namespace ash
