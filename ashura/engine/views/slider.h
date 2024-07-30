/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief Multi-directional Slider
struct Slider : public View
{
  bool           disabled : 1        = false;
  bool           pointer_enter : 1   = false;
  bool           pointer_leave : 1   = false;
  bool           pointer_down : 1    = false;
  bool           pointer_up : 1      = false;
  bool           hovered : 1         = false;
  bool           pressed : 1         = false;
  Fn<void(f32)>  on_changed          = fn([](f32) {});
  Axis           direction           = Axis::X;
  Frame          frame               = {.width = {100}, .height = {30}};
  Frame          thumb_frame         = {.width = {20}, .height = {20}};
  SizeConstraint track_height        = {10};
  Vec4           track_color         = DEFAULT_THEME.inactive;
  Vec4           thumb_color         = DEFAULT_THEME.primary;
  Vec4           thumb_hovered_color = DEFAULT_THEME.primary_variant;
  Vec4           track_corner_radii  = Vec4::splat(0.125F);
  Vec4           thumb_corner_radii  = Vec4::splat(0.125F);
  f32            t                   = 0;
  f32            low                 = 0;
  f32            high                = 1;
  u8             levels              = 0;

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    u8 const main_axis = (direction == Axis::X) ? 0 : 1;
    if (events.dragging)
    {
      t = unlerp(region.begin()[main_axis], region.end()[main_axis],
                 ctx.mouse_position[main_axis]);
      t = (levels == 0) ? t : grid_snap(t, 1.0F / levels);
      f32 const value = clamp(lerp(low, high, t), low, high);
      on_changed(value);
    }
    return ViewState{.pointable = !disabled, .draggable = !disabled};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return frame(allocated);
  }

  virtual void render(CRect const &region, CRect const &,
                      Canvas      &canvas) override
  {
    Vec2 const track_extent = Frame{frame.width, track_height}(region.extent);
    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = track_extent,
                           .corner_radii = track_corner_radii,
                           .tint         = ColorGradient::all(track_color)});

    if (levels != 0)
    {
      f32 const unit = 1 / (f32) levels;
      for (i32 i = 1; i < levels; i++)
      {
        Vec2 center = region.begin();
        center.x += unit * i * region.extent.x;
        center.y += region.extent.y / 2;

        canvas.rrect(ShapeDesc{
            .center       = center,
            .extent       = Vec2::splat(min(track_extent.y, track_extent.x)),
            .corner_radii = Vec4::splat(1.0F),
            .tint         = ColorGradient::all(thumb_color)});
      }
    }

    Vec2 thumb_center =
        region.begin() + Vec2{region.extent.x * t, region.extent.y / 2};
    Vec4 thumb_color  = (hovered && !pressed && !disabled) ?
                            this->thumb_hovered_color :
                            this->thumb_color;
    f32  dilation     = (hovered && !pressed && !disabled) ? 1.0F : 0.8F;
    Vec2 thumb_extent = thumb_frame(region.extent) * dilation;

    canvas.rrect(ShapeDesc{.center       = thumb_center,
                           .extent       = thumb_extent,
                           .corner_radii = thumb_corner_radii * thumb_extent.y,
                           .tint         = ColorGradient::all(thumb_color)});
  }
};

}        // namespace ash