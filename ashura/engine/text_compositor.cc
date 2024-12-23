/// SPDX-License-Identifier: MIT
#include "ashura/engine/text_compositor.h"
#include "ashura/std/error.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"
#include "ashura/std/text.h"

namespace ash
{

void TextCompositor::pop_records(u32 num)
{
  CHECK(num <= records_.size32());
  u32 reclaimed = 0;
  for (u32 i = 0; i < num; i++)
  {
    reclaimed += records_[i].slice.span;
  }
  mem::move(span(buffer_).slice(reclaimed), span(buffer_));
  mem::move(span(records_).slice(num), span(records_));
  buffer_usage_ -= reclaimed;
  latest_record_ -= num;
  current_record_ -= num;
}

void TextCompositor::append_record(bool is_insert, u32 text_pos,
                                   Span<c32 const> segment)
{
  if (segment.size32() > buffer_.size32())
  {
    // clear all records as we can't insert a new record without invalidating
    // the history
    pop_records(records_.size32());
    return;
  }

  while (buffer_usage_ + segment.size32() > buffer_.size32())
  {
    // pop half, to amortize shifting cost.
    // always pop by atleast 1. since the buffer can fit it and atleast 1
    // record would be using the available memory.
    pop_records(max(records_.size32() >> 1, 1U));
  }

  if (current_record_ + 1 >= records_.size32())
  {
    pop_records(max(records_.size32() >> 1, 1U));
  }

  mem::copy(segment, span(buffer_).slice(buffer_pos_));

  current_record_++;
  latest_record_           = current_record_;
  records_[latest_record_] = TextEditRecord{
    .slice = Slice32{text_pos, segment.size32()},
      .is_insert = is_insert
  };
  buffer_pos_ += segment.size32();
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
           span(buffer_).slice(buffer_pos_, record.slice.span));
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
           span(buffer_).slice(buffer_pos_, record.slice.span));
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
  cursor_ = cursor_.unselect();
}

