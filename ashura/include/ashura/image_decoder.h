#pragma once
#include <utility>

#include "ashura/image.h"
#include "ashura/primitives.h"
#include "stx/span.h"

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

enum class ImageLoadError : u8
{
  /// the image path provided is invalid
  InvalidPath,
  /// detected image but image seems to be corrupted
  InvalidData,
  /// image contains unsupported channel types
  UnsupportedChannels,
  /// the image file format is unsupported
  UnsupportedFormat
};

inline stx::Result<ImageBuffer, ImageLoadError> decode_webp(
    stx::Span<u8 const> data)
{
  int width, height;

  if (!WebPGetInfo(data.data(), data.size(), &width, &height))
  {
    return stx::Err(ImageLoadError::InvalidData);
  }

  stx::Memory memory =
      stx::mem::allocate(stx::os_allocator, width * height * 4).unwrap();

  u8 *pixels = AS(u8 *, memory.handle);

  if (WebPDecodeRGBAInto(data.data(), data.size(), pixels, width * height * 4,
                         width * 4) == nullptr)
  {
    return stx::Err(ImageLoadError::InvalidData);
  }

  return stx::Ok(ImageBuffer{.memory = std::move(memory),
                             .extent = extent{AS(u32, width), AS(u32, height)},
                             .format = ImageFormat::Rgba});
}

inline void png_stream_reader(png_structp png_ptr, unsigned char *out,
                              usize nbytes_to_read)
{
  stx::Span<u8 const> *input =
      AS(stx::Span<u8 const> *, png_get_io_ptr(png_ptr));
  stx::Span{out, nbytes_to_read}.copy(*input);
  *input = input->slice(nbytes_to_read);
}

inline stx::Result<ImageBuffer, ImageLoadError> decode_png(
    stx::Span<u8 const> data)
{
  // skip magic number
  data = data.slice(8);

  png_structp png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  ASH_CHECK(png_ptr != nullptr);

  png_infop info_ptr = png_create_info_struct(png_ptr);

  ASH_CHECK(info_ptr != nullptr);

  stx::Span stream = data;
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
    return stx::Err(ImageLoadError::InvalidData);

  usize ncomponents = 0;

  if (color_type == PNG_COLOR_TYPE_RGB)
  {
    ncomponents = 3;
  }
  else if (color_type == PNG_COLOR_TYPE_RGBA)
  {
    ncomponents = 4;
  }
  else
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return stx::Err(ImageLoadError::UnsupportedChannels);
  }

  stx::Memory pixels_mem =
      stx::mem::allocate(stx::os_allocator, width * height * 4UL).unwrap();

  stx::Memory row_mem =
      stx::mem::allocate(stx::os_allocator, width * ncomponents).unwrap();

  u8 *output = AS(u8 *, pixels_mem.handle);
  u8 *row    = AS(u8 *, row_mem.handle);

  for (usize i = 0; i < height; i++)
  {
    png_read_row(png_ptr, row, nullptr);

    u8 const *input = row;

    if (ncomponents == 3)
    {
      for (usize i = 0; i < width; i++)
      {
        output[0] = input[0];
        output[1] = input[1];
        output[2] = input[2];
        output[3] = 0xFF;

        input += 3;
        output += 4;
      }
    }
    else if (ncomponents == 4)
    {
      for (usize i = 0; i < width * 4; i++)
      {
        output[0] = input[i];
        output++;
      }
    }
    else
    {
      png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
      return stx::Err(ImageLoadError::UnsupportedChannels);
    }
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

  return stx::Ok(ImageBuffer{.memory = std::move(pixels_mem),
                             .extent = extent{width, height},
                             .format = ImageFormat::Rgba});
}

inline stx::Result<ImageBuffer, ImageLoadError> decode_jpg(
    stx::Span<u8 const> bytes)
{
  jpeg_decompress_struct info;
  jpeg_error_mgr         error_mgr;
  info.err = jpeg_std_error(&error_mgr);
  jpeg_create_decompress(&info);

  jpeg_mem_src(&info, bytes.data(), AS(unsigned long, bytes.size()));
  int return_code = jpeg_read_header(&info, true);
  jpeg_start_decompress(&info);

  u32 width       = info.output_width;
  u32 height      = info.output_height;
  u32 ncomponents = info.num_components;

  if (ncomponents != 3 && ncomponents != 4)
  {
    jpeg_destroy_decompress(&info);
    return stx::Err(ImageLoadError::UnsupportedChannels);
  }

  stx::Memory row_mem =
      stx::mem::allocate(stx::os_allocator, width * ncomponents).unwrap();

  stx::Memory pixels_mem =
      stx::mem::allocate(stx::os_allocator, height * width * 4).unwrap();

  u8 *row    = AS(u8 *, row_mem.handle);
  u8 *pixels = AS(u8 *, pixels_mem.handle);

  while (info.output_scanline < height)
  {
    u8 *scanlines[] = {row};
    jpeg_read_scanlines(&info, scanlines, 1);

    if (ncomponents == 3)
    {
      u8 const *input = row;
      for (usize i = 0; i < width; i++)
      {
        pixels[0] = input[0];
        pixels[1] = input[1];
        pixels[2] = input[2];
        pixels[3] = 0xFF;

        input += 3;
        pixels += 4;
      }
    }
    else if (ncomponents == 4)
    {
      u8 const *input = row;
      for (usize i = 0; i < width * 4; i++)
      {
        pixels[0] = input[i];
        pixels++;
      }
    }
    else
    {
      jpeg_destroy_decompress(&info);
      return stx::Err(ImageLoadError::UnsupportedChannels);
    }
  }

  jpeg_finish_decompress(&info);
  jpeg_destroy_decompress(&info);

  return stx::Ok(ImageBuffer{.memory = std::move(pixels_mem),
                             .extent = extent{width, height},
                             .format = ImageFormat::Rgba});
}

// TODO(lamarrr): support avif
inline stx::Result<ImageBuffer, ImageLoadError> decode_image(
    stx::Span<u8 const> bytes)
{
  constexpr u8 JPG_MAGIC[] = {0xFF, 0xD8, 0xFF};

  constexpr u8 PNG_MAGIC[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

  // RIFF[file size: 4 bytes]WEBP
  constexpr u8 WEBP_MAGIC1[] = {'R', 'I', 'F', 'F'};
  constexpr u8 WEBP_MAGIC2[] = {'W', 'E', 'B', 'P'};

  if (bytes.slice(0, std::size(JPG_MAGIC)).equals(stx::Span{JPG_MAGIC}))
  {
    return decode_jpg(bytes);
  }
  else if (bytes.slice(0, std::size(PNG_MAGIC))
               .equals(stx::Span{PNG_MAGIC}))
  {
    return decode_png(bytes);
  }
  else if (bytes.slice(0, std::size(WEBP_MAGIC1))
               .equals(stx::Span{WEBP_MAGIC1}) &&
           bytes.slice(8, std::size(WEBP_MAGIC2))
               .equals(stx::Span{WEBP_MAGIC2}))
  {
    return decode_webp(bytes);
  }
  else
  {
    return stx::Err(ImageLoadError::UnsupportedFormat);
  }
}

}        // namespace ash
