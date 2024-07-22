/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/std/text.h"
#include "ashura/std/types.h"

namespace ash
{

// TODO(lamarrr): selection for copy and paste, copyable attribute

// [ ] font hashmap and loader
//

/// @brief Controls and manages GUI text state for rendering
/// - manages runs and run styling
/// - manages and checks for text layout invalidation
/// - recalculate text layout when it changes if necessary
/// - renders the text using the computed style information
struct RenderText
{
  struct
  {
    bool             dirty     = true;
    Vec<u32>         text      = {};
    Vec<u32>         runs      = {};
    Vec<TextStyle>   styles    = {};
    Vec<FontStyle>   fonts     = {};
    TextDirection    direction = TextDirection::LeftToRight;
    Span<char const> language  = {};
    f32              alignment = -1;
    TextLayout       layout    = {};

    /// @brief  Styles specified runs of text, performing run merging and
    /// splitting in the process. If there's previously no runs, the first added
    /// run will be the default and span the whole of the text.
    /// @param first first codepoint index to be patched
    /// @param count range of the number of codepoints to be patched
    /// @param style font style to be applied
    /// @param font font configuration to be applied
    void style(u32 first, u32 count, TextStyle const &style,
               FontStyle const &font)
    {
      if (count == 0)
      {
        return;
      }

      if (runs.is_empty())
      {
        runs.push(U32_MAX).unwrap();
        styles.push(style);
        fonts.push(font);
        dirty = true;
        return;
      }

      // TODO(lamarrr): clamp first

      u32 const end = (count == U32_MAX) ? U32_MAX : (first + count);

      // if this becomes a bottleneck, might be improvable by using a binary
      // search. although more complex.
      u32 first_run       = 0;
      u32 first_run_begin = 0;
      for (; first_run < runs.size32(); first_run++)
      {
        if (first_run_begin >= first)
        {
          break;
        }
        first_run_begin = runs[first_run];
      }

      u32 last_run     = first_run;
      u32 last_run_end = 0;

      for (; last_run < runs.size32(); last_run++)
      {
        last_run_end = runs[last_run];
        if (last_run_end >= end)
        {
          break;
        }
      }

      /// should really never happen since there's always a U32_MAX run end
      CHECK(first_run < runs.size32());
      CHECK(last_run < runs.size32());

      /// delete middle runs
      if (last_run > (first_run + 1))
      {
        u32       first_erase = first_run + 1;
        u32 const num_erase   = (last_run - 1) - (first_run + 1);
        runs.erase(first_erase, num_erase);
        styles.erase(first_erase, num_erase);
        fonts.erase(first_erase, num_erase);
        last_run -= num_erase;
      }

      /// run splitting

      if (last_run == first_run)
      {
        if (first_run_begin == first && last_run_end == end)
        {
          // already same end and begin, replace styles
          styles[first_run] = style;
          fonts[first_run]  = font;
        }
        else if (first_run_begin == first)
        {
          // split with new on left
          runs.insert(first_run, end).unwrap();
          styles.insert(first_run, style).unwrap();
          fonts.insert(first_run, font).unwrap();
        }
        else if (last_run_end == end)
        {
          // split with new on right
          runs.insert(first_run, first).unwrap();
          styles.insert(first_run + 1, style).unwrap();
          fonts.insert(first_run + 1, font).unwrap();
        }
        else
        {
          // split with new in the middle of the run
          runs[first_run] = first;
          runs.insert_span_copy(first_run, span({end, last_run_end})).unwrap();
          styles
              .insert_span_copy(first_run + 1, span({style, styles[first_run]}))
              .unwrap();
          fonts.insert_span_copy(first_run + 1, span({font, fonts[first_run]}))
              .unwrap();
        }
      }
      else
      {
        // split left and right
        runs[first_run] = first;
        runs.insert(first_run + 1, end);
        styles.insert(first_run + 1, style);
        fonts.insert(first_run + 1, font);
      }

      dirty = true;
    }
  } inner = {};

  void flush_text()
  {
    inner.dirty = true;
  }

  void set_direction(TextDirection direction)
  {
    inner.direction = direction;
    flush_text();
  }

  void set_language(Span<char const> language)
  {
    inner.language = language;
    flush_text();
  }

  void set_alignment(f32 alignment)
  {
    inner.alignment = alignment;
    flush_text();
  }

  void set_text(Span<u32 const> utf32, TextStyle const &style,
                FontStyle const &font)
  {
    inner.style(0, U32_MAX, style, font);
    inner.text.clear();
    inner.text.extend_copy(utf32).unwrap();
    flush_text();
  }

