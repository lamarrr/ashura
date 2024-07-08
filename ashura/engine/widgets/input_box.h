#pragma once

#include "ashura/engine/color.h"
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

/// @brief
/// @param text_pos index of the first codepoint belonging to this record in the
/// history buffer (only valid if it was an erase event)
/// @param buffer_pos position of this text in the history buffer
/// @param num number of codepoints this text record spans in the history buffer
/// @param is_insert whether this is a erase or insert record
struct TextEditRecord
{
  u32  text_pos  = 0;
  u32  num       = 0;
  bool is_insert = false;
};

/// @brief A simple stack-based text compositor
/// @param selection_first first codepoint in the selection range
/// @param selection_last last codepoint in the selection range, this is where
/// the cursor is at
struct TextCompositor
{
  AllocatorImpl                  allocator       = default_allocator;
  u32                            selection_first = 0;
  u32                            selection_last  = 0;
  Vec<u32>                       buffer          = {};
  Vec<TextEditRecord>            records         = {};
  u32                            buffer_size     = 0;
  u32                            buffer_pos      = 0;
  u32                            latest_record   = 0;
  u32                            current_record  = 0;
  Fn<void(u32, Span<u32 const>)> on_insert = to_fn([](u32, Span<u32 const>) {});
  Fn<void(u32, u32)>             on_erase  = to_fn([](u32, u32) {});

  void init(u32 num_buffer_codepoints, u32 num_records)
  {
    CHECK(num_buffer_codepoints > 0);
    CHECK(num_records > 0);
    CHECK(is_pow2(num_buffer_codepoints));
    CHECK(is_pow2(num_records));
    CHECK(buffer.resize_uninitialized(num_buffer_codepoints));
    CHECK(records.resize_defaulted(num_records));
  }

  void reset()
  {
    allocator       = default_allocator;
    selection_first = 0;
    selection_last  = 0;
    buffer.reset();
    records.reset();
    // current_record = U32_MAX;
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

  /// @brief given a position in the laid-out text return the grapheme the
  /// cursor points to.
  /// @param pos position in laid-out text to return from
  /// @return U32_MAX if no grapheme overlaps the selection, otherwise the
  /// grapheme index in the original text, see GlyphShape::grapheme
  u32 hit(TextLayout const &layout, Vec2 pos) const
  {
    // TODO(lamarrr): hit tests always have to return a result
    f32       current_top = 0;
    u32       line        = 0;
    u32 const num_lines   = layout.lines.size32();
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

  void pop_records(u32 num)
  {
    CHECK(num <= records.size32());
    u32 reclaimed = 0;
    for (u32 i = 0; i < num; i++)
    {
      reclaimed += records[i].num;
    }
    mem::move(to_span(buffer).slice(reclaimed).as_const(), to_span(buffer));
    mem::move(to_span(records).slice(num).as_const(), to_span(records));
    buffer_size -= reclaimed;
    latest_record -= num;
    current_record -= num;
  }

  void append_record(bool is_insert, u32 text_pos, Span<u32 const> text)
  {
    if (text.size32() > buffer.size32())
    {
      // clear all records as we can't insert a new record without invalidating
      // the history
      pop_records(records.size32());
      return;
    }

    while (buffer_size + text.size32() > buffer.size32())
    {
      // pop half, to amortize shifting cost.
      // always pop by atleast 1. since the buffer can fit it and atleast 1
      // record would be using the available memory.
      pop_records(max(records.size32() >> 1, 1U));
    }

    if (current_record + 1 >= records.size32())
    {
      pop_records(max(records.size32() >> 1, 1U));
    }

    mem::copy(text, to_span(buffer).slice(buffer_size));

    current_record++;
    latest_record           = current_record;
    records[current_record] = TextEditRecord{
        .text_pos = text_pos, .num = text.size32(), .is_insert = is_insert};
  }

  void undo()
  {
    if (current_record == 0)
    {
      return;
    }
    // undo changes of current record
    TextEditRecord &record = records[current_record];
    buffer_pos -= record.num;
    if (record.is_insert)
    {
      on_erase(record.text_pos, record.num);
    }
    else
    {
      on_insert(record.text_pos, to_span(buffer).slice(buffer_pos, record.num));
    }
    current_record--;
  }

  void redo()
  {
    if (current_record + 1 > latest_record)
    {
      return;
    }
    current_record++;
    // apply changes of next record
    TextEditRecord &record = records[current_record];
    if (record.is_insert)
    {
      on_insert(record.text_pos, to_span(buffer).slice(buffer_pos, record.num));
    }
    else
    {
      on_erase(record.text_pos, record.num);
    }

    buffer_pos += record.num;
  }

  void delete_selection(Span<u32 const> text)
  {
    if (selection_first == selection_last)
    {
      return;
    }

    if (selection_first > selection_last)
    {
      swap(selection_first, selection_last);
    }

    u32 first      = selection_first;
    u32 num        = (selection_last - selection_first) + 1;
    selection_last = selection_first;

    append_record(false, first, text.slice(first, num));
    on_erase(first, num);
  }

  /// @brief  Uses key up/down states to determine next composition state
  /// @param key_states
  void commands(u64 const (&key_states)[NUM_KEYS / 64]);
  void commit_insert(Vec<u32> &text, u32 pos, Span<u32 const> insert);

  // move to selection last
  // move to selection first
  // move to word previous
  // move to word next
  // select word

  /// @brief: on mouse down, move the cursor to the clicked location, and reset
  /// the selection
  void click(TextLayout const &layout, Vec2 pos)
  {
    selection_last  = hit(layout, pos);
    selection_first = selection_last;
  }

  void drag(TextLayout const &layout, Vec2 pos)
  {
    selection_last = hit(layout, pos);
  }

  void double_click(TextLayout const &layout, Vec2 pos, Span<u32 const> text)
  {
    // select word
    selection_first = hit(layout, pos);
    selection_last  = selection_first;
    // find word boundary forward and backward from current point
    //
  }

  void triple_click(TextLayout const &layout, Vec2 pos, Span<u32 const> text)
  {
    // select line
    selection_first = hit(layout, pos);
    selection_last  = selection_first;
  }

  void key_up()
  {
    // move the cursor up to the previous line
    // remove cursor selection
  }

  void key_down()
  {
    // move the cursor down to the next line
    // remove cursor selection
  }

  void key_home()
  {
    // move the cursor to the beginning of the line
    // remove cursor selection
  }

  void key_end()
  {
    // move the cursor to the end of the line
    // remove cursor selection
  }

  void key_left()
  {
    // if has selection, move to left of selection
    if (selection_first != selection_last)
    {
      selection_last  = min(selection_first, selection_last);
      selection_first = selection_last;
      return;
    }

    // move the cursor to the previous codepoint
    selection_first = max(selection_last, 1U) - 1;
    selection_last  = selection_first;
  }

  void key_right()
  {
    if (selection_first != selection_last)
    {
      selection_last  = max(selection_first, selection_last);
      selection_first = selection_last;
      return;
    }

    // move the cursor to the next codepoint
    selection_first = selection_last + 1;
    selection_last  = selection_first;
  }

  void key_page_up();
  void key_page_down();
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
