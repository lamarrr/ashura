/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/engine.h"
#include "ashura/engine/render_text.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/std/text.h"
#include "ashura/std/types.h"

namespace ash
{

struct TextBox : View, Pin<>
{
  bool           copyable : 1           = false;
  Vec4           highlight_color        = {};
  Vec4           highlight_corner_radii = {0, 0, 0, 0};
  RenderText     text                   = {};
  TextCompositor compositor             = {};

  TextBox()
  {
    text.style(
        0, U32_MAX,
        TextStyle{.foreground = ColorGradient::all(DEFAULT_THEME.on_surface)},
        FontStyle{.font        = engine->default_font,
                  .font_height = DEFAULT_THEME.body_font_height,
                  .line_height = DEFAULT_THEME.line_height});
  }

  virtual ~TextBox() override
  {
    text.reset();
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    TextCommand cmd = TextCommand::None;
    if (events.drag_start)
    {
      cmd = TextCommand::Hit;
    }
    else if (events.dragging)
    {
      cmd = TextCommand::HitSelect;
    }

    compositor.command(text.get_text(), text.inner.layout, region.extent.x,
                       text.inner.alignment, cmd,
                       fn([](u32, Span<u32 const>) {}), fn([](Slice32) {}), {},
                       *ctx.clipboard, 1, ctx.mouse_position);
    text.set_highlight(
        compositor.get_cursor().as_slice(text.get_text().size32()));
    text.set_highlight_style(ColorGradient::all(highlight_color),
                             highlight_corner_radii);

    return ViewState{.draggable = copyable};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    text.calculate_layout(allocated.x);
    return text.inner.layout.extent;
  }

  virtual void render(CRect const &region, CRect const &clip,
                      Canvas &canvas) override
  {
    text.render(region, clip, canvas);
  }

  virtual Cursor cursor(CRect const &, Vec2) override
  {
    return copyable ? Cursor::Text : Cursor::Default;
  }
};

struct TextInput : View, Pin<>
{
  bool           disabled : 1           = false;
  bool           editing : 1            = false;
  bool           submit : 1             = false;
  bool           focus_in : 1           = false;
  bool           focus_out : 1          = false;
  bool           focused : 1            = false;
  bool           is_multiline : 1       = false;
  bool           enter_submits : 1      = false;
  bool           tab_input : 1          = false;
  Vec4           highlight_color        = {};
  Vec4           highlight_corner_radii = {0, 0, 0, 0};
  u32            lines_per_page         = 1;
  RenderText     content                = {};
  RenderText     placeholder            = {};
  TextCompositor compositor             = {};
  Fn<void()>     on_edit                = fn([] {});
  Fn<void()>     on_submit              = fn([] {});
  Fn<void()>     on_focus_in            = fn([] {});
  Fn<void()>     on_focus_out           = fn([] {});

  TextInput()
  {
    content.style(
        0, U32_MAX,
        TextStyle{.foreground = ColorGradient::all(DEFAULT_THEME.on_surface)},
        FontStyle{.font        = engine->default_font,
                  .font_height = DEFAULT_THEME.body_font_height,
                  .line_height = DEFAULT_THEME.line_height});
    placeholder.style(
        0, U32_MAX,
        TextStyle{.foreground = ColorGradient::all(DEFAULT_THEME.on_surface)},
        FontStyle{.font        = engine->default_font,
                  .font_height = DEFAULT_THEME.body_font_height,
                  .line_height = DEFAULT_THEME.line_height});
  }

  virtual ~TextInput() override
  {
    content.reset();
    placeholder.reset();
  }

