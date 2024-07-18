/// SPDX-License-Identifier: MIT
#include "ashura/engine/text_compositor.h"
#include "ashura/std/error.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"

namespace ash
{

static inline u32 goto_line(TextLayout const &layout, u32 alignment, u32 line)
{
  if (layout.lines.is_empty())
  {
    return 0;
  }

  line = min(line, layout.lines.size32() - 1);

  Line const &ln = layout.lines[line];

  if (alignment > ln.num_codepoints)
  {
    return ln.first_codepoint + (ln.num_codepoints - 1);
  }

  return ln.first_codepoint + alignment;
}

void TextCompositor::pop_records(u32 num)
{
  CHECK(num <= records.size32());
  u32 reclaimed = 0;
  for (u32 i = 0; i < num; i++)
  {
    reclaimed += records[i].slice.span;
  }
  mem::move(span(buffer).slice(reclaimed).as_const(), span(buffer));
  mem::move(span(records).slice(num).as_const(), span(records));
  buffer_usage -= reclaimed;
  latest_record -= num;
  current_record -= num;
}

void TextCompositor::append_record(bool is_insert, u32 text_pos,
                                   Span<u32 const> segment)
{
  if (segment.size32() > buffer.size32())
  {
    // clear all records as we can't insert a new record without invalidating
    // the history
    pop_records(records.size32());
    return;
  }

  while (buffer_usage + segment.size32() > buffer.size32())
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

  mem::copy(segment, span(buffer).slice(buffer_pos));

  current_record++;
  latest_record          = current_record;
  records[latest_record] = TextEditRecord{
      .slice = Slice32{text_pos, segment.size32()}, .is_insert = is_insert};
  buffer_pos += segment.size32();
}

void TextCompositor::undo(Insert insert, Erase erase)
{
  if (current_record == 0)
  {
    return;
  }
  // undo changes of current record
  TextEditRecord &record = records[current_record];
  buffer_pos -= record.slice.span;
  if (record.is_insert)
  {
    erase(record.slice);
  }
  else
  {
    insert(record.slice.offset,
           span(buffer).slice(buffer_pos, record.slice.span));
  }
  current_record--;
  if (record.is_insert)
  {
    cursor = TextCursor::from_slice(record.slice);
  }
}

void TextCompositor::redo(Insert insert, Erase erase)
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
    insert(record.slice.offset,
           span(buffer).slice(buffer_pos, record.slice.span));
  }
  else
  {
    erase(record.slice);
  }

  buffer_pos += record.slice.span;
  if (!record.is_insert)
  {
    cursor = TextCursor::from_slice(record.slice);
  }
}

void TextCompositor::delete_selection(Span<u32 const> text, Erase erase)
{
  if (cursor.is_empty())
  {
    return;
  }

  Slice32 const selection = cursor.as_slice(text.size32());
  append_record(false, selection.offset, text.slice(selection));
  erase(selection);
  cursor = cursor.to_begin();
}

static constexpr bool is_symbol(Span<u32 const> symbols, u32 c)
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
static constexpr Slice32 find_boundary(Span<u32 const> text, u32 const pos,
                                       Span<u32 const> symbols)
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

