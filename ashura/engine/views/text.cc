/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/text.h"
#include "ashura/engine/engine.h"

namespace ash
{

namespace ui
{

struct TextCfg
{
  bool multiline_input : 1 = false;
  bool enter_submits   : 1 = false;
  bool tab_input       : 1 = false;
  bool copyable        : 1 = false;
  bool editable        : 1 = false;
  bool highlightable   : 1 = false;
};

TextCommand text_command(Ctx const & ctx, Events const & events,
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

    // [ ] unselect logic; also needs to unfocus when focus out
  }

  return TextCommand::None;
}

Text::Text(Str32 t, TextStyle const & style, FontStyle const & font,
           AllocatorRef allocator) :
  text_{allocator},
  compositor_{TextCompositor::create(allocator)}
{
  text(t).run(style, font);
}

Text::Text(Str8 t, TextStyle const & style, FontStyle const & font,
           AllocatorRef allocator) :
  text_{allocator},
  compositor_{TextCompositor::create(allocator)}
{
  text(t).run(style, font);
}

Text & Text::copyable(bool allow)
{
  state_.copyable = allow;
  return *this;
}

Text & Text::highlight_style(TextHighlightStyle highlight)
{
  style_.highlight = highlight;
  return *this;
}

Text & Text::run(TextStyle const & style, FontStyle const & font, usize first,
                 usize count)
{
  text_.run(style, font, first, count);
  return *this;
}

Text & Text::text(Str32 t)
{
  text_.text(t);
  return *this;
}

Text & Text::text(Str8 t)
{
  text_.text(t);
  return *this;
}

Str32 Text::text() const
{
  return text_.get_text();
}

ui::State Text::tick(Ctx const & ctx, Events const & events, Fn<void(View &)>)
{
  TextCommand cmd = text_command(ctx, events,
                                 TextCfg{.multiline_input = false,
                                         .enter_submits   = false,
                                         .tab_input       = false,
                                         .copyable        = state_.copyable,
                                         .editable        = false,
                                         .highlightable   = state_.copyable});

  auto hit_info = events.hit_info.map([](auto s) { return s; }).unwrap_or();

  compositor_.command(
    text_, cmd, {}, engine->clipboard, 1, 1, hit_info.viewport_region.center,
    hit_info.viewport_region.extent.x, hit_info.canvas_hit,
    transform2d_to_3d(hit_info.canvas_transform), default_allocator);

  // [ ] copyable for input
  text_.clear_highlights()
    .add_highlight(compositor_.cursor().selection())
    .highlight_style(style_.highlight);

  return ui::State{.draggable = state_.copyable};
}

Layout Text::fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
  text_.layout(allocated.x);
  return {.extent = text_.layout_.extent};
}

void Text::render(Canvas & canvas, RenderInfo const & info)
{
  text_.render(canvas.text_renderer(), info.viewport_region.center,
               info.viewport_region.extent.x,
               transform2d_to_3d(info.canvas_transform), info.clip);
}

Cursor Text::cursor(f32x2, f32x2)
{
  return state_.copyable ? Cursor::Text : Cursor::Default;
}

}    // namespace ui

}    // namespace ash
