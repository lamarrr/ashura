#include "ashura/engine/text_compositor.h"
#include "ashura/std/error.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"

namespace ash
{

TextLocation TextCompositor::hit(TextLayout const &layout, Vec2 pos) const
{
  f32       current_top = 0;
  u32       l           = 0;
  u32 const num_lines   = layout.lines.size32();

  if (num_lines == 0)
  {
    return TextLocation{.first = 0, .num = 0, .line = 0, .column = 0};
  }

  // separated vertical and horizontal clamped hit test
  for (; l < num_lines; l++)
  {
    if (current_top <= pos.y &&
        (current_top + layout.lines[l].metrics.height) >= pos.y)
    {
      break;
    }
  }

  l = min(l, num_lines - 1);

  Line const &line = layout.lines[l];

  f32 cursor_x = 0;
  u32 r        = 0;
  for (; r < line.num_runs; r++)
  {
    TextRun const &run        = layout.runs[line.first_run + r];
    bool           intersects = (cursor_x <= pos.x &&
                       (cursor_x + pt_to_px(run.metrics.advance,
                                                      run.font_height)) >= pos.x) ||
                      (r == line.num_runs - 1);
    if (!intersects)
    {
      continue;
    }
    for (u32 g = 0; g < run.num_glyphs; g++)
    {
      // TODO(lamarrr): not correct, needs to perform actual intersection
      // test also, need to take care of directionality
      GlyphShape const &glyph = layout.glyphs[run.first_glyph + g];
      // based on direction, find first grapheme that is lesser, doesn't have
      // to strictly intersect
      bool intersects =
          (pt_to_px(glyph.advance.x, run.font_height) + cursor_x < pos.x) ||
          (g == run.num_glyphs - 1 && r == line.num_runs - 1);
      if (intersects)
      {
        return TextLocation{
            .first = glyph.cluster, .num = 1, .line = l, .column = 0};
      }
    }
    cursor_x += pt_to_px(run.metrics.advance, run.font_height);
  }

  return TextLocation{.first = 0, .num = 0, .line = l, .column = 0};
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
                                   Span<u32 const> text)
{
  if (text.size32() > buffer.size32())
  {
    // clear all records as we can't insert a new record without invalidating
    // the history
    pop_records(records.size32());
    return;
  }

  while (buffer_size + text.size32() > buffer.size32())
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

  mem::copy(text, to_span(buffer).slice(buffer_size));

  current_record++;
  latest_record           = current_record;
  records[current_record] = TextEditRecord{
      .text_pos = text_pos, .num = text.size32(), .is_insert = is_insert};
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
  u32 grapheme = hit(layout, pos).first;
  if (grapheme < selection.offset)
  {
    u32 extra        = selection.offset - grapheme;
    selection.offset = grapheme;
    selection.span += extra;
  }
  else
  {
    u32 extra = grapheme - selection.offset;
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

  if (selection.is_empty())
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
  u32 grapheme = hit(layout, pos).first;
  selection    = Slice32{grapheme, 0};
}

void TextCompositor::double_click(TextLayout const &layout, Vec2 pos,
                                  Span<u32 const> text)
{
  single_click(layout, pos);
  select_word(text);
}

void TextCompositor::triple_click(TextLayout const &layout, Vec2 pos,
                                  Span<u32 const> text)
{
  single_click(layout, pos);
  select_line(text);
}

void TextCompositor::quad_click(TextLayout const &layout, Vec2 pos,
                                Span<u32 const> text)
{
  single_click(layout, pos);
  select_all(text);
}

void TextCompositor::click(TextLayout const &layout, Span<u32 const> text,
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
      double_click(layout, pos, text);
      break;

    case 3:
      triple_click(layout, pos, text);
      break;

    case 4:
    default:
      quad_click(layout, pos, text);
      break;
  }
}

void TextCompositor::up()
{
  // move the cursor up to the previous line
  // remove cursor selection
}

void TextCompositor::down()
{
  // move the cursor down to the next line
  // remove cursor selection
}

void TextCompositor::line_start()
{
  // move the cursor to the beginning of the line
  // remove cursor selection
}

void TextCompositor::line_end()
{
  // move the cursor to the end of the line
  // remove cursor selection
}

void TextCompositor::left()
{
  // if has selection, move to left of selection
  if (selection.span != 0)
  {
    selection.span = 0;
    return;
  }

  // move the cursor to the previous codepoint
  if (selection.offset != 0)
  {
    selection.offset--;
  }
}

void TextCompositor::right()
{
  // move to the next codepoint
  if (selection.span == 0)
  {
    if (++selection.offset == U32_MAX)
    {
      selection.span = 0;
    }
    return;
  }

  // move to the last codepoint
  selection.offset = selection.end() - 1;
  selection.span   = 0;
}

void TextCompositor::page_up()
{
}
void TextCompositor::page_down()
{
}

void TextCompositor::escape()
{
  selection.span = 0;
}

void TextCompositor::command(TextCommand cmd, Span<u32 const> text,
                             Insert insert, Erase erase,
                             Fn<Span<u32 const>()>     get_content,
                             Fn<void(Span<u32 const>)> set_content)
{
  switch (cmd)
  {
    case TextCommand::Escape:
    {
      escape();
    }
    break;
    case TextCommand::BackSpace:
    {
    }
    break;
    case TextCommand::Delete:
    {
    }
    break;
    case TextCommand::Left:
    {
      left();
    }
    break;
    case TextCommand::Right:
    {
      right();
    }
    break;
    case TextCommand::Up:
    {
      up();
    }
    break;
    case TextCommand::Down:
    {
      down();
    }
    break;
    case TextCommand::WordStart:
    {
      //
    }
    break;
    case TextCommand::WordEnd:
    {
      //
    }
    break;
    case TextCommand::LineStart:
    {
      line_start();
    }
    break;
    case TextCommand::LineEnd:
    {
      line_end();
    }
    break;
    case TextCommand::PageUp:
    {
      page_up();
    }
    break;
    case TextCommand::PageDown:
    {
      page_down();
    }
    break;
    case TextCommand::Cut:
    {
      set_content(text.slice(selection));
      delete_selection(text, insert, erase);
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
    case TextCommand::SelectCodepoint:
    {
      select_codepoint(text);
    }
    break;
    case TextCommand::SelectWord:
    {
      select_word(text);
    }
    break;
    case TextCommand::SelectLine:
    {
      select_line(text);
    }
    break;
    case TextCommand::SelectAll:
    {
      select_all(text);
    }
    break;
    default:
      break;
  }
}

}        // namespace ash