void TextCompositor::delete_selection(Span<c32 const> text, Erase erase)
{
  if (cursor_.is_empty())
  {
    return;
  }

  Slice32 const selection = cursor_.as_slice(text.size32());
  append_record(false, selection.offset, text.slice(selection));
  erase(selection);
  cursor_ = cursor_.to_begin();
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
static constexpr Slice32 find_boundary(Span<c32 const> text, u32 const pos,
                                       Span<c32 const> symbols)
{
  u32 fwd = pos;
  u32 bwd = pos;

  if (is_symbol(symbols, text[pos]))
  {
    while (fwd < text.size32() && is_symbol(symbols, text[fwd]))
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
    while (fwd < text.size32() && !is_symbol(symbols, text[fwd]))
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

  return Slice32{.offset = bwd, .span = (fwd - bwd) + 1};
}

static inline Slice32 cursor_boundary(Span<c32 const> text,
                                      Span<c32 const> symbols,
                                      TextCursor      cursor)
{
  Slice32 selection = cursor.as_slice(text.size32());
  if (text.is_empty() || selection.is_empty())
  {
    return Slice32{};
  }
  return find_boundary(text, selection.offset, symbols);
}

struct LinePosition
{
  u32 line      = 0;
  u32 alignment = 0;
};

static inline LinePosition line_translate(TextLayout const & layout, u32 cursor,
                                          i64 dy)
{
  if (layout.lines.is_empty())
  {
    return LinePosition{};
  }

  for (u32 ln = 0; ln < layout.lines.size32(); ln++)
  {
    Line const & line = layout.lines[ln];
    if (line.first_codepoint <= cursor &&
        (line.first_codepoint + line.num_codepoints) > cursor)
    {
      u32 const alignment = cursor - line.first_codepoint;
      u32 const next_line =
        (u32) clamp(ln + dy, (i64) 0, (i64) (layout.lines.size32() - 1));
      return LinePosition{.line = next_line, .alignment = alignment};
    }
  }

  return LinePosition{};
}

void TextCompositor::command(Span<c32 const> text, TextLayout const & layout,
                             f32 align_width, f32 alignment, TextCommand cmd,
                             Insert insert, Erase erase, Span<c32 const> input,
                             ClipBoard & clipboard, u32 lines_per_page,
                             Vec2 pos)
{
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
        cursor_ = cursor_.to_begin();
        cursor_.first--;
        cursor_ = cursor_.clamp(text.size32());
      }
      delete_selection(text, erase);
    }
    break;
    case TextCommand::Delete:
    {
      if (cursor_.is_empty())
      {
        cursor_.last++;
        cursor_ = cursor_.clamp(text.size32());
      }
      delete_selection(text, erase);
    }
    break;
    case TextCommand::InputText:
    {
      Slice32 selection = cursor_.as_slice(text.size32());
      delete_selection(text, noop);
      append_record(true, selection.offset, input);
      erase(selection);
      insert(selection.offset, input);
    }
    break;
    case TextCommand::Left:
    {
      if (!cursor_.is_empty())
      {
        cursor_ = cursor_.to_begin();
        break;
      }

      cursor_       = cursor_.to_begin();
      cursor_.first = --cursor_.last;
      cursor_       = cursor_.clamp(text.size32());
    }
    break;
    case TextCommand::Right:
    {
      if (!cursor_.is_empty())
      {
        cursor_ = cursor_.to_end();
        return;
      }

      cursor_       = cursor_.to_end();
      cursor_.first = ++cursor_.last;
      cursor_       = cursor_.clamp(text.size32());
    }
    break;
    case TextCommand::WordStart:
    {
      Slice32 word  = cursor_boundary(text, word_symbols_, cursor_);
      cursor_.first = cursor_.last = word.offset;
    }
    break;
    case TextCommand::WordEnd:
    {
      Slice32 word  = cursor_boundary(text, word_symbols_, cursor_);
      cursor_.first = cursor_.last =
        (word.span == 0) ? word.offset : (word.offset + word.span - 1);
    }
    break;
    case TextCommand::LineStart:
    {
      Slice32 line  = cursor_boundary(text, line_symbols_, cursor_);
      cursor_.first = cursor_.last = line.offset;
    }
    break;
    case TextCommand::LineEnd:
    {
      Slice32 line  = cursor_boundary(text, line_symbols_, cursor_);
      cursor_.first = cursor_.last =
        (line.span == 0) ? line.offset : (line.offset + line.span - 1);
    }
    break;
    case TextCommand::Up:
    {
      LinePosition line = line_translate(layout, (u32) cursor_.last, -1);
      cursor_.first     = cursor_.last =
        goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::Down:
    {
      LinePosition line = line_translate(layout, (u32) cursor_.last, 1);
      cursor_.first     = cursor_.last =
        goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::PageUp:
    {
      LinePosition line =
        line_translate(layout, (u32) cursor_.last, -(i64) lines_per_page);
      cursor_.first = cursor_.last =
        goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::PageDown:
    {
      LinePosition line =
        line_translate(layout, (u32) cursor_.last, lines_per_page);
      cursor_.first = cursor_.last =
        goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::SelectLeft:
    {
      cursor_.last--;
      cursor_ = cursor_.clamp(text.size32());
    }
    break;
    case TextCommand::SelectRight:
    {
      cursor_.last++;
      cursor_ = cursor_.clamp(text.size32());
    }
    break;
    case TextCommand::SelectUp:
    {
      LinePosition line = line_translate(layout, (u32) cursor_.last, -1);
      cursor_.last      = goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::SelectDown:
    {
      LinePosition line = line_translate(layout, (u32) cursor_.last, 1);
      cursor_.last      = goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::SelectWordStart:
    {
      Slice32 word = cursor_boundary(text, word_symbols_, cursor_);
      cursor_.last = word.offset;
    }
    break;
    case TextCommand::SelectWordEnd:
    {
      Slice32 word = cursor_boundary(text, word_symbols_, cursor_);
      cursor_.last =
        (word.span == 0) ? word.offset : (word.offset + word.span - 1);
    }
    break;
    case TextCommand::SelectLineStart:
    {
      Slice32 line = cursor_boundary(text, line_symbols_, cursor_);
      cursor_.last = line.offset;
    }
    break;
    case TextCommand::SelectLineEnd:
    {
      Slice32 line = cursor_boundary(text, line_symbols_, cursor_);
      cursor_.last =
        (line.span == 0) ? line.offset : (line.offset + line.span - 1);
    }
    break;
    case TextCommand::SelectPageUp:
    {
      LinePosition line =
        line_translate(layout, (u32) cursor_.last, lines_per_page);
      cursor_.last = goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::SelectPageDown:
    {
      LinePosition line =
        line_translate(layout, (u32) cursor_.last, lines_per_page);
      cursor_.last = goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::SelectCodepoint:
    {
      cursor_.first = cursor_.last++;
      cursor_       = cursor_.clamp(text.size32());
    }
    break;
    case TextCommand::SelectWord:
    {
      Slice32 word  = cursor_boundary(text, word_symbols_, cursor_);
      cursor_.first = word.offset;
      cursor_.last =
        (word.span == 0) ? word.offset : (word.offset + word.span - 1);
    }
    break;
    case TextCommand::SelectLine:
    {
      Slice32 line  = cursor_boundary(text, line_symbols_, cursor_);
      cursor_.first = line.offset;
      cursor_.last =
        (line.span == 0) ? line.offset : (line.offset + line.span - 1);
    }
    break;
    case TextCommand::SelectAll:
    {
      cursor_.first = 0;
      cursor_.last  = text.size32();
    }
    break;
    case TextCommand::Cut:
    {
      Vec<c8> data_u8{allocator_};
      utf8_encode(text.slice(cursor_.as_slice(text.size32())), data_u8)
        .unwrap();
      delete_selection(text, erase);
      clipboard.set_text(data_u8).unwrap();
    }
    break;
    case TextCommand::Copy:
    {
      Vec<c8> data_u8{allocator_};
      utf8_encode(text.slice(cursor_.as_slice(text.size32())), data_u8)
        .unwrap();
      clipboard.set_text(data_u8).unwrap();
    }
    break;
    case TextCommand::Paste:
    {
      Vec<c32> data_u32{allocator_};
      Vec<c8>  data_u8{allocator_};
      clipboard.get_text(data_u8).unwrap();
      utf8_decode(data_u8, data_u32).unwrap();
      Slice32 selection = cursor_.as_slice(text.size32());
      delete_selection(text, noop);
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
      TextHitResult const hit = hit_text(layout, align_width, alignment, pos);
      cursor_.first = cursor_.last = hit.cluster;
    }
    break;
    case TextCommand::HitSelect:
    {
      TextHitResult const hit = hit_text(layout, align_width, alignment, pos);
      cursor_.last            = hit.cluster;
    }
    break;
    case TextCommand::NewLine:
    {
      Span<c32 const> input     = U"\n"_str;
      Slice32         selection = cursor_.as_slice(text.size32());
      delete_selection(text, noop);
      append_record(true, selection.offset, input);
      erase(selection);
      insert(selection.offset, input);
    }
    break;
    case TextCommand::Tab:
    {
      Span<c32 const> input     = span(TAB_STRING).slice(0, tab_width_);
      Slice32         selection = cursor_.as_slice(text.size32());
      delete_selection(text, noop);
      append_record(true, selection.offset, input);
      erase(selection);
      insert(selection.offset, input);
    }
    break;
    case TextCommand::None:
    default:
      break;
  }
}

}    // namespace ash
