/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct RadioBox : public View
{
  struct State
  {
    bool disabled : 1      = false;
    bool pointer_enter : 1 = false;
    bool pointer_leave : 1 = false;
    bool pointer_down : 1  = false;
    bool pointer_up : 1    = false;
    bool hovered : 1       = false;
    bool focused : 1       = false;
    bool pressed : 1       = false;
    bool value : 1         = false;
  } state;

  struct Style
  {
    Frame         frame        = {.width = {50}, .height = {50}};
    CornerRadii   corner_radii = CornerRadii::all({.scale = 0.125F});
    f32           thickness    = 1.0F;
    ColorGradient color        = ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient inner_color  = ColorGradient::all(DEFAULT_THEME.primary);
    ColorGradient inner_hovered_color =
        ColorGradient::all(DEFAULT_THEME.primary_variant);
  } style;

  Fn<void(bool)> on_changed = fn([](bool) {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events, Fn<void(View &)>) override
  {
    if (events.mouse_in)
    {
      state.hovered = true;
    }

    if (events.mouse_out)
    {
      state.hovered = false;
    }

    if (events.focus_in)
    {
      state.focused = true;
    }

    if (events.focus_out)
    {
      state.focused = false;
    }

    if ((events.mouse_down && ctx.mouse_down(MouseButtons::Primary)) ||
        (events.key_down && ctx.key_down(KeyCode::Return)))
    {
      state.pressed = true;
      state.value   = !state.value;
      on_changed(state.value);
    }

    if ((events.mouse_up && ctx.mouse_down(MouseButtons::Primary)) ||
        (events.key_up && ctx.key_up(KeyCode::Return)))
    {
      state.pressed = false;
    }

    return ViewState{.pointable = !state.disabled,
                     .clickable = !state.disabled,
                     .focusable = !state.disabled};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return style.frame(allocated);
  }

  virtual void render(CRect const &region, CRect const &,
                      Canvas      &canvas) override
  {
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = style.corner_radii(region.extent.y),
                           .stroke       = 1,
                           .thickness    = style.thickness,
                           .tint         = style.color});

    if (state.value)
    {
      Vec2 inner_extent = region.extent * (state.hovered ? 0.75F : 0.5F);
      ColorGradient inner_color =
          state.hovered ? style.inner_hovered_color : style.inner_color;

      canvas.circle(ShapeDesc{.center = region.center,
                              .extent = inner_extent,
                              .tint   = inner_color});
    }
  }
};

}        // namespace ash