  constexpr TextCommand command(ViewContext const &ctx) const
  {
    if (ctx.key_down(KeyCode::Escape))
    {
      return TextCommand::Unselect;
    }
    if (ctx.key_down(KeyCode::Backspace))
    {
      return TextCommand::BackSpace;
    }
    if (ctx.key_down(KeyCode::Delete))
    {
      return TextCommand::Delete;
    }
    if (ctx.key_down(KeyCode::Left))
    {
      return TextCommand::Left;
    }
    if (ctx.key_down(KeyCode::Right))
    {
      return TextCommand::Right;
    }
    if (ctx.key_down(KeyCode::Home))
    {
      return TextCommand::LineStart;
    }
    if (ctx.key_down(KeyCode::End))
    {
      return TextCommand::LineEnd;
    }
    if (ctx.key_down(KeyCode::Up))
    {
      return TextCommand::Up;
    }
    if (ctx.key_down(KeyCode::Down))
    {
      return TextCommand::Down;
    }
    if (ctx.key_down(KeyCode::PageUp))
    {
      return TextCommand::PageUp;
    }
    if (ctx.key_down(KeyCode::PageDown))
    {
      return TextCommand::PageDown;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::Left))
    {
      return TextCommand::SelectLeft;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::Right))
    {
      return TextCommand::SelectRight;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::Up))
    {
      return TextCommand::SelectUp;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::Down))
    {
      return TextCommand::SelectDown;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::PageUp))
    {
      return TextCommand::SelectPageUp;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::PageDown))
    {
      return TextCommand::SelectPageDown;
    }
    if ((ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)) &&
        ctx.key_down(KeyCode::A))
    {
      return TextCommand::SelectAll;
    }
    if ((ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)) &&
        ctx.key_down(KeyCode::X))
    {
      return TextCommand::Cut;
    }
    if ((ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)) &&
        ctx.key_down(KeyCode::C))
    {
      return TextCommand::Copy;
    }
    if ((ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)) &&
        ctx.key_down(KeyCode::V))
    {
      return TextCommand::Paste;
    }
    if ((ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)) &&
        ctx.key_down(KeyCode::Z))
    {
      return TextCommand::Undo;
    }
    if ((ctx.key_down(KeyCode::LCtrl) || ctx.key_down(KeyCode::RCtrl)) &&
        ctx.key_down(KeyCode::Y))
    {
      return TextCommand::Redo;
    }
    if ((ctx.key_down(KeyCode::LShift) || ctx.key_down(KeyCode::RShift)) &&
        ctx.key_down(KeyCode::Left) &&
        has_bits(ctx.mouse_buttons, MouseButtons::Primary))
    {
      return TextCommand::HitSelect;
    }
    if (is_multiline && !enter_submits && ctx.key_down(KeyCode::Return))
    {
      return TextCommand::NewLine;
    }
    if (tab_input && ctx.key_down(KeyCode::Tab))
    {
      return TextCommand::Tab;
    }
    return TextCommand::None;
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    bool edited = false;
    auto erase  = [this, &edited](Slice32 s) {
      this->content.inner.text.erase(s);
      edited |= s.is_empty();
      this->content.flush_text();
    };

    auto insert = [this, &edited](u32 pos, Span<u32 const> t) {
      CHECK(this->content.inner.text.insert_span_copy(pos, t));
      edited |= t.is_empty();
      this->content.flush_text();
    };

    editing   = false;
    submit    = false;
    focus_in  = events.focus_in;
    focus_out = events.focus_out;

    if (events.focus_in)
    {
      focused = true;
    }

    if (events.focus_out)
    {
      focused = false;
    }

    TextCommand cmd = TextCommand::None;
    if (events.text_input)
    {
      cmd = TextCommand::InputText;
    }
    else if (events.key_down)
    {
      cmd = command(ctx);
    }
    else if (events.drag_start)
    {
      cmd = TextCommand::Hit;
    }
    else if (events.dragging)
    {
      cmd = TextCommand::HitSelect;
    }

    compositor.command(span(content.inner.text), content.inner.layout,
                       region.extent.x, content.inner.alignment, cmd,
                       fn(&insert), fn(&erase), ctx.text, *ctx.clipboard,
                       lines_per_page, ctx.mouse_position - region.begin());

    content.set_highlight(
        compositor.get_cursor().as_slice(content.inner.text.size32()));
    content.set_highlight_style(ColorGradient::all(highlight_color),
                                highlight_corner_radii);

    if (edited)
    {
      editing = true;
    }

    if (events.focus_out)
    {
      compositor.unselect();
    }

    if (events.key_down && enter_submits && ctx.key_down(KeyCode::Return))
    {
      submit = true;
    }

    if (focus_in)
    {
      on_focus_in();
    }

    if (focus_out)
    {
      on_focus_out();
    }

    if (submit)
    {
      on_submit();
    }

    if (edited)
    {
      on_edit();
    }

    return ViewState{.draggable  = !disabled,
                     .focusable  = !disabled,
                     .text_input = !disabled,
                     .tab_input  = tab_input,
                     .lose_focus = ctx.key_down(KeyCode::Escape)};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    placeholder.calculate_layout(allocated.x);
    content.calculate_layout(allocated.x);
    if (content.inner.text.is_empty())
    {
      return placeholder.inner.layout.extent;
    }
    return content.inner.layout.extent;
  }

  virtual void render(CRect const &region, CRect const &clip,
                      Canvas &canvas) override
  {
    if (content.inner.text.is_empty())
    {
      placeholder.render(region, clip, canvas);
    }
    else
    {
      content.render(region, clip, canvas);
    }
  }

  virtual Cursor cursor(CRect const &, Vec2) override
  {
    return Cursor::Text;
  }
};

// struct ScrollableTextInput : ScrollBox, Pin<>
// {
//   TextInput input;
//   ScrollableTextInput()          = default;
//   virtual ~ScrollableTextInput() = default;
//   // [ ] calculate lines per page
//   // [ ] viewport text with scrollable region, scroll direction
//   // [ ] text input while in view, i.e. page down
// };

}        // namespace ash