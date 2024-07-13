/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/widget.h"
#include "ashura/std/types.h"

namespace ash
{

struct ScrollBar : public Widget
{
  Axis          direction         = Axis::X;
  f32           opacity           = 0.65F;
  Vec4          thumb_color       = material::GRAY_400.norm();
  Vec4          track_color       = material::GRAY_800.norm();
  Vec2          frame_extent      = {};
  Vec2          content_extent    = {};
  f32           scroll_percentage = 0;
  bool          disabled          = false;
  Fn<void(f32)> on_scrolled       = to_fn([](f32) {});

  explicit ScrollBar(Axis direction) : direction{direction}
  {
  }

  virtual WidgetAttributes attributes() override
  {
    return WidgetAttributes::Visible | WidgetAttributes::Clickable |
           WidgetAttributes::Draggable;
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return allocated;
  }

  virtual void render(CRect const &region, Canvas &canvas) override
  {
    u8 const   main_axis    = (direction == Axis::X) ? 0 : 1;
    u8 const   cross_axis   = (direction == Axis::Y) ? 1 : 0;
    Vec4 const border_radii = Vec4::splat(region.extent.y * 0.09F);

    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .border_radii = border_radii,
                           .stroke       = 0,
                           .tint = ColorGradient::uniform(track_color)});

    // calculate thumb main axis extent
    f32 const scale = frame_extent[main_axis] / content_extent[main_axis];
    Vec2      thumb_extent   = {0, 0};
    thumb_extent[cross_axis] = region.extent[cross_axis];
    thumb_extent[main_axis]  = scale * region.extent[main_axis];

    // align thumb to remaining space based on size of visible region
    Vec2 const bar_offset  = region.begin();
    f32 const main_spacing = thumb_extent[main_axis] - region.extent[main_axis];
    Vec2      thumb_center;
    thumb_center[main_axis] = bar_offset[main_axis] +
                              main_spacing * scroll_percentage +
                              thumb_extent[main_axis] / 2;
    thumb_center[cross_axis] = region.center[cross_axis];

    canvas.rrect(ShapeDesc{.center       = region.center,
                           .extent       = region.extent,
                           .border_radii = border_radii,
                           .stroke       = 1,
                           .thickness    = 1,
                           .tint         = ColorGradient::uniform(
                               track_color * to_vec4({1, 1, 1}, opacity))});

    canvas.rrect(ShapeDesc{.center       = thumb_center,
                           .extent       = thumb_extent,
                           .border_radii = border_radii,
                           .stroke       = 0,
                           .tint         = ColorGradient::uniform(
                               thumb_color * to_vec4({1, 1, 1}, opacity))});
  }

  virtual void tick(WidgetContext const &ctx, CRect const &region, nanoseconds,
                    WidgetEventTypes events) override
  {
    u8 const main_axis = (direction == Axis::X) ? 0 : 1;

    if (!disabled && has_bits(events, WidgetEventTypes::DragUpdate))
    {
      scroll_percentage +=
          ctx.mouse_translation[main_axis] / region.extent[main_axis];
      scroll_percentage = clamp(scroll_percentage, 0.0f, 1.0f);
      on_scrolled(scroll_percentage);
    }

    if (!disabled && has_bits(events, WidgetEventTypes::DragEnd))
    {
      scroll_percentage =
          clamp((ctx.mouse_position[main_axis] - region.extent[main_axis] / 2) /
                    region.extent[main_axis],
                0.0f, 1.0f);
      on_scrolled(scroll_percentage);
    }
  }
};

struct ScrollBox : public Widget
{
  ScrollBar      x_bar      = ScrollBar{Axis::X};
  ScrollBar      y_bar      = ScrollBar{Axis::Y};
  Axes           axes       = Axes::X | Axes::Y;
  SizeConstraint width      = {.scale = 1, .max = 200};
  SizeConstraint height     = {.scale = 1, .max = 200};
  SizeConstraint x_bar_size = {.offset = 10};
  SizeConstraint y_bar_size = {.offset = 10};

  virtual WidgetAttributes attributes() override
  {
    return WidgetAttributes::Visible;
  }

  virtual Widget *child(u32 i) override final
  {
    return child_iter({&x_bar, &y_bar, item()}, i);
  }

  virtual Widget *item()
  {
    return nullptr;
  }

  virtual i32 stack(i32 z_index, Span<i32> allocation) override
  {
    static constexpr i32 ELEVATION = 128;
    allocation[0]                  = z_index + ELEVATION;
    allocation[1]                  = z_index + ELEVATION;
    fill(allocation.slice(2), z_index + 1);
    return z_index;
  }

  virtual void size(Vec2 allocated, Span<Vec2> sizes) override
  {
    Vec2      frame{width.resolve(allocated.x), height.resolve(allocated.y)};
    f32 const x_bar_size_r = x_bar_size.resolve(allocated.x);
    f32 const y_bar_size_r = y_bar_size.resolve(allocated.y);

    sizes[0] = {frame.x, x_bar_size_r};

    if (has_bits(axes, Axes::Y))
    {
      sizes[0].x -= y_bar_size_r;
    }

    sizes[1] = {y_bar_size_r, frame.y};

    fill(sizes.slice(2), frame);

    x_bar.frame_extent = frame;
    y_bar.frame_extent = frame;
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const> sizes,
                   Span<Vec2> offsets) override
  {
    Vec2 frame{width.resolve(allocated.x), height.resolve(allocated.y)};

    offsets[0] = space_align(frame, sizes[0], Vec2{1, 0});
    offsets[1] = space_align(frame, sizes[1], Vec2{-1, 1});

    Vec2 content_size;
    for (Vec2 const &sz : sizes.slice(2))
    {
      content_size.x = max(content_size.x, sz.x);
      content_size.y = max(content_size.y, sz.y);
    }

    x_bar.content_extent = content_size;
    y_bar.content_extent = content_size;

    Vec2 const displacement =
        -1 * (content_size - frame) *
        Vec2{x_bar.scroll_percentage, y_bar.scroll_percentage};

    fill(offsets.slice(2), displacement);
    return frame;
  }

  virtual CRect clip(CRect const &region, CRect const &allocated) override
  {
    return intersect(region.offseted(), allocated.offseted()).centered();
  }
};

};        // namespace ash