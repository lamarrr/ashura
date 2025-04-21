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

  Escape = 1,

  /// Cursor State
  Unselect = 2,

  /// Editing
  BackSpace = 3,
  Delete    = 4,
  InputText = 5,

  /// Cursor Positioning
  Left      = 6,
  Right     = 7,
  WordStart = 8,
  WordEnd   = 9,
  LineStart = 10,
  LineEnd   = 11,
  Up        = 12,
  Down      = 13,
  PageUp    = 14,
  PageDown  = 15,

  /// Cursor Selection
  SelectLeft        = 16,
  SelectRight       = 17,
  SelectUp          = 18,
  SelectDown        = 19,
  SelectToWordStart = 20,
  SelectToWordEnd   = 21,
  SelectToLineStart = 22,
  SelectToLineEnd   = 23,
  SelectPageUp      = 24,
  SelectPageDown    = 25,

  /// Semantic Selection
  SelectCodepoint = 26,
  SelectWord      = 27,
  SelectLine      = 28,
  SelectAll       = 29,

  /// ClipBoard
  Cut   = 30,
  Copy  = 31,
  Paste = 32,

  /// Redo/Undo
  Undo = 33,
  Redo = 34,

  /// Mouse Selection (Visual)
  Hit       = 35,
  HitSelect = 36,

  /// Insert new line
  NewLine = 37,
  Tab     = 38,

  Submit = 39
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
  CaretAlignment      caret_alignment_;
  Vec<c32>            buffer_;
  Vec<TextEditRecord> records_;
  Span<c32 const>     word_symbols_;
  Span<c32 const>     line_symbols_;

  /// @brief record representing the current text composition state;
  /// the base state is always at index 0
  usize state_;

  TextCompositor(Vec<c32> buffer, Vec<TextEditRecord> records,
                 Span<c32 const> word_symbols, Span<c32 const> line_symbols) :
    cursor_{},
    caret_alignment_{CaretAlignment::LineStart},
    buffer_{std::move(buffer)},
    records_{std::move(records)},
    word_symbols_{word_symbols},
    line_symbols_{line_symbols},
    state_{0}
  {
  }

  static TextCompositor
    create(AllocatorRef allocator, usize buffer_size = DEFAULT_BUFFER_SIZE,
           usize           records_size = DEFAULT_RECORDS_SIZE,
           Span<c32 const> word_symbols = DEFAULT_WORD_SYMBOLS,
           Span<c32 const> line_symbols = DEFAULT_LINE_SYMBOLS);

  TextCompositor(TextCompositor const &)             = delete;
  TextCompositor(TextCompositor &&)                  = default;
  TextCompositor & operator=(TextCompositor const &) = delete;
  TextCompositor & operator=(TextCompositor &&)      = default;
  ~TextCompositor()                                  = default;

  TextCursor cursor() const;

  Slice buffer_slice(Slice records) const;

  /// @brief pop the first `num` earliest records
  void pop_records(usize num);

  /// @brief truncate all records from `state + 1` to last record
  /// i.e. when editing from a present state, all redo history from that point is cleared.
  void truncate_records();

  void push_record(TextEditRecordType type, usize text_pos, Str32 erase,
                   Str32 insert);

  Option<Slice> undo(Vec<c32> & str);

  Option<Slice> redo(Vec<c32> & str);

  bool erase(Vec<c32> & str, Slice slice);

  /// @param input text from IME to insert
  bool command(RenderText & text, TextCommand cmd, Str32 input,
               ClipBoard & clipboard, usize lines_per_page, usize tab_width,
               CRect const & region, Vec2 pos, f32 zoom,
               AllocatorRef scratch_allocator);
};

}    // namespace ash
