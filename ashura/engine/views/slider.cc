/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.h"
#include "ashura/engine/views/slider.h"

namespace ash
{

namespace ui
{

Slider & Slider::disable(bool disable)
{
  state_.disabled = disable;
  return *this;
}

Slider & Slider::range(f32 low, f32 high)
{
  state_.low  = low;
  state_.high = high;
  return *this;
}

Slider & Slider::interp(f32 t)
{
  state_.t = t;
  return *this;
}

Slider & Slider::axis(Axis a)
{
  style_.axis = a;
  return *this;
}

Slider & Slider::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

Slider & Slider::thumb_size(f32 size)
{
  style_.thumb_size = size;
  return *this;
}

Slider & Slider::track_size(f32 size)
{
  style_.track_size = size;
  return *this;
}

Slider & Slider::thumb_color(Vec4U8 c)
{
  style_.thumb_color = c;
  return *this;
}

Slider & Slider::thumb_hovered_color(Vec4U8 c)
{
  style_.thumb_hovered_color = c;
  return *this;
}

Slider & Slider::thumb_dragging_color(Vec4U8 c)
{
  style_.thumb_dragging_color = c;
  return *this;
}

Slider & Slider::thumb_corner_radii(CornerRadii const & c)
{
  style_.thumb_corner_radii = c;
  return *this;
}

Slider & Slider::track_color(Vec4U8 c)
{
  style_.track_color = c;
  return *this;
}

Slider & Slider::track_corner_radii(CornerRadii const & c)
{
  style_.track_corner_radii = c;
  return *this;
}

Slider & Slider::on_changed(Fn<void(f32)> f)
{
  cb.changed = f;
  return *this;
}

ui::State Slider::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  u32 const main_axis = (style_.axis == Axis::X) ? 0 : 1;

  if (events.drag_update())
  {
    auto      h = events.hit_info.unwrap_or();
    f32 const thumb_begin =
      h.viewport_region.begin()[main_axis] + style_.thumb_size * 0.5F;
    f32 const thumb_end =
      h.viewport_region.end()[main_axis] - style_.thumb_size * 0.5F;
    state_.t = clamp(unlerp(thumb_begin, thumb_end, h.viewport_hit[main_axis]),
                     0.0F, 1.0F);
    f32 const value =
      clamp(lerp(state_.low, state_.high, state_.t), state_.low, state_.high);
    cb.changed(value);
  }

  if (events.focus_over())
  {
    if ((style_.axis == Axis::X && ctx.key.down(KeyCode::Left)) ||
        (style_.axis == Axis::Y && ctx.key.down(KeyCode::Up)))
    {
      state_.t = max(state_.t - style_.delta, 0.0F);
    }
    else if ((style_.axis == Axis::X && ctx.key.down(KeyCode::Right)) ||
             (style_.axis == Axis::Y && ctx.key.down(KeyCode::Down)))
    {
      state_.t = min(state_.t + style_.delta, 1.0F);
    }
  }

  state_.dragging = events.drag_update();
  state_.hovered  = events.pointer_over();

  return ui::State{.pointable = !state_.disabled,
                   .draggable = !state_.disabled,
                   .focusable = !state_.disabled};
}

Layout Slider::fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>)
{
  return {.extent = style_.frame(allocated)};
}

void Slider::render(Canvas & canvas, RenderInfo const & info)
{
  u32 const main_axis  = (style_.axis == Axis::X) ? 0 : 1;
  u32 const cross_axis = (style_.axis == Axis::Y) ? 0 : 1;

  Vec4U8 thumb_color;

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

  f32 dilation = 1.0F;

  if (state_.dragging || state_.hovered)
  {
    dilation = 1.0F;
  }
  else
  {
    dilation = 0.8F;
  }

  f32 const thumb_begin =
    info.canvas_region.begin()[main_axis] + style_.thumb_size * 0.5F;
  f32 const thumb_end =
    info.canvas_region.end()[main_axis] - style_.thumb_size * 0.5F;
  f32 const thumb_center = lerp(thumb_begin, thumb_end, state_.t);

  CRect thumb_rect;

  thumb_rect.center[main_axis]  = thumb_center;
  thumb_rect.center[cross_axis] = info.canvas_region.center[cross_axis];
  thumb_rect.extent             = Vec2::splat(style_.thumb_size);

  CRect track_rect;

  track_rect.center             = info.canvas_region.center;
  track_rect.extent[main_axis]  = thumb_end - thumb_begin;
  track_rect.extent[cross_axis] = style_.track_size;

  Vec2 coverage_begin;
  coverage_begin[main_axis]  = thumb_begin;
  coverage_begin[cross_axis] = track_rect.begin()[cross_axis];

  Vec2 coverage_end;
  coverage_end[main_axis]  = thumb_center;
  coverage_end[cross_axis] = track_rect.end()[cross_axis];

  CRect const coverage_rect = CRect::range(coverage_begin, coverage_end);

  canvas
    .rrect({
      .area         = track_rect,
      .corner_radii = style_.track_corner_radii,
      .tint         = style_.track_color
  })
    .rrect({.area         = coverage_rect,
            .corner_radii = style_.track_corner_radii,
            .tint         = thumb_color})
    .rrect({.area{thumb_rect.center, thumb_rect.extent * dilation},
            .corner_radii = style_.thumb_corner_radii * dilation,
            .tint         = thumb_color});
}

Cursor Slider::cursor(Vec2, Vec2)
{
  return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

}    // namespace ui

}    // namespace ash
