/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/input.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

Input::Input(Str32 s, TextStyle const & style, FontStyle const & font,
             AllocatorRef allocator) :
  allocator_{allocator},
  content_{allocator},
  stub_{allocator},
  compositor_{TextCompositor::create(allocator)}
{
  content(U""_str).content_run(style, font).stub(s).stub_run(style, font);
}

Input::Input(Str8 s, TextStyle const & style, FontStyle const & font,
             AllocatorRef allocator) :
  allocator_{allocator},
  content_{allocator},
  stub_{allocator},
  compositor_{TextCompositor::create(allocator)}
{
  content(U""_str).content_run(style, font).stub(s).stub_run(style, font);
}

Input & Input::disable(bool disable)
{
  state_.disabled = disable;
  return *this;
}

Input & Input::multiline(bool e)
{
  state_.multiline = e;
  return *this;
}

Input & Input::enter_submits(bool e)
{
  state_.enter_submits = e;
  return *this;
}

Input & Input::tab_input(bool e)
{
  state_.tab_input = e;
  return *this;
}

Input & Input::on_edit(Fn<void()> f)
{
  cb.edit = f;
  return *this;
}

Input & Input::on_submit(Fn<void()> f)
{
  cb.submit = f;
  return *this;
}

Input & Input::on_focus_in(Fn<void()> f)
{
  cb.focus_in = f;
  return *this;
}

Input & Input::on_focus_out(Fn<void()> f)
{
  cb.focus_out = f;
  return *this;
}

Input & Input::content(Str8 t)
{
  content_.text(t);
  return *this;
}

Input & Input::content(Str32 t)
{
  content_.text(t);
  return *this;
}

Input & Input::content_run(TextStyle const & style, FontStyle const & font,
                           usize first, usize count)
{
  content_.run(style, font, first, count);
  return *this;
}

Input & Input::stub(Str8 t)
{
  stub_.text(t);
  return *this;
}

Input & Input::stub(Str32 t)
{
  stub_.text(t);
  return *this;
}

Input & Input::stub_run(TextStyle const & style, FontStyle const & font,
                        usize first, usize count)
{
  stub_.run(style, font, first, count);
  return *this;
}

ui::State Input::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  bool edited = false;

  state_.editing = false;
  state_.submit  = false;

  u8 buffer[512];

  FallbackAllocator allocator{Arena::from(buffer), allocator_};

  Vec<c32> input_u32{allocator};

  if (events.text_input())
  {
    utf8_decode(ctx.key.text, input_u32).unwrap();
  }

  TextCommand cmd = text_command(ctx, events,
                                 TextCfg{.multiline_input = state_.multiline,
                                         .enter_submits = state_.enter_submits,
                                         .tab_input     = state_.tab_input,
                                         .copyable      = true,
                                         .editable      = true,
                                         .highlightable = true});

  auto hit_info = events.hit_info.map([](auto s) { return s; }).unwrap_or();

  compositor_.command(content_, cmd, input_u32, engine->clipboard,
                      style_.lines_per_page, style_.tab_width,
                      hit_info.viewport_region.center,
                      hit_info.viewport_region.extent.x, hit_info.canvas_hit,
                      transform2d_to_3d(hit_info.canvas_transform), allocator);

  auto cursor = compositor_.cursor();

  content_.clear_highlights()
    .clear_carets()
    .add_highlight(cursor.selection())
    .highlight_style(style_.highlight)
    .caret_style(style_.caret);

  if (events.focus_over())
  {
    content_.add_caret(cursor.caret());
  }

  if (edited)
  {
    state_.editing = true;
  }

  if (cmd == TextCommand::Submit)
  {
    state_.submit = true;
    cb.submit();
  }

  if (events.focus_in())
  {
    cb.focus_in();
  }

  if (events.focus_out())
  {
    cb.focus_out();
  }

  if (edited)
  {
    cb.edit();
  }

  return ui::State{
    .text       = TextInputInfo{.multiline = state_.multiline,
                                .tab_input = state_.tab_input},
    .draggable  = !state_.disabled,
    .focusable  = !state_.disabled,
    .grab_focus = events.drag_start()
  };
}

Layout Input::fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
  content_.layout(allocated.x);
  stub_.layout(allocated.x);
  if (content_.text_.is_empty())
  {
    return {.extent = stub_.layout_.extent};
  }
  return {.extent = content_.layout_.extent};
}

void Input::render(Canvas & canvas, RenderInfo const & info)
{
  if (content_.text_.is_empty())
  {
    // [ ] placeholder overlay when empty. use child view instead
    stub_.render(canvas.text_renderer(), info.viewport_region.center,
                 info.viewport_region.extent.x,
                 transform2d_to_3d(info.canvas_transform), info.clip);
  }
  else
  {
    // [ ] need to draw caret even if line is empty; SET placeholder caret to 0;  use place holder when focused
    content_.render(canvas.text_renderer(), info.viewport_region.center,
                    info.viewport_region.extent.x,
                    transform2d_to_3d(info.canvas_transform), info.clip);
  }
}

Cursor Input::cursor(f32x2, f32x2)
{
  return Cursor::Text;
}

}    // namespace ui

}    // namespace ash
