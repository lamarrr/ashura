/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/input.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

/*
    struct TextCfg
    {
      bool multiline_input : 1 = false;
      bool enter_submits   : 1 = false;
      bool tab_input       : 1 = false;
      bool copyable        : 1 = false;
      bool editable        : 1 = false;
      bool highlightable   : 1 = false;
    };

    TextCommand text_command(Scope const & scope, Events const & events,
                             TextCfg const & cfg)
    {
      if (events.focus_out())
      {
        return TextCommand::Escape;
      }

      if (cfg.editable && events.text_input())
      {
        return TextCommand::InputText;
      }

      auto const shift = ctx.key.held(KeyModifiers::LeftShift) ||
                         ctx.key.held(KeyModifiers::RightShift);
      auto const ctrl = ctx.key.held(KeyModifiers::LeftCtrl) ||
                        ctx.key.held(KeyModifiers::RightCtrl);

      if (events.key_down())
      {
        if (cfg.highlightable)
        {
          if (shift && ctx.key.down(KeyCode::Left))
          {
            return TextCommand::SelectLeft;
          }

          if (shift && ctx.key.down(KeyCode::Right))
          {
            return TextCommand::SelectRight;
          }

          if (shift && ctx.key.down(KeyCode::Up))
          {
            return TextCommand::SelectUp;
          }

          if (shift && ctx.key.down(KeyCode::Down))
          {
            return TextCommand::SelectDown;
          }

          if (shift && ctx.key.down(KeyCode::Home))
          {
            return TextCommand::SelectToLineStart;
          }

          if (shift && ctx.key.down(KeyCode::End))
          {
            return TextCommand::SelectToLineEnd;
          }

          if (shift && ctx.key.down(KeyCode::PageUp))
          {
            return TextCommand::SelectPageUp;
          }

          if (shift && ctx.key.down(KeyCode::PageDown))
          {
            return TextCommand::SelectPageDown;
          }

          if (ctrl && ctx.key.down(KeyCode::A))
          {
            return TextCommand::SelectAll;
          }

          if (ctx.key.down(KeyCode::Escape))
          {
            return TextCommand::Unselect;
          }
        }

        if (cfg.editable)
        {
          if (ctrl && ctx.key.down(KeyCode::X))
          {
            return TextCommand::Cut;
          }

          if (cfg.copyable && ctrl && ctx.key.down(KeyCode::C))
          {
            return TextCommand::Copy;
          }

          if (ctrl && ctx.key.down(KeyCode::V))
          {
            return TextCommand::Paste;
          }

          if (ctrl && ctx.key.down(KeyCode::Z))
          {
            return TextCommand::Undo;
          }

          if (ctrl && ctx.key.down(KeyCode::Y))
          {
            return TextCommand::Redo;
          }

          if (cfg.multiline_input && !cfg.enter_submits &&
              ctx.key.down(KeyCode::Return))
          {
            return TextCommand::NewLine;
          }

          if (cfg.tab_input && ctx.key.down(KeyCode::Tab))
          {
            return TextCommand::Tab;
          }

          if (ctx.key.down(KeyCode::Backspace))
          {
            return TextCommand::BackSpace;
          }

          if (ctx.key.down(KeyCode::Delete))
          {
            return TextCommand::Delete;
          }

          if (ctx.key.down(KeyCode::Left))
          {
            return TextCommand::Left;
          }

          if (ctx.key.down(KeyCode::Right))
          {
            return TextCommand::Right;
          }

          if (ctx.key.down(KeyCode::Home))
          {
            return TextCommand::LineStart;
          }

          if (ctx.key.down(KeyCode::End))
          {
            return TextCommand::LineEnd;
          }

          if (ctx.key.down(KeyCode::Up))
          {
            return TextCommand::Up;
          }

          if (ctx.key.down(KeyCode::Down))
          {
            return TextCommand::Down;
          }

          if (ctx.key.down(KeyCode::PageUp))
          {
            return TextCommand::PageUp;
          }

          if (ctx.key.down(KeyCode::PageDown))
          {
            return TextCommand::PageDown;
          }
        }

        if (cfg.enter_submits && ctx.key.down(KeyCode::Return))
        {
          return TextCommand::Submit;
        }
      }

      if (cfg.highlightable)
      {
        if (events.drag_update())
        {
          if (ctx.mouse.clicks(MouseButton::Primary) == 2)
          {
            return TextCommand::SelectWord;
          }

          if (ctx.mouse.clicks(MouseButton::Primary) == 3)
          {
            return TextCommand::SelectLine;
          }

          if (ctx.mouse.clicks(MouseButton::Primary) == 4)
          {
            return TextCommand::SelectAll;
          }
        }

        if (events.drag_start())
        {
          return TextCommand::Hit;
        }

        if (events.drag_update())
        {
          return TextCommand::HitSelect;
        }

        // TODO: unselect logic; also needs to unfocus when focus out
      }

      return TextCommand::None;
    }
    */

Input::Input(Str32 s, TextStyle const & style, FontStyle const & font,
             Allocator allocator) :
  allocator_{allocator},
  content_{allocator},
  stub_{allocator},
  compositor_{TextCompositor::create(allocator)}
{
    content(U""_s).content_run(style, font).stub(s).stub_run(style, font);
}

Input::Input(Str8 s, TextStyle const & style, FontStyle const & font,
             Allocator allocator) :
  allocator_{allocator},
  content_{allocator},
  stub_{allocator},
  compositor_{TextCompositor::create(allocator)}
{
    content(U""_s).content_run(style, font).stub(s).stub_run(style, font);
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

Input & Input::content_run(TextStyle const & style, FontStyle const & font, usize first,
                           usize count)
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

Input & Input::stub_run(TextStyle const & style, FontStyle const & font, usize first,
                        usize count)
{
    stub_.run(style, font, first, count);
    return *this;
}

ui::State Input::tick(Scope const & scope, Events const & events, Fn<void(View &)>)
{
    bool edited = false;

    state_.editing = false;
    state_.submit  = false;

    u8 buffer[512];

    Vec<c32> input_u32{allocator};

    if (events.text_input())
    {
        utf8_decode(ctx.key.text, input_u32).unwrap();
    }

    TextCommand cmd = text_command(ctx, events,
                                   TextCfg{.multiline_input = state_.multiline,
                                           .enter_submits   = state_.enter_submits,
                                           .tab_input       = state_.tab_input,
                                           .copyable        = true,
                                           .editable        = true,
                                           .highlightable   = true});

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
      .text =
        TextInputInfo{.multiline = state_.multiline, .tab_input = state_.tab_input},
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
        // TODO: placeholder overlay when empty. use child view instead
        stub_.render(canvas.text_renderer(), info.viewport_region.center,
                     info.viewport_region.extent.x,
                     transform2d_to_3d(info.canvas_transform), info.clip);
    }
    else
    {
        // TODO: need to draw caret even if line is empty; SET placeholder caret to 0;
        // use place holder when focused
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
