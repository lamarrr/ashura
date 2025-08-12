/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/scroll_view.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

ScrollBar::ScrollBar() : style_{}
{
}

ScrollBar & ScrollBar::center(f32 v)
{
  state_.scroll.center(v);
  return *this;
}

ScrollBar & ScrollBar::delta(f32 v)
{
  state_.scroll.delta(v);
  return *this;
}

ScrollBar & ScrollBar::extent(f32 visible, f32 content, f32 track)
{
  state_.scroll.extent(visible, content, track);
  return *this;
}

ScrollBar & ScrollBar::thickness(f32 t)
{
  style_.thickness = t;
  return *this;
}

ScrollBar & ScrollBar::disable(bool d)
{
  state_.disabled = d;
  return *this;
}

ScrollBar & ScrollBar::thumb_color(u8x4 color)
{
  style_.thumb_color = color;
  return *this;
}

ScrollBar & ScrollBar::thumb_hovered_color(u8x4 color)
{
  style_.thumb_hovered_color = color;
  return *this;
}

ScrollBar & ScrollBar::thumb_dragging_color(u8x4 color)
{
  style_.thumb_dragging_color = color;
  return *this;
}

ScrollBar & ScrollBar::thumb_corner_radii(CornerRadii const & c)
{
  style_.thumb_corner_radii = c;
  return *this;
}

ScrollBar & ScrollBar::track_color(u8x4 color)
{
  style_.track_color = color;
  return *this;
}

ScrollBar & ScrollBar::track_corner_radii(CornerRadii const & c)
{
  style_.track_corner_radii = c;
  return *this;
}

ScrollBar & ScrollBar::axis(Axis axis)
{
  style_.axis = axis;
  return *this;
}

ui::State ScrollBar::tick(Ctx const & ctx, Events const & events,
                          Fn<void(View &)>)
{
  u32 const main_axis = (style_.axis == Axis::X) ? 0 : 1;

  if (events.drag_update())
  {
    // [ ] +- nan,  +- inf
    // [ ] center is relative to the content extent
    auto h     = events.hit_info.unwrap_or();
    auto begin = h.viewport_region.begin()[main_axis];
    auto end   = h.viewport_region.end()[main_axis];
    auto scale =
      h.viewport_region.extent[main_axis] / state_.scroll.content_extent();
    auto thumb_extent = scale * state_.scroll.visible_extent();
    auto track_begin  = begin + 0.5F * thumb_extent;
    auto track_end    = end - 0.5F * thumb_extent;
    auto thumb_pos = clamp(h.viewport_hit[main_axis], track_begin, track_end);
    auto t         = unlerp(track_begin, track_end, thumb_pos);
    state_.scroll.center(
      lerp(0.0F,
           state_.scroll.content_extent() - state_.scroll.visible_extent(), t));
  }

  if (events.focus_over())
  {
    if ((style_.axis == Axis::X && ctx.key.down(KeyCode::Left)) ||
        (style_.axis == Axis::Y && ctx.key.down(KeyCode::Up)))
    {
      state_.scroll.center(clamp(
        state_.scroll.center() -
          state_.scroll.delta() * state_.scroll.visible_extent(),
        0.0F, state_.scroll.content_extent() - state_.scroll.visible_extent()));
    }
    else if ((style_.axis == Axis::X && ctx.key.down(KeyCode::Right)) ||
             (style_.axis == Axis::Y && ctx.key.down(KeyCode::Down)))
    {
      state_.scroll.center(clamp(
        state_.scroll.center() +
          state_.scroll.delta() * state_.scroll.visible_extent(),
        0.0F, state_.scroll.content_extent() - state_.scroll.visible_extent()));
    }
  }

  state_.dragging = events.drag_update();
  state_.hovered  = events.pointer_over();
  state_.focused  = events.focus_over();

  return ui::State{.hidden    = state_.hidden,
                   .pointable = !state_.disabled,
                   .draggable = !state_.disabled,
                   .focusable = !state_.disabled};
}

