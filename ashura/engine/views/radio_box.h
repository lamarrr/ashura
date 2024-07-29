/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct RadioBox : public View
{
  bool           disabled : 1        = false;
  bool           pointer_enter : 1   = false;
  bool           pointer_leave : 1   = false;
  bool           pointer_down : 1    = false;
  bool           pointer_up : 1      = false;
  bool           hovered : 1         = false;
  bool           focused : 1         = false;
  bool           pressed : 1         = false;
  bool           value : 1           = false;
  Fn<void(bool)> on_changed          = fn([](bool) {});
  Frame          frame               = {.width = {50}, .height = {50}};
  Vec4           corner_radii        = Vec4::splat(0.125F);
  f32            thickness           = 1.0F;
  Vec4           color               = DEFAULT_THEME.inactive;
  Vec4           inner_color         = DEFAULT_THEME.primary;
  Vec4           inner_hovered_color = DEFAULT_THEME.primary_variant;

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    if (events.mouse_enter)
    {
      hovered = true;
    }

    if (events.mouse_leave)
    {
      hovered = false;
    }

    if (events.focus_in)
    {
      focused = true;
    }

    if (events.focus_out)
    {
      focused = false;
    }

    if ((events.mouse_down && ctx.mouse_down(MouseButtons::Primary)) ||
        (events.key_down && ctx.key_down(KeyCode::Return)))
    {
      pressed = true;
      value   = !value;
      on_changed(value);
    }

    if ((events.mouse_up && ctx.mouse_down(MouseButtons::Primary)) ||
        (events.key_up && ctx.key_up(KeyCode::Return)))
    {
      pressed = false;
    }

    return ViewState{
        .pointable = !disabled, .clickable = !disabled, .focusable = !disabled};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return frame(allocated);
  }

  virtual void render(CRect const &region, CRect const &clip,
                      Canvas &canvas) override
  {
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = corner_radii * region.extent.y,
                           .stroke       = 1,
                           .thickness    = thickness,
                           .tint         = ColorGradient::all(color)});

    if (value)
    {
      Vec2 inner_extent = region.extent * (hovered ? 0.75F : 0.5F);
      Vec4 inner_color =
          hovered ? this->inner_hovered_color : this->inner_color;

      canvas.circle(ShapeDesc{.center = region.center,
                              .extent = inner_extent,
                              .tint   = ColorGradient::all(inner_color)});
    }
  }
};

}        // namespace ash