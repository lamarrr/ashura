/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief Multi-directional Slider
struct Slider : public View
{
  struct State
  {
    bool disabled : 1      = false;
    bool pointer_enter : 1 = false;
    bool pointer_leave : 1 = false;
    bool pointer_down : 1  = false;
    bool pointer_up : 1    = false;
    bool hovered : 1       = false;
    bool pressed : 1       = false;
  } state;

  struct Style
  {
    Axis          direction    = Axis::X;
    Frame         frame        = {.width = {100}, .height = {30}};
    Frame         thumb_frame  = {.width = {20}, .height = {20}};
    Size          track_height = {10};
    ColorGradient track_color  = ColorGradient::all(DEFAULT_THEME.inactive);
    ColorGradient thumb_color  = ColorGradient::all(DEFAULT_THEME.primary);
    ColorGradient thumb_hovered_color =
        ColorGradient::all(DEFAULT_THEME.primary_variant);
    CornerRadii track_corner_radii = CornerRadii::all({.scale = 0.125F});
    CornerRadii thumb_corner_radii = CornerRadii::all({.scale = 0.125F});
    u8          levels             = 0;
  } style;

  f32 t    = 0;
  f32 low  = 0;
  f32 high = 1;

  Fn<void(f32)> on_changed = fn([](f32) {});

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events, Fn<void(View *)>) override
  {
    u8 const main_axis = (style.direction == Axis::X) ? 0 : 1;
    if (events.dragging)
    {
      t = unlerp(region.begin()[main_axis], region.end()[main_axis],
                 ctx.mouse.position[main_axis]);
      t = (style.levels == 0) ? t : grid_snap(t, 1.0F / style.levels);
      f32 const value = clamp(lerp(low, high, t), low, high);
      on_changed(value);
    }
    return ViewState{.pointable = !state.disabled,
                     .draggable = !state.disabled};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return style.frame(allocated);
  }

  virtual void render(CRect const &region, CRect const &,
                      Canvas      &canvas) override
  {
    Vec2 const track_extent =
        Frame{style.frame.width, style.track_height}(region.extent);
    canvas.rrect(
        ShapeDesc{.center       = region.center,
                  .extent       = track_extent,
                  .corner_radii = style.track_corner_radii(region.extent.y),
                  .tint         = style.track_color});

    if (style.levels != 0)
    {
      f32 const unit = 1 / (f32) style.levels;
      for (i32 i = 1; i < style.levels; i++)
      {
        Vec2 center = region.begin();
        center.x += unit * i * region.extent.x;
        center.y += region.extent.y / 2;

        canvas.rrect(ShapeDesc{
            .center       = center,
            .extent       = Vec2::splat(min(track_extent.y, track_extent.x)),
            .corner_radii = Vec4::splat(1.0F),
            .tint         = style.thumb_color});
      }
    }

    Vec2 thumb_center =
        region.begin() + Vec2{region.extent.x * t, region.extent.y / 2};
    ColorGradient thumb_color =
        (state.hovered && !state.pressed && !state.disabled) ?
            style.thumb_hovered_color :
            style.thumb_color;
    f32 dilation =
        (state.hovered && !state.pressed && !state.disabled) ? 1.0F : 0.8F;
    Vec2 thumb_extent = style.thumb_frame(region.extent) * dilation;

    canvas.rrect(
        ShapeDesc{.center       = thumb_center,
                  .extent       = thumb_extent,
                  .corner_radii = style.thumb_corner_radii(thumb_extent.y),
                  .tint         = thumb_color});
  }
};

}        // namespace ash