static inline Slice32 cursor_boundary(Span<u32 const> text,
                                      Span<u32 const> symbols,
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

static inline LinePosition line_translate(TextLayout const &layout, u32 cursor,
                                          i64 dy)
{
  if (layout.lines.is_empty())
  {
    return LinePosition{};
  }

  for (u32 ln = 0; ln < layout.lines.size32(); ln++)
  {
    Line const &line = layout.lines[ln];
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

void TextCompositor::command(Span<u32 const> text, TextLayout const &layout,
                             TextBlockStyle const &style, TextCommand cmd,
                             Insert insert, Erase erase, Span<u32 const> input,
                             Fn<Span<u32 const>()>     get_clipboard,
                             Fn<void(Span<u32 const>)> set_clipboard,
                             u32 lines_per_page, Vec2 pos)
{
  switch (cmd)
  {
    case TextCommand::Escape:
    {
      cursor.first = cursor.last;
    }
    break;
    case TextCommand::BackSpace:
    {
      if (cursor.is_empty())
      {
        cursor = cursor.to_begin();
        cursor.first--;
        cursor = cursor.clamp(text.size32());
      }
      delete_selection(text, erase);
    }
    break;
    case TextCommand::Delete:
    {
      if (cursor.is_empty())
      {
        cursor.last++;
        cursor = cursor.clamp(text.size32());
      }
      delete_selection(text, erase);
    }
    break;
    case TextCommand::InputText:
    {
      Slice32 selection = cursor.as_slice(text.size32());
      delete_selection(text, fn([](Slice32) {}));
      append_record(true, selection.offset, input);
      erase(selection);
      insert(selection.offset, input);
    }
    break;
    case TextCommand::Left:
    {
      if (!cursor.is_empty())
      {
        cursor = cursor.to_begin();
        break;
      }

      cursor       = cursor.to_begin();
      cursor.first = --cursor.last;
      cursor       = cursor.clamp(text.size32());
    }
    break;
    case TextCommand::Right:
    {
      if (!cursor.is_empty())
      {
        cursor = cursor.to_end();
        return;
      }

      cursor       = cursor.to_end();
      cursor.first = ++cursor.last;
      cursor       = cursor.clamp(text.size32());
    }
    break;
    case TextCommand::WordStart:
    {
      Slice32 word = cursor_boundary(text, word_symbols, cursor);
      cursor.first = cursor.last = word.offset;
    }
    break;
    case TextCommand::WordEnd:
    {
      Slice32 word = cursor_boundary(text, word_symbols, cursor);
      cursor.first = cursor.last =
          (word.span == 0) ? word.offset : (word.offset + word.span - 1);
    }
    break;
    case TextCommand::LineStart:
    {
      Slice32 line = cursor_boundary(text, line_symbols, cursor);
      cursor.first = cursor.last = line.offset;
    }
    break;
    case TextCommand::LineEnd:
    {
      Slice32 line = cursor_boundary(text, line_symbols, cursor);
      cursor.first = cursor.last =
          (line.span == 0) ? line.offset : (line.offset + line.span - 1);
    }
    break;
    case TextCommand::Up:
    {
      LinePosition line = line_translate(layout, (u32) cursor.last, -1);
      cursor.first = cursor.last = goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::Down:
    {
      LinePosition line = line_translate(layout, (u32) cursor.last, 1);
      cursor.first = cursor.last = goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::PageUp:
    {
      LinePosition line =
          line_translate(layout, (u32) cursor.last, -(i64) lines_per_page);
      cursor.first = cursor.last = goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::PageDown:
    {
      LinePosition line =
          line_translate(layout, (u32) cursor.last, lines_per_page);
      cursor.first = cursor.last = goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::SelectLeft:
    {
      cursor.last--;
      cursor = cursor.clamp(text.size32());
    }
    break;
    case TextCommand::SelectRight:
    {
      cursor.last++;
      cursor = cursor.clamp(text.size32());
    }
    break;
    case TextCommand::SelectUp:
    {
      LinePosition line = line_translate(layout, (u32) cursor.last, -1);
      cursor.last       = goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::SelectDown:
    {
      LinePosition line = line_translate(layout, (u32) cursor.last, 1);
      cursor.last       = goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::SelectWordStart:
    {
      Slice32 word = cursor_boundary(text, word_symbols, cursor);
      cursor.last  = word.offset;
    }
    break;
    case TextCommand::SelectWordEnd:
    {
      Slice32 word = cursor_boundary(text, word_symbols, cursor);
      cursor.last =
          (word.span == 0) ? word.offset : (word.offset + word.span - 1);
    }
    break;
    case TextCommand::SelectLineStart:
    {
      Slice32 line = cursor_boundary(text, line_symbols, cursor);
      cursor.last  = line.offset;
    }
    break;
    case TextCommand::SelectLineEnd:
    {
      Slice32 line = cursor_boundary(text, line_symbols, cursor);
      cursor.last =
          (line.span == 0) ? line.offset : (line.offset + line.span - 1);
    }
    break;
    case TextCommand::SelectPageUp:
    {
      LinePosition line =
          line_translate(layout, (u32) cursor.last, lines_per_page);
      cursor.last = goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::SelectPageDown:
    {
      LinePosition line =
          line_translate(layout, (u32) cursor.last, lines_per_page);
      cursor.last = goto_line(layout, line.alignment, line.line);
    }
    break;
    case TextCommand::SelectCodepoint:
    {
      cursor.first = cursor.last++;
      cursor       = cursor.clamp(text.size32());
    }
    break;
    case TextCommand::SelectWord:
    {
      Slice32 word = cursor_boundary(text, word_symbols, cursor);
      cursor.first = word.offset;
      cursor.last =
          (word.span == 0) ? word.offset : (word.offset + word.span - 1);
    }
    break;
    case TextCommand::SelectLine:
    {
      Slice32 line = cursor_boundary(text, line_symbols, cursor);
      cursor.first = line.offset;
      cursor.last =
          (line.span == 0) ? line.offset : (line.offset + line.span - 1);
    }
    break;
    case TextCommand::SelectAll:
    {
      cursor.first = 0;
      cursor.last  = text.size32();
    }
    break;
    case TextCommand::Cut:
    {
      set_clipboard(text.slice(cursor.as_slice(text.size32())));
      delete_selection(text, erase);
    }
    break;
    case TextCommand::Copy:
    {
      set_clipboard(text.slice(cursor.as_slice(text.size32())));
    }
    break;
    case TextCommand::Paste:
    {
      Span<u32 const> clipboard_input = get_clipboard();
      Slice32         selection       = cursor.as_slice(text.size32());
      delete_selection(text, fn([](Slice32) {}));
      append_record(true, selection.offset, clipboard_input);
      erase(selection);
      insert(selection.offset, clipboard_input);
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
      TextHitResult const hit = hit_text(layout, style, pos);
      cursor.first = cursor.last = hit.cluster;
    }
    break;
    case TextCommand::HitSelect:
    {
      TextHitResult const hit = hit_text(layout, style, pos);
      cursor.last             = hit.cluster;
    }
    break;
    case TextCommand::None:
    default:
      break;
  }
}

}        // namespace ash
