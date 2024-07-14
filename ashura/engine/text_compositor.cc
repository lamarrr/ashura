/// SPDX-License-Identifier: MIT
#include "ashura/engine/text_compositor.h"
#include "ashura/std/error.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"

namespace ash
{

/// @brief given a cluser index in the text, get the line number the cluster
/// belongs to, otherwise return U32_MAX.
/// @param cluster cluster to
/// @return
static inline u32 find_cluster_line(TextLayout const &layout, u32 cluster)
{
  for (u32 i = 0; i < layout.lines.size32(); i++)
  {
    if (layout.lines[i].first_codepoint <= cluster &&
        (layout.lines[i].first_codepoint + layout.lines[i].num_codepoints) >
            cluster)
    {
      return i;
    }
  }
  return U32_MAX;
}

void TextCompositor::goto_line(TextLayout const     &layout,
                               TextBlockStyle const &style, u32 line)
{
  if (layout.lines.is_empty())
  {
    return;
  }

  line = min(line, layout.lines.size32() - 1);

  cursor.last = min(layout.lines[line].first_codepoint + alignment,
                    layout.lines[line].first_codepoint +
                        ((layout.lines[line].num_codepoints == 0) ?
                             0 :
                             (layout.lines[line].num_codepoints - 1)));
}

void TextCompositor::pop_records(u32 num)
{
  CHECK(num <= records.size32());
  u32 reclaimed = 0;
  for (u32 i = 0; i < num; i++)
  {
    reclaimed += records[i].num;
  }
  mem::move(to_span(buffer).slice(reclaimed).as_const(), to_span(buffer));
  mem::move(to_span(records).slice(num).as_const(), to_span(records));
  buffer_size -= reclaimed;
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

  while (buffer_size + segment.size32() > buffer.size32())
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

  mem::copy(segment, to_span(buffer).slice(buffer_size));

  current_record++;
  latest_record           = current_record;
  records[current_record] = TextEditRecord{
      .text_pos = text_pos, .num = segment.size32(), .is_insert = is_insert};
}

void TextCompositor::undo(Insert insert, Erase erase)
{
  if (current_record == 0)
  {
    return;
  }
  // undo changes of current record
  TextEditRecord &record = records[current_record];
  buffer_pos -= record.num;
  if (record.is_insert)
  {
    erase(Slice32{record.text_pos, record.num});
  }
  else
  {
    insert(record.text_pos, to_span(buffer).slice(buffer_pos, record.num));
  }
  current_record--;
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
    insert(record.text_pos, to_span(buffer).slice(buffer_pos, record.num));
  }
  else
  {
    erase(Slice32{record.text_pos, record.num});
  }

  buffer_pos += record.num;
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
  if (selection_last > selection_first)
  {
    selection_last = selection_first;
  }
  else
  {
    selection_first = selection_last;
  }
}

void TextCompositor::input_text(Span<u32 const> text, Span<u32 const> input,
                                Insert insert, Erase erase)
{
  Slice32 selection = selection_slice(text.size32());
  delete_selection(text, to_fn([](Slice32) {}));
  append_record(true, selection.offset, input);
  erase(selection);
  insert(selection.offset, input);
}

