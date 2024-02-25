
#pragma once
#include "ashura/std/types.h"

namespace ash
{

/// @InvalidPath: the image path provided is invalid
/// @InvalidData: detected image but image seems to be corrupted
/// @UnsupportedChannels: image contains unsupported channel types
/// @UnsupportedFormat: the image file format is unsupported
enum class DecodeError : i32
{
  None                = 0,
  OutOfMemory         = 1,
  InvalidPath         = 2,
  DecodeFailed        = 3,
  UnsupportedFormat   = 5
};

}        // namespace ash
