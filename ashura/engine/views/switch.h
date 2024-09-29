/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct Switch : public View
{
  struct State
  {
    bool disabled : 1 = false;
    bool value : 1    = false;
    bool hovered : 1  = false;
    bool pressed : 1  = false;
  } state;

  struct Style
  {
    ColorGradient on_color = ColorGradient::all(DEFAULT_THEME.primary);
    ColorGradient on_hovered_color =
        ColorGradient::all(DEFAULT_THEME.primary_variant);
    ColorGradient off_color = ColorGradient::all(DEFAULT_THEME.active);
    ColorGradient off_hovered_color =
        ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient track_color  = ColorGradient::all(DEFAULT_THEME.inactive);
    Frame         frame        = {.width = {40}, .height = {20}};
    Frame         thumb_frame  = {.width = {17.5}, .height = {17.5}};
    CornerRadii   corner_radii = CornerRadii::all({.scale = 0.125F});
  } style;

  Fn<void(bool)> on_changed = fn([](bool) {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events, Fn<void(View *)>) override
  {
    if (events.mouse_down && ctx.mouse_down(MouseButtons::Primary))
    {
      state.value = !state.value;
      on_changed(state.value);
      state.pressed = true;
    }

    if (events.mouse_up && ctx.mouse_up(MouseButtons::Primary))
    {
      state.pressed = false;
    }

    if (events.mouse_enter)
    {
      state.hovered = true;
    }

    if (events.mouse_leave)
    {
      state.hovered = false;
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
                           .tint         = style.track_color});

    Vec2 const thumb_extent = style.thumb_frame(region.extent);
    Vec2 const thumb_center =
        region.begin() + space_align(region.extent, thumb_extent,
                                     Vec2{state.value ? -1.0F : 1.0F, 0});

    ColorGradient thumb_color;
    if (state.hovered)
    {
      thumb_color =
          state.value ? style.on_hovered_color : style.off_hovered_color;
    }
    else
    {
      thumb_color = state.value ? style.on_color : style.off_color;
    }

    canvas.rrect(ShapeDesc{.center       = thumb_center,
                           .extent       = thumb_extent,
                           .corner_radii = style.corner_radii(region.extent.y),
                           .tint         = thumb_color});
  }
};

}        // namespace ash