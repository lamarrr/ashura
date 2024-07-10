#pragma once
#include "ashura/engine/text.h"
#include "ashura/std/types.h"

namespace ash
{

struct TextHitResult
{
  u32 cluster = 0;
  u32 line    = 0;
};

/// @brief given a position in the laid-out text return the location of the
/// grapheme the cursor points to. returns the last column if the position
/// overlaps with the row and returns the last line if no overlap was found.
/// @param pos position in laid-out text to return from.
TextHitResult hit_text(TextLayout const &layout, Vec2 pos);

/// @brief given a cluser index in the text, get the line number the cluster
/// belongs to, otherwise return U32_MAX.
/// @param cluster cluster to
/// @return
u32 find_cluster_line(TextLayout const &layout, u32 cluster);

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

enum class TextCommand : u8
{
  Escape          = 0,
  BackSpace       = 1,
  Delete          = 2,
  Left            = 3,
  Right           = 4,
  Up              = 5,
  Down            = 6,
  WordStart       = 7,
  WordEnd         = 8,
  LineStart       = 9,
  LineEnd         = 10,
  PageUp          = 11,
  PageDown        = 12,
  Cut             = 13,
  Copy            = 14,
  Paste           = 15,
  Undo            = 16,
  Redo            = 17,
  SelectCodepoint = 18,
  SelectWord      = 19,
  SelectLine      = 20,
  SelectAll       = 21
};

/// @brief A simple stack-based text compositor
/// @param selection_first first codepoint in the selection range. not always a
/// valid index into the text.
/// @param selection_last last (inclusive) codepoint in the selection range,
/// this is where the cursor was last at, i.e. when dragging or highlighting.
/// not always a valid index into the text.
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
  static constexpr u32 DEFAULT_LINE_SYMBOLS[] = {'\n'};

  Slice32             selection      = {0, 0};
  u32                 cursor         = 0;
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
    selection = {0, 0};
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
  void goto_line(TextLayout const &layout, u32 line);

  void pop_records(u32 num);

  void append_record(bool is_insert, u32 text_pos, Span<u32 const> segment);

  void undo(Insert insert, Erase erase);

  void redo(Insert insert, Erase erase);

  void delete_selection(Span<u32 const> text, Insert insert, Erase erase);

  /// @brief IME-text input
  /// @param text original text
  /// @param input text from IME to insert
  void input_text(Span<u32 const> text, Span<u32 const> input, Insert insert,
                  Erase erase);

  void drag(TextLayout const &layout, Vec2 pos);

  void update_cursor();

  void select_codepoint(Span<u32 const> text);

  void select_word(Span<u32 const> text);

  void select_line(Span<u32 const> text);

  void select_all(Span<u32 const> text);

  /// @brief: on mouse press, move the cursor to the clicked location, and reset
  /// the selection
  void single_click(TextLayout const &layout, Vec2 pos);

  void double_click(Span<u32 const> text, TextLayout const &layout, Vec2 pos);

  void triple_click(Span<u32 const> text, TextLayout const &layout, Vec2 pos);

  void quad_click(Span<u32 const> text, TextLayout const &layout, Vec2 pos);

  void click(Span<u32 const> text, TextLayout const &layout, u32 num_clicks,
             Vec2 pos);

  void up(TextLayout const &layout);

  void down(TextLayout const &layout);

  void left();

  void right();

  void page_up(TextLayout const &layout, u32 lines_per_page);

  void page_down(TextLayout const &layout, u32 lines_per_page);

  void escape();

  void command(Span<u32 const> text, TextLayout const &layout, TextCommand cmd,
               Insert insert, Erase erase, Fn<Span<u32 const>()> get_content,
               Fn<void(Span<u32 const>)> set_content, u32 lines_per_page);
};

}        // namespace ash
