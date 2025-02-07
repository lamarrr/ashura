/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/engine/input.h"
#include "ashura/engine/render_text.h"
#include "ashura/engine/text.h"
#include "ashura/std/types.h"

namespace ash
{

/// @brief
/// @param slice region of the original text this info belongs to
/// @param is_insert whether this is a erase or insert record
struct TextEditRecord
{
  Slice slice     = {};
  bool  is_insert = false;
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

/// @param first the first selected codepoint in the selection range.
/// @param span the number of selected codepoints in either direction.
struct [[nodiscard]] TextCursor
{
  i64 first = 0;
  i64 span  = 0;

  static constexpr TextCursor from_slice(Slice s)
  {
    return TextCursor{(i64) s.offset, (i64) s.span};
  }

  constexpr TextCursor & span_to(i64 pos)
  {
    span = first - pos;
    return *this;
  }

  constexpr bool is_empty() const
  {
    return span == 0;
  }

  constexpr i64 last() const
  {
    return span == 0 ? first : ((first + span) - 1);
  }

  constexpr Slice as_slice() const
  {
    usize begin = 0;
    usize end   = 0;

    if (span >= 0)
    {
      begin = (usize) max(first, (i64) 0);
      end   = (usize) max((first + span), (i64) 0);
    }
    else
    {
      begin = (usize) max(((first + span) - 1), (i64) 0);
      end   = (usize) max((first + 1), (i64) 0);
    }

    return Slice{begin, end - begin};
  }

  constexpr TextCursor & to_first()
  {
    span = 0;
    return *this;
  }

  constexpr TextCursor & to_last()
  {
    first = last();
    span  = 0;
    return *this;
  }

  constexpr TextCursor & unselect()
  {
    return to_last();
  }

  constexpr TextCursor & clamp(usize len)
  {
    first = ash::clamp(first, (i64) 0, (i64) (len == 0 ? 0 : len - 1));
    span  = ash::clamp(first + span, (i64) 0, (i64) len) - first;
    return *this;
  }
};

/// @brief A stack-based text compositor
/// @param buffer_pos the end of the buffer for the current text edit
/// record
struct TextCompositor
{
  static constexpr u32 MAX_TAB_WIDTH = 8;

  /// @brief Text insert callback.
  /// @param index destination index, needs to be clamped to the size of the
  /// destination container.
  /// @param text text to be inserted
  typedef Fn<void(usize, Span<c32 const>)> Insert;

  /// @brief Text erase callback.
  /// @param index index to be erased from, needs to be clamped to the size of
  /// the destination container.
  /// @param num number of items to be erased
  typedef Fn<void(Slice)> Erase;

  static constexpr c32 DEFAULT_WORD_SYMBOLS[] = {' ', '\t'};
  static constexpr c32 DEFAULT_LINE_SYMBOLS[] = {'\n', 0x2029};

  AllocatorRef        allocator_;
  TextCursor          cursor_ = {};
  Vec<c32>            buffer_;
  Vec<TextEditRecord> records_;
  usize               buffer_size_;
  usize               records_size_;
  usize               buffer_usage_   = 0;
  usize               buffer_pos_     = 0;
  usize               latest_record_  = 0;
  usize               current_record_ = 0;
  u32                 tab_width_;
  Span<c32 const>     word_symbols_;
  Span<c32 const>     line_symbols_;

  TextCompositor(AllocatorRef allocator, u32 tab_width = 2,
                 usize buffer_size = 4'096, usize records_size = 1'024,
                 Span<c32 const> word_symbols = DEFAULT_WORD_SYMBOLS,
                 Span<c32 const> line_symbols = DEFAULT_LINE_SYMBOLS) :
    allocator_{allocator},
    buffer_{allocator},
    records_{allocator},
    buffer_size_{buffer_size},
    records_size_{records_size},
    tab_width_{tab_width},
    word_symbols_{word_symbols},
    line_symbols_{line_symbols}
  {
  }

  TextCompositor(TextCompositor const &)             = delete;
  TextCompositor(TextCompositor &&)                  = default;
  TextCompositor & operator=(TextCompositor const &) = delete;
  TextCompositor & operator=(TextCompositor &&)      = default;
  ~TextCompositor()                                  = default;

  static u32 goto_line(TextLayout const & layout, u32 alignment, u32 line);

  TextCursor get_cursor() const
  {
    return cursor_;
  }

  void pop_records(usize num);

  void append_record(bool is_insert, usize text_pos, Span<c32 const> segment);

  void undo(Insert insert, Erase erase);

  void redo(Insert insert, Erase erase);

  void unselect();

  void delete_selection(Span<c32 const> text, Erase erase);

  /// @param input text from IME to insert
  Slice command(RenderText const & text, TextCommand cmd, Insert insert,
                Erase erase, Span<c32 const> input, ClipBoard & clipboard,
                u32 lines_per_page, CRect const & region, Vec2 pos, f32 zoom);
};

}    // namespace ash
