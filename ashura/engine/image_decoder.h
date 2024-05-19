#pragma once
#include <utility>

#include "ashura/gfx/image.h"
#include "ashura/std/result.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

/// @InvalidPath: the image path provided is invalid
/// @InvalidData: detected image but image seems to be corrupted
/// @UnsupportedChannels: image contains unsupported channel types
/// @UnsupportedFormat: the image file format is unsupported
enum class DecodeError : i32
{
  None              = 0,
  OutOfMemory       = 1,
  InvalidPath       = 2,
  DecodeFailed      = 3,
  UnsupportedFormat = 5
};

Result<gfx::ImageSpan<u8>, DecodeError> decode_webp(Span<u8 const> bytes,
                                                    Vec<u8>       &vec);

Result<gfx::ImageSpan<u8>, DecodeError> decode_jpg(Span<u8 const> bytes,
                                                   Vec<u8>       &vec);

Result<gfx::ImageSpan<u8>, DecodeError> decode_png(Span<u8 const> bytes,
                                                   Vec<u8>       &vec);

Result<gfx::ImageSpan<u8>, DecodeError> decode_image(Span<u8 const> bytes,
                                                     Vec<u8>       &vec);

}        // namespace ash
