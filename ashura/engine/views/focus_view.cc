/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/focus_view.h"
#include "ashura/engine/engine.h"
#include "ashura/std/sformat.h"
#include "fast_float/fast_float.h"

namespace ash
{

namespace ui
{

ui::State FocusView::tick(Ctx const & ctx, Events const &, Fn<void(View &)>)
{
  canvas_region_ =
    ctx.focused.map([](FocusRect r) { return r.area; }).unwrap_or();
  return ui::State{};
}

Layout FocusView::fit(Vec2, Span<Vec2 const>, Span<Vec2>)
{
  return Layout{
    .extent{0.01F, 0.01F},
    .fixed_center = {}
  };
}

void FocusView::render(Canvas & canvas, RenderInfo const & info)
{
  // [ ] fix-up
  canvas.rrect(ShapeInfo{
    .area      = canvas_region_,
    .stroke    = 1,
    .thickness = Vec2::splat(0.5F),
    .tint      = ColorGradient{colors::CYAN},
  });
}

i32 FocusView::layer(i32, Span<i32>)
{
  return LAYERS.overlays;
}
}    // namespace ui
}    // namespace ash
