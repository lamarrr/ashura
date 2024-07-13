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

void TextCompositor::goto_line(TextLayout const &layout, u32 line)
{
  if (layout.lines.is_empty())
  {
    return;
  }

  line = min(line, layout.lines.size32() - 1);

  cursor = min(layout.lines[line].first_codepoint + alignment,
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

void TextCompositor::delete_selection(Span<u32 const> text, Insert insert,
                                      Erase erase)
{
  (void) insert;
  if (selection.is_empty() || selection.offset > text.size32())
  {
    selection = {};
    return;
  }

  append_record(false, selection.offset, text.slice(selection));
  erase(selection);
  selection.span = 0;
}

void TextCompositor::input_text(Span<u32 const> text, Span<u32 const> input,
                                Insert insert, Erase erase)
{
  if (!selection.is_empty())
  {
    delete_selection(text.slice(selection), insert, erase);
    selection.span = 0;
  }

  append_record(true, selection.offset, text.slice(selection));
  insert(selection.offset, input);
}

void TextCompositor::drag(TextLayout const &layout, Vec2 pos)
{
  TextHitResult const hit     = hit_text(layout, pos);
  u32 const           cluster = hit.cluster;
  cursor                      = cluster;
  if (cluster < selection.offset)
  {
    u32 extra        = selection.offset - cluster;
    selection.offset = cluster;
    selection.span += extra;
  }
  else
  {
    u32 extra = cluster - selection.offset;
    selection.span += extra;
  }
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
static constexpr void find_boundary(Span<u32 const> text, u32 const pos,
                                    Span<u32 const> symbols, Slice32 &slice)
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

  slice.offset = bwd;
  slice.span   = (fwd - bwd) + 1;
}

void TextCompositor::update_cursor(TextLayout const &layout)
{
  cursor =
      selection.offset + (((selection.span == 0) ? 0 : (selection.span - 1)));
  u32 const   l    = find_cluster_line(layout, cursor);
  Line const &line = layout.lines[l];
  alignment =
      cursor > line.first_codepoint ? (cursor - line.first_codepoint) : 0;
}

void TextCompositor::select_codepoint(Span<u32 const> text)
{
  selection.span = 1;
  selection      = selection.resolve(text.size32());
}

void TextCompositor::select_word(Span<u32 const> text)
{
  selection = selection.resolve(text.size32());
  if (text.is_empty())
  {
    selection = Slice32{};
    return;
  }

  find_boundary(text, selection.offset, word_symbols, selection);
}

void TextCompositor::select_line(Span<u32 const> text)
{
  selection = selection.resolve(text.size32());
  if (text.is_empty())
  {
    selection = Slice32{};
    return;
  }

  find_boundary(text, selection.offset, line_symbols, selection);
}

void TextCompositor::select_all(Span<u32 const> text)
{
  selection = Slice32{0, text.size32()};
}

void TextCompositor::single_click(TextLayout const &layout, Vec2 pos)
{
  TextHitResult const hit = hit_text(layout, pos);
  selection               = Slice32{hit.cluster, 0};
  update_cursor(layout);
}

void TextCompositor::double_click(Span<u32 const>   text,
                                  TextLayout const &layout, Vec2 pos)
{
  TextHitResult const hit = hit_text(layout, pos);
  selection               = Slice32{hit.cluster, 0};
  select_word(text);
  update_cursor(layout);
}

void TextCompositor::triple_click(Span<u32 const>   text,
                                  TextLayout const &layout, Vec2 pos)
{
  TextHitResult const hit = hit_text(layout, pos);
  selection               = Slice32{hit.cluster, 0};
  select_line(text);
  update_cursor(layout);
}

void TextCompositor::quad_click(Span<u32 const> text, TextLayout const &layout,
                                Vec2 pos)
{
  TextHitResult const hit = hit_text(layout, pos);
  selection               = Slice32{hit.cluster, 0};
  select_all(text);
  update_cursor(layout);
}

void TextCompositor::click(Span<u32 const> text, TextLayout const &layout,
                           u32 num_clicks, Vec2 pos)
{
  switch (num_clicks)
  {
    case 0:
      break;

    case 1:
      single_click(layout, pos);
      break;

    case 2:
      double_click(text, layout, pos);
      break;

    case 3:
      triple_click(text, layout, pos);
      break;

    case 4:
    default:
      quad_click(text, layout, pos);
      break;
  }
}

void TextCompositor::up(TextLayout const &layout)
{
  page_up(layout, 1);
}

void TextCompositor::down(TextLayout const &layout)
{
  page_down(layout, 1);
}

void TextCompositor::left(Span<u32 const> text)
{
  // if has selection, move to left of selection
  if (selection.span != 0)
  {
    selection.span = 0;
    selection      = selection.resolve(text.size32());
    return;
  }

  // move the cursor to the previous codepoint
  if (selection.offset != 0)
  {
    selection.offset--;
    selection = selection.resolve(text.size32());
  }
}

void TextCompositor::right(Span<u32 const> text)
{
  // move to the next codepoint
  if (selection.span == 0)
  {
    selection.offset++;
    selection = selection.resolve(text.size32());
    return;
  }

  // move to the last codepoint
  selection.offset = selection.end() - 1;
  selection.span   = 0;
  selection        = selection.resolve(text.size32());
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

void TextCompositor::page_up(TextLayout const &layout, u32 lines_per_page)
{
  u32 line = line_translate(layout, cursor, -(i64) lines_per_page);
  goto_line(layout, line);
}

void TextCompositor::page_down(TextLayout const &layout, u32 lines_per_page)
{
  u32 line = line_translate(layout, cursor, (i64) lines_per_page);
  goto_line(layout, line);
}

void TextCompositor::escape()
{
  selection.span = 0;
}

void TextCompositor::command(Span<u32 const> text, TextLayout const &layout,
                             TextCommand cmd, Insert insert, Erase erase,
                             Fn<Span<u32 const>()>     get_content,
                             Fn<void(Span<u32 const>)> set_content,
                             u32                       lines_per_page)
{
  switch (cmd)
  {
    case TextCommand::Escape:
    {
      escape();
      update_cursor(layout);
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
      update_cursor(layout);
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
      update_cursor(layout);
    }
    break;
    case TextCommand::Left:
    {
      left(text);
      update_cursor(layout);
    }
    break;
    case TextCommand::Right:
    {
      right(text);
      update_cursor(layout);
    }
    break;
    case TextCommand::Up:
    {
      up(layout);
      update_cursor(layout);
    }
    break;
    case TextCommand::Down:
    {
      down(layout);
      update_cursor(layout);
    }
    break;
    case TextCommand::WordStart:
    {
      select_word(text);
      left(text);
      update_cursor(layout);
    }
    break;
    case TextCommand::WordEnd:
    {
      select_word(text);
      right(text);
      update_cursor(layout);
    }
    break;
    case TextCommand::LineStart:
    {
      select_line(text);
      left(text);
      update_cursor(layout);
    }
    break;
    case TextCommand::LineEnd:
    {
      select_line(text);
      right(text);
      update_cursor(layout);
    }
    break;
    case TextCommand::PageUp:
    {
      page_up(layout, lines_per_page);
      update_cursor(layout);
    }
    break;
    case TextCommand::PageDown:
    {
      page_down(layout, lines_per_page);
      update_cursor(layout);
    }
    break;
    case TextCommand::Cut:
    {
      set_content(text.slice(selection));
      delete_selection(text, insert, erase);
      update_cursor(layout);
    }
    break;
    case TextCommand::Copy:
    {
      set_content(text.slice(selection));
    }
    break;
    case TextCommand::Paste:
    {
      input_text(text, get_content(), insert, erase);
      update_cursor(layout);
    }
    break;
    case TextCommand::Undo:
    {
      undo(insert, erase);
      update_cursor(layout);
    }
    break;
    case TextCommand::Redo:
    {
      redo(insert, erase);
      update_cursor(layout);
    }
    break;
    case TextCommand::SelectCodepoint:
    {
      select_codepoint(text);
      update_cursor(layout);
    }
    break;
    case TextCommand::SelectWord:
    {
      select_word(text);
      update_cursor(layout);
    }
    break;
    case TextCommand::SelectLine:
    {
      select_line(text);
      update_cursor(layout);
    }
    break;
    case TextCommand::SelectAll:
    {
      select_all(text);
      update_cursor(layout);
    }
    break;
    default:
      break;
  }
}

}        // namespace ash
