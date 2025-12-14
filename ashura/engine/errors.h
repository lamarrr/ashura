/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/types.h"

namespace ash
{

enum class [[nodiscard]] SysErr : i32
{
  OutOfMemory           = 0,
  InvalidPath           = 1,
  IoErr                 = 2,
  DecodeFailed          = 3,
  FaceNotFound          = 4,
  UnsupportedFormat     = 5,
  CompileFailed         = 6,
  LinkFailed            = 7,
  SpirvConversionFailed = 8
};

}    // namespace ash
