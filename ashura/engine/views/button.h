
/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/view.h"
#include "ashura/engine/views/text_box.h"
#include "ashura/std/types.h"

namespace ash
{

struct Button : public View
{
  bool       disabled   = false;
  Frame      frame      = {};
  Fn<void()> on_clicked = fn([] {});
  Fn<void()> on_hovered = fn([] {});
  Fn<void()> on_focused = fn([] {});

  virtual View *child(u32 i) const override final
  {
    return subview({item()}, i);
  }

  virtual View *item() const
  {
    return nullptr;
  }

  // [ ] handle focus
};

// [ ]
struct TextButton : public Button
{
  TextBox text = {};
};

}        // namespace ash