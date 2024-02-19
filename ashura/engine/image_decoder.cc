#include "ashura/engine/image_decoder.h"

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

Result<ImageBuffer, LoadError> decode_webp(AllocatorImpl const &allocator,
                                           Span<u8 const>       data)
{
  WebPBitstreamFeatures features;

  if (WebPGetFeatures(data.data(), data.size(), &features) != VP8_STATUS_OK)
  {
    return Err(LoadError::InvalidData);
  }

  stx::Memory memory =
      stx::mem::allocate(stx::os_allocator,
                         static_cast<usize>(features.width) *
                             static_cast<usize>(features.height) *
                             (features.has_alpha ? 4ULL : 3ULL))
          .unwrap();

  u8 *pixels = (u8 *) memory.handle;

  if (features.has_alpha)
  {
    if (WebPDecodeRGBAInto(data.data(), data.size(), pixels,
                           features.width * features.height * 4,
                           features.width * 4) == nullptr)
    {
      return Err(LoadError::InvalidData);
    }
  }
  else
  {
    if (WebPDecodeRGBInto(data.data(), data.size(), pixels,
                          features.width * features.height * 3,
                          features.width * 3) == nullptr)
    {
      return Err(LoadError::InvalidData);
    }
  }

  return Ok(ImageBuffer{.memory = std::move(memory),
                        .extent = Extent{static_cast<u32>(features.width),
                                         static_cast<u32>(features.height)},
                        .format = features.has_alpha ? ImageFormat::Rgba8888 :
                                                       ImageFormat::Rgb888});
}

inline void png_stream_reader(png_structp png_ptr, unsigned char *out,
                              usize nbytes_to_read)
{
  Span<u8 const> *input =
      reinterpret_cast<Span<u8 const> *>(png_get_io_ptr(png_ptr));
  Span{out, nbytes_to_read}.copy(*input);
  *input = input->slice(nbytes_to_read);
}

Result<ImageBuffer, LoadError> decode_png(AllocatorImpl const &allocator,
                                          Span<u8 const>       data)
{
  // skip magic number
  data = data.slice(8);

  png_structp png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

  ASH_CHECK(png_ptr != nullptr);

  png_infop info_ptr = png_create_info_struct(png_ptr);

  ASH_CHECK(info_ptr != nullptr);

  Span stream = data;
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
    return Err(LoadError::InvalidData);
  }

  usize       ncomponents = 0;
  ImageFormat fmt         = ImageFormat::Rgba8888;

  if (color_type == PNG_COLOR_TYPE_RGB)
  {
    ncomponents = 3;
    fmt         = ImageFormat::Rgb888;
  }
  else if (color_type == PNG_COLOR_TYPE_RGBA)
  {
    ncomponents = 4;
    fmt         = ImageFormat::Rgba8888;
  }
  else
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return Err(LoadError::UnsupportedChannels);
  }

  stx::Memory pixels_mem =
      stx::mem::allocate(stx::os_allocator, width * height * ncomponents)
          .unwrap();

  u8 *pixels = (u8 *) pixels_mem.handle;

  for (u32 i = 0; i < height; i++)
  {
    png_read_row(png_ptr, pixels, nullptr);
    pixels += width * ncomponents;
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

  return Ok{ImageBuffer{.memory = std::move(pixels_mem),
                        .extent = Extent{width, height},
                        .format = fmt}};
}

Result<ImageBuffer, LoadError> decode_jpg(AllocatorImpl const &allocator,
                                          Span<u8 const>       bytes)
{
  jpeg_decompress_struct info;
  jpeg_error_mgr         error_mgr;
  info.err = jpeg_std_error(&error_mgr);
  jpeg_create_decompress(&info);

  jpeg_mem_src(&info, bytes.data(), static_cast<unsigned long>(bytes.size()));

  if (jpeg_read_header(&info, true) != JPEG_HEADER_OK)
  {
    jpeg_destroy_decompress(&info);
    return stx::Err(LoadError::InvalidData);
  }

  if (!jpeg_start_decompress(&info))
  {
    jpeg_destroy_decompress(&info);
    return stx::Err(LoadError::InvalidData);
  }

  u32         width       = info.output_width;
  u32         height      = info.output_height;
  u32         ncomponents = info.num_components;
  ImageFormat fmt         = ImageFormat::Rgba8888;

  if (ncomponents == 3)
  {
    fmt = ImageFormat::Rgb888;
  }
  else if (ncomponents == 4)
  {
    fmt = ImageFormat::Rgba8888;
  }
  else
  {
    jpeg_destroy_decompress(&info);
    return stx::Err(LoadError::UnsupportedChannels);
  }

  stx::Memory pixels_mem =
      stx::mem::allocate(stx::os_allocator, height * width * ncomponents)
          .unwrap();

  u8 *pixels = (u8 *) pixels_mem.handle;

  while (info.output_scanline < height)
  {
    u8 *scanlines[] = {pixels};
    pixels += jpeg_read_scanlines(&info, scanlines, 1) * width * ncomponents;
  }

  jpeg_finish_decompress(&info);
  jpeg_destroy_decompress(&info);

  return stx::Ok{ImageBuffer{.memory = std::move(pixels_mem),
                             .extent = Extent{width, height},
                             .format = fmt}};
}

Result<ImageBuffer, LoadError> decode_image(AllocatorImpl const &allocator,
                                            Span<u8 const>       bytes)
{
  constexpr u8 JPG_MAGIC[] = {0xFF, 0xD8, 0xFF};

  constexpr u8 PNG_MAGIC[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

  // RIFF-[file size: 4 bytes]-WEBP
  constexpr u8 WEBP_MAGIC1[] = {'R', 'I', 'F', 'F'};
  constexpr u8 WEBP_MAGIC2[] = {'W', 'E', 'B', 'P'};

  if (bytes.slice(0, std::size(JPG_MAGIC)).equals(Span{JPG_MAGIC}))
  {
    return decode_jpg(bytes);
  }
  else if (bytes.slice(0, std::size(PNG_MAGIC)).equals(Span{PNG_MAGIC}))
  {
    return decode_png(bytes);
  }
  else if (bytes.slice(0, std::size(WEBP_MAGIC1)).equals(Span{WEBP_MAGIC1}) &&
           bytes.slice(8, std::size(WEBP_MAGIC2)).equals(Span{WEBP_MAGIC2}))
  {
    return decode_webp(bytes);
  }
  else
  {
    return Err(LoadError::UnsupportedFormat);
  }
}

}        // namespace ash