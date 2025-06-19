/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/button.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

Button & Button::disable(bool d)
{
  state_.disabled = d;
  return *this;
}

Button & Button::color(Vec4U8 c)
{
  style_.color = c;
  return *this;
}

Button & Button::hovered_color(Vec4U8 c)
{
  style_.hovered_color = c;
  return *this;
}

Button & Button::disabled_color(Vec4U8 c)
{
  style_.disabled_color = c;
  return *this;
}

Button & Button::rrect(CornerRadii const & c)
{
  style_.corner_radii = c;
  style_.shape        = ButtonShape::RRect;
  return *this;
}

Button & Button::squircle(f32 degree)
{
  style_.corner_radii = CornerRadii{degree, degree, degree, degree};
  style_.shape        = ButtonShape::Squircle;
  return *this;
}

Button & Button::bevel(CornerRadii const & c)
{
  style_.corner_radii = c;
  style_.shape        = ButtonShape::Bevel;
  return *this;
}

Button & Button::frame(Frame f)
{
  style_.frame = f;
  return *this;
}

Button & Button::stroke(f32 stroke)
{
  style_.stroke = stroke;
  return *this;
}

Button & Button::thickness(f32 thickness)
{
  style_.thickness = thickness;
  return *this;
}

Button & Button::padding(Padding p)
{
  style_.padding = p;
  return *this;
}

Button & Button::on_pressed(Fn<void()> f)
{
  cb.pressed = f;
  return *this;
}

Button & Button::on_hovered(Fn<void()> f)
{
  cb.hovered = f;
  return *this;
}

ui::State Button::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  if (events.pointer_over())
  {
    cb.hovered();
  }

  if (events.pointer_down() ||
      (events.focus_over() && ctx.key.down(KeyCode::Return)))
  {
    cb.pressed();
  }

  state_.held = events.pointer_over() && ctx.mouse.held(MouseButton::Primary);
  state_.hovered = events.pointer_over();

  return ui::State{.pointable = !state_.disabled,
                   .clickable = !state_.disabled,
                   .focusable = !state_.disabled};
}

void Button::size(Vec2 allocated, Span<Vec2> sizes)
{
  auto const frame = style_.frame(allocated);
  auto       size  = frame - style_.padding.axes();
  size.x           = max(size.x, 0.0F);
  size.y           = max(size.y, 0.0F);

  fill(sizes, size);
}

Layout Button::fit(Vec2, Span<Vec2 const> sizes, Span<Vec2> centers)
{
  fill(centers, Vec2{0, 0});
  auto size   = sizes.is_empty() ? Vec2{0, 0} : sizes[0];
  auto padded = size + style_.padding.axes();

  if (style_.shape == ButtonShape::Squircle)
  {
    padded.x = padded.y = max(padded.x, padded.y);
  }

  return {.extent = padded};
}

void Button::render(Canvas & canvas, RenderInfo const & info)
{
  Vec4U8 tint;

  if (state_.disabled)
  {
    tint = style_.disabled_color;
  }
  else if (state_.hovered && !state_.held)
  {
    tint = style_.hovered_color;
  }
  else
  {
    tint = style_.color;
  }

  switch (style_.shape)
  {
    case ButtonShape::RRect:
      canvas.rrect({.area         = info.canvas_region,
                    .corner_radii = style_.corner_radii,
                    .stroke       = style_.stroke,
                    .thickness    = Vec2::splat(style_.thickness),
                    .tint         = tint,
                    .clip         = info.clip});
      break;
    case ButtonShape::Squircle:
      canvas.squircle({.area         = info.canvas_region,
                       .corner_radii = style_.corner_radii,
                       .stroke       = style_.stroke,
                       .thickness    = Vec2::splat(style_.thickness),
                       .tint         = tint,
                       .clip         = info.clip});
      break;
    case ButtonShape::Bevel:
      canvas.brect({.area         = info.canvas_region,
                    .corner_radii = style_.corner_radii,
                    .stroke       = style_.stroke,
                    .thickness    = Vec2::splat(style_.thickness),
                    .tint         = tint,
                    .clip         = info.clip});
      break;
    default:
      break;
  }
}

Cursor Button::cursor(Vec2, Vec2)
{
  return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

TextButton::TextButton(Str32 text, TextStyle const & style,
                       FontStyle const & font, AllocatorRef allocator) :
  text_{text, style, font, allocator}
{
}

TextButton::TextButton(Str8 text, TextStyle const & style,
                       FontStyle const & font, AllocatorRef allocator) :
  text_{text, style, font, allocator}
{
}

TextButton & TextButton::disable(bool d)
{
  Button::disable(d);
  return *this;
}

TextButton & TextButton::run(TextStyle const & style, FontStyle const & font,
                             usize first, usize count)
{
  text_.run(style, font, first, count);
  return *this;
}

TextButton & TextButton::text(Str32 t)
{
  text_.text(t);
  return *this;
}

TextButton & TextButton::text(Str8 t)
{
  text_.text(t);
  return *this;
}

TextButton & TextButton::color(Vec4U8 c)
{
  Button::color(c);
  return *this;
}

TextButton & TextButton::hovered_color(Vec4U8 c)
{
  Button::color(c);
  return *this;
}

TextButton & TextButton::disabled_color(Vec4U8 c)
{
  Button::color(c);
  return *this;
}

TextButton & TextButton::rrect(CornerRadii const & c)
{
  Button::rrect(c);
  return *this;
}

TextButton & TextButton::squircle(f32 degree)
{
  Button::squircle(degree);
  return *this;
}

TextButton & TextButton::bevel(CornerRadii const & c)
{
  Button::bevel(c);
  return *this;
}

TextButton & TextButton::frame(Frame f)
{
  Button::frame(f);
  return *this;
}

TextButton & TextButton::stroke(f32 stroke)
{
  Button::stroke(stroke);
  return *this;
}

TextButton & TextButton::thickness(f32 thickness)
{
  Button::thickness(thickness);
  return *this;
}

TextButton & TextButton::padding(Padding p)
{
  Button::padding(p);
  return *this;
}

TextButton & TextButton::on_pressed(Fn<void()> f)
{
  Button::on_pressed(f);
  return *this;
}

TextButton & TextButton::on_hovered(Fn<void()> f)
{
  Button::on_hovered(f);
  return *this;
}

ui::State TextButton::tick(Ctx const & ctx, Events const & events,
                           Fn<void(View &)> build)
{
  ui::State state_ = Button::tick(ctx, events, build);
  build(text_);
  return state_;
}

}    // namespace ui

}    // namespace ash
