#pragma once

#include "ashura/primitives.h"
#include "stx/enum.h"

namespace ash
{

enum class WindowType : u8
{
  Normal,
  Utility,
  Tooltip,
  Popup
};

enum class WindowCreateFlags
{
  None         = 0,
  Hidden       = 1 << 0,
  NonResizable = 1 << 1,
  Borderless   = 1 << 2,
  FullScreen   = 1 << 3,
  AlwaysOnTop  = 1 << 4
};

STX_DEFINE_ENUM_BIT_OPS(WindowCreateFlags)

}        // namespace ash
