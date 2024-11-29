/// SPDX-License-Identifier: MIT
#pragma once
#include <utility>

#include "ashura/gpu/gpu.h"
#include "ashura/std/image.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

/// @param InvalidPath the image path provided is invalid
/// @param InvalidData detected image but image seems to be corrupted
/// @param UnsupportedChannels image contains unsupported channel types
/// @param UnsupportedFormat the image file format is unsupported
enum class [[nodiscard]] ImageDecodeError : i32
{
  None              = 0,
  OutOfMemory       = 1,
  InvalidPath       = 2,
  DecodeFailed      = 3,
  UnsupportedFormat = 4
};

struct DecodedImage
{
  Vec<u8>     channels = {};
  u32         width    = 0;
  u32         height   = 0;
  gpu::Format format   = gpu::Format::Undefined;
};

ImageDecodeError decode_webp(Span<u8 const> bytes, DecodedImage & image);
ImageDecodeError decode_jpg(Span<u8 const> bytes, DecodedImage & image);
ImageDecodeError decode_png(Span<u8 const> bytes, DecodedImage & image);
ImageDecodeError decode_image(Span<u8 const> bytes, DecodedImage & image);

}        // namespace ash
