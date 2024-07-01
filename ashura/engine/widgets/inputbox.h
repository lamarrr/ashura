#pragma once

#include "ashura/engine/color.h"
#include "ashura/engine/widget.h"
#include "ashura/engine/widgets/button.h"

namespace ash
{

enum class ScalarInputType : u8
{
  i32 = 0,
  f32 = 1
};

struct ScalarInput
{
  union
  {
    i32 i32 = 0;
    f32 f32;
  };
  ScalarInputType type = ScalarInputType::f32;
};

namespace fmt
{

inline bool push(Context &ctx, Spec const &spec, ScalarInput const &value)
{
  switch (value.type)
  {
    case ScalarInputType::i32:
      return push(ctx, spec, value.i32);
    case ScalarInputType::f32:
      return push(ctx, spec, value.f32);
    default:
      return true;
  }
}

}        // namespace fmt

// operates on clusters, need to map graphemes to clusters, and clusters to
// codepoints and vice-versa
struct Caret
{
  u32 text_begin     = 0;
  u32 text_span      = 0;
  u32 line           = 0;
  u32 line_alignment = 0;

  // grapheme + text selection, grapheme iteration should be able to know when
  // the grapheme has been iterated finished.

  void undo();
  void redo();
  void clear();
  void highlight();
  void copy();
  void cut();

  // move the cursor up to the previous line
  void up(TextLayout const &layout, TextBlock const &block)
  {
    text_span = 0;
    if (line == 0 || layout.lines.size() <= 1)
    {
      return;
    }
    if (line >= layout.lines.size())
    {
      line = (u32) (layout.lines.size() - 1);
    }
    Line const &next_line      = layout.lines[line - 1];
    u32         best_fit_begin = 0;
    for (TextRun const &r :
         to_span(layout.runs).slice(next_line.first_run, next_line.num_runs))
    {
      for (GlyphShape const &sh :
           to_span(layout.glyphs).slice(r.first_glyph, r.num_glyphs))
      {
      }
    }
  }
  void down();
  void home();
  void left();
  void right();
  void end();
  void select_word();
};

struct TextInput : Widget
{
  bool secret    = false;
  bool disabled  = false;
  bool multiline = false;
  // clear
  //
  // on_editing
  // on_edited

  virtual void tick(WidgetContext const &ctx, CRect const &region,
                    nanoseconds dt, WidgetEventTypes events) override
  {
    (void) ctx;
    (void) dt;
    (void) events;

    // TODO(lamarrr):
    // copy
    // paste
    // editing
    // escape
    // highlighting
    // selecting with keyboard nav
    // composition via IME
    // home, end
    // do undo
    // how to process enter events, submit? ignore?
    // escape resets
    // blinking default cursor
    // custom cursor
    // multi-line

    // check has keyboard
    // textinputbegin
    // textinputend
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
