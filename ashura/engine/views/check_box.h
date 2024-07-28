/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct CheckBox : public View
{
  bool           disabled : 1      = false;
  bool           hovered : 1       = false;
  bool           pressed : 1       = false;
  bool           value : 1         = false;
  Fn<void(bool)> on_changed        = fn([](bool) {});
  Vec4           box_color         = DEFAULT_THEME.inactive;
  Vec4           box_hovered_color = DEFAULT_THEME.active;
  Vec4           tick_color        = DEFAULT_THEME.primary;
  SizeConstraint width             = {.offset = 20};
  f32            stroke            = 1;
  f32            thickness         = 1;
  f32            tick_thickness    = 1.5F;
  Vec4           corner_radius     = Vec4::splat(0.125F);

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events) override
  {
    if (events.mouse_enter)
    {
      hovered = true;
    }

    if (events.mouse_leave)
    {
      hovered = false;
    }

    if ((events.mouse_down &&
         has_bits(ctx.mouse_buttons, MouseButtons::Primary)) ||
        (events.key_down && ctx.key_down(KeyCode::Return)))
    {
      pressed = true;
      value   = !value;
      on_changed(value);
    }
    else if ((events.mouse_up &&
              !has_bits(ctx.mouse_buttons, MouseButtons::Primary)) ||
             (events.key_up && !ctx.key_down(KeyCode::Return)))
    {
      pressed = false;
    }

    return ViewState{.clickable = !disabled, .focusable = !disabled};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    Vec2 extent = Frame{width, width}(allocated);
    return Vec2::splat(min(extent.x, extent.y));
  }

  virtual void render(CRect const &region, CRect const &,
                      Canvas      &canvas) override
  {
    Vec4 tint = (hovered && !pressed) ? box_hovered_color : box_color;
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = corner_radius * region.extent.y,
                           .stroke       = 1,
                           .thickness    = 2,
                           .tint         = ColorGradient::all(tint)});

    if (value)
    {
      canvas.line(
          ShapeDesc{.center    = region.center,
                    .extent    = region.extent,
                    .stroke    = 0,
                    .thickness = tick_thickness,
                    .tint      = ColorGradient::all(tick_color)},
          span<Vec2>({{0.125f, 0.5f}, {0.374f, 0.75f}, {0.775f, 0.25f}}));
    }
  }
};

}        // namespace ash