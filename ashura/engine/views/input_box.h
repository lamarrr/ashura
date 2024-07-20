/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/engine/views/button.h"
#include "ashura/std/text.h"

namespace ash
{

enum class ScalarInputType : u8
{
  u8  = 0,
  u16 = 1,
  u32 = 2,
  i8  = 5,
  i16 = 6,
  i32 = 7,
  f32 = 10
};

/// @brief Numeric Scalar Text Input.
/// 64-bit precision is not supported.
struct ScalarInput
{
  union
  {
    u8  u8 = 0;
    u16 u16;
    u32 u32;
    i8  i8;
    i16 i16;
    i32 i32;
    f32 f32;
  };
  ScalarInputType type = ScalarInputType::u8;
};

namespace fmt
{

inline bool push(Context const &ctx, Spec const &spec, ScalarInput const &value)
{
  switch (value.type)
  {
    case ScalarInputType::u8:
      return push(ctx, spec, value.u8);
    case ScalarInputType::u16:
      return push(ctx, spec, value.u16);
    case ScalarInputType::u32:
      return push(ctx, spec, value.u32);
    case ScalarInputType::i8:
      return push(ctx, spec, value.i8);
    case ScalarInputType::i16:
      return push(ctx, spec, value.i16);
    case ScalarInputType::i32:
      return push(ctx, spec, value.i32);
    case ScalarInputType::f32:
      return push(ctx, spec, value.f32);
    default:
      return true;
  }
}

}        // namespace fmt

struct TextInput : View
{
  struct Text
  {
    Vec<u32>         text       = {};
    Vec<u32>         runs       = {};
    Vec<TextStyle>   run_styles = {};
    TextDirection    direction  = TextDirection::LeftToRight;
    Span<char const> language   = {};
    TextLayout       layout     = {};
    f32              alignment  = -1;

    void reset()
    {
      text.reset();
      runs.reset();
      run_styles.reset();
      layout.reset();
    }
  };

  bool           disabled : 1      = false;
  bool           is_multiline : 1  = false;
  bool           enter_submits : 1 = false;
  bool           tab_input : 1     = false;
  u32            lines_per_page    = 1;
  mutable Text   content           = {};
  mutable Text   placeholder       = {};
  TextCompositor compositor        = {};
  Fn<void()>     on_edit           = fn([] {});
  Fn<void()>     on_submit         = fn([] {});
  Fn<void()>     on_focus_in       = fn([] {});
  Fn<void()>     on_focus_out      = fn([] {});

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
      this->content.text.erase(s);
      edited |= s.is_empty();
    };

    auto insert = [this, &edited](u32 pos, Span<u32 const> t) {
      CHECK(this->content.text.insert_span_copy(pos, t));
      edited |= t.is_empty();
    };

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
    compositor.command(span(content.text), content.layout,
                       TextBlockStyle{.runs        = span(content.run_styles),
                                      .alignment   = content.alignment,
                                      .align_width = region.extent.x},
                       cmd, fn(&insert), fn(&erase), ctx.text, *ctx.clipboard,
                       lines_per_page, offset);

    if (edited)
    {
      on_edit();
    }

    if (events.focus_in)
    {
      on_focus_in();
    }
    else if (events.focus_out)
    {
      on_focus_out();
      compositor.unselect();
    }

    // TODO(lamarrr):
    //
    // [ ] font hashmap
    // [ ] word layout cache
    //
    //
    // https://github.com/ocornut/imgui/issues/787#issuecomment-361419796 enter
    // parent: prod children, nav to children
    // https://user-images.githubusercontent.com/8225057/74143829-ce67b900-4bfb-11ea-90d9-0de40c944b26.gif
    //

    if (enter_submits && ctx.key_down(KeyCode::Return))
    {
      on_submit();
    }

    return ViewState{.draggable  = !disabled,
                     .focusable  = !disabled,
                     .text_input = !disabled,
                     .tab_input  = tab_input,
                     .lose_focus = ctx.key_down(KeyCode::Escape)};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) const override
  {
    if (content.text.is_empty())
    {
      layout_text(TextBlock{.text          = span(placeholder.text),
                            .runs          = span(placeholder.runs),
                            .fonts         = {},
                            .direction     = placeholder.direction,
                            .language      = placeholder.language,
                            .use_kerning   = true,
                            .use_ligatures = true},
                  allocated.x, placeholder.layout);
      return placeholder.layout.extent;
    }

    layout_text(TextBlock{.text          = span(content.text),
                          .runs          = span(content.runs),
                          .fonts         = {},
                          .direction     = content.direction,
                          .language      = content.language,
                          .use_kerning   = true,
                          .use_ligatures = true},
                allocated.x, content.layout);
    return content.layout.extent;
  }

  virtual void render(CRect const &region, Canvas &canvas) const override
  {
    if (content.text.is_empty())
    {
      canvas.text({.center = region.center},
                  TextBlock{.text          = span(placeholder.text),
                            .runs          = span(placeholder.runs),
                            .fonts         = {},
                            .direction     = placeholder.direction,
                            .language      = placeholder.language,
                            .use_kerning   = true,
                            .use_ligatures = true},
                  placeholder.layout,
                  TextBlockStyle{.runs        = span(placeholder.run_styles),
                                 .alignment   = placeholder.alignment,
                                 .align_width = region.extent.x},
                  Span<FontAtlasResource const *>{});
    }
    else
    {
      canvas.text({.center = region.center},
                  TextBlock{.text          = span(content.text),
                            .runs          = span(content.runs),
                            .fonts         = {},
                            .direction     = content.direction,
                            .language      = content.language,
                            .use_kerning   = true,
                            .use_ligatures = true},
                  placeholder.layout,
                  TextBlockStyle{.runs        = span(content.run_styles),
                                 .alignment   = content.alignment,
                                 .align_width = region.extent.x},
                  Span<FontAtlasResource const *>{});
    }
  }

  virtual Cursor cursor(CRect const &, Vec2) const override
  {
    return Cursor::Text;
  }
};

