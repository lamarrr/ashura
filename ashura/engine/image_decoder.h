/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/gpu/gpu.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

enum class [[nodiscard]] ImageLoadErr : i32
{
  OutOfMemory       = 0,
  DecodeFailed      = 1,
  UnsupportedFormat = 2,
  InvalidPath       = 3,
  IoErr             = 4
};

struct DecodedImageInfo
{
  gpu::Extent extent{1, 1};
  gpu::Format format = gpu::Format::Undefined;
};

Result<DecodedImageInfo, ImageLoadErr> decode_webp(Span<u8 const> bytes,
                                                   Vec<u8> &      channels);

Result<DecodedImageInfo, ImageLoadErr> decode_png(Span<u8 const> bytes,
                                                  Vec<u8> &      channels);

Result<DecodedImageInfo, ImageLoadErr> decode_jpg(Span<u8 const> bytes,
                                                  Vec<u8> &      channels);

Result<DecodedImageInfo, ImageLoadErr> decode_image(Span<u8 const> bytes,
                                                    Vec<u8> &      channels);

}    // namespace ash
