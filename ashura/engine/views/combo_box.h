/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct ComboItem : public View
{
  // wrap child
};

// [ ]
struct ComboBox : public View
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