struct TextInputView : View
{
  TextInput input;
  // [ ] calculate lines per page
  // [ ] viewport text with scrollable region
  //  ViewAttributes::Scrollable |
};

// DragBox: text input + dragging when alt is pressed down
struct ScalarDragBox : View
{
  Fn<void(fmt::Context &, ScalarInput)> format     = fn(default_format);
  ScalarInput                           value      = {};
  ScalarInput                           min        = {};
  ScalarInput                           max        = {};
  ScalarInput                           step       = {};
  Fn<void(ScalarInput)>                 on_changed = fn([](ScalarInput) {});
  SizeConstraint                        width      = {.offset = 100};
  SizeConstraint                        height     = {.offset = 20};
  bool                                  disabled   = false;

  static void default_format(fmt::Context &ctx, ScalarInput v)
  {
    fmt::format(ctx, v);
  }

  virtual ViewState tick(ViewContext const &ctx, CRect const &region,
                         ViewEvents events) override
  {
    // if focused, read text input and parse
    // if dragging, update
    // if clicked, move to position
    // should views handle this? no! systems.
    // mouse enter + mouse down => focuse enter, mouse click elsewhere (focus
    // lost), navigation event
    return ViewState{.clickable = true, .draggable = true};
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) const override
  {
    return Vec2{width(allocated.x), height(allocated.y)};
  }

  virtual void render(CRect const &region, Canvas &canvas) const override
  {
    (void) region;
    (void) canvas;
  }
};

/// @brief
/// REQUIREMENTS:
/// Custom Scaling & Custom Stepping: i.e. log, linear, Angular Input
/// Drag-Based Input
/// Text-Field Input of exact values
/// Generic Numeric Input: Scalars, Vectors, Matrices, Tensors
struct ScalarBox : View
{
  Fn<void(ScalarInput &, ScalarInput, ScalarInput, ScalarInput, bool)> step =
      fn(default_step);
  bool steppable = false;
  bool draggable = false;
  bool disabled  = false;

  TextButton            negative_stepper;
  TextButton            positive_stepper;
  ScalarDragBox         dragger;
  Fn<void(ScalarInput)> on_changed = fn([](ScalarInput) {});

  ScalarBox()
  {
    negative_stepper.text.block.text = utf(U"-"_span);
    positive_stepper.text.block.text = utf(U"+"_span);

    positive_stepper.on_clicked = fn(this, [](ScalarBox *b) {
      b->step(b->dragger.value, b->dragger.min, b->dragger.max, b->dragger.step,
              true);
      b->dragger.on_changed(b->dragger.value);
    });

    negative_stepper.on_clicked = fn(this, [](ScalarBox *b) {
      b->step(b->dragger.value, b->dragger.min, b->dragger.max, b->dragger.step,
              false);
      b->dragger.on_changed(b->dragger.value);
    });

    dragger.on_changed =
        fn(this, [](ScalarBox *b, ScalarInput value) { b->on_changed(value); });
  }

  static void default_step(ScalarInput &v, ScalarInput min, ScalarInput max,
                           ScalarInput step, bool direction)
  {
    switch (v.type)
    {
      case ScalarInputType::u8:
        v.u8 = (u8) ash::clamp((i64) v.u32 +
                                   (i64) (direction ? step.u32 : -step.u32),
                               (i64) min.u32, (i64) max.u32);
        return;
      case ScalarInputType::u16:
        v.u16 = (u16) ash::clamp((i64) v.u32 +
                                     (i64) (direction ? step.u32 : -step.u32),
                                 (i64) min.u32, (i64) max.u32);
        return;
      case ScalarInputType::u32:
        v.u32 = (u32) ash::clamp((i64) v.u32 +
                                     (i64) (direction ? step.u32 : -step.u32),
                                 (i64) min.u32, (i64) max.u32);
        return;
      case ScalarInputType::i8:
        v.i8 = (i8) ash::clamp((i64) v.i32 +
                                   (i64) (direction ? step.i32 : -step.i32),
                               (i64) min.i32, (i64) max.i32);
        return;
      case ScalarInputType::i16:
        v.i16 = (i16) ash::clamp((i64) v.i32 +
                                     (i64) (direction ? step.i32 : -step.i32),
                                 (i64) min.i32, (i64) max.i32);
        return;
      case ScalarInputType::i32:
        v.i32 = (i32) ash::clamp((i64) v.i32 +
                                     (i64) (direction ? step.i32 : -step.i32),
                                 (i64) min.i32, (i64) max.i32);
        return;
      case ScalarInputType::f32:
        v.f32 = (f32) ash::clamp((f64) v.f32 +
                                     (f64) (direction ? step.f32 : -step.f32),
                                 (f64) min.f32, (f64) max.f32);
        return;
      default:
        return;
    }
  }
};

struct VectorInputBox : View
{
  ScalarBox scalars[8];
  u32       num = 0;
};

struct MatrixInputBox : View
{
  VectorInputBox vectors[8];
  u32            num_rows    = 0;
  u32            num_columns = 0;
};

}        // namespace ash
