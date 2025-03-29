/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/input.h"
#include "ashura/engine/render_text.h"
#include "ashura/engine/text.h"
#include "ashura/std/types.h"

namespace ash
{

enum class TextEditRecordType : u32
{
  Erase   = 0,
  Insert  = 1,
  /// @brief Erase + Insert
  Replace = 2
};

/// @brief
/// @param slice region of the original text this info belongs to
/// @param is_insert whether this is a erase or insert record
struct TextEditRecord
{
  usize              text_pos    = 0;
  usize              erase_size  = 0;
  usize              insert_size = 0;
  TextEditRecordType type        = TextEditRecordType::Insert;

  constexpr usize buffer_usage() const
  {
    return erase_size + insert_size;
  }
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
  Tab     = 37,

  Submit = 38
};

struct TextCursor
{
  /// @brief the first selected codepoint in the selection range. [-1, n]
  i64 begin = 0;

  /// @brief 1 past the last selected codepoint in either direction. [-1, n]
  i64 end = 0;

  static constexpr TextCursor from_slice(Slice s)
  {
    return TextCursor{(i64) s.offset, (i64) (s.offset + s.span)};
  }

  constexpr TextCursor & unselect()
  {
    end = begin;
    return *this;
  }

  constexpr bool has_selection() const
  {
    return begin != end;
  }

  constexpr bool direction() const
  {
    return begin < end;
  }

  constexpr TextCursor & span_to(i64 pos)
  {
    end = max(pos + 1, (i64) -1);
    return *this;
  }

  constexpr TextCursor & span_by(i64 distance)
  {
    end = max(begin + distance, (i64) -1);
    return *this;
  }

  constexpr i64 distance() const
  {
    return end - begin;
  }

  constexpr Slice as_slice() const
  {
    if (this->begin <= this->end)
    {
      auto const begin = (usize) max(this->begin, (i64) 0);
      auto const end   = (usize) max(this->end, (i64) 0);
      return Slice::from_range(begin, end);
    }
    else
    {
      auto const begin = (usize) max(this->end - 1, (i64) 0);
      auto const end   = (usize) max(this->begin + 1, (i64) 0);
      return Slice::from_range(begin, end);
    }
  }

  constexpr TextCursor & to_begin()
  {
    end = begin;
    return *this;
  }

  constexpr TextCursor & to_end()
  {
    begin = end;
    return *this;
  }

  constexpr TextCursor & to_left()
  {
    end = begin = min(begin, end);
    return *this;
  }

  constexpr TextCursor & to_right()
  {
    end = begin = max(begin, end);
    return *this;
  }

  constexpr TextCursor & normalize(usize len)
  {
    begin = clamp(begin, (i64) -1, (i64) len);
    end   = clamp(end, (i64) -1, (i64) len);
    return *this;
  }

  constexpr TextCursor & shift(i64 n)
  {
    begin += n;
    end += n;
    begin = max(begin, (i64) -1);
    end   = max(end, (i64) -1);
    return *this;
  }
};

/// @brief A stack-based text compositor
struct TextCompositor
{
  static constexpr u32   MAX_TAB_WIDTH          = 32;
  static constexpr usize DEFAULT_BUFFER_SIZE    = 2'048;
  static constexpr usize DEFAULT_RECORDS_SIZE   = 2'048;
  static constexpr c32   DEFAULT_WORD_SYMBOLS[] = {U' ', U'\t'};
  static constexpr c32   DEFAULT_LINE_SYMBOLS[] = {U'\n', 0x2029};

  TextCursor          cursor_;
  Vec<c32>            buffer_;
  Vec<TextEditRecord> records_;
  Str32               word_symbols_;
  Str32               line_symbols_;

  /// @brief record representing the current text composition state
  usize state_;

  TextCompositor(Vec<c32> buffer, Vec<TextEditRecord> records,
                 Str32 word_symbols, Str32 line_symbols) :
    buffer_{std::move(buffer)},
    records_{std::move(records)},
    word_symbols_{word_symbols},
    line_symbols_{line_symbols},
    state_{0}
  {
  }

  static TextCompositor create(AllocatorRef allocator,
                               usize        buffer_size  = DEFAULT_BUFFER_SIZE,
                               usize        records_size = DEFAULT_RECORDS_SIZE,
                               Str32        word_symbols = DEFAULT_WORD_SYMBOLS,
                               Str32 line_symbols = DEFAULT_LINE_SYMBOLS);

  TextCompositor(TextCompositor const &)             = delete;
  TextCompositor(TextCompositor &&)                  = default;
  TextCompositor & operator=(TextCompositor const &) = delete;
  TextCompositor & operator=(TextCompositor &&)      = default;
  ~TextCompositor()                                  = default;

  static usize goto_line(TextLayout const & layout, usize alignment,
                         usize line);

  TextCursor cursor() const;

  Slice buffer_slice(Slice records) const;

  /// @brief pop the first `num` earliest records
  void pop_records(usize num);

  /// @brief truncate all records from `state + 1` to last record
  /// i.e. when editing from a present state, all redo history from that point is cleared.
  void truncate_records();

  void push_record(TextEditRecordType type, usize text_pos, Str32 erase,
                   Str32 insert);

  bool undo(Vec<c32> & str);

  bool redo(Vec<c32> & str);

  bool delete_selection(Vec<c32> & str);

  /// @param input text from IME to insert
  Tuple<bool, Slice> command(RenderText & text, TextCommand cmd, Str32 input,
                             ClipBoard & clipboard, usize lines_per_page,
                             usize tab_width, CRect const & region, Vec2 pos,
                             f32 zoom, AllocatorRef scratch_allocator);
};

}    // namespace ash