  void set_text_utf8(Span<u8 const> utf8, TextStyle const &style,
                     FontStyle const &font)
  {
    inner.style(0, U32_MAX, style, font);
    inner.text.clear();
    utf8_decode(utf8, inner.text);
    flush_text();
  }

  void style(u32 first, u32 count, TextStyle const &style,
             FontStyle const &font)
  {
    inner.style(first, count, style, font);
    flush_text();
  }

  void calculate_layout(f32 max_width)
  {
    if (!inner.dirty && max_width == inner.layout.extent.x)
    {
      return;
    }
    layout_text(TextBlock{.text          = span(inner.text),
                          .runs          = span(inner.runs),
                          .fonts         = span(inner.fonts),
                          .direction     = inner.direction,
                          .language      = inner.language,
                          .use_kerning   = true,
                          .use_ligatures = true},
                max_width, inner.layout);
  }

  void render(CRect const &region, Canvas &canvas) const
  {
    canvas.text({.center = region.center},
                TextBlock{.text          = span(inner.text),
                          .runs          = span(inner.runs),
                          .fonts         = span(inner.fonts),
                          .direction     = inner.direction,
                          .language      = inner.language,
                          .use_kerning   = true,
                          .use_ligatures = true},
                inner.layout,
                TextBlockStyle{.runs        = span(inner.styles),
                               .alignment   = inner.alignment,
                               .align_width = region.extent.x});
  }

  void reset()
  {
    inner.text.reset();
    inner.runs.reset();
    inner.styles.reset();
    inner.fonts.reset();
    inner.layout.reset();
  }
};

struct TextBoxCore : View, Pin<void>
{
  bool       copyable : 1 = false;
  RenderText text         = {};
};

struct TextBox : TextBoxCore, Pin<void>
{
  // [ ] set highlight color, set run properties, etc.
};

struct TextInputCore : View, Pin<void>
{
  bool               disabled : 1      = false;
  bool               editing : 1       = false;
  bool               submit : 1        = false;
  bool               focus_in : 1      = false;
  bool               focus_out : 1     = false;
  bool               is_multiline : 1  = false;
  bool               enter_submits : 1 = false;
  bool               tab_input : 1     = false;
  u32                lines_per_page    = 1;
  mutable RenderText content           = {};
  mutable RenderText placeholder       = {};
  TextCompositor     compositor        = {};
  Fn<void()>         on_edit           = fn([] {});
  Fn<void()>         on_submit         = fn([] {});
  Fn<void()>         on_focus_in       = fn([] {});
  Fn<void()>         on_focus_out      = fn([] {});

  TextInputCore()                   = default;
  virtual ~TextInputCore() override = default;

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

  void flush_text()
  {
    compositor.reset();
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
    focus_in  = false;
    focus_out = false;

    TextCommand cmd = TextCommand::None;
    if (!disabled)
    {
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
      else if (events.drag_update)
      {
        cmd = TextCommand::HitSelect;
      }
    }

    Vec2 offset = region.begin() - ctx.mouse_position;
    compositor.command(span(content.inner.text), content.inner.layout,
                       TextBlockStyle{.runs        = span(content.inner.styles),
                                      .alignment   = content.inner.alignment,
                                      .align_width = region.extent.x},
                       cmd, fn(&insert), fn(&erase), ctx.text, *ctx.clipboard,
                       lines_per_page, offset);

    if (edited)
    {
      editing = true;
    }

    if (events.focus_in)
    {
      focus_in = true;
    }
    else if (events.focus_out)
    {
      compositor.unselect();
      focus_out = true;
    }

    if (enter_submits && ctx.key_down(KeyCode::Return))
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

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) const override
  {
    placeholder.calculate_layout(allocated.x);
    content.calculate_layout(allocated.x);
    if (content.inner.text.is_empty())
    {
      return placeholder.inner.layout.extent;
    }
    return content.inner.layout.extent;
  }

  virtual void render(CRect const &region, Canvas &canvas) const override
  {
    if (content.inner.text.is_empty())
    {
      placeholder.render(region, canvas);
    }
    else
    {
      content.render(region, canvas);
    }
  }

  virtual Cursor cursor(CRect const &, Vec2) const override
  {
    return Cursor::Text;
  }
};

struct TextInput : TextInputCore, Pin<void>
{
  // use strings for fonts and its atlases
};

}        // namespace ash