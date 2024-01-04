#pragma once
#include "ashura/gfx.h"
#include "ashura/primitives.h"
#include "ashura/utils.h"
#include "stx/memory.h"
#include "stx/span.h"
#include "stx/try_ok.h"

namespace ash
{

inline u64 pixel_byte_size(gfx::Format fmt)
{
  switch (fmt)
  {
    case gfx::Format::Undefined:
      return 0;
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
    case gfx::Format::R32G32_SFLOAT:
      return 8;
    case gfx::Format::R32G32B32_SFLOAT:
      return 24;
    case gfx::Format::R32G32B32A32_SFLOAT:
      return 16;
    case gfx::Format::A8_UNORM:
      return 1;
    default:
      ASH_UNIMPLEMENTED();
  }
}

inline u64 packed_image_size(u32 width, u32 height, gfx::Format format)
{
  return (u64) width * (u64) height * pixel_byte_size(format);
}

/// B: must be u8 or u8 const
///
/// This is a linear-tiled image with homogenic channels
///
/// Supported Formats:
/// Undefined
/// R8_UNORM
/// R8G8B8_UNORM
/// B8G8R8_UNORM
/// R8G8B8A8_UNORM
/// B8G8R8A8_UNORM
/// R32G32_SFLOAT
/// R32G32B32_SFLOAT
/// R32G32B32A32_SFLOAT
/// A8_UNORM
///
///
/// @pitch: number of bytes to skip to get to the next row
///
template <typename B>
struct ImageView
{
  static_assert(std::is_same_v<B, u8> || std::is_same_v<B, u8 const>);

  Span<B>     span;
  u32         width  = 0;
  u32         height = 0;
  u64         pitch  = 0;
  gfx::Format format = gfx::Format::Undefined;

  u64 row_bytes() const
  {
    return width * pixel_byte_size(format);
  }

  operator ImageView<B const>() const
  {
    return ImageView<B const>{.span   = Span<B const>{span},
                              .width  = width,
                              .height = height,
                              .pitch  = pitch,
                              .format = format};
  }

  ImageView subview(Vec2U offset, Vec2U extent = Vec2U{U32_MAX, U32_MAX}) const
  {
    ASH_CHECK(sub_offset.x <= extent.x);
    ASH_CHECK(sub_offset.y <= extent.y);
    ASH_CHECK(sub_offset.x + sub_extent.x <= extent.x);
    ASH_CHECK(sub_offset.y + sub_extent.y <= extent.y);

    u64 const pixel_bytes = pixel_byte_size(format);
    u64 const byte_offset = sub_offset.y * pitch + sub_offset.x * pixel_bytes;
    u64 const byte_span =
        sub_extent.y > 0 ?
            (sub_extent.x * pixel_bytes + (sub_extent.y - 1U) * pitch) :
            0;

    return ImageView{.span   = span.slice(byte_offset, byte_span),
                     .extent = sub_extent,
                     .pitch  = pitch,
                     .format = format};
  }
};

template <typename T, typename U>
void copy_image(ImageView<T const> src, ImageView<U> dst)
{
  ASH_CHECK(src.format == dst.format);
  ASH_CHECK(extent.x <= view.extent.x);
  ASH_CHECK(extent.y <= view.extent.y);

  u8       *out       = span.as_u8().data();
  u8 const *in        = view.span.as_u8().data();
  u64 const row_bytes = this->row_bytes();

  for (u64 irow = 0; irow < extent.y; irow++, out += pitch, in += view.pitch)
  {
    Span{out, row_bytes}.copy(Span{in, row_bytes});
  }
}

// struct ImageBuffer
// {
//   stx::Memory memory;
//   u64         width  = 0;
//   u64         height = 0;
//   gfx::Format format = gfx::Format::Undefined;

//   static stx::Result<ImageBuffer, stx::AllocError> make(ash::Extent extent,
//                                                         gfx::Format format)
//   {
//     TRY_OK(memory, stx::mem::allocate(stx::os_allocator,
//                                       packed_image_size(extent, format)));
//     return stx::Ok(ImageBuffer{
//         .memory = std::move(memory), .extent = extent, .format = format});
//   }

//   u8 const *data() const
//   {
//     return (u8 const *) memory.handle;
//   }

//   u8 *data()
//   {
//     return (u8 *) memory.handle;
//   }

//   u64 row_bytes() const
//   {
//     return extent.x * pixel_byte_size(format);
//   }

//   u64 pitch() const
//   {
//     return row_bytes();
//   }

//   u64 size_bytes() const
//   {
//     return packed_image_size(extent, format);
//   }

//   Span<u8 const> span() const
//   {
//     return Span{data(), size_bytes()};
//   }

//   Span<u8> span()
//   {
//     return Span{data(), size_bytes()};
//   }

//   operator ImageView<u8 const>() const
//   {
//     return ImageView<u8 const>{
//         .span = span(), .extent = extent, .pitch = pitch(), .format =
//         format};
//   }

//   operator ImageView<u8>()
//   {
//     return ImageView<u8>{
//         .span = span(), .extent = extent, .pitch = pitch(), .format =
//         format};
//   }

//   ImageView<u8 const> view() const
//   {
//     return static_cast<ImageView<u8 const>>(*this);
//   }

//   ImageView<u8> view()
//   {
//     return static_cast<ImageView<u8>>(*this);
//   }
// };

}        // namespace ash
