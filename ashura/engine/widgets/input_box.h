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

enum class TextEditKey : u32
{
};

/// @brief
/// @param text_pos index of the first codepoint belonging to this record in the
/// history buffer (only valid if it was a delete event)
/// @param history_pos position of this text in the history buffer
/// @param num number of codepoints this text record spans in the history buffer
/// @param is_delete whether this is a delete or insert record
struct TextEditRecord
{
  u32  text_pos    = 0;
  u32  history_pos = 0;
  u32  num         = 0;
  bool is_delete   = false;
};

/// @brief A simple stack-based text compositor
/// @param cursor currently selected codepoint
/// @param selection_first first codepoint in the selection range
/// @param selection_last last codepoint in the selection range
struct TextCompositor
{
  AllocatorImpl       allocator       = default_allocator;
  u32                 cursor          = U32_MAX;
  u32                 selection_first = U32_MAX;
  u32                 selection_last  = U32_MAX;
  Vec<u32>            buffer          = {};
  Vec<TextEditRecord> records         = {};
  u32                 next_record     = 0;
  u32                 state           = 0;

  void init(u32 num_buffer_codepoints, u32 num_records)
  {
    CHECK(is_pow2(num_buffer_codepoints));
    CHECK(is_pow2(num_records));
    CHECK(buffer.resize_uninitialized(num_buffer_codepoints));
    CHECK(records.resize_defaulted(num_records));
  }

  void reset()
  {
    allocator       = default_allocator;
    cursor          = U32_MAX;
    selection_first = U32_MAX;
    selection_last  = U32_MAX;
    buffer.reset();
    records.reset();
    next_record = U32_MAX;
  }

  void clear();
  void uninit()
  {
    reset();
  }

  constexpr bool has_selection() const
  {
    return selection_first != selection_last;
  }

  // given a cursor position get grapheme at position
  // . if doing multi selection, need to do min/max to get as some advance
  // negatively

  /// @brief given a position in the laid-out text return the grapheme the
  /// cursor points to.
  /// @param pos position in laid-out text to return from
  /// @return U32_MAX if no grapheme overlaps the selection, otherwise the
  /// grapheme index in the original text, see GlyphShape::grapheme
  u32 hit(TextLayout const &layout, Vec2 pos) const
  {
    f32       current_top = 0;
    u32       line        = 0;
    u32 const num_lines   = (u32) layout.lines.size();
    for (; line < num_lines; line++)
    {
      if (current_top <= pos.y &&
          (current_top + layout.lines[line].metrics.height) >= pos.y)
      {
        break;
      }
    }

    if (line != num_lines)
    {
      f32 line_x = 0;
      for (u32 r = 0; r < layout.lines[line].num_runs; r++)
      {
        TextRun const &run = layout.runs[layout.lines[line].first_run + r];
        if (!(line_x <= pos.x && (line_x + pt_to_px(run.metrics.advance,
                                                    run.font_height)) >= pos.x))
        {
          continue;
        }
        for (u32 g = 0; g < run.num_glyphs; g++)
        {
          // TODO(lamarrr): not correct, needs to perform actual intersection
          // test also, need to take care of directionality
          GlyphShape const &glyph = layout.glyphs[run.first_glyph + g];
          if (pt_to_px(glyph.advance.x, run.font_height) + line_x < pos.x)
          {
            return glyph.cluster;
          }
        }
        line_x += pt_to_px(run.metrics.advance, run.font_height);
      }

      // return last char on line
    }

    // if non-empty line, return char on last line

    return U32_MAX;
  }

  /// given a codepoint index, return the position in the text
  Vec2 locate()
  {
  }

  /// @brief: on mouse down, move the cursor to the clicked location, and reset
  /// the selection
  void click(TextLayout const &layout, Vec2 pos)
  {
    cursor          = hit(layout, pos);
    selection_first = cursor;
    selection_last  = cursor;
  }

  void drag(TextLayout const &layout, Vec2 pos)
  {
    if (selection_last == selection_first)
    {
      selection_first = cursor;
    }
    selection_last = hit(layout, pos);
    cursor         = selection_last;
  }

  void double_click()
  {
  }

  // move the cursor up to the previous line
  void up();
  void down();
  void home();
  void left();
  void right();
  void end();

  void undo()
  {
    // apply undo
  }

  void redo()
  {
    // apply
    //
    // how do we go back once the history buffer is full?
  }

  void commit_insert_record(u32 text_pos, u32 num)
  {
    TextEditRecord &record = records[next_record];
    record.text_pos        = text_pos;
    record.history_pos     = 0;
    record.num             = num;
    record.is_delete       = false;
    next_record            = (next_record + 1) & ((u32) records.size() - 1);
  }

  void commit_delete_record(Span<u32 const> text, u32 text_pos, u32 num)
  {
    TextEditRecord &record = records[next_record];
    record.text_pos        = text_pos;
    record.num             = num;
    record.is_delete       = true;
    next_record            = (next_record + 1) & ((u32) records.size() - 1);

    // TODO(lamarrr): history buffer management for insert and delete!!!
    // TODO(lamarrr): pop enough records to fit within the history buffer.
    // if more than history buffer size. clear record. and reject.
    // TODO(lamarrr): after adding undo record, delete redo record and
    // vice-versa?
    // undo is done by moving backwards. we might need to mark the present point
    // as redo point

    mem::copy(text.slice(text_pos, num), to_span(buffer).slice(0, 1));
  }

  void clear_text();
  void highlight();
  void copy();
  void cut();
  void paste();

  void delete_selection()
  {
    // delete selection
    // TODO(clamping)
    if (has_selection())
    {
      if (selection_first < selection_last)
      {
        // delete selection_first -> selection_last
        selection_last = selection_first;
        cursor         = selection_first;
      }
      else
      {
        selection_first = selection_last;
        cursor          = selection_last;
      }
    }
  }

  // move to selection last
  // move to selection first
  // move to word previous
  // move to word next
  // select word

  /// @brief commit text editing history to buffer

  /// @brief  Uses key up/down states to determine next composition state
  /// @param key_states
  void tick(u64 const (&key_states)[NUM_KEYS / 64]);
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
