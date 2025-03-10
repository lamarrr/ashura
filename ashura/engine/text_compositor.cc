/// SPDX-License-Identifier: MIT
#include "ashura/engine/text_compositor.h"
#include "ashura/std/error.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"
#include "ashura/std/text.h"

namespace ash
{

u32 TextCompositor::goto_line(TextLayout const & layout, u32 alignment,
                              u32 line)
{
  if (layout.lines.is_empty())
  {
    return 0;
  }

  line = min(line, layout.lines.size32() - 1);

  Line const & ln = layout.lines[line];

  if (alignment > ln.codepoints.span)
  {
    return ln.codepoints.end() - 1;
  }

  return ln.codepoints.offset + alignment;
}

void TextCompositor::pop_records(usize num)
{
  CHECK(num <= records_.size(), "");
  u32 reclaimed = 0;
  for (u32 i = 0; i < num; i++)
  {
    reclaimed += records_[i].slice.span;
  }
  mem::move(buffer_.view().slice(reclaimed), buffer_.view());
  mem::move(records_.view().slice(num), records_.view());
  buffer_usage_ -= reclaimed;
  latest_record_ -= num;
  current_record_ -= num;
}

void TextCompositor::append_record(bool is_insert, usize text_pos,
                                   Span<c32 const> segment)
{
  if (segment.size() > buffer_.size())
  {
    // clear all records as we can't insert a new record without invalidating
    // the history
    pop_records(records_.size());
    return;
  }

  while (buffer_usage_ + segment.size() > buffer_.size())
  {
    // pop half, to amortize shifting cost.
    // always pop by atleast 1. since the buffer can fit it and atleast 1
    // record would be using the available memory.
    pop_records(max(records_.size() >> 1, (usize) 1));
  }

  if (current_record_ + 1 >= records_.size())
  {
    pop_records(max(records_.size() >> 1, (usize) 1U));
  }

  mem::copy(segment, buffer_.view().slice(buffer_pos_));

  current_record_++;
  latest_record_           = current_record_;
  records_[latest_record_] = TextEditRecord{
    .slice = Slice{text_pos, segment.size()},
      .is_insert = is_insert
  };
  buffer_pos_ += segment.size();
}

void TextCompositor::undo(Insert insert, Erase erase)
{
  if (current_record_ == 0)
  {
    return;
  }
  // undo changes of current record
  TextEditRecord & record = records_[current_record_];
  buffer_pos_ -= record.slice.span;
  if (record.is_insert)
  {
    erase(record.slice);
  }
  else
  {
    insert(record.slice.offset,
           buffer_.view().slice(buffer_pos_, record.slice.span));
  }
  current_record_--;
  if (record.is_insert)
  {
    cursor_ = TextCursor::from_slice(record.slice);
  }
}

void TextCompositor::redo(Insert insert, Erase erase)
{
  if (current_record_ + 1 > latest_record_)
  {
    return;
  }
  current_record_++;
  // apply changes of next record
  TextEditRecord & record = records_[current_record_];
  if (record.is_insert)
  {
    insert(record.slice.offset,
           buffer_.view().slice(buffer_pos_, record.slice.span));
  }
  else
  {
    erase(record.slice);
  }

  buffer_pos_ += record.slice.span;
  if (!record.is_insert)
  {
    cursor_ = TextCursor::from_slice(record.slice);
  }
}

void TextCompositor::unselect()
{
  cursor_.unselect();
}

void TextCompositor::delete_selection(Span<c32 const> text, Erase erase)
{
  if (cursor_.is_empty())
  {
    return;
  }

  Slice const selection = cursor_.as_slice()(text.size());
  append_record(false, selection.offset, text.slice((Slice) selection));
  erase(selection);
  cursor_.to_first();
}

static constexpr bool is_symbol(Span<c32 const> symbols, u32 c)
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
static constexpr Slice find_boundary(Span<c32 const> text, u32 const pos,
                                     Span<c32 const> symbols)
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

static inline Slice cursor_boundary(Span<c32 const> text,
                                    Span<c32 const> symbols, TextCursor cursor)
{
  Slice selection = cursor.as_slice()(text.size());
  if (text.is_empty() || selection.is_empty())
  {
    return Slice{};
  }
  return find_boundary(text, selection.offset, symbols);
}

struct LinePosition
{
  u32 line      = 0;
  u32 alignment = 0;
};

static inline LinePosition line_translate(TextLayout const & layout, i64 cursor,
                                          i64 dy)
{
  for (u32 ln = 0; ln < layout.lines.size32(); ln++)
  {
    Line const & line = layout.lines[ln];
    if (line.codepoints.contains(cursor))
    {
      u32 const alignment = cursor - line.codepoints.offset;
      u32 const next_line =
        (u32) clamp(ln + dy, (i64) 0, (i64) (layout.lines.size32() - 1));
      return LinePosition{.line = next_line, .alignment = alignment};
    }
  }

  return LinePosition{};
}

Slice TextCompositor::command(RenderText const & text, TextCommand cmd,
                              Insert insert, Erase erase, Span<c32 const> input,
                              ClipBoard & clipboard, u32 lines_per_page,
                              CRect const & region, Vec2 pos, f32 zoom)
{
  TextBlock const block = text.block();

  // don't allocate the buffers until we receive an edit command
  switch (cmd)
  {
    case TextCommand::BackSpace:
    case TextCommand::Delete:
    case TextCommand::InputText:
    case TextCommand::Cut:
    case TextCommand::Paste:
    case TextCommand::Undo:
    case TextCommand::Redo:
    case TextCommand::NewLine:
    case TextCommand::Tab:
    {
      buffer_.resize(buffer_size_).unwrap();
      records_.resize(records_size_).unwrap();
      break;
    }

    default:
      break;
  }

  switch (cmd)
  {
    case TextCommand::Unselect:
    {
      unselect();
    }
    break;
    case TextCommand::BackSpace:
    {
      if (cursor_.is_empty())
      {
        cursor_.to_first();
        cursor_.first--;
        cursor_.clamp(block.text.size());
      }
      delete_selection(block.text, erase);
    }
    break;
    case TextCommand::Delete:
    {
      if (cursor_.is_empty())
      {
        cursor_.span = 1;
        cursor_.clamp(block.text.size());
      }
      delete_selection(block.text, erase);
    }
    break;
    case TextCommand::InputText:
    {
      Slice selection = cursor_.as_slice()(block.text.size());
      delete_selection(block.text, noop);
      append_record(true, selection.offset, input);
      erase(selection);
      insert(selection.offset, input);
      // [ ] selection.offset: advance selection offset
    }
    break;
    case TextCommand::Left:
    {
      if (!cursor_.is_empty())
      {
        cursor_.to_first();
        break;
      }

      cursor_.to_first();
      cursor_.first--;
      cursor_.clamp(block.text.size());
    }
    break;
    case TextCommand::Right:
    {
      if (!cursor_.is_empty())
      {
        cursor_.to_last();
        break;
      }

      cursor_.to_last();
      cursor_.first++;
      cursor_.clamp(block.text.size());
    }
    break;
    case TextCommand::WordStart:
    {
      Slice word    = cursor_boundary(block.text, word_symbols_, cursor_);
      cursor_.first = word.offset;
      cursor_.span  = 0;
    }
    break;
    case TextCommand::WordEnd:
    {
      Slice word = cursor_boundary(block.text, word_symbols_, cursor_);
      cursor_.first =
        (word.span == 0) ? word.offset : (word.offset + word.span - 1);
      cursor_.span = 0;
    }
    break;
    case TextCommand::LineStart:
    {
      Slice line    = cursor_boundary(block.text, line_symbols_, cursor_);
      cursor_.first = line.offset;
      cursor_.span  = 0;
    }
    break;
    case TextCommand::LineEnd:
    {
      Slice line = cursor_boundary(block.text, line_symbols_, cursor_);
      cursor_.first =
        (line.span == 0) ? line.offset : (line.offset + line.span - 1);
      cursor_.span = 0;
    }
    break;
    case TextCommand::Up:
    {
      LinePosition line = line_translate(text.layout(), cursor_.last(), -1);
      cursor_.first     = goto_line(text.layout(), line.alignment, line.line);
      cursor_.span      = 0;
    }
    break;
    case TextCommand::Down:
    {
      LinePosition line = line_translate(text.layout(), cursor_.last(), 1);
      cursor_.first     = goto_line(text.layout(), line.alignment, line.line);
      cursor_.span      = 0;
    }
    break;
    case TextCommand::PageUp:
    {
      LinePosition line =
        line_translate(text.layout(), cursor_.last(), -(i64) lines_per_page);
      cursor_.first = goto_line(text.layout(), line.alignment, line.line);
      cursor_.span  = 0;
    }
    break;
    case TextCommand::PageDown:
    {
      LinePosition line =
        line_translate(text.layout(), cursor_.last(), lines_per_page);
      cursor_.first = goto_line(text.layout(), line.alignment, line.line);
      cursor_.span  = 0;
    }
    break;
    case TextCommand::SelectLeft:
    {
      cursor_.span--;
      cursor_.clamp(block.text.size());
    }
    break;
    case TextCommand::SelectRight:
    {
      cursor_.span++;
      cursor_.clamp(block.text.size());
    }
    break;
    case TextCommand::SelectUp:
    {
      LinePosition line = line_translate(text.layout(), cursor_.last(), -1);
      cursor_.span_to(goto_line(text.layout(), line.alignment, line.line));
    }
    break;
    case TextCommand::SelectDown:
    {
      LinePosition line = line_translate(text.layout(), cursor_.last(), 1);
      cursor_.span_to(goto_line(text.layout(), line.alignment, line.line));
    }
    break;
    case TextCommand::SelectWordStart:
    {
      Slice word = cursor_boundary(block.text, word_symbols_, cursor_);
      cursor_.span_to(word.offset);
    }
    break;
    case TextCommand::SelectWordEnd:
    {
      Slice word = cursor_boundary(block.text, word_symbols_, cursor_);
      cursor_.span_to((word.span == 0) ? word.offset :
                                         (word.offset + word.span - 1));
    }
    break;
    case TextCommand::SelectLineStart:
    {
      Slice line = cursor_boundary(block.text, line_symbols_, cursor_);
      cursor_.span_to(line.offset);
    }
    break;
    case TextCommand::SelectLineEnd:
    {
      Slice line = cursor_boundary(block.text, line_symbols_, cursor_);
      cursor_.span_to((line.span == 0) ? line.offset :
                                         (line.offset + line.span - 1));
    }
    break;
    case TextCommand::SelectPageUp:
    {
      LinePosition line =
        line_translate(text.layout(), cursor_.last(), lines_per_page);
      cursor_.span_to(goto_line(text.layout(), line.alignment, line.line));
    }
    break;
    case TextCommand::SelectPageDown:
    {
      LinePosition line =
        line_translate(text.layout(), cursor_.last(), lines_per_page);
      cursor_.span_to(goto_line(text.layout(), line.alignment, line.line));
    }
    break;
    case TextCommand::SelectCodepoint:
    {
      cursor_.span = 1;
      cursor_      = cursor_.clamp(block.text.size());
    }
    break;
    case TextCommand::SelectWord:
    {
      Slice word = cursor_boundary(block.text, word_symbols_, cursor_);
      cursor_    = TextCursor::from_slice(word);
    }
    break;
    case TextCommand::SelectLine:
    {
      Slice line = cursor_boundary(block.text, line_symbols_, cursor_);
      cursor_    = TextCursor::from_slice(line);
    }
    break;
    case TextCommand::SelectAll:
    {
      cursor_.first = 0;
      cursor_.span  = block.text.size();
    }
    break;
    case TextCommand::Cut:
    {
      Vec<c8> data_u8{allocator_};
      utf8_encode(block.text.slice(cursor_.as_slice()), data_u8).unwrap();
      delete_selection(block.text, erase);
      clipboard.set_text(data_u8.view().as_u8()).unwrap();
    }
    break;
    case TextCommand::Copy:
    {
      Vec<c8> data_u8{allocator_};
      utf8_encode(block.text.slice(cursor_.as_slice()), data_u8).unwrap();
      clipboard.set_text(data_u8.view().as_u8()).unwrap();
    }
    break;
    case TextCommand::Paste:
    {
      Vec<c32> data_u32{allocator_};
      Vec<u8>  data_u8{allocator_};
      clipboard.get_text(data_u8).unwrap();
      utf8_decode(data_u8.view().as_c8(), data_u32).unwrap();
      Slice selection = cursor_.as_slice()(block.text.size());
      delete_selection(block.text, noop);
      append_record(true, selection.offset, data_u32);
      erase(selection);
      insert(selection.offset, data_u32);
    }
    break;
    case TextCommand::Undo:
    {
      undo(insert, erase);
    }
    break;
    case TextCommand::Redo:
    {
      redo(insert, erase);
    }
    break;
    case TextCommand::Hit:
    {
      TextHitResult const hit =
        text.hit(region, pos, zoom).unwrap_or(TextHitResult{});
      cursor_.first = hit.cluster;
      cursor_.span  = 0;
    }
    break;
    case TextCommand::HitSelect:
    {
      TextHitResult const hit =
        text.hit(region, pos, zoom).unwrap_or(TextHitResult{});
      cursor_.span_to(hit.cluster);
    }
    break;
    case TextCommand::NewLine:
    {
      Span<c32 const> input     = U"\n"_str;
      Slice           selection = cursor_.as_slice()(block.text.size());
      delete_selection(block.text, noop);
      append_record(true, selection.offset, input);
      erase(selection);
      insert(selection.offset, input);
    }
    break;
    case TextCommand::Tab:
    {
      static constexpr c32 TAB_STRING[] = {'\t', '\t', '\t', '\t',
                                           '\t', '\t', '\t', '\t'};
      Span<c32 const>      input        = span(TAB_STRING).slice(0, tab_width_);
      Slice                selection    = cursor_.as_slice()(block.text.size());
      delete_selection(block.text, noop);
      append_record(true, selection.offset, input);
      erase(selection);
      insert(selection.offset, input);
    }
    break;
    case TextCommand::None:
    default:
      break;
  }

  return cursor_.as_slice();
}

}    // namespace ash
