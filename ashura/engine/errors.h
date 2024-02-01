
#pragma once

namespace ash
{

/// @InvalidPath: the image path provided is invalid
/// @InvalidData: detected image but image seems to be corrupted
/// @UnsupportedChannels: image contains unsupported channel types
/// @UnsupportedFormat: the image file format is unsupported
enum class LoadError : int
{
  None                = 0,
  InvalidPath         = 1,
  InvalidData         = 2,
  UnsupportedChannels = 3,
  UnsupportedFormat   = 4
};

}        // namespace ash
