/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct CheckBox : public View
{
  struct State
  {
    bool disabled : 1 = false;
    bool hovered : 1  = false;
    bool pressed : 1  = false;
    bool value : 1    = false;
  } state;

  struct Style
  {
    ColorGradient box_color = ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient box_hovered_color = ColorGradient::all(DEFAULT_THEME.active);
    ColorGradient tick_color        = ColorGradient::all(DEFAULT_THEME.primary);
    f32           stroke            = 1;
    f32           thickness         = 1;
    f32           tick_thickness    = 1.5F;
    CornerRadii   corner_radii      = CornerRadii::all({.scale = 0.125F});
    Frame         frame             = Frame{.width = {20}, .height = {20}};
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

    if ((events.mouse_down && ctx.mouse_down(MouseButtons::Primary)) ||
        (events.key_down && ctx.key_down(KeyCode::Return)))
    {
      state.pressed = true;
      state.value   = !state.value;
      on_changed(state.value);
    }

    if ((events.mouse_up && ctx.mouse_up(MouseButtons::Primary)) ||
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
    Vec2 extent = style.frame(allocated);
    return Vec2::splat(min(extent.x, extent.y));
  }

  virtual void render(CRect const &region, CRect const &,
                      Canvas      &canvas) override
  {
    ColorGradient tint = (state.hovered && !state.pressed && !state.disabled) ?
                             style.box_hovered_color :
                             style.box_color;
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = style.corner_radii(region.extent.y),
                           .stroke       = 1,
                           .thickness    = 2,
                           .tint         = tint});

    if (state.value)
    {
      canvas.line(
          ShapeDesc{.center    = region.center,
                    .extent    = region.extent,
                    .stroke    = 0,
                    .thickness = style.tick_thickness,
                    .tint      = style.tick_color},
          span<Vec2>({{0.125f, 0.5f}, {0.374f, 0.75f}, {0.775f, 0.25f}}));
    }
  }
};

}        // namespace ash