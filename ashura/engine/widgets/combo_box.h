/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/widget.h"
#include "ashura/std/types.h"

namespace ash
{

struct ComboItem : public Widget
{
  // wrap child
};

// TODO(lamarrr):
struct ComboBox : public Widget
{
  Fn<void(u32)> on_selected = fn([](u32) {});
  // or produce as many combo boxes as the number of children. or use one combo
  // box for all children.

  virtual ComboItem *item(u32 i)
  {
    (void) i;
    return nullptr;
  }
};

// combo text

}        // namespace ash