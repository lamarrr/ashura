/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

/// multi-directional slider
struct Slider : public View
{
  bool          disabled : 1      = false;
  bool          pointer_enter : 1 = false;
  bool          pointer_leave : 1 = false;
  bool          pointer_down : 1  = false;
  bool          pointer_up : 1    = false;
  bool          hovered : 1       = false;
  bool          pressed : 1       = false;
  Fn<void(f32)> on_changed        = fn([](f32) {});
  Axis          direction         = Axis::X;
  Frame         frame             = {};
  Frame         thumb             = {};
  ColorGradient track_color       = DEFAULT_THEME.inactive;
  ColorGradient thumb_color       = DEFAULT_THEME.active;
  f32           t                 = 0;
  f32           min               = 0;
  f32           max               = 1;
  u8            steps             = U8_MAX;

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    u8 const main_axis  = (direction == Axis::X) ? 0 : 1;
    u8 const cross_axis = (direction == Axis::X) ? 1 : 0;
    if (events.mouse_down)
    {
      t         = unlerp(region.begin()[main_axis], region.end()[main_axis],
                         ctx.mouse_position[main_axis]);
      t         = (steps == U8_MAX) ? t : grid_snap(t, 1.0F / steps);
      f32 value = clamp(lerp(min, max, t), min, max);
      on_changed(value);
    }
    return ViewState{.draggable = !disabled};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) override
  {
    fill(offsets, Vec2{0, 0});
    return Vec2{0, 0};
  }

  virtual void render(CRect const &region, CRect const &clip,
                      Canvas &canvas) override
  {
    // inactive tracks, then active thumb
    if (steps != U8_MAX)
    {
      for (u8 i = 0; i < steps; i++)
      {
      }
    }
    else
    {
    }
  }
};

}        // namespace ash