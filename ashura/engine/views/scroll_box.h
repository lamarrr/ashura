/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

struct ScrollBar : public View
{
  bool          disabled : 1         = false;
  bool          hovered : 1          = false;
  bool          dragging : 1         = false;
  Fn<void(f32)> on_scrolled          = fn([](f32) {});
  Axis          direction : 2        = Axis::X;
  Vec4          thumb_color          = DEFAULT_THEME.inactive * opacity(0.75F);
  Vec4          thumb_dragging_color = DEFAULT_THEME.active * opacity(0.75F);
  Vec4          track_color          = DEFAULT_THEME.inactive * opacity(0.75F);
  Vec2          frame_extent         = {};
  Vec2          content_extent       = {};
  f32           t                    = 0;

  explicit ScrollBar(Axis direction) : direction{direction}
  {
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    u8 const main_axis = (direction == Axis::X) ? 0 : 1;

    if (events.mouse_enter)
    {
      hovered = true;
    }

    if (events.mouse_leave)
    {
      hovered = false;
    }

    if (events.dragging)
    {
      dragging = true;
      t = clamp((ctx.mouse_position[main_axis] - region.extent[main_axis] / 2) /
                    region.extent[main_axis],
                0.0f, 1.0f);
      on_scrolled(t);
    }

    if (events.drag_end)
    {
      dragging = false;
    }

    return ViewState{.pointable = !disabled, .draggable = !disabled};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return allocated;
  }

  virtual void render(CRect const &region, CRect const &,
                      Canvas      &canvas) override
  {
    u8 const   main_axis    = (direction == Axis::X) ? 0 : 1;
    u8 const   cross_axis   = (direction == Axis::Y) ? 1 : 0;
    Vec4 const corner_radii = Vec4::splat(region.extent.y * 0.09F);

    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = corner_radii,
                           .stroke       = 0,
                           .tint         = ColorGradient::all(track_color)});

    // calculate thumb main axis extent
    f32 const scale = frame_extent[main_axis] / content_extent[main_axis];
    Vec2      thumb_extent   = {0, 0};
    thumb_extent[cross_axis] = region.extent[cross_axis];
    thumb_extent[main_axis]  = scale * region.extent[main_axis];

    // align thumb to remaining space based on size of visible region
    Vec2 const bar_offset  = region.begin();
    f32 const main_spacing = thumb_extent[main_axis] - region.extent[main_axis];
    Vec2      thumb_center;
    thumb_center[main_axis] =
        bar_offset[main_axis] + main_spacing * t + thumb_extent[main_axis] / 2;
    thumb_center[cross_axis] = region.center[cross_axis];

    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .corner_radii = corner_radii,
                           .stroke       = 1,
                           .thickness    = 1,
                           .tint         = ColorGradient::all(track_color)});

    canvas.rrect(ShapeDesc{.center       = thumb_center,
                           .extent       = thumb_extent,
                           .corner_radii = corner_radii,
                           .stroke       = 0,
                           .tint         = ColorGradient::all(
                               dragging ? thumb_dragging_color : thumb_color)});
  }
};

struct ScrollBox : public View
{
  bool           disabled : 1 = false;
  Axes           axes         = Axes::X | Axes::Y;
  ScrollBar      x_bar        = ScrollBar{Axis::X};
  ScrollBar      y_bar        = ScrollBar{Axis::Y};
  Frame          frame        = {.width = {200}, .height = {200}};
  SizeConstraint x_bar_size   = {.offset = 10};
  SizeConstraint y_bar_size   = {.offset = 10};

  virtual View *iter(u32 i) override final
  {
    return subview({&x_bar, &y_bar, item()}, i);
  }

  virtual View *item()
  {
    return nullptr;
  }

  virtual ViewState tick(ViewContext const &, CRect const &,
                         ViewEvents) override
  {
    x_bar.disabled = disabled;
    y_bar.disabled = disabled;
    return ViewState{};
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2 const frame        = this->frame(allocated);
    f32 const  x_bar_size_r = x_bar_size(allocated.x);
    f32 const  y_bar_size_r = y_bar_size(allocated.y);

    sizes[0] = {frame.x, x_bar_size_r};

    if (has_bits(axes, Axes::Y))
    {
      sizes[0].x -= y_bar_size_r;
    }

    sizes[1] = {y_bar_size_r, frame.y};

    fill(sizes.slice(2), frame);
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) override
  {
    Vec2 const frame = this->frame(allocated);
    offsets[0]       = space_align(frame, sizes[0], Vec2{1, 0});
    offsets[1]       = space_align(frame, sizes[1], Vec2{-1, 1});

    Vec2 content_size;
    for (Vec2 const &sz : sizes.slice(2))
    {
      content_size.x = max(content_size.x, sz.x);
      content_size.y = max(content_size.y, sz.y);
    }

    Vec2 const displacement =
        -1 * (content_size - frame) * Vec2{x_bar.t, y_bar.t};

    fill(offsets.slice(2), displacement);

    x_bar.content_extent = content_size;
    x_bar.frame_extent   = frame;
    y_bar.content_extent = content_size;
    y_bar.frame_extent   = frame;

    return frame;
  }

  virtual i32 stack(i32 z_index, Span<i32> indices) override
  {
    static constexpr i32 ELEVATION = 128;
    indices[0]                     = z_index + ELEVATION;
    indices[1]                     = z_index + ELEVATION;
    fill(indices.slice(2), z_index + 1);
    return z_index;
  }

  virtual CRect clip(CRect const &region, CRect const &allocated) override
  {
    return intersect(region.offseted(), allocated.offseted()).centered();
  }
};

};        // namespace ash