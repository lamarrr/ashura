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
  InvalidPath       = 1,
  IoErr             = 2,
  DecodeFailed      = 3,
  UnsupportedFormat = 4
};

struct DecodedImageInfo
{
  Vec2U       extent{1, 1};
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
