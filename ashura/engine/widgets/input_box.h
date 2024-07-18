/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/widget.h"
#include "ashura/engine/widgets/button.h"
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

struct TextInput : Widget
{
  bool            disabled            = false;
  bool            is_multiline        = false;
  bool            is_submittable      = false;
  Vec<u32>        text                = {};
  TextLayout      layout              = {};
  TextBlockStyle  style               = {};
  Span<u32 const> placeholder_text    = {};
  TextBlockStyle  placeholder_style   = {};
  TextCompositor  compositor          = {};
  Fn<void()>      on_editing          = fn([] {});
  Fn<void()>      on_editing_finished = fn([] {});
  Fn<void()>      on_submit           = fn([] {});

  virtual WidgetAttributes attributes() override
  {
    WidgetAttributes attributes =
        WidgetAttributes::Visible | WidgetAttributes::TextArea;

    if (!disabled)
    {
      attributes |= WidgetAttributes::Focusable | WidgetAttributes::Draggable;
    }

    return attributes;
  }

  virtual Cursor cursor(CRect const &region, Vec2 offset) override
  {
    (void) region;
    (void) offset;
    return Cursor::Text;
  }

  static constexpr TextCommand key_to_command(WidgetContext const &ctx)
  {
    if (ctx.key_down(KeyCode::Escape))
    {
      return TextCommand::Escape;
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

    return TextCommand::None;
  }

  virtual void tick(WidgetContext const &ctx, CRect const &region,
                    WidgetEventTypes events) override
  {
    Vec<u32> clipboard_data_u32;
    Vec<u8>  clipboard_data_u8;
    defer    clipboard_data_u32_del{[&] { clipboard_data_u32.reset(); }};
    defer    clipboard_data_u8_del{[&] { clipboard_data_u8.reset(); }};
    auto     erase  = [this](Slice32 s) { this->text.erase(s); };
    auto     insert = [this](u32 pos, Span<u32 const> t) {
      CHECK(this->text.insert_span_copy(pos, t));
    };
    auto get_content = [&clipboard_data_u32, &ctx]() -> Span<u32 const> {
      clipboard_data_u32.clear();
      CHECK(utf8_decode(ctx.get_clipboard_data(span(MIME_TEXT_UTF8)),
                        clipboard_data_u32));
      return span(clipboard_data_u32);
    };
    auto set_content = [&clipboard_data_u8, &ctx](Span<u32 const> data) {
      clipboard_data_u8.clear();
      CHECK(utf8_encode(data, clipboard_data_u8));
      ctx.set_clipboard_data(span(MIME_TEXT_UTF8), span(clipboard_data_u8));
    };

    if (disabled)
    {
      return;
    }
    TextCommand cmd = TextCommand::None;
    if (has_bits(events, WidgetEventTypes::TextInput))
    {
      cmd = TextCommand::InputText;
    }
    else if (has_bits(events, WidgetEventTypes::DragStart))
    {
      cmd = TextCommand::Hit;
    }
    else if (has_bits(events, WidgetEventTypes::DragUpdate))
    {
      cmd = TextCommand::HitSelect;
    }
    else if (has_bits(events, WidgetEventTypes::KeyDown))
    {
      cmd = key_to_command(ctx);
    }

    Vec2 offset = region.begin() - ctx.mouse_position;
    compositor.command(span(text), layout, style, cmd, fn(&insert), fn(&erase),
                       ctx.text, fn(&get_content), fn(&set_content), 1, offset);

    // TODO(lamarrr):
    //
    // [ ] tab (navigate? enter text?): depends on attributes?
    // [ ] escape?
    // [ ] multi-line mode or single line mode (i.e.) -> on press enter ?
    // submit
    // [ ] submittable: clicking with enter keyboard when focused
    // [ ] use placeholder when text empty
    // [ ] present one won't work when we have say ctrl + click
    //
    //
    // [ ] SDL_SetCursor()
    //     SDL_CreateSystemCursor(); - all created at startup
    //     SDL_HideCursor();
    //     SDL_ShowCursor();
    //
    //
    // https://github.com/ocornut/imgui/issues/787#issuecomment-361419796 enter
    // parent: prod children, nav to children
    // https://user-images.githubusercontent.com/8225057/74143829-ce67b900-4bfb-11ea-90d9-0de40c944b26.gif
    //

    layout_text({}, F32_MAX, layout);
  }
};

struct TextInputView : Widget
{
  TextInput input;
  // [ ] calculate lines per page
  // [ ] viewport text with scrollable region
  //  WidgetAttributes::Scrollable |
};

// DragBox: text input + dragging when alt is pressed down
struct ScalarDragBox : Widget
{
  Fn<void(fmt::Context &, ScalarInput)> formatter  = fn(default_formatter);
  ScalarInput                           value      = {};
  ScalarInput                           min        = {};
  ScalarInput                           max        = {};
  ScalarInput                           step       = {};
  Fn<void(ScalarInput)>                 on_changed = fn([](ScalarInput) {});
  SizeConstraint                        width      = {.offset = 100};
  SizeConstraint                        height     = {.offset = 20};
  bool                                  disabled   = false;

  static void default_formatter(fmt::Context &ctx, ScalarInput v)
  {
    fmt::format(ctx, v);
  }

  virtual Vec2 fit(Vec2 allocated, Span<Vec2 const>, Span<Vec2>) override
  {
    return Vec2{width.resolve(allocated.x), height.resolve(allocated.y)};
  }

  virtual WidgetAttributes attributes() override
  {
    return WidgetAttributes::Visible | WidgetAttributes::Clickable |
           WidgetAttributes::Draggable;
  }

  virtual void render(CRect const &region, Canvas &canvas) override
  {
    (void) region;
    (void) canvas;
  }

  virtual void tick(WidgetContext const &ctx, CRect const &region,
                    WidgetEventTypes events) override
  {
    // if focused, read text input and parse
    // if dragging, update
    // if clicked, move to position
    // should widgets handle this? no! systems.
    // mouse enter + mouse down => focuse enter, mouse click elsewhere (focus
    // lost), navigation event
  }
};

/// @brief
/// REQUIREMENTS:
/// Custom Scaling & Custom Stepping: i.e. log, linear, Angular Input
/// Drag-Based Input
/// Text-Field Input of exact values
/// Generic Numeric Input: Scalars, Vectors, Matrices, Tensors
struct ScalarBox : Widget
{
  Fn<void(ScalarInput &, ScalarInput, ScalarInput, ScalarInput, bool)> stepper =
      fn(default_stepper);
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
      b->stepper(b->dragger.value, b->dragger.min, b->dragger.max,
                 b->dragger.step, true);
      b->dragger.on_changed(b->dragger.value);
    });

    negative_stepper.on_clicked = fn(this, [](ScalarBox *b) {
      b->stepper(b->dragger.value, b->dragger.min, b->dragger.max,
                 b->dragger.step, false);
      b->dragger.on_changed(b->dragger.value);
    });

    dragger.on_changed =
        fn(this, [](ScalarBox *b, ScalarInput value) { b->on_changed(value); });
  }

  static void default_stepper(ScalarInput &v, ScalarInput min, ScalarInput max,
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

struct VectorInputBox : Widget
{
  ScalarBox scalars[8];
  u32       num = 0;
};

struct MatrixInputBox : Widget
{
  VectorInputBox vectors[8];
  u32            num_rows    = 0;
  u32            num_columns = 0;
};

}        // namespace ash
