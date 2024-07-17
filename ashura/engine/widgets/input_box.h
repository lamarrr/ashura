/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/widget.h"
#include "ashura/engine/widgets/button.h"

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

inline bool push(Context &ctx, Spec const &spec, ScalarInput const &value)
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
  bool            is_secret           = false;
  bool            is_multiline        = false;
  bool            is_submittable      = false;
  Span<u32 const> placeholder         = {};
  Vec<u32>        text                = {};
  Vec<u32>        secret              = {};
  Fn<void()>      on_editing          = to_fn([] {});
  Fn<void()>      on_editing_finished = to_fn([] {});
  Fn<void()>      on_submit           = to_fn([] {});
  TextCompositor  compositor          = {};

  virtual void tick(WidgetContext const &ctx, CRect const &region,
                    nanoseconds dt, WidgetEventTypes events) override
  {
    (void) ctx;
    (void) dt;
    (void) events;
    // on click or focus, request keyboard
    // for text area, change cursor type to editing
    //

    if (has_bits(events, WidgetEventTypes::TextInput))
    {
      compositor.command();
    }
    else if (has_bits(events, WidgetEventTypes::MouseDown))
    {
      // drag
    }
    else if (has_bits(events, WidgetEventTypes::MouseScroll))
    {
      // if scrollable region, scroll
    }
    else if (has_bits(events, WidgetEventTypes::KeyDown))
    {
      // control codes
    }

    // TODO(lamarrr):
    // - [ ] focus model (keymap navigation Tab to move focus backwards, Shift +
    // Tab to move focus forwards)
    // - [ ] scrolling
    // - [ ] key debouncing
    // - [ ] clipboard api
    // - [ ] keymap
    // - [ ] multi-line mode or single line mode (i.e.) -> on press enter ?
    // submit
    // - [ ] cursor rendering
    // - [ ] textinputbegin
    // - [ ] textinputend
    // - [ ] debouncing of keyboard and mouse
    // - [ ] time-based debouncing of keyboard pressing window mouse
    // and key focus should propagate to tree AND focus or unfocus widgets
    // SDL_StartTextInput
    // - [ ] color space, pixel info for color pickers
    // - [ ] spatial navigation model
    /// use spatial testing and scrolling information instead
    /// when moved, move to the closest non-obscured one. clipping? CHILDREN
    /// navigatable but not visible. as in imgui.
    // - [ ] scroll on child focus
    // https://github.com/ocornut/imgui/issues/787#issuecomment-361419796 enter
    /// parent: prod children, nav to children
    /// https://user-images.githubusercontent.com/8225057/74143829-ce67b900-4bfb-11ea-90d9-0de40c944b26.gif
    /// clicking with enter keyboard when focused
  }
};

// DragBox: text input + dragging when alt is pressed down
struct ScalarDragBox : Widget
{
  Fn<void(fmt::Context &, ScalarInput)> formatter  = to_fn(default_formatter);
  ScalarInput                           value      = {};
  ScalarInput                           min        = {};
  ScalarInput                           max        = {};
  ScalarInput                           step       = {};
  Fn<void(ScalarInput)>                 on_changed = to_fn([](ScalarInput) {});
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
                    nanoseconds dt, WidgetEventTypes events) override
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
      to_fn(default_stepper);
  bool steppable = false;
  bool draggable = false;
  bool disabled  = false;

  TextButton            negative_stepper;
  TextButton            positive_stepper;
  ScalarDragBox         dragger;
  Fn<void(ScalarInput)> on_changed = to_fn([](ScalarInput) {});

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