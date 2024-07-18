
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
  SizeConstraint width      = {};
  SizeConstraint height     = {};
  bool           disabled   = false;
  Fn<void()>     on_clicked = fn([] {});
  Fn<void()>     on_hovered = fn([] {});

  virtual View *child(u32 i) override final
  {
    return child_iter({item()}, i);
  }

  virtual View *item()
  {
    return nullptr;
  }
};

// TODO(lamarrr):
struct TextButton : public Button
{
  TextBox text = {};
};

}        // namespace ash