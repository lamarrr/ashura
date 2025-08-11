/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/check_box.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

CheckBox::CheckBox(Str32 text, TextStyle const & style, FontStyle const & font,
                   Allocator allocator) :
  icon_{text, style, font, allocator}
{
}

CheckBox::CheckBox(Str8 text, TextStyle const & style, FontStyle const & font,
                   Allocator allocator) :
  icon_{text, style, font, allocator}
{
}

Icon & CheckBox::icon()
{
  return icon_;
}

CheckBox & CheckBox::disable(bool d)
{
  state_.disabled = d;
  return *this;
}

CheckBox & CheckBox::box_color(u8x4 c)
{
  style_.box_color = c;
  return *this;
}

CheckBox & CheckBox::box_hovered_color(u8x4 c)
{
  style_.box_hovered_color = c;
  return *this;
}

CheckBox & CheckBox::stroke(f32 s)
{
  style_.stroke = s;
  return *this;
}

CheckBox & CheckBox::thickness(f32 t)
{
  style_.thickness = t;
  return *this;
}

CheckBox & CheckBox::corner_radii(CornerRadii const & r)
{
  style_.corner_radii = r;
  return *this;
}

CheckBox & CheckBox::on_changed(Fn<void(bool)> f)
{
  cb.changed = f;
  return *this;
}

ui::State CheckBox::tick(Ctx const & ctx, Events const & events,
                         Fn<void(View &)> build)
{
  if (events.pointer_down() ||
      (events.focus_over() && ctx.key.down(KeyCode::Return)))
  {
    state_.value = !state_.value;
    cb.changed(state_.value);
  }

  icon_.hide(!state_.value);

  build(icon_);

  return ui::State{.pointable = !state_.disabled,
                   .clickable = !state_.disabled,
                   .focusable = !state_.disabled};
}

void CheckBox::size(f32x2 allocated, Span<f32x2> sizes)
{
  fill(sizes, allocated);
}

Layout CheckBox::fit(f32x2, Span<f32x2 const> sizes, Span<f32x2> centers)
{
  fill(centers, f32x2{});
  return {.extent = style_.frame(sizes[0])};
}

void CheckBox::render(Canvas & canvas, RenderInfo const & info)
{
  u8x4 tint;
  if (state_.hovered && !state_.held && !state_.disabled)
  {
    tint = style_.box_hovered_color;
  }
  else
  {
    tint = style_.box_color;
  }

  canvas.rrect({.area         = info.canvas_region,
                .corner_radii = style_.corner_radii,
                .stroke       = 1,
                .thickness    = f32x2::splat(style_.thickness),
                .tint         = tint,
                .clip         = info.clip});
}

Cursor CheckBox::cursor(f32x2, f32x2)
{
  return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

}    // namespace ui

}    // namespace ash
