/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/types.h"

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

enum class FontLoadErr : u8
{
  OutOfMemory       = 0,
  InvalidPath       = 1,
  IoErr             = 2,
  DecodeFailed      = 3,
  FaceNotFound      = 4,
  UnsupportedFormat = 5
};

enum class ShaderLoadErr : u32
{
  OutOfMemory           = 0,
  InvalidPath           = 1,
  IOErr                 = 2,
  CompileFailed         = 3,
  LinkFailed            = 4,
  SpirvConversionFailed = 5,
  InitErr               = 6
};

}    // namespace ash
