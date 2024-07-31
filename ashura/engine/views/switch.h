/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct Switch : public View
{
  bool           disabled : 1      = false;
  bool           value : 1         = false;
  bool           hovered : 1       = false;
  bool           pressed : 1       = false;
  Fn<void(bool)> on_changed        = fn([](bool) {});
  Vec4           on_color          = DEFAULT_THEME.primary;
  Vec4           on_hovered_color  = DEFAULT_THEME.primary_variant;
  Vec4           off_color         = DEFAULT_THEME.active;
  Vec4           off_hovered_color = DEFAULT_THEME.inactive;
  Vec4           track_color       = DEFAULT_THEME.inactive;
  Frame          frame             = {.width = {40}, .height = {20}};
  Frame          thumb_frame       = {.width = {17.5}, .height = {17.5}};
  Vec4           corner_radii      = Vec4::splat(0.125F);

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events) override
  {
    if (events.mouse_down && ctx.mouse_down(MouseButtons::Primary))
    {
      value = !value;
      on_changed(value);
      pressed = true;
    }

    if (events.mouse_up && ctx.mouse_up(MouseButtons::Primary))
    {
      pressed = false;
    }

    if (events.mouse_enter)
    {
      hovered = true;
    }

    if (events.mouse_leave)
    {
      hovered = false;
    }

    return ViewState{
        .pointable = !disabled, .clickable = !disabled, .focusable = !disabled};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return frame(allocated);
  }

  virtual void render(CRect const &region, CRect const &,
                      Canvas      &canvas) override
  {
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = corner_radii,
                           .tint         = ColorGradient::all(track_color)});

    Vec2 const thumb_extent = thumb_frame(region.extent);
    Vec2 const thumb_center =
        region.begin() +
        space_align(region.extent, thumb_extent, Vec2{value ? -1.0F : 1.0F, 0});

    Vec4 thumb_color;
    if (hovered)
    {
      thumb_color = value ? on_hovered_color : off_hovered_color;
    }
    else
    {
      thumb_color = value ? on_color : off_color;
    }

    canvas.rrect(ShapeDesc{.center       = thumb_center,
                           .extent       = thumb_extent,
                           .corner_radii = corner_radii,
                           .tint         = ColorGradient::all(thumb_color)});
  }
};

}        // namespace ash