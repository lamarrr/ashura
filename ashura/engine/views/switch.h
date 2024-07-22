/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct Switch : public View
{
  bool           disabled : 1   = false;
  bool           value : 1      = false;
  Fn<void(bool)> on_changed     = fn([](bool) {});
  Vec4           active_color   = material::BLUE_A700.norm();
  Vec4           inactive_color = material::GRAY_500.norm();
  f32            height         = 20;

  virtual ViewState tick(ViewContext const &ctx, CRect const &,
                         ViewEvents         events) override
  {
    if (!disabled && events.mouse_down &&
        has_bits(ctx.mouse_buttons, MouseButtons::Primary))
    {
      value = !value;
      on_changed(value);
    }

    // TODO(lamarrr): handle focus
    return ViewState{.clickable = true, .focusable = true};
  }

  virtual Vec2 fit(Vec2, Span<Vec2 const>, Span<Vec2>) const override
  {
    return Vec2{height * 2, height};
  }

  virtual void render(CRect const &region, CRect const &,
                      Canvas      &canvas) const override
  {
    f32 const  padding       = (1.75f / 20) * region.extent.y;
    f32 const  corner_radius = 0.06125f * region.extent.y;
    f32 const  thumb_radius  = max(region.extent.y / 2 - padding, 0.0f);
    Vec2 const thumb_extent  = Vec2::splat(thumb_radius * 2);
    f32 const  off_pos       = padding + thumb_radius;
    f32 const  on_pos = max(region.extent.x - padding - thumb_radius, 0.0f);

    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = Vec4::splat(corner_radius),
                           .stroke       = 1,
                           .thickness    = 1,
                           .tint = ColorGradient::uniform(active_color)});

    canvas.circle(ShapeDesc{
        .center       = {value ? on_pos : off_pos, region.center.y},
        .extent       = thumb_extent,
        .corner_radii = Vec4::splat(thumb_radius),
        .stroke       = 0,
        .tint = ColorGradient::uniform(value ? active_color : inactive_color)});
  }
};

}        // namespace ash