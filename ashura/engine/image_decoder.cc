/// SPDX-License-Identifier: MIT
#include "ashura/engine/image_decoder.h"
#include "ashura/std/image.h"
#include "ashura/std/range.h"

extern "C"
{
#include "jerror.h"
#include "jpeglib.h"
#include "png.h"
#include "webp/decode.h"
}

namespace ash
{

Result<DecodedImageInfo, ImageLoadErr> decode_webp(Span<u8 const> bytes,
                                                   Vec<u8> &      channels)
{
  WebPBitstreamFeatures features;

  if (WebPGetFeatures(bytes.data(), bytes.size(), &features) != VP8_STATUS_OK)
  {
    return Err{ImageLoadErr::DecodeFailed};
  }

  u32 const pitch       = features.width * (features.has_alpha == 0 ? 3U : 4U);
  gpu::Format const fmt = features.has_alpha == 0 ? gpu::Format::R8G8B8_UNORM :
                                                    gpu::Format::R8G8B8A8_UNORM;
  Vec2U             extent{(u32) features.width, (u32) features.height};

  u64 const buffer_size = pixel_size_bytes(extent, features.has_alpha ? 3 : 4);

  if (!channels.resize_uninit(buffer_size))
  {
    return Err{ImageLoadErr::OutOfMemory};
  }

  if (features.has_alpha != 0)
  {
    if (WebPDecodeRGBAInto(bytes.data(), bytes.size(), channels.data(),
                           buffer_size, pitch) == nullptr)
    {
      channels.clear();
      return Err{ImageLoadErr::DecodeFailed};
    }
  }
  else
  {
    if (WebPDecodeRGBInto(bytes.data(), bytes.size(), channels.data(),
                          buffer_size, pitch) == nullptr)
    {
      channels.clear();
      return Err{ImageLoadErr::DecodeFailed};
    }
  }

  return Ok{
    DecodedImageInfo{.extent = extent, .format = fmt}
  };
}

inline void png_stream_reader(png_structp png_ptr, unsigned char * out,
                              usize nbytes_to_read)
{
  Span<u8 const> * input = (Span<u8 const> *) png_get_io_ptr(png_ptr);
  mem::copy(input->slice(0, nbytes_to_read), out);
  *input = input->slice(nbytes_to_read);
}

Result<DecodedImageInfo, ImageLoadErr> decode_png(Span<u8 const> bytes,
                                                  Vec<u8> &      channels)
{
  // skip magic number
  bytes = bytes.slice(8);

  png_structp png_ptr =
    png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  if (png_ptr == nullptr)
  {
    return Err{ImageLoadErr::OutOfMemory};
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);

  if (png_ptr == nullptr)
  {
    png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    return Err{ImageLoadErr::OutOfMemory};
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
    return Err{ImageLoadErr::DecodeFailed};
  }

  if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGBA)
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return Err{ImageLoadErr::UnsupportedFormat};
  }

  u32         ncomponents = (color_type == PNG_COLOR_TYPE_RGB) ? 3 : 4;
  gpu::Format fmt         = (ncomponents == 3) ? gpu::Format::R8G8B8_UNORM :
                                                 gpu::Format::R8G8B8A8_UNORM;
  u32         pitch       = width * ncomponents;
  u64         buffer_size = (u64) height * pitch;

  if (!channels.resize_uninit(buffer_size))
  {
    return Err{ImageLoadErr::OutOfMemory};
  }

  u8 * row = channels.data();
  for (u32 i = 0; i < height; i++)
  {
    png_read_row(png_ptr, row, nullptr);
    row += pitch;
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

  return Ok{
    DecodedImageInfo{.extent{width, height}, .format = fmt}
  };
}

Result<DecodedImageInfo, ImageLoadErr> decode_jpg(Span<u8 const> bytes,
                                                  Vec<u8> &      channels)
{
  jpeg_decompress_struct info;
  jpeg_error_mgr         error_mgr;
  info.err = jpeg_std_error(&error_mgr);
  jpeg_create_decompress(&info);

  jpeg_mem_src(&info, bytes.data(), static_cast<unsigned long>(bytes.size()));

  if (jpeg_read_header(&info, true) != JPEG_HEADER_OK)
  {
    jpeg_destroy_decompress(&info);
    return Err{ImageLoadErr::DecodeFailed};
  }

  if (jpeg_start_decompress(&info) == 0)
  {
    jpeg_destroy_decompress(&info);
    return Err{ImageLoadErr::DecodeFailed};
  }

  if (info.num_components != 3 && info.num_components != 4)
  {
    jpeg_destroy_decompress(&info);
    return Err{ImageLoadErr::UnsupportedFormat};
  }

  u32               width       = info.output_width;
  u32               height      = info.output_height;
  u32               ncomponents = info.num_components;
  u32               pitch       = width * ncomponents;
  u64               buffer_size = (u64) height * pitch;
  gpu::Format const fmt = (ncomponents == 3) ? gpu::Format::R8G8B8_UNORM :
                                               gpu::Format::R8G8B8A8_UNORM;

  if (!channels.resize_uninit(buffer_size))
  {
    jpeg_destroy_decompress(&info);
    return Err{ImageLoadErr::OutOfMemory};
  }

  u8 * scanline = channels.data();
  while (info.output_scanline < height)
  {
    u8 * scanlines[] = {scanline};
    scanline += (u64) jpeg_read_scanlines(&info, scanlines, 1) * pitch;
  }

  jpeg_finish_decompress(&info);
  jpeg_destroy_decompress(&info);

  return Ok{
    DecodedImageInfo{.extent{width, height}, .format = fmt}
  };
}

Result<DecodedImageInfo, ImageLoadErr> decode_image(Span<u8 const> bytes,
                                                    Vec<u8> &      channels)
{
  static constexpr u8 JPG_MAGIC[]   = {0xFF, 0xD8, 0xFF};
  static constexpr u8 PNG_MAGIC[]   = {0x89, 0x50, 0x4E, 0x47,
                                       0x0D, 0x0A, 0x1A, 0x0A};
  // RIFF-[file size: 4 bytes]-WEBP
  static constexpr u8 WEBP_MAGIC1[] = {'R', 'I', 'F', 'F'};
  static constexpr u8 WEBP_MAGIC2[] = {'W', 'E', 'B', 'P'};

  if (range_eq(bytes.slice(0, size(JPG_MAGIC)), JPG_MAGIC))
  {
    return decode_jpg(bytes, channels);
  }

  if (range_eq(bytes.slice(0, size(PNG_MAGIC)), PNG_MAGIC))
  {
    return decode_png(bytes, channels);
  }

  if (range_eq(bytes.slice(0, size(WEBP_MAGIC1)), WEBP_MAGIC1) &&
      range_eq(bytes.slice(8, size(WEBP_MAGIC2)), WEBP_MAGIC2))
  {
    return decode_webp(bytes, channels);
  }

  return Err{ImageLoadErr::UnsupportedFormat};
}

}    // namespace ash
