#include "ashura/engine/image_decoder.h"
#include "ashura/std/range.h"

extern "C"
{
#include "jerror.h"
#include "jpeglib.h"
#include "png.h"
#include "webp/decode.h"
#include "webp/types.h"
}

namespace ash
{

constexpr u64 rgba8_size(bool has_alpha, u32 width, u32 height)
{
  return ((u64) width) * ((u64) height) * (has_alpha ? 4ULL : 3ULL);
}

Result<Allocation, DecodeError> decode_webp(AllocatorImpl const &allocator,
                                            Span<u8 const>       bytes,
                                            ImageSpan<u8>       &span)
{
  WebPBitstreamFeatures features;

  if (WebPGetFeatures(bytes.data(), bytes.size(), &features) != VP8_STATUS_OK)
  {
    return Err{DecodeError::DecodeFailed};
  }

  u32 const pitch = features.width * (features.has_alpha == 0 ? 3U : 4U);
  u64 const buffer_size =
      rgba8_size(features.has_alpha != 0, features.width, features.height);
  u8 *buffer = allocator.allocate_typed<u8>(buffer_size);

  if (buffer == nullptr)
  {
    return Err{DecodeError::OutOfMemory};
  }

  if (features.has_alpha != 0)
  {
    if (WebPDecodeRGBAInto(bytes.data(), bytes.size(), buffer, buffer_size,
                           pitch) == nullptr)
    {
      allocator.deallocate_typed(buffer, buffer_size);
      return Err{DecodeError::DecodeFailed};
    }
  }
  else
  {
    if (WebPDecodeRGBInto(bytes.data(), bytes.size(), buffer, buffer_size,
                          pitch) == nullptr)
    {
      allocator.deallocate_typed(buffer, buffer_size);
      return Err{DecodeError::DecodeFailed};
    }
  }

  span = ImageSpan<u8>{.span   = {buffer, buffer_size},
                       .format = features.has_alpha == 0 ?
                                     gfx::Format::R8G8B8_UNORM :
                                     gfx::Format::R8G8B8A8_UNORM,
                       .pitch  = pitch,
                       .width  = (u32) features.width,
                       .height = (u32) features.height};

  return Ok{Allocation{.memory = buffer, .alignment = 1, .size = buffer_size}};
}

inline void png_stream_reader(png_structp png_ptr, unsigned char *out,
                              usize nbytes_to_read)
{
  Span<u8 const> *input = (Span<u8 const> *) png_get_io_ptr(png_ptr);
  mem::copy(input->slice(0, nbytes_to_read), out);
  *input = input->slice(nbytes_to_read);
}

Result<Allocation, DecodeError> decode_png(AllocatorImpl const &allocator,
                                           Span<u8 const>       bytes,
                                           ImageSpan<u8>       &span)
{
  // skip magic number
  bytes = bytes.slice(8);

  png_structp png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  if (png_ptr == nullptr)
  {
    return Err{DecodeError::OutOfMemory};
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (png_ptr == nullptr)
  {
    png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    return Err{DecodeError::OutOfMemory};
  }

  Span stream = bytes;
  png_set_read_fn(png_ptr, &stream, png_stream_reader);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  u32 width;
  u32 height;
  int color_type;
  int bit_depth;

  u32 status = png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
                            &color_type, nullptr, nullptr, nullptr);

  if (status != 1)
  {
    return Err{DecodeError::DecodeFailed};
  }

  if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGBA)
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return Err{DecodeError::UnsupportedFormat};
  }

  usize       ncomponents = (color_type == PNG_COLOR_TYPE_RGB) ? 3 : 4;
  gfx::Format fmt         = (ncomponents == 3) ? gfx::Format::R8G8B8_UNORM :
                                                 gfx::Format::R8G8B8A8_UNORM;
  u32         pitch       = width * ncomponents;
  u64         buffer_size = (u64) height * pitch;

  u8 *buffer = allocator.allocate_typed<u8>(buffer_size);

  if (buffer == nullptr)
  {
    return Err{DecodeError::OutOfMemory};
  }

  u8 *row = buffer;
  for (u32 i = 0; i < height; i++)
  {
    png_read_row(png_ptr, row, nullptr);
    row += pitch;
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

  span = ImageSpan<u8>{.span   = {buffer, buffer_size},
                       .format = fmt,
                       .pitch  = pitch,
                       .width  = width,
                       .height = height};

  return Ok{Allocation{.memory = buffer, .alignment = 1, .size = buffer_size}};
}

