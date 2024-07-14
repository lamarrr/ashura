/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/text.h"
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

enum class TextCommand : u32
{
  None = 0,

  /// Cursor State
  Escape = 1,

  /// Editing
  BackSpace = 2,
  Delete    = 3,

  /// Cursor Navigation (Selection Resetting)
  Left      = 4,
  Right     = 5,
  Up        = 6,
  Down      = 7,
  WordStart = 8,
  WordEnd   = 9,
  LineStart = 10,
  LineEnd   = 11,
  PageUp    = 12,
  PageDown  = 13,

  /// Cursor Navigation (Selection Spanning)
  SelectLeft      = 14,
  SelectRight     = 15,
  SelectUp        = 16,
  SelectDown      = 17,
  SelectWordStart = 18,
  SelectWordEnd   = 19,
  SelectLineStart = 20,
  SelectLineEnd   = 21,
  SelectPageUp    = 22,
  SelectPageDown  = 23,

  /// Semantic Selection
  SelectCodepoint = 24,
  SelectWord      = 25,
  SelectLine      = 26,
  SelectAll       = 27,

  /// Copy & Paste
  Cut   = 28,
  Copy  = 29,
  Paste = 30,

  /// Redo/Undo
  Undo = 31,
  Redo = 32,

  /// Mouse Selection (Visual)
  HitCodepoint = 33,
  HitWord      = 34,
  HitLine      = 35,
  HitAll       = 36,
  Drag         = 37
};

/// @param first includes the first selected codepoint in the selection range.
/// @param last includes the last selected codepoint in the selection range.
struct [[nodiscard]] TextCursor
{
  i64 first = 0;
  i64 last  = 0;

  static constexpr TextCursor from_slice(Slice32 s)
  {
    return TextCursor{s.offset,
                      (s.span == 0) ? s.offset : (s.offset + (s.span - 1))};
  }

  constexpr bool direction() const
  {
    return last >= first;
  }

  constexpr bool is_empty() const
  {
    return first == last;
  }

  constexpr Slice32 as_slice(u32 len) const
  {
    u32 rfirst = (u32) clamp(first, (i64) 0, (i64) len);
    u32 rlast  = (u32) clamp(last, (i64) 0, (i64) len);
    if (rfirst > rlast)
    {
      swap(rfirst, rlast);
    }
    rlast++;
    return Slice32{(u32) rfirst, (u32) (rlast - rfirst)};
  }

  constexpr TextCursor to_begin() const
  {
    TextCursor c;
    c.first = min(first, last);
    c.last  = c.first;
    return c;
  }

  constexpr TextCursor to_end() const
  {
    TextCursor c;
    c.first = max(first, last);
    c.last  = c.first;
    return c;
  }

  constexpr TextCursor escape() const
  {
    return TextCursor{last, last};
  }
};

/// @brief A simple stack-based text compositor
/// @param buffer_size current size of the buffer
/// @param buffer_pos marks the end of the buffer for the current text edit
/// record
struct TextCompositor
{
  /// @brief Text insert callback.
  /// @param index destination index, needs to be clamped to the size of the
  /// destination container.
  /// @param text text to be inserted
  typedef Fn<void(u32, Span<u32 const>)> Insert;

  /// @brief Text erase callback.
  /// @param index index to be erased from, needs to be clamped to the size of
  /// the destination container.
  /// @param num number of items to be erased
  typedef Fn<void(Slice32)> Erase;

  static constexpr u32 DEFAULT_WORD_SYMBOLS[] = {' ', '\t'};
  static constexpr u32 DEFAULT_LINE_SYMBOLS[] = {'\n', 0x2029};

  TextCursor          cursor         = {};
  u32                 alignment      = 0;
  Vec<u32>            buffer         = {};
  Vec<TextEditRecord> records        = {};
  u32                 buffer_size    = 0;
  u32                 buffer_pos     = 0;
  u32                 latest_record  = 0;
  u32                 current_record = 0;
  Span<u32 const>     word_symbols   = to_span(DEFAULT_WORD_SYMBOLS);
  Span<u32 const>     line_symbols   = to_span(DEFAULT_LINE_SYMBOLS);

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
    cursor = {};
    buffer.reset();
    records.reset();
    buffer_size    = 0;
    buffer_pos     = 0;
    latest_record  = 0;
    current_record = 0;
    word_symbols   = to_span(DEFAULT_WORD_SYMBOLS);
    line_symbols   = to_span(DEFAULT_LINE_SYMBOLS);
  }

  void uninit()
  {
    reset();
  }

  /// @brief adjust cursor to specified line whilst respecting cursor alignment
  void goto_line(TextLayout const &layout, TextBlockStyle const &style,
                 u32 line);

  void pop_records(u32 num);

  void append_record(bool is_insert, u32 text_pos, Span<u32 const> segment);

  void undo(Insert insert, Erase erase);

  void redo(Insert insert, Erase erase);

  void delete_selection(Span<u32 const> text, Erase erase);

  /// @brief IME-text input
  /// @param text original text
  /// @param input text from IME to insert
  void input_text(Span<u32 const> text, Span<u32 const> input, Insert insert,
                  Erase erase);

  void drag(TextLayout const &layout, TextBlockStyle const &style, Vec2 pos);

  void update_cursor(TextLayout const &layout, TextBlockStyle const &style);

  void select_codepoint();

  void select_word(Span<u32 const> text);

  void select_line(Span<u32 const> text);

  void select_all(Span<u32 const> text);

  void hit_codepoint(TextLayout const &layout, TextBlockStyle const &style,
                     Vec2 pos);

  void hit_word(Span<u32 const> text, TextLayout const &layout,
                TextBlockStyle const &style, Vec2 pos);

  void hit_line(Span<u32 const> text, TextLayout const &layout,
                TextBlockStyle const &style, Vec2 pos);

  void hit_all(Span<u32 const> text, TextLayout const &layout,
               TextBlockStyle const &style, Vec2 pos);

  void up(TextLayout const &layout, TextBlockStyle const &style, u32 lines);

  void down(TextLayout const &layout, TextBlockStyle const &style, u32 lines);

  void left();

  void right();

  void escape();

  void command(Span<u32 const> text, TextLayout const &layout,
               TextBlockStyle const &style, TextCommand cmd, Insert insert,
               Erase erase, Fn<Span<u32 const>()> get_content,
               Fn<void(Span<u32 const>)> set_content, u32 lines_per_page,
               Vec2 pos);
};

}        // namespace ash
