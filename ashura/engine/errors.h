/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/types.h"

namespace ash
{

enum class [[nodiscard]] SysErr : i32
{
  None                  = 0,
  OutOfMemory           = 1,
  InvalidPath           = 2,
  IoErr                 = 3,
  DecodeFailed          = 4,
  FaceNotFound          = 5,
  UnsupportedFormat     = 6,
  CompileFailed         = 7,
  LinkFailed            = 8,
  SpirvConversionFailed = 9
};

}    // namespace ash
