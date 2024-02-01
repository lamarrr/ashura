#pragma once

#include "ashura/std/enum.h"
#include "ashura/std/types.h"

namespace ash
{

enum class WindowEvents : u32
{
  None           = 0x0000,
  Shown          = 0x0001,
  Hidden         = 0x0002,
  Exposed        = 0x0004,
  Moved          = 0x0008,
  Resized        = 0x0010,
  Minimized      = 0x0020,
  Maximized      = 0x0040,
  Restored       = 0x0080,
  MouseEnter     = 0x0100,
  MouseLeave     = 0x0200,
  FocusGained    = 0x0400,
  FocusLost      = 0x0800,
  CloseRequested = 0x1000,
  TakeFocus      = 0x2000,
  All            = 0xFFFF
};

ASH_DEFINE_ENUM_BIT_OPS(WindowEvents)

}        // namespace ash