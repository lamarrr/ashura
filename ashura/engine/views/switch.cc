/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/switch.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

Switch & Switch::disable(bool disable)
{
  state_.disabled = disable;
  return *this;
}

Switch & Switch::on()
{
  state_.value = true;
  cb.changed(true);
  return *this;
}

Switch & Switch::off()
{
  state_.value = false;
  cb.changed(false);
  return *this;
}

Switch & Switch::toggle()
{
  if (state_.value)
  {
    on();
  }
  else
  {
    off();
  }
  return *this;
}

Switch & Switch::on_color(u8x4 c)
{
  style_.on_color = c;
  return *this;
}

Switch & Switch::on_hovered_color(u8x4 c)
{
  style_.on_hovered_color = c;
  return *this;
}

Switch & Switch::off_color(u8x4 c)
{
  style_.off_color = c;
  return *this;
}

Switch & Switch::off_hovered_color(u8x4 c)
{
  style_.off_hovered_color = c;
  return *this;
}

Switch & Switch::track_color(u8x4 c)
{
  style_.track_color = c;
  return *this;
}

Switch & Switch::corner_radii(CornerRadii const & r)
{
  style_.corner_radii = r;
  return *this;
}

Switch & Switch::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

ui::State Switch::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
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

Layout Switch::fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
  return {.extent = style_.frame(allocated)};
}

void Switch::render(Canvas & canvas, RenderInfo const & info)
{
  auto thumb_extent = info.canvas_region.extent;
  thumb_extent.x *= 0.5F;
  f32x2 const alignment{state_.value ? ALIGNMENT_RIGHT : ALIGNMENT_LEFT,
                        ALIGNMENT_CENTER};
  auto const thumb_center =
    info.canvas_region.center +
    space_align(info.canvas_region.extent, thumb_extent, alignment);

  u8x4 thumb_color;
  if (state_.hovered)
  {
    thumb_color =
      state_.value ? style_.on_hovered_color : style_.off_hovered_color;
  }
  else
  {
    thumb_color = state_.value ? style_.on_color : style_.off_color;
  }

  canvas
    .rrect({
      .area         = info.canvas_region,
      .corner_radii = style_.corner_radii,
      .tint         = style_.track_color,
      .clip         = info.clip
  })
    .rrect({.area{thumb_center, thumb_extent},
            .corner_radii = style_.corner_radii,
            .tint         = thumb_color,
            .clip         = info.clip});
}

Cursor Switch::cursor(f32x2, f32x2)
{
  return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

}    // namespace ui

}    // namespace ash
