#pragma once
#include "ashura/primitives.h"
#include "stx/memory.h"
#include "stx/span.h"
#include "stx/try_ok.h"

namespace ash
{

namespace gfx
{

/// NOTE: resource image with index 0 must be a transparent white image
using image = u64;

constexpr image WHITE_IMAGE = 0;

}        // namespace gfx

enum class ImageFormat : u8
{
  /// R8G8B8A8 image, stored on GPU as is
  Rgba8888,
  /// B8G8R8A8 image, stored on GPU as is
  Bgra8888,
  /// R8G8B8 image, stored on GPU as R8G8B8A8 with alpha = 255
  Rgb888,
  /// R8 image, stored on GPU as is
  R8
};

enum class ColorSpace : u8
{
  Rgb,
  Srgb,
  AdobeRgb,
  Dp3,
  DciP3
};

inline usize pixel_byte_size(ImageFormat fmt)
{
  switch (fmt)
  {
    case ImageFormat::Rgba8888:
      return 4;
    case ImageFormat::Bgra8888:
      return 4;
    case ImageFormat::Rgb888:
      return 3;
    case ImageFormat::R8:
      return 1;
    default:
      ASH_UNREACHABLE();
  }
}

inline usize fitted_byte_size(extent extent, ImageFormat format)
{
  return extent.height * extent.width * pixel_byte_size(format);
}

template <typename ByteT>
inline stx::Span<ByteT> view_slice(stx::Span<ByteT> src_span, extent src_extent, usize pitch, ImageFormat format, urect slice)
{
  static_assert(sizeof(ByteT) == 1);
  ASH_CHECK(slice.min().x <= src_extent.width);
  ASH_CHECK(slice.min().y <= src_extent.height);
  ASH_CHECK(slice.max().x <= src_extent.width);
  ASH_CHECK(slice.max().y <= src_extent.height);

  usize const pixel_bytes = pixel_byte_size(format);
  usize const byte_offset = slice.offset.y * pitch + slice.offset.x * pixel_bytes;
  usize const byte_span   = slice.extent.height > 0 ? (slice.extent.width * pixel_bytes + (slice.extent.height - 1U) * pitch) : 0;

  return src_span.slice(byte_offset, byte_span);
}

template<typename T>
struct ImageView{
static_assert( std::is_same_v<T, u8> || std::is_same_v<T, u8 const> );
stx::Span<u8 const> span;
  ash::extent         extent;
  usize               pitch  = 0;
  ImageFormat         format = ImageFormat::Rgba8888;
};

struct ImageView
{
  stx::Span<u8 const> span;
  ash::extent         extent;
  usize               pitch  = 0;
  ImageFormat         format = ImageFormat::Rgba8888;

  ImageView subview(urect slice) const
  {
    return ImageView{.span   = view_slice(span, extent, pitch, format, slice),
                     .extent = slice.extent,
                     .pitch  = pitch,
                     .format = format};
  }

  ImageView subview(offset slice) const
  {
    return subview(urect{.offset = slice,
                         .extent = ash::extent{.width  = extent.width - std::min(extent.width, slice.x),
                                               .height = extent.height - std::min(extent.height, slice.y)}});
  }
};

struct ImageMutView
{
  stx::Span<u8> span;
  ash::extent   extent;
  usize         pitch  = 0;
  ImageFormat   format = ImageFormat::Rgba8888;

  constexpr operator ImageView() const
  {
    return ImageView{.span = span, .extent = extent, .pitch = pitch, .format = format};
  }

  ImageMutView subview(urect slice)
  {
    return ImageMutView{.span   = view_slice(span, extent, pitch, format, slice),
                        .extent = slice.extent,
                        .pitch  = pitch,
                        .format = format};
  }

  ImageMutView subview(offset slice)
  {
    return subview(urect{.offset = slice,
                         .extent = ash::extent{.width  = extent.width - std::min(extent.width, slice.x),
                                               .height = extent.height - std::min(extent.height, slice.y)}});
  }

  ImageView subview(urect slice) const
  {
    return ImageView{.span   = view_slice(span, extent, pitch, format, slice),
                     .extent = slice.extent,
                     .pitch  = pitch,
                     .format = format};
  }

  ImageView subview(offset slice) const
  {
    return subview(urect{.offset = slice,
                         .extent = ash::extent{.width  = extent.width - std::min(extent.width, slice.x),
                                               .height = extent.height - std::min(extent.height, slice.y)}});
  }

  ImageMutView copy(ImageView const &view) const
  {
    ASH_CHECK(format == view.format);
    ASH_CHECK(extent.width <= view.extent.width);
    ASH_CHECK(extent.height <= view.extent.height);

    u8         *out       = span.data();
    u8 const   *in        = view.span.begin();
    usize const row_bytes = extent.width * pixel_byte_size(format);

    for (usize irow = 0; irow < extent.height; out += pitch, in += view.pitch)
    {
      stx::Span{out, row_bytes}.copy(stx::Span{in, row_bytes});
    }

    return *this;
  }
};

struct ImageBuffer
{
  stx::Memory memory;
  ash::extent extent;
  ImageFormat format = ImageFormat::Rgba8888;

  static stx::Result<ImageBuffer, stx::AllocError> make(ash::extent extent, ImageFormat format)
  {
    TRY_OK(memory, stx::mem::allocate(stx::os_allocator, fitted_byte_size(extent, format)));
    return stx::Ok(ImageBuffer{.memory = std::move(memory), .extent = extent, .format = format});
  }

  u8 const *data() const
  {
    return AS(u8 *, memory.handle);
  }

  u8 *data()
  {
    return AS(u8 *, memory.handle);
  }

  u32 pitch() const
  {
    return extent.width * pixel_byte_size(format);
  }

  usize size_bytes() const
  {
    return extent.height * pitch();
  }

  stx::Span<u8 const> span() const
  {
    return stx::Span{data(), size_bytes()};
  }

  stx::Span<u8> span()
  {
    return stx::Span{data(), size_bytes()};
  }

  operator ImageView() const
  {
    return ImageView{.span = span(), .extent = extent, .pitch = pitch(), .format = format};
  }

  operator ImageMutView()
  {
    return ImageMutView{.span = span(), .extent = extent, .pitch = pitch(), .format = format};
  }

  ImageView view() const
  {
    return AS(ImageView, *this);
  }

  ImageMutView view()
  {
    return AS(ImageMutView, *this);
  }

  void resize(ash::extent new_extent)
  {
    if (extent.area() != new_extent.area())
    {
      stx::mem::reallocate(memory, fitted_byte_size(new_extent, format)).unwrap();
    }
    extent = new_extent;
  }
};

}        // namespace ash
