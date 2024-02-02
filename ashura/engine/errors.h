
#pragma once
#include "ashura/std/types.h"

namespace ash
{

/// @InvalidPath: the image path provided is invalid
/// @InvalidData: detected image but image seems to be corrupted
/// @UnsupportedChannels: image contains unsupported channel types
/// @UnsupportedFormat: the image file format is unsupported
enum class LoadError : i32
{
  None                = 0,
  InvalidPath         = 1,
  InvalidData         = 2,
  UnsupportedChannels = 3,
  UnsupportedFormat   = 4
};

enum class RenderError : i32
{
  __RenderErrorReserved = 0,
  OutOfMemory           = 1
};

}        // namespace ash
