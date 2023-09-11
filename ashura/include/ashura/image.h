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

inline usize fitted_byte_size(Extent extent, ImageFormat format)
{
  return extent.height * extent.width * pixel_byte_size(format);
}

template <typename B>
struct ImageView
{
  static_assert(sizeof(B) == 1);

  stx::Span<B> span;
  ash::Extent  extent;
  usize        pitch  = 0;
  ImageFormat  format = ImageFormat::Rgba8888;

  bool is_fitted() const
  {
    return pitch == (extent.width * pixel_byte_size(format));
  }

  usize row_bytes() const
  {
    return extent.width * pixel_byte_size(format);
  }

  usize fitted_size_bytes() const
  {
    return fitted_byte_size(extent, format);
  }

  operator ImageView<B const>() const
  {
    return ImageView<B const>{.span = span.as_const(), .extent = extent, .pitch = pitch, .format = format};
  }

  ImageView subview(URect slice) const
  {
    ASH_CHECK(slice.min().x <= extent.width);
    ASH_CHECK(slice.min().y <= extent.height);
    ASH_CHECK(slice.max().x <= extent.width);
    ASH_CHECK(slice.max().y <= extent.height);

    usize const pixel_bytes = pixel_byte_size(format);
    usize const byte_offset = slice.offset.y * pitch + slice.offset.x * pixel_bytes;
    usize const byte_span   = slice.extent.height > 0 ? (slice.extent.width * pixel_bytes + (slice.extent.height - 1U) * pitch) : 0;

    return ImageView{.span = span.slice(byte_offset, byte_span), .extent = slice.extent, .pitch = pitch, .format = format};
  }

  ImageView subview(Offset slice) const
  {
    return subview(URect{.offset = slice, .extent = ash::Extent{.width = extent.width - std::min(extent.width, slice.x), .height = extent.height - std::min(extent.height, slice.y)}});
  }

  ImageView<B const> as_const() const
  {
    return ImageView{.span = span.as_const(), .extent = extent, .pitch = pitch, .format = format};
  }

  template <typename U>
  ImageView copy(ImageView<U> view) const
  {
    static_assert(!std::is_const_v<B>);

    ASH_CHECK(format == view.format);
    ASH_CHECK(extent.width <= view.extent.width);
    ASH_CHECK(extent.height <= view.extent.height);

    u8         *out       = span.as_u8().data();
    u8 const   *in        = view.span.as_u8().data();
    usize const row_bytes = this->row_bytes();

    for (usize irow = 0; irow < extent.height; irow++, out += pitch, in += view.pitch)
    {
      stx::Span{out, row_bytes}.copy(stx::Span{in, row_bytes});
    }

    return *this;
  }
};

struct ImageBuffer
{
  stx::Memory memory;
  ash::Extent extent;
  ImageFormat format = ImageFormat::Rgba8888;

  static stx::Result<ImageBuffer, stx::AllocError> make(ash::Extent extent, ImageFormat format)
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

  usize row_bytes() const
  {
    return extent.width * pixel_byte_size(format);
  }

  usize pitch() const
  {
    return row_bytes();
  }

  usize size_bytes() const
  {
    return fitted_byte_size(extent, format);
  }

  stx::Span<u8 const> span() const
  {
    return stx::Span{data(), size_bytes()};
  }

  stx::Span<u8> span()
  {
    return stx::Span{data(), size_bytes()};
  }

  operator ImageView<u8 const>() const
  {
    return ImageView<u8 const>{.span = span(), .extent = extent, .pitch = pitch(), .format = format};
  }

  operator ImageView<u8>()
  {
    return ImageView<u8>{.span = span(), .extent = extent, .pitch = pitch(), .format = format};
  }

  ImageView<u8 const> view() const
  {
    return AS(ImageView<u8 const>, *this);
  }

  ImageView<u8> view()
  {
    return AS(ImageView<u8>, *this);
  }

  void resize(ash::Extent new_extent)
  {
    if (extent.area() != new_extent.area())
    {
      stx::mem::reallocate(memory, fitted_byte_size(new_extent, format)).unwrap();
    }
    extent = new_extent;
  }
};

}        // namespace ash