Result<Allocation, DecodeError> decode_jpg(AllocatorImpl const &allocator,
                                           Span<u8 const>       bytes,
                                           ImageSpan<u8>       &span)
{
  jpeg_decompress_struct info;
  jpeg_error_mgr         error_mgr;
  info.err = jpeg_std_error(&error_mgr);
  jpeg_create_decompress(&info);

  jpeg_mem_src(&info, bytes.data(), static_cast<unsigned long>(bytes.size()));

  if (jpeg_read_header(&info, true) != JPEG_HEADER_OK)
  {
    jpeg_destroy_decompress(&info);
    return Err{DecodeError::DecodeFailed};
  }

  if (jpeg_start_decompress(&info) == 0)
  {
    jpeg_destroy_decompress(&info);
    return Err{DecodeError::DecodeFailed};
  }

  if (info.num_components != 3 && info.num_components != 4)
  {
    jpeg_destroy_decompress(&info);
    return Err{DecodeError::UnsupportedFormat};
  }

  u32         width       = info.output_width;
  u32         height      = info.output_height;
  u32         ncomponents = info.num_components;
  u32         pitch       = width * ncomponents;
  u64         buffer_size = (u64) height * pitch;
  gfx::Format fmt         = ncomponents == 3 ? gfx::Format::R8G8B8_UNORM :
                                               gfx::Format::R8G8B8A8_UNORM;

  u8 *buffer = allocator.allocate_typed<u8>(buffer_size);
  if (buffer == nullptr)
  {
    jpeg_destroy_decompress(&info);
    return Err{DecodeError::OutOfMemory};
  }

  u8 *scanline = buffer;
  while (info.output_scanline < height)
  {
    u8 *scanlines[] = {scanline};
    scanline += (u64) jpeg_read_scanlines(&info, scanlines, 1) * pitch;
  }

  jpeg_finish_decompress(&info);
  jpeg_destroy_decompress(&info);

  span = ImageSpan<u8>{.span   = {buffer, buffer_size},
                       .format = fmt,
                       .pitch  = pitch,
                       .width  = width,
                       .height = height};

  return Ok{Allocation{.memory = buffer, .alignment = 1, .size = buffer_size}};
}

Result<Allocation, DecodeError> decode_image(AllocatorImpl const &allocator,
                                             Span<u8 const>       bytes,
                                             ImageSpan<u8>       &span)
{
  constexpr u8 JPG_MAGIC[] = {0xFF, 0xD8, 0xFF};

  constexpr u8 PNG_MAGIC[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

  // RIFF-[file size: 4 bytes]-WEBP
  constexpr u8 WEBP_MAGIC1[] = {'R', 'I', 'F', 'F'};
  constexpr u8 WEBP_MAGIC2[] = {'W', 'E', 'B', 'P'};

  if (range_equal(bytes.slice(0, size(JPG_MAGIC)), JPG_MAGIC))
  {
    return decode_jpg(allocator, bytes, span);
  }

  if (range_equal(bytes.slice(0, size(PNG_MAGIC)), PNG_MAGIC))
  {
    return decode_png(allocator, bytes, span);
  }

  if (range_equal(bytes.slice(0, size(WEBP_MAGIC1)), WEBP_MAGIC1) &&
      range_equal(bytes.slice(8, size(WEBP_MAGIC2)), WEBP_MAGIC2))
  {
    return decode_webp(allocator, bytes, span);
  }

  return Err{DecodeError::UnsupportedFormat};
}

}        // namespace ash