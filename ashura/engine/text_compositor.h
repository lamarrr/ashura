/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/input.h"
#include "ashura/engine/text.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief
/// @param slice region of the original text this info belongs to
/// @param is_insert whether this is a erase or insert record
struct TextEditRecord
{
  Slice32 slice     = {};
  bool    is_insert = false;
};

enum class TextCommand : u32
{
  None = 0,

  /// Cursor State
  Unselect = 1,

  /// Editing
  BackSpace = 2,
  Delete    = 3,
  InputText = 4,

  /// Cursor Positioning
  Left      = 5,
  Right     = 6,
  WordStart = 7,
  WordEnd   = 8,
  LineStart = 9,
  LineEnd   = 10,
  Up        = 11,
  Down      = 12,
  PageUp    = 13,
  PageDown  = 14,

  /// Cursor Selection
  SelectLeft      = 15,
  SelectRight     = 16,
  SelectUp        = 17,
  SelectDown      = 18,
  SelectWordStart = 19,
  SelectWordEnd   = 20,
  SelectLineStart = 21,
  SelectLineEnd   = 22,
  SelectPageUp    = 23,
  SelectPageDown  = 24,

  /// Semantic Selection
  SelectCodepoint = 25,
  SelectWord      = 26,
  SelectLine      = 27,
  SelectAll       = 28,

  /// ClipBoard
  Cut   = 29,
  Copy  = 30,
  Paste = 31,

  /// Redo/Undo
  Undo = 32,
  Redo = 33,

  /// Mouse Selection (Visual)
  Hit       = 34,
  HitSelect = 35,

  /// Insert new line
  NewLine = 36,
  Tab     = 37
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
    u32 rfirst = (u32) ash::clamp(first, (i64) 0, (i64) len);
    u32 rlast  = (u32) ash::clamp(last, (i64) 0, (i64) len);
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

  constexpr TextCursor unselect() const
  {
    return TextCursor{last, last};
  }

  constexpr TextCursor clamp(u32 len) const
  {
    if (len == 0)
    {
      return TextCursor{};
    }
    return TextCursor{ash::clamp(first, (i64) 0, (i64) (len - 1)),
                      ash::clamp(last, (i64) 0, (i64) (len - 1))};
  }
};

/// @brief A simple stack-based text compositor
/// @param buffer_pos the end of the buffer for the current text edit
/// record
struct TextCompositor
{
  static constexpr u32 MAX_TAB_WIDTH = 8;

  /// @brief Text insert callback.
  /// @param index destination index, needs to be clamped to the size of the
  /// destination container.
  /// @param text text to be inserted
  typedef Fn<void(u32, Span<c32 const>)> Insert;

  /// @brief Text erase callback.
  /// @param index index to be erased from, needs to be clamped to the size of
  /// the destination container.
  /// @param num number of items to be erased
  typedef Fn<void(Slice32)> Erase;

  static constexpr c32 DEFAULT_WORD_SYMBOLS[] = {' ', '\t'};
  static constexpr c32 DEFAULT_LINE_SYMBOLS[] = {'\n', 0x2029};
  static constexpr c32 TAB_STRING[]           = {'\t', '\t', '\t', '\t',
                                                 '\t', '\t', '\t', '\t'};

  struct Inner
  {
    TextCursor          cursor         = {};
    Vec<c32>            buffer         = {};
    Vec<TextEditRecord> records        = {};
    u32                 buffer_usage   = 0;
    u32                 buffer_pos     = 0;
    u32                 latest_record  = 0;
    u32                 current_record = 0;
    u32                 tab_width      = 1;
    Span<c32 const>     word_symbols   = DEFAULT_WORD_SYMBOLS;
    Span<c32 const>     line_symbols   = DEFAULT_LINE_SYMBOLS;

    Inner(u32 num_buffer_codepoints, u32 num_records)
    {
      CHECK(num_buffer_codepoints > 0);
      CHECK(num_records > 0);
      CHECK(is_pow2(num_buffer_codepoints));
      CHECK(is_pow2(num_records));
      buffer.resize_uninit(num_buffer_codepoints).unwrap();
      records.resize_defaulted(num_records).unwrap();
    }

    void pop_records(u32 num);

    void append_record(bool is_insert, u32 text_pos, Span<c32 const> segment);

    void undo(Insert insert, Erase erase);

    void redo(Insert insert, Erase erase);

    void unselect();

    void delete_selection(Span<c32 const> text, Erase erase);

    void command(Span<c32 const> text, TextLayout const & layout,
                 f32 align_width, f32 alignment, TextCommand cmd, Insert insert,
                 Erase erase, Span<c32 const> input, ClipBoard & clipboard,
                 u32 lines_per_page, Vec2 pos);
  };

  Inner inner;

  TextCompositor(u32 num_buffer_codepoints = 16_KB, u32 num_records = 1'024) :
      inner{num_buffer_codepoints, num_records}
  {
  }

  TextCursor get_cursor() const
  {
    return inner.cursor;
  }

  void pop_records(u32 num)
  {
    inner.pop_records(num);
  }

  void append_record(bool is_insert, u32 text_pos, Span<c32 const> segment)
  {
    inner.append_record(is_insert, text_pos, segment);
  }

  void undo(Insert insert, Erase erase)
  {
    inner.undo(insert, erase);
  }

  void redo(Insert insert, Erase erase)
  {
    inner.redo(insert, erase);
  }

  void unselect()
  {
    inner.unselect();
  }

  void delete_selection(Span<c32 const> text, Erase erase)
  {
    inner.delete_selection(text, erase);
  }

  /// @param text original text
  /// @param input text from IME to insert
  void command(Span<c32 const> text, TextLayout const & layout, f32 align_width,
               f32 alignment, TextCommand cmd, Insert insert, Erase erase,
               Span<c32 const> input, ClipBoard & clipboard, u32 lines_per_page,
               Vec2 pos)
  {
    inner.command(text, layout, align_width, alignment, cmd, insert, erase,
                  input, clipboard, lines_per_page, pos);
  }
};

}        // namespace ash
