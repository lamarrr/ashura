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

  virtual Vec2 fit(Vec2, Span<Vec2 const>, Span<Vec2>) override
  {
    return Vec2{width, width};
  }

  virtual ViewAttributes attributes() override
  {
    return ViewAttributes::Visible | ViewAttributes::Clickable;
  }

  virtual void render(CRect const &region, Canvas &canvas) override
  {
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .border_radii = Vec4::splat(region.extent.x / 8),
                           .stroke       = 1,
                           .thickness    = 2,
                           .tint         = ColorGradient::uniform(color)});

    if (value)
    {
      canvas.line(
          ShapeDesc{.center       = region.center,
                    .extent       = region.extent,
                    .border_radii = Vec4::splat(0),
                    .stroke       = 0,
                    .thickness    = 2.5,
                    .tint         = ColorGradient::uniform(color)},
          span<Vec2>({{0.125f, 0.5f}, {0.374f, 0.75f}, {0.775f, 0.25f}}));
    }
  }

  virtual void tick(ViewContext const &ctx, CRect const &,
                    ViewEventTypes     events) override
  {
    if (!disabled && has_bits(events, ViewEventTypes::MouseDown) &&
        has_bits(ctx.mouse_buttons, MouseButtons::Primary))
    {
      value = !value;
      on_changed(value);
    }
  }
};

}        // namespace ash