Layout ScrollBar::fit(f32x2, Span<f32x2 const>, Span<f32x2>)
{
  u32 const main_axis  = (style_.axis == Axis::X) ? 0 : 1;
  u32 const cross_axis = (style_.axis == Axis::X) ? 1 : 0;

  f32x2 size;

  size[main_axis]  = state_.scroll.track_extent();
  size[cross_axis] = style_.thickness;

  return {.extent = size};
}

void ScrollBar::render(Canvas & canvas, RenderInfo const & info)
{
  u32 const main_axis  = (style_.axis == Axis::X) ? 0 : 1;
  u32 const cross_axis = (style_.axis == Axis::X) ? 1 : 0;

  // [ ] nan, inf
  auto const scale =
    info.canvas_region.extent[main_axis] / state_.scroll.content_extent();
  auto const thumb_extent = state_.scroll.visible_extent() * scale;
  auto const t            = unlerp(
    0.0F, state_.scroll.content_extent() - state_.scroll.visible_extent(),
    state_.scroll.center());
  f32 const thumb_center =
    info.canvas_region.begin()[main_axis] + 0.5F * thumb_extent +
    t * (info.canvas_region.extent[main_axis] - thumb_extent);

  CRect thumb_rect;

  thumb_rect.center[main_axis]  = thumb_center;
  thumb_rect.center[cross_axis] = info.canvas_region.center[cross_axis];
  thumb_rect.extent[main_axis]  = thumb_extent;
  thumb_rect.extent[cross_axis] = info.canvas_region.extent[cross_axis];

  u8x4 thumb_color;
  u8x4 track_color = style_.track_color;

  if (state_.dragging)
  {
    thumb_color = style_.thumb_dragging_color;
  }
  else if (state_.hovered)
  {
    thumb_color = style_.thumb_hovered_color;
  }
  else
  {
    thumb_color = style_.thumb_color;
  }

  canvas
    .rrect({.area         = info.canvas_region,
            .corner_radii = style_.track_corner_radii,
            .stroke       = 0,
            .tint         = track_color,
            .clip         = info.clip})
    .rrect({.area         = thumb_rect,
            .corner_radii = style_.thumb_corner_radii,
            .stroke       = 0,
            .tint         = thumb_color,
            .clip         = info.clip});
}

ScrollContent::ScrollContent(View & child) : child_{child}
{
}

ScrollContent & ScrollContent::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

ui::State ScrollContent::tick(Ctx const &, Events const &,
                              Fn<void(View &)> build)
{
  build(child_);
  return ui::State{};
}

void ScrollContent::size(f32x2 allocated, Span<f32x2> sizes)
{
  sizes[0] = style_.frame(allocated);
}

Layout ScrollContent::fit(f32x2, Span<f32x2 const> sizes, Span<f32x2> centers)
{
  centers[0] = f32x2::splat(0);
  return {.extent = sizes[0]};
}

ScrollPort::ScrollPort(View & child) : content_{child}
{
}

ScrollPort & ScrollPort::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

ui::State ScrollPort::tick(Ctx const &, Events const &, Fn<void(View &)> build)
{
  build(content_);
  return ui::State{.viewport = true};
}

void ScrollPort::size(f32x2 allocated, Span<f32x2> sizes)
{
  fill(sizes, allocated);
}

Layout ScrollPort::fit(f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers)
{
  centers[0]                = f32x2::splat(0);
  auto const content_extent = sizes[0];
  auto const visible_extent = style_.frame(allocated);

  state_.content_extent = content_extent;
  state_.visible_extent = visible_extent;

  return {.extent          = visible_extent,
          .viewport_extent = content_extent,
          .viewport_center = state_.center,
          .viewport_zoom   = state_.zoom};
}

ScrollView::ScrollView(View & child) : x_bar_{}, y_bar_{}, port_{child}
{
  x_bar_.axis(Axis::X);
  y_bar_.axis(Axis::Y);
}

ScrollView & ScrollView::disable(bool d)
{
  x_bar_.disable(d);
  y_bar_.disable(d);
  return *this;
}

ScrollView & ScrollView::item(View & v)
{
  port_.content_.child_ = v;
  return *this;
}

ScrollView & ScrollView::thumb_color(u8x4 c)
{
  x_bar_.thumb_color(c);
  y_bar_.thumb_color(c);
  return *this;
}

ScrollView & ScrollView::thumb_hovered_color(u8x4 c)
{
  x_bar_.thumb_hovered_color(c);
  y_bar_.thumb_hovered_color(c);
  return *this;
}

ScrollView & ScrollView::thumb_dragging_color(u8x4 c)
{
  x_bar_.thumb_dragging_color(c);
  y_bar_.thumb_dragging_color(c);
  return *this;
}

ScrollView & ScrollView::thumb_corner_radii(CornerRadii const & c)
{
  x_bar_.thumb_corner_radii(c);
  y_bar_.thumb_corner_radii(c);
  return *this;
}

ScrollView & ScrollView::track_color(u8x4 c)
{
  x_bar_.track_color(c);
  y_bar_.track_color(c);
  return *this;
}

ScrollView & ScrollView::track_corner_radii(CornerRadii const & c)
{
  x_bar_.track_corner_radii(c);
  y_bar_.track_corner_radii(c);
  return *this;
}

ScrollView & ScrollView::axes(Axes a)
{
  x_bar_.state_.hidden = has_bits(a, Axes::X);
  y_bar_.state_.hidden = has_bits(a, Axes::Y);
  return *this;
}

ScrollView & ScrollView::view_frame(Frame f)
{
  port_.style_.frame = f;
  return *this;
}

ScrollView & ScrollView::content_frame(Frame f)
{
  port_.content_.frame(f);
  return *this;
}

ScrollView & ScrollView::bar_thickness(f32 x, f32 y)
{
  x_bar_.thickness(x);
  y_bar_.thickness(y);
  return *this;
}

ui::State ScrollView::tick(Ctx const &, Events const & events,
                           Fn<void(View &)> build)
{
  f32 y_bar_nudge = 0;

  if (!x_bar_.state_.disabled && !y_bar_.state_.disabled)
  {
    // prevent overlap of the bars
    y_bar_nudge = x_bar_.style_.thickness + x_bar_.style_.nudge;
  }

  x_bar_.extent(port_.state_.visible_extent.x, port_.state_.content_extent.x,
                port_.state_.visible_extent.x);
  y_bar_.extent(port_.state_.visible_extent.y, port_.state_.content_extent.y,
                max(port_.state_.visible_extent.y - y_bar_nudge, 0.0F));

  if (events.scroll())
  {
    auto scroll = events.scroll_info.unwrap();

    if (!x_bar_.state_.disabled)
    {
      x_bar_.state_.scroll.center(scroll.center.x);
    }

    if (!y_bar_.state_.disabled)
    {
      y_bar_.state_.scroll.center(scroll.center.y);
    }
  }

  // [ ] remove
  port_.state_.center = f32x2{0, -700} + f32x2{x_bar_.state_.scroll.center(),
                                               y_bar_.state_.scroll.center()};

  build(x_bar_);
  build(y_bar_);
  build(port_);

  return ui::State{.scrollable =
                     !(x_bar_.state_.disabled && y_bar_.state_.disabled)};
}

void ScrollView::size(f32x2 allocated, Span<f32x2> sizes)
{
  // [ ] the barsd will have invalid extents
  fill(sizes, allocated);
}

Layout ScrollView::fit(f32x2, Span<f32x2 const> sizes, Span<f32x2> centers)
{
  centers[0] =
    space_align(port_.state_.visible_extent, sizes[0], ALIGNMENT_BOTTOM_LEFT);
  centers[1] =
    space_align(port_.state_.visible_extent, sizes[1], ALIGNMENT_TOP_RIGHT);
  centers[2] = {0, 0};

  return {.extent = port_.state_.visible_extent};
}

i32 ScrollView::layer(i32 allocated, Span<i32> layers)
{
  // needs to be at a different stacking context since this will be placed
  // on top of the viewport
  layers[0] = LAYERS.viewport_bars;
  layers[1] = LAYERS.viewport_bars;
  layers[2] = allocated;
  return allocated;
}

}    // namespace ui

}    // namespace ash
