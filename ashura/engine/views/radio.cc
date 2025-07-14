/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/radio.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

Radio & Radio::disable(bool disable)
{
  state_.disabled = disable;
  return *this;
}

Radio & Radio::corner_radii(CornerRadii const & c)
{
  style_.corner_radii = c;
  return *this;
}

Radio & Radio::thickness(f32 t)
{
  style_.thickness = t;
  return *this;
}

Radio & Radio::color(u8x4 c)
{
  style_.color = c;
  return *this;
}

Radio & Radio::inner_color(u8x4 c)
{
  style_.inner_color = c;
  return *this;
}

Radio & Radio::inner_hovered_color(u8x4 c)
{
  style_.inner_hovered_color = c;
  return *this;
}

Radio & Radio::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

Radio & Radio::on_changed(Fn<void(bool)> f)
{
  cb.changed = f;
  return *this;
}

ui::State Radio::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  if (events.pointer_down() ||
      (events.focus_over() && ctx.key.down(KeyCode::Return)))
  {
    state_.value = !state_.value;
    cb.changed(state_.value);
  }

  state_.hovered = events.pointer_over();

  return ui::State{.pointable = !state_.disabled,
                   .clickable = !state_.disabled,
                   .focusable = !state_.disabled};
}

Layout Radio::fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
  return {.extent = style_.frame(allocated)};
}

void Radio::render(Canvas & canvas, RenderInfo const & info)
{
  canvas.rrect({.area         = info.canvas_region,
                .corner_radii = style_.corner_radii,
                .stroke       = 1,
                .thickness    = f32x2::splat(style_.thickness),
                .tint         = style_.color,
                .clip         = info.clip});

  if (state_.value)
  {
    auto inner_extent =
      info.canvas_region.extent * (state_.hovered ? 0.75F : 0.5F);
    auto inner_color =
      state_.hovered ? style_.inner_hovered_color : style_.inner_color;

    canvas.circle({
      .area{info.canvas_region.center, inner_extent},
      .tint = inner_color,
      .clip = info.clip
    });
  }
}

Cursor Radio::cursor(f32x2, f32x2)
{
  return Cursor::Pointer;
}

}    // namespace ui

}    // namespace ash