void TextCompositor::drag(TextLayout const &layout, TextBlockStyle const &style,
                          Vec2 pos)
{
  TextHitResult const hit = hit_text(layout, style, pos);
  cursor.last             = hit.cluster;
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

void TextCompositor::update_cursor(TextLayout const     &layout,
                                   TextBlockStyle const &style)
{
  cursor =
      selection.offset + (((selection.span == 0) ? 0 : (selection.span - 1)));
  u32 const   l    = find_cluster_line(layout, cursor);
  Line const &line = layout.lines[l];
  alignment =
      cursor > line.first_codepoint ? (cursor - line.first_codepoint) : 0;
}

void TextCompositor::select_codepoint()
{
  cursor.first = cursor.last++;
}

void TextCompositor::select_word(Span<u32 const> text)
{
  if (text.is_empty())
  {
    return;
  }
  u32 pos =
      (selection_last >= text.size32()) ? (text.size32() - 1) : selection_last;

  find_boundary(text, selection.offset, word_symbols, selection);
}

void TextCompositor::select_line(Span<u32 const> text)
{
  Slice32 selection = cursor.as_slice(text.size32());
  if (text.is_empty() || selection.is_empty())
  {
    return;
  }
  selection = find_boundary(text, selection.offset, line_symbols);
  cursor    = TextCursor::from_slice(selection);
}

void TextCompositor::select_all(Span<u32 const> text)
{
  cursor.first = 0;
  cursor.last  = text.size32();
}

void TextCompositor::hit_codepoint(TextLayout const     &layout,
                                   TextBlockStyle const &style, Vec2 pos)
{
  TextHitResult const hit = hit_text(layout, style, pos);
  cursor.last             = hit.cluster;
}

void TextCompositor::hit_word(Span<u32 const> text, TextLayout const &layout,
                              TextBlockStyle const &style, Vec2 pos)
{
  TextHitResult const hit = hit_text(layout, style, pos);
  cursor.last             = hit.cluster;
  select_word(text);
}

void TextCompositor::hit_line(Span<u32 const> text, TextLayout const &layout,
                              TextBlockStyle const &style, Vec2 pos)
{
  TextHitResult const hit = hit_text(layout, style, pos);
  cursor.first = cursor.last = hit.cluster;
  select_line(text);
}

void TextCompositor::hit_all(Span<u32 const> text, TextLayout const &layout,
                             TextBlockStyle const &style, Vec2 pos)
{
  TextHitResult const hit = hit_text(layout, style, pos);
  cursor.first = cursor.last = hit.cluster;
  select_all(text);
}

void TextCompositor::hit_select(Span<u32 const> text, TextLayout const &layout,
                                TextBlockStyle const &style, Vec2 pos)
{
  TextHitResult const hit = hit_text(layout, style, pos);
  cursor.last             = hit.cluster;
}

void TextCompositor::up(TextLayout const &layout, TextBlockStyle const &style,
                        u32 lines)
{
  u32 line = line_translate(layout, cursor, -(i64) lines);
  goto_line(layout, style, line);
}

void TextCompositor::down(TextLayout const &layout, TextBlockStyle const &style,
                          u32 lines)
{
  u32 line = line_translate(layout, cursor, (i64) lines);
  goto_line(layout, style, line);
}

void TextCompositor::left()
{
  // if has selection, move to left of selection
  if (!cursor.is_empty())
  {
    cursor = cursor.to_begin();
    return;
  }

  // move the cursor to the previous codepoint
  cursor       = cursor.to_begin();
  cursor.first = --cursor.last;
}

void TextCompositor::right()
{
  // if has selection, move to right of selection
  if (!cursor.is_empty())
  {
    cursor = cursor.to_end();
    return;
  }

  // move to the next codepoint
  cursor       = cursor.to_end();
  cursor.first = ++cursor.last;
}

void TextCompositor::escape()
{
  cursor = cursor.escape();
}

static u32 line_translate(TextLayout const &layout, u32 cursor, i64 dy)
{
  if (layout.lines.is_empty())
  {
    return 0;
  }

  i64 line = find_cluster_line(layout, cursor);
  if (line == U32_MAX)
  {
    return 0;
  }

  line += dy;

  return (u32) clamp(line, (i64) 0, (i64) (layout.lines.size32() - 1));
}

void TextCompositor::command(Span<u32 const> text, TextLayout const &layout,
                             TextBlockStyle const &style, TextCommand cmd,
                             Insert insert, Erase erase,
                             Fn<Span<u32 const>()>     get_clipboard,
                             Fn<void(Span<u32 const>)> set_clipboard,
                             u32 lines_per_page, Vec2 pos)
{
  switch (cmd)
  {
    case TextCommand::Escape:
    {
      escape();
      update_cursor(layout, style);
    }
    break;
    case TextCommand::BackSpace:
    {
      if (selection.is_empty())
      {
        left(text);
        selection.span = 1;
      }
      delete_selection(text, insert, erase);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::Delete:
    {
      if (selection.is_empty())
      {
        right(text);
        selection.span = 1;
      }
      delete_selection(text, insert, erase);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::Left:
    {
      left(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::Right:
    {
      right(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::Up:
    {
      up(layout, style);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::Down:
    {
      down(layout, style);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::WordStart:
    {
      select_word(text);
      left(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::WordEnd:
    {
      select_word(text);
      right(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::LineStart:
    {
      select_line(text);
      left(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::LineEnd:
    {
      select_line(text);
      right(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::PageUp:
    {
      page_up(layout, style, lines_per_page);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::PageDown:
    {
      // TODO(lamarrr): page down should reset selection to cursor not update
      // cursor
      page_down(layout, style, lines_per_page);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectLeft:
    {
      left(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectRight:
    {
      right(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectUp:
    {
      up(layout, style);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectDown:
    {
      down(layout, style);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectWordStart:
    {
      select_word(text);
      left(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectWordEnd:
    {
      select_word(text);
      right(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectLineStart:
    {
      select_line(text);
      left(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectLineEnd:
    {
      select_line(text);
      right(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectPageUp:
    {
      page_up(layout, style, lines_per_page);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectPageDown:
    {
      page_down(layout, style, lines_per_page);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::Cut:
    {
      set_clipboard(text.slice(selection));
      delete_selection(text, insert, erase);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::Copy:
    {
      set_clipboard(text.slice(selection));
    }
    break;
    case TextCommand::Paste:
    {
      input_text(text, get_clipboard(), insert, erase);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::Undo:
    {
      undo(insert, erase);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::Redo:
    {
      redo(insert, erase);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectCodepoint:
    {
      select_codepoint(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectWord:
    {
      select_word(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectLine:
    {
      select_line(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::SelectAll:
    {
      select_all(text);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::HitCodepoint:
    {
      hit_codepoint(layout, style, pos);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::HitWord:
    {
      hit_word(text, layout, style, pos);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::HitLine:
    {
      hit_line(text, layout, style, pos);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::HitAll:
    {
      hit_all(text, layout, style, pos);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::HitSelect:
    {
      hit_all(text, layout, style, pos);
      update_cursor(layout, style);
    }
    break;
    case TextCommand::None:
    default:
      break;
  }
}

}        // namespace ash
