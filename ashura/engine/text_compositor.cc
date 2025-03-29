/// SPDX-License-Identifier: MIT
#include "ashura/engine/text_compositor.h"
#include "ashura/std/error.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"
#include "ashura/std/text.h"

namespace ash
{

TextCompositor TextCompositor::create(AllocatorRef allocator, usize buffer_size,
                                      usize records_size, Str32 word_symbols,
                                      Str32 line_symbols)
{
  CHECK(buffer_size > 1, "");
  CHECK(records_size > 1, "");

  Vec<c32> buffer{allocator};

  buffer.reserve(buffer_size).unwrap();

  Vec<TextEditRecord> records{allocator};

  records.reserve(records_size).unwrap();
  records
    .push(TextEditRecord{.text_pos    = 0,
                         .erase_size  = 0,
                         .insert_size = 0,
                         .type        = TextEditRecordType::Erase})
    .unwrap();

  return TextCompositor{std::move(buffer), std::move(records), word_symbols,
                        line_symbols};
}

usize TextCompositor::goto_line(TextLayout const & layout, usize alignment,
                                usize line)
{
  if (layout.lines.is_empty())
  {
    return 0;
  }

  line = min(line, layout.lines.size() - 1);

  auto const & ln = layout.lines[line];

  if (alignment > ln.codepoints.span)
  {
    return ln.codepoints.end() - 1;
  }

  return ln.codepoints.offset + alignment;
}

TextCursor TextCompositor::cursor() const
{
  return cursor_;
}

void TextCompositor::pop_records(usize num)
{
  CHECK(!records_.is_empty(), "");
  CHECK(num <= records_.size(), "");

  // exclude the first entry
  Slice erase;

  if (num > state_)
  {
    // history will need to be cleared.
    // reset to base state and clear history. exclude base record.
    erase  = {1, USIZE_MAX};
    state_ = 0;
  }
  else
  {
    // clear the specified top `num` entries. exclude the base record.
    erase = {1, num};
    state_ -= num;
  }

  erase                   = erase(records_.size());
  auto const buffer_erase = buffer_slice(erase);

  records_.erase(erase);
  buffer_.erase(buffer_erase);
}

Slice TextCompositor::buffer_slice(Slice records) const
{
  Slice slice;
  for (usize i = 0; i < records.offset; i++)
  {
    slice.offset += records_[i].buffer_usage();
  }

  for (usize i = records.offset; i < records.end(); i++)
  {
    slice.span += records_[i].buffer_usage();
  }

  return slice;
}

void TextCompositor::truncate_records()
{
  usize      reclaimed = 0;
  auto const first     = state_ + 1;
  for (usize i = first; i < records_.size(); i++)
  {
    reclaimed += records_[i].buffer_usage();
  }
  buffer_.erase(reclaimed, USIZE_MAX);
  records_.erase(first, USIZE_MAX);
}

void TextCompositor::push_record(TextEditRecordType type, usize text_pos,
                                 Str32 erase, Str32 insert)
{
  auto const total_size = insert.size() + erase.size();
  if (total_size > buffer_.capacity())
  {
    // clear all records as we can't insert a new record without invalidating
    // the history
    pop_records(records_.size());
    return;
  }

  // try to allocate buffer space
  while ((buffer_.size() + total_size) > buffer_.capacity())
  {
    // pop half, to amortize shifting cost.
    // always pop by atleast 1. since the buffer can fit it and atleast 1
    // record would be using the available memory.
    auto const num_pop = max(records_.size() >> 1, (usize) 1);
    pop_records(num_pop);
  }

  // try to allocate record entries
  if ((records_.size() + 1) > records_.capacity())
  {
    auto const num_pop = max(records_.size() >> 1, (usize) 1);
    pop_records(num_pop);
  }

  truncate_records();

  buffer_.extend(erase).unwrap();
  buffer_.extend(insert).unwrap();

  auto const idx = records_.size();

  records_
    .push(TextEditRecord{.text_pos    = text_pos,
                         .erase_size  = erase.size(),
                         .insert_size = insert.size(),
                         .type        = type})
    .unwrap();

  state_ = idx;
}

bool TextCompositor::undo(Vec<c32> & str)
{
  if (state_ == 0)
  {
    return false;
  }

  // undo changes of current record
  auto const & record = records_[state_];
  auto const   slice  = buffer_slice(Slice{state_, 1});

  switch (record.type)
  {
    case TextEditRecordType::Erase:
    {
      str
        .insert_span(record.text_pos,
                     buffer_.view().slice(slice.offset, record.erase_size))
        .unwrap();
      cursor_ = TextCursor::from_slice({record.text_pos, record.erase_size});
    }
    break;
    case TextEditRecordType::Insert:
    {
      str.erase(record.text_pos, record.insert_size);
      cursor_ = TextCursor::from_slice({record.text_pos, 0});
    }
    break;
    case TextEditRecordType::Replace:
    {
      str.erase(record.text_pos, record.insert_size);
      str
        .insert_span(record.text_pos,
                     buffer_.view().slice(slice.offset, record.erase_size))
        .unwrap();
      cursor_ = TextCursor::from_slice({record.text_pos, record.erase_size});
    }
    break;
  }

  state_--;

  return true;
}

bool TextCompositor::redo(Vec<c32> & str)
{
  if ((state_ + 1) > records_.size())
  {
    return false;
  }

  state_++;

  // apply changes of next record
  auto const & record = records_[state_];
  auto const   slice  = buffer_slice(Slice{state_, 1});

  switch (record.type)
  {
    case TextEditRecordType::Erase:
    {
      str.erase(record.text_pos, record.erase_size);
      cursor_ = TextCursor::from_slice({record.text_pos, 0});
    }
    break;
    case TextEditRecordType::Insert:
    {
      str
        .insert_span(record.text_pos,
                     buffer_.view().slice(slice.offset, record.insert_size))
        .unwrap();
      cursor_ = TextCursor::from_slice({record.text_pos, record.insert_size});
    }
    break;
    case TextEditRecordType::Replace:
    {
      str.erase(record.text_pos, record.erase_size);
      str
        .insert_span(record.text_pos,
                     buffer_.view().slice(slice.offset + record.erase_size,
                                          record.insert_size))
        .unwrap();
      cursor_ = TextCursor::from_slice({record.text_pos, record.insert_size});
    }
    break;
  }

  return true;
}

bool TextCompositor::delete_selection(Vec<c32> & str)
{
  if (str.is_empty())
  {
    return false;
  }

  auto const selection = cursor_.as_slice()(str.size());
  push_record(TextEditRecordType::Erase, selection.offset,
              str.view().slice(selection), {});
  str.erase(selection);

  return true;
}

static constexpr bool is_symbol(Str32 symbols, u32 c)
{
  return !find(symbols, c).is_empty();
}

/// @brief given a sequence of text and symbols find the symbol boundary from
/// a given codepoint index.
/// @param text must be non-empty
/// @param pos must be valid index into the text
/// @param symbols additional unicode symbols that specify word boundaries,
/// i.e. a code editor may use '(',')' as word boundaries.
/// @param[out] first index of first char in symbolic boundary
/// @param[out] last index of last char in symbolic boundary
static constexpr Slice find_boundary(Str32 text, usize const pos, Str32 symbols)
{
  usize fwd = pos;
  usize bwd = pos;

  if (is_symbol(symbols, text[pos]))
  {
    while (fwd < text.size() && is_symbol(symbols, text[fwd]))
    {
      fwd++;
    }

    while (bwd != 0 && is_symbol(symbols, text[bwd]))
    {
      bwd--;
    }
  }
  else
  {
    while (fwd < text.size() && !is_symbol(symbols, text[fwd]))
    {
      fwd++;
    }

    while (bwd != 0 && !is_symbol(symbols, text[bwd]))
    {
      bwd--;
    }
  }

  bwd = (bwd == 0) ? bwd : (bwd + 1);
  fwd = fwd - 1;

  return Slice{.offset = bwd, .span = (fwd - bwd) + 1};
}

static inline Slice cursor_boundary(Str32 text, Str32 symbols,
                                    TextCursor cursor)
{
  auto const selection = cursor.as_slice()(text.size());
  if (text.is_empty() || selection.is_empty())
  {
    return Slice{};
  }
  return find_boundary(text, selection.offset, symbols);
}

struct LinePosition
{
  usize line      = 0;
  usize alignment = 0;
};

static LinePosition line_translate(TextLayout const & layout, i64 cursor,
                                   i64 dy)
{
  for (auto [i, ln] : enumerate(layout.lines))
  {
    if (ln.codepoints.contains(cursor))
    {
      auto const alignment = cursor - ln.codepoints.offset;
      auto const next_line =
        (usize) clamp((i64) i + dy, (i64) 0, (i64) (layout.lines.size() - 1));
      return LinePosition{.line = next_line, .alignment = alignment};
    }
  }

  return LinePosition{};
}

// [ ] test all
Tuple<bool, Slice> TextCompositor::command(
  RenderText & text, TextCommand cmd, Str32 keyboard_input,
  ClipBoard & clipboard, usize lines_per_page, usize tab_width,
  CRect const & region, Vec2 pos, f32 zoom, AllocatorRef scratch_allocator)
{
  CHECK(tab_width <= MAX_TAB_WIDTH, "");
  cursor_.normalize(text.text_.size());

  bool modified = false;

  switch (cmd)
  {
    case TextCommand::Unselect:
    {
      cursor_.to_begin();
    }
    break;
    case TextCommand::BackSpace:
    {
      if (!cursor_.has_selection())
      {
        cursor_.shift(-1).span_by(1);
      }
      modified |= delete_selection(text.text_);
      cursor_.to_begin();
    }
    break;
    case TextCommand::Delete:
    {
      if (!cursor_.has_selection())
      {
        cursor_.span_by(1);
      }
      modified |= delete_selection(text.text_);
      cursor_.to_begin();
    }
    break;
    case TextCommand::Paste:
    case TextCommand::InputText:
    case TextCommand::NewLine:
    case TextCommand::Tab:
    {
      Vec<c32> data_u32{scratch_allocator};
      Vec<u8>  data_u8{scratch_allocator};

      Str32                          input;
      InplaceVec<c32, MAX_TAB_WIDTH> tabs;

      if (cmd == TextCommand::Paste)
      {
        clipboard.get_text(data_u8).unwrap();
        utf8_decode(data_u8.view().as_c8(), data_u32).unwrap();
        input = data_u32;
      }
      else if (cmd == TextCommand::InputText)
      {
        input = keyboard_input;
      }
      else if (cmd == TextCommand::NewLine)
      {
        input = U"\n"_str;
      }
      else if (cmd == TextCommand::Tab)
      {
        for (usize i = 0; i < tab_width; i++)
        {
          tabs.push(U'\t').unwrap();
        }

        input = tabs;
      }

      if (!input.is_empty())
      {
        auto const selection = cursor_.as_slice()(text.text_.size());
        if (cursor_.has_selection())
        {
          push_record(TextEditRecordType::Replace, selection.offset,
                      text.text_.view().slice(selection), input);
        }
        else
        {
          push_record(TextEditRecordType::Insert, selection.offset, {}, input);
        }
        text.text_.erase(selection);
        text.text_.insert_span(selection.offset, input).unwrap();
        cursor_.span_by((i64) input.size()).to_end();
        modified = true;
      }
    }
    break;
    case TextCommand::Left:
    {
      if (cursor_.has_selection())
      {
        cursor_.to_left();
      }
      else
      {
        cursor_.shift(-1);
      }
      // [ ] caret
    }
    break;
    case TextCommand::Right:
    {
      if (cursor_.has_selection())
      {
        cursor_.to_right();
      }
      else
      {
        cursor_.shift(1);
      }
      // [ ] caret
    }
    break;
    case TextCommand::WordStart:
    {
      auto const word = cursor_boundary(text.text_, word_symbols_, cursor_);
      cursor_         = TextCursor::from_slice({word.offset, 0});
    }
    break;
    case TextCommand::WordEnd:
    {
      auto const word = cursor_boundary(text.text_, word_symbols_, cursor_);
      cursor_         = TextCursor::from_slice({word.end(), 0});
    }
    break;
    case TextCommand::LineStart:
    {
      auto const line = cursor_boundary(text.text_, line_symbols_, cursor_);
      cursor_         = TextCursor::from_slice({line.offset, 0});
    }
    break;
    case TextCommand::LineEnd:
    {
      auto const line = cursor_boundary(text.text_, line_symbols_, cursor_);
      cursor_         = TextCursor::from_slice({line.end(), 0});
    }
    break;
    case TextCommand::Up:
    {
      auto const line = line_translate(text.layout(), cursor_.end, -1);
      auto const pos  = goto_line(text.layout(), line.alignment, line.line);
      cursor_         = TextCursor::from_slice({pos, 0});
    }
    break;
    case TextCommand::Down:
    {
      auto const line = line_translate(text.layout(), cursor_.end, 1);
      auto const pos  = goto_line(text.layout(), line.alignment, line.line);
      cursor_         = TextCursor::from_slice({pos, 0});
    }
    break;
    case TextCommand::PageUp:
    {
      auto const line =
        line_translate(text.layout(), cursor_.end, -(i64) lines_per_page);
      auto const pos = goto_line(text.layout(), line.alignment, line.line);
      cursor_        = TextCursor::from_slice({pos, 0});
    }
    break;
    case TextCommand::PageDown:
    {
      auto const line =
        line_translate(text.layout(), cursor_.end, lines_per_page);
      auto const pos = goto_line(text.layout(), line.alignment, line.line);
      cursor_        = TextCursor::from_slice({pos, 0});
    }
    break;
    case TextCommand::SelectLeft:
    {
      cursor_.span_by(cursor_.distance() - 1);
    }
    break;
    case TextCommand::SelectRight:
    {
      cursor_.span_by(cursor_.distance() + 1);
    }
    break;
    case TextCommand::SelectUp:
    {
      auto const line = line_translate(text.layout(), cursor_.end, -1);
      cursor_.span_to(goto_line(text.layout(), line.alignment, line.line));
    }
    break;
    case TextCommand::SelectDown:
    {
      auto const line = line_translate(text.layout(), cursor_.end, 1);
      cursor_.span_to(goto_line(text.layout(), line.alignment, line.line));
    }
    break;
    case TextCommand::SelectWordStart:
    {
      auto const word = cursor_boundary(text.text_, word_symbols_, cursor_);
      cursor_.span_to(word.offset);
    }
    break;
    case TextCommand::SelectWordEnd:
    {
      auto const word = cursor_boundary(text.text_, word_symbols_, cursor_);
      cursor_.span_to((word.span == 0) ? word.offset :
                                         (word.offset + word.span - 1));
    }
    break;
    case TextCommand::SelectLineStart:
    {
      auto const line = cursor_boundary(text.text_, line_symbols_, cursor_);
      cursor_.span_to(line.offset);
    }
    break;
    case TextCommand::SelectLineEnd:
    {
      auto const line = cursor_boundary(text.text_, line_symbols_, cursor_);
      cursor_.span_to((line.span == 0) ? line.offset :
                                         (line.offset + line.span - 1));
    }
    break;
    case TextCommand::SelectPageUp:
    {
      auto const line =
        line_translate(text.layout(), cursor_.end, lines_per_page);
      cursor_.span_to(goto_line(text.layout(), line.alignment, line.line));
    }
    break;
    case TextCommand::SelectPageDown:
    {
      auto const line =
        line_translate(text.layout(), cursor_.end, lines_per_page);
      cursor_.span_to(goto_line(text.layout(), line.alignment, line.line));
    }
    break;
    case TextCommand::SelectCodepoint:
    {
      cursor_.span_by(1);
    }
    break;
    case TextCommand::SelectWord:
    {
      auto const word = cursor_boundary(text.text_, word_symbols_, cursor_);
      cursor_         = TextCursor::from_slice(word);
    }
    break;
    case TextCommand::SelectLine:
    {
      auto const line = cursor_boundary(text.text_, line_symbols_, cursor_);
      cursor_         = TextCursor::from_slice(line);
    }
    break;
    case TextCommand::SelectAll:
    {
      cursor_ = TextCursor::from_slice({0, text.text_.size()});
    }
    break;
    case TextCommand::Cut:
    {
      Vec<c8> data_u8{scratch_allocator};
      utf8_encode(text.text_.view().slice(cursor_.as_slice()), data_u8)
        .unwrap();
      delete_selection(text.text_);
      clipboard.set_text(data_u8.view().as_u8()).unwrap();
    }
    break;
    case TextCommand::Copy:
    {
      Vec<c8> data_u8{scratch_allocator};
      utf8_encode(text.text_.view().slice(cursor_.as_slice()), data_u8)
        .unwrap();
      clipboard.set_text(data_u8.view().as_u8()).unwrap();
    }
    break;
    case TextCommand::Undo:
    {
      modified |= undo(text.text_);
    }
    break;
    case TextCommand::Redo:
    {
      modified |= redo(text.text_);
    }
    break;
    case TextCommand::Hit:
    {
      auto const hit = text.hit(region, pos, zoom).unwrap_or(TextHitResult{});
      cursor_        = TextCursor::from_slice({hit.cluster, 0});
      // [ ] caret
    }
    break;
    case TextCommand::HitSelect:
    {
      auto const hit = text.hit(region, pos, zoom).unwrap_or(TextHitResult{});
      cursor_.span_to(hit.cluster);
      // [ ] next
    }
    break;

    case TextCommand::None:
    default:
      break;
  }

  cursor_.normalize(text.text_.size());

  return {modified, cursor_.as_slice()};
}

}    // namespace ash
