#pragma once

#include "ashura/std/error.h"
#include "ashura/std/mem.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief A dense, multi-channel, and row-major image span
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

  Span<R> data   = {};
  u32     width  = 0;
  u32     height = 0;
  u64     stride = 0;

  constexpr bool is_empty() const
  {
    return width == 0 || height == 0;
  }

  /// @brief number of pixel elements to skip to move from row i to row i+1
  /// i+1
  constexpr u64 pitch() const
  {
    return stride * C;
  }

  constexpr ImageSpan slice(Vec2U offset, Vec2U extent) const
  {
    offset.x = offset.x > width ? width : offset.x;
    offset.y = offset.y > height ? height : offset.y;
    extent.x = (width - offset.x) > extent.x ? extent.x : (width - offset.x);
    extent.y = (height - offset.y) > extent.y ? extent.y : (height - offset.y);

    u64 const data_offset = offset.y * stride * C + offset.x * C;
    u64 const data_span   = extent.y * stride * C;

    return ImageSpan{.data   = data.slice(data_offset, data_span),
                     .width  = extent.x,
                     .height = extent.y,
                     .stride = stride};
  }

  constexpr ImageSpan slice(Vec2U offset) const
  {
    return slice(offset, Vec2::splat(U32_MAX));
  }

  constexpr operator ImageSpan<R const, C>() const
  {
    return ImageSpan<R const, C>{.data   = Span<R const>{data},
                                 .width  = width,
                                 .height = height,
                                 .stride = stride};
  }
};

/// @brief similar to ImageSpan but expresses the layers of a multi-layered
/// image
/// @tparam R pixel element type, one of [f32, u32, u8]
/// @tparam C number of channels in the image, range [1, 4]
template <typename R, u32 C>
struct ImageLayerSpan
{
  Span<R> data   = {};
  u32     width  = 0;
  u32     height = 0;
  u32     layers = 0;

  constexpr operator ImageLayerSpan<R const, C>() const
  {
    return ImageLayerSpan<R const, C>{.data   = Span<R const>{data},
                                      .width  = width,
                                      .height = height,
                                      .layers = layers};
  }

  constexpr ImageSpan<R, C> get_layer(u32 layer) const
  {
    u64 data_offset = (u64) layer * (u64) width * (u64) height * C;
    u64 data_span   = (u64) width * (u64) height * C;
    return ImageSpan{.data   = data.slice(data_offset, data_span),
                     .width  = width,
                     .height = height,
                     .stride = width};
  }
};

template <typename T, u32 C>
void copy_image(ImageSpan<T const, C> src, ImageSpan<T const, C> dst)
{
  src.width  = min(src.width, dst.width);
  src.height = min(src.height, dst.height);

  T const *in  = src.span.data();
  T       *out = dst.span.data();

  for (u32 i = 0; i < src.height; i++, in += src.pitch(), out += dst.pitch())
  {
    mem::copy(in, out, src.width * C);
  }
}

template <typename T>
void copy_alpha_image_to_BGRA(ImageSpan<T const, 1> src, ImageSpan<T, 4> dst,
                              T B, T G, T R)
{
  src.width  = min(src.width, dst.width);
  src.height = min(src.height, dst.width);

  T const *in  = src.data.data();
  T       *out = dst.data.data();

  for (u32 i = 0; i < src.height; i++, in += src.pitch(), out += dst.pitch())
  {
    T const *in_p  = in;
    T       *out_p = out;
    for (u32 j = 0; j < src.width; j++, in_p += 1, out_p += 4)
    {
      out_p[0] = B;
      out_p[1] = G;
      out_p[2] = R;
      out_p[3] = in_p[0];
    }
  }
}

}        // namespace ash