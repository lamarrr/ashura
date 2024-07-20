/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct CheckBox : public View
{
  Fn<void(bool)> on_changed = fn([](bool) {});
  bool           value      = false;
  Vec4           color      = material::BLUE_A700.norm();
  f32            width      = 20;
  bool           disabled   = false;

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events) override
  {
    if (!disabled && events.mouse_down &&
        has_bits(ctx.mouse_buttons, MouseButtons::Primary))
    {
      value = !value;
      on_changed(value);
    }

    // TODO(lamarrr): process enter keyboard

    return ViewState{.clickable = !disabled, .focusable = true};
  }

  virtual Vec2 fit(Vec2, Span<Vec2 const>, Span<Vec2>) const override
  {
    return Vec2{width, width};
  }

  virtual void render(CRect const &region, Canvas &canvas) const override
  {
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = Vec4::splat(region.extent.x / 8),
                           .stroke       = 1,
                           .thickness    = 2,
                           .tint         = ColorGradient::uniform(color)});

    if (value)
    {
      canvas.line(
          ShapeDesc{.center       = region.center,
                    .extent       = region.extent,
                    .corner_radii = Vec4::splat(0),
                    .stroke       = 0,
                    .thickness    = 2.5,
                    .tint         = ColorGradient::uniform(color)},
          span<Vec2>({{0.125f, 0.5f}, {0.374f, 0.75f}, {0.775f, 0.25f}}));
    }
  }
};

}        // namespace ash