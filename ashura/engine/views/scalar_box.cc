/// SPDX-License-Identifier: MIT
#include "ashura/engine/engine.h"
#include "ashura/engine/views/scalar_box.h"

namespace ash
{

namespace ui
{

ScalarDragBox::ScalarDragBox(TextStyle const & style, FontStyle const & font,
                             AllocatorRef allocator) :
  input_{U""_str, style, font, allocator}
{
  input_.multiline(false).tab_input(false).enter_submits(false);
}

void ScalarDragBox::scalar_parse(Str32 text, ScalarInfo const & spec,
                                 Scalar & scalar)
{
  if (text.is_empty())
  {
    return;
  }

  spec.match(
    [&](F32Info const & spec) {
      f32 value = 0;
      auto [ptr, ec] =
        fast_float::from_chars(text.pbegin(), text.pend(), value);
      if (ec != std::errc{} || value < spec.min || value > spec.max)
      {
        return;
      }
      scalar = value;
    },
    [&](I32Info const & spec) {
      i32 value = 0;
      auto [ptr, ec] =
        fast_float::from_chars(text.pbegin(), text.pend(), value);
      if (ec != std::errc{} || value < spec.min || value > spec.max)
      {
        return;
      }
      scalar = value;
    });
}

void ScalarDragBox::format_()
{
  u8                buffer[1'024];
  FallbackAllocator allocator{Arena::from(buffer), default_allocator};
  sformat(allocator, style_.format, state_.scalar)
    .match([&](auto & text) { input_.content(text.view().as_c8()); },
           [&](auto &) { input_.content(U"[Truncated]"_str); });
}

ScalarDragBox & ScalarDragBox::on_update(Fn<void(Scalar)> fn)
{
  cb.update = fn;
  return *this;
}

ui::State ScalarDragBox::tick(Ctx const & ctx, Events const & events,
                              Fn<void(View &)> build)
{
  state_.dragging = events.drag_update();

  // [ ] fix input

  if (events.drag_start() &&
      (ctx.key.down(KeyCode::LeftCtrl) || ctx.key.down(KeyCode::RightCtrl)))
  {
    state_.input_mode = !state_.input_mode;
  }

  if (state_.dragging && !state_.input_mode)
  {
    auto      h = events.hit_info.unwrap_or();
    f32 const t = clamp(unlerp(h.viewport_region.begin().x,
                               h.viewport_region.end().x, h.viewport_hit.x),
                        0.0F, 1.0F);
    state_.scalar =
      state_.spec.match([t](F32Info & v) -> Scalar { return v.interp(t); },
                        [t](I32Info & v) -> Scalar { return v.interp(t); });

    format_();
    cb.update(state_.scalar);
  }
  else if (input_.state_.editing)
  {
    scalar_parse(input_.content_.get_text(), state_.spec, state_.scalar);
    cb.update(state_.scalar);
  }

  input_.state_.disabled = !state_.input_mode;

  build(input_);

  return ui::State{.pointable = !state_.disabled,
                   .draggable = !state_.disabled,
                   .focusable = !state_.disabled};
}

void ScalarDragBox::size(Vec2 allocated, Span<Vec2> sizes)
{
  auto child = style_.frame(allocated) - style_.padding.axes();
  child.x    = max(child.x, 0.0F);
  child.y    = max(child.y, 0.0F);
  fill(sizes, child);
}

Layout ScalarDragBox::fit(Vec2 allocated, Span<Vec2 const> sizes,
                          Span<Vec2> centers)
{
  auto frame  = style_.frame(allocated);
  auto padded = sizes[0] + style_.padding.axes();
  frame.x     = max(frame.x, padded.x);
  frame.y     = max(frame.y, padded.y);
  fill(centers, Vec2{0, 0});

  return {.extent = frame};
}

void ScalarDragBox::render(Canvas & canvas, RenderInfo const & info)
{
  canvas.rrect({.area         = info.canvas_region,
                .corner_radii = style_.corner_radii,
                .stroke       = style_.stroke,
                .thickness    = Vec2::splat(style_.thickness),
                .tint         = style_.color,
                .clip         = info.clip});

  if (!state_.input_mode)
  {
    f32 const t = state_.spec.match(
      [&](F32Info & v) { return v.uninterp(state_.scalar[v0]); },
      [&](I32Info & v) { return v.uninterp(state_.scalar[v1]); });

    CRect const thumb_rect = CRect::from_offset(
      info.canvas_region.begin(), info.canvas_region.extent * Vec2{t, 1});

    canvas.rrect({.area         = thumb_rect,
                  .corner_radii = style_.corner_radii,
                  .tint         = style_.thumb_color,
                  .clip         = info.clip});
  }
}

Cursor ScalarDragBox::cursor(Vec2, Vec2)
{
  return state_.disabled ? Cursor::Default : Cursor::EWResize;
}

ScalarBox::ScalarBox(Str32 decrease_text, Str32 increase_text,
                     TextStyle const & button_text_style,
                     TextStyle const & drag_text_style,
                     FontStyle const & icon_font, FontStyle const & text_font,
                     AllocatorRef allocator) :
  Flex{allocator},
  dec_{decrease_text, button_text_style, icon_font, allocator},
  inc_{increase_text, button_text_style, icon_font, allocator},
  drag_{drag_text_style, text_font, allocator}
{
  Flex::axis(Axis::X)
    .wrap(false)
    .main_align(MainAlign::Start)
    .cross_align(0)
    .frame(Frame{}.rel({1, 1}));

  padding(Padding::all(5)).corner_radii(CornerRadii::all(7.5F));

  dec_.on_pressed({this, [](ScalarBox * b) { b->step(-1); }});

  inc_.on_pressed({this, [](ScalarBox * b) { b->step(1); }});

  drag_.on_update({this, +[](ScalarBox * b, Scalar in) { b->cb.update(in); }});
}

ScalarBox & ScalarBox::step(i32 direction)
{
  auto & state_ = drag_.state_;
  state_.scalar = state_.spec.match(
    [&](F32Info const & spec) -> Scalar {
      return spec.step_value(state_.scalar[v0], direction);
    },
    [&](I32Info const & spec) -> Scalar {
      return spec.step_value(state_.scalar[v1], direction);
    });
  drag_.format_();
  cb.update(state_.scalar);
  return *this;
}

ScalarBox & ScalarBox::stub(Str32 text)
{
  drag_.input_.stub(text);
  return *this;
}

ScalarBox & ScalarBox::stub(Str8 text)
{
  drag_.input_.stub(text);
  return *this;
}

ScalarBox & ScalarBox::format(Str format)
{
  drag_.style_.format = format;
  drag_.format_();
  return *this;
}

ScalarBox & ScalarBox::spec(f32 scalar, F32Info info)
{
  drag_.state_.scalar = scalar;
  drag_.state_.spec   = info;
  drag_.format_();
  return *this;
}

ScalarBox & ScalarBox::spec(i32 scalar, I32Info info)
{
  drag_.state_.scalar = scalar;
  drag_.state_.spec   = info;
  drag_.format_();
  return *this;
}

ScalarBox & ScalarBox::stroke(f32 s)
{
  drag_.style_.stroke = s;
  return *this;
}

ScalarBox & ScalarBox::thickness(f32 t)
{
  drag_.style_.thickness = t;
  return *this;
}

ScalarBox & ScalarBox::padding(Padding p)
{
  dec_.padding(p);
  inc_.padding(p);
  drag_.style_.padding = p;
  return *this;
}

ScalarBox & ScalarBox::frame(Frame f)
{
  dec_.frame(f);
  inc_.frame(f);
  drag_.style_.frame = f;
  return *this;
}

ScalarBox & ScalarBox::corner_radii(CornerRadii const & r)
{
  dec_.rrect(r);
  inc_.rrect(r);
  drag_.style_.corner_radii = r;
  return *this;
}

ScalarBox & ScalarBox::on_update(Fn<void(Scalar)> f)
{
  cb.update = f;
  return *this;
}

ScalarBox & ScalarBox::button_text_style(TextStyle const & style,
                                         FontStyle const & font, usize first,
                                         usize count)
{
  dec_.run(style, font, first, count);
  inc_.run(style, font, first, count);
  return *this;
}

ScalarBox & ScalarBox::drag_text_style(TextStyle const & style,
                                       FontStyle const & font, usize first,
                                       usize count)
{
  drag_.input_.content_run(style, font, first, count)
    .stub_run(style, font, first, count);
  return *this;
}

ui::State ScalarBox::tick(Ctx const &, Events const &, Fn<void(View &)> build)
{
  build(dec_);
  build(drag_);
  build(inc_);
  return ui::State{};
}

}    // namespace ui

}    // namespace ash
