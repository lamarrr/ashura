#pragma once
#include "ashura/engine/key.h"
#include "ashura/engine/text.h"
#include "ashura/std/error.h"
#include "ashura/std/mem.h"
#include "ashura/std/types.h"


namespace ash
{

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
/// @param selection_first first codepoint in the selection range. not always a
/// valid index into the text.
/// @param selection_last last (inclusive) codepoint in the selection range,
/// this is where the cursor was last at, i.e. when dragging or highlighting.
/// not always a valid index into the text.
struct TextCompositor
{
  static constexpr u32           DEFAULT_WORD_SYMBOLS[] = {' ', '\t'};
  static constexpr u32           DEFAULT_LINE_SYMBOLS[] = {'\n'};
  AllocatorImpl                  allocator              = default_allocator;
  u32                            selection_first        = 0;
  u32                            selection_last         = 0;
  Vec<u32>                       buffer                 = {};
  Vec<TextEditRecord>            records                = {};
  u32                            buffer_size            = 0;
  u32                            buffer_pos             = 0;
  u32                            latest_record          = 0;
  u32                            current_record         = 0;
  Fn<void(u32, Span<u32 const>)> on_insert = to_fn([](u32, Span<u32 const>) {});
  Fn<void(u32, u32)>             on_erase  = to_fn([](u32, u32) {});
  Span<u32 const>                word_symbols = to_span(DEFAULT_WORD_SYMBOLS);
  Span<u32 const>                line_symbols = to_span(DEFAULT_LINE_SYMBOLS);

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

  void click(TextLayout const &layout, Span<u32 const> text, u32 num, Vec2 pos)
  {
    switch (num)
    {
      case 0:
        break;

      case 1:
        single_click(layout, pos);
        break;

      case 2:
        double_click(layout, pos, text);
        break;

      case 3:
        triple_click(layout, pos, text);
        break;

      case 4:
      default:
        quad_click(layout, pos, text);
        break;
    }
  }

  /// @brief: on mouse press, move the cursor to the clicked location, and reset
  /// the selection
  void single_click(TextLayout const &layout, Vec2 pos)
  {
    // TODO(lamarrr): handle U32_MAX
    selection_last  = hit(layout, pos);
    selection_first = selection_last;
  }

  void drag(TextLayout const &layout, Vec2 pos)
  {
    // TODO(lamarrr): handle U32_MAX
    selection_last = hit(layout, pos);
  }

  static constexpr Slice32 selection_slice(u32 size, u32 first, u32 last)
  {
    if (first > last)
    {
      swap(first, last);
    }
    return Slice32{first, (last + 1) - first}.resolve(size);
  }

  static constexpr bool is_symbol(u32 c, Span<u32 const> symbols)
  {
    return c == ' ' || c == '\t' || !find(symbols, c).is_empty();
  }

  /// @brief given a sequence of text and symbols find the symbol boundary from
  /// a given codepoint index.
  /// @param text must be non-empty
  /// @param pos must be valid index into the text
  /// @param symbols additional unicode symbols that specify word boundaries,
  /// i.e. a code editor may use '(',')' as word boundaries.
  /// @param[out] first index of first char in symbolic boundary
  /// @param[out] last index of last char in symbolic boundary
  static constexpr void find_boundary(Span<u32 const> text, u32 const pos,
                                      Span<u32 const> symbols, u32 &first,
                                      u32 &last)
  {
    u32 f = pos;
    u32 b = pos;

    if (is_symbol(text[pos], symbols))
    {
      while (f < text.size32() && is_symbol(text[f], symbols))
      {
        f++;
      }

      while (b != 0 && is_symbol(text[b], symbols))
      {
        b--;
      }
    }
    else
    {
      while (f < text.size32() && !is_symbol(text[f], symbols))
      {
        f++;
      }

      while (b != 0 && !is_symbol(text[f], symbols))
      {
        b--;
      }
    }

    // TODO(lamarrr): is this correct?
    f     = f - 1;
    b     = (b == 0) ? b : (b + 1);
    first = b;
    last  = f;
  }

  void double_click(TextLayout const &layout, Vec2 pos, Span<u32 const> text)
  {
    // select word
    u32 sym = hit(layout, pos);
    // find word boundary forward and backward from current point
    //
    if (sym == U32_MAX)
    {
    }

    find_boundary(text, 0x0000f, word_symbols, );
  }

  void triple_click(TextLayout const &layout, Vec2 pos, Span<u32 const> text)
  {
    // select line
    u32 sym = hit(layout, pos);
    //
    // TODO(lamarrr): function to return indices as

    find_boundary(text, pos, line_symbols, , );
  }

  void quad_click(TextLayout const &layout, Vec2 pos, Span<u32 const> text)
  {
    // select whole text
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

}        // namespace ash
