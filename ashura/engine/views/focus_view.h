/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

// [ ] DEFAULT FOCUS VIEW
// - set the global focus rect, focus view can move there
struct FocusView : View
{
  CRect canvas_region_;

  virtual ui::State tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build) override;

  virtual Layout fit(Vec2 allocated, Span<Vec2 const> sizes,
                     Span<Vec2> centers) override;

  // [ ] properly handle fixed-centered, should they be counted as part of the children?
  virtual void render(Canvas & canvas, RenderInfo const & info) override;

  virtual i32 layer(i32 allocated, Span<i32> children) override;
};

}    // namespace ui
}    // namespace ash
