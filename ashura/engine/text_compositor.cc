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

  erase            = erase(records_.size());
  auto const slice = buffer_slice(erase);

  buffer_.erase(slice);
  records_.erase(erase);
}

Slice TextCompositor::buffer_slice(Slice records) const
{
  records = records(records_.size());

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
  auto const first = state_ + 1;
  auto       slice = buffer_slice(Slice{first, USIZE_MAX});
  buffer_.erase(slice);
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

Option<Slice> TextCompositor::undo(Vec<c32> & str)
{
  if (state_ == 0)
  {
    return none;
  }

  // undo changes of current record
  auto const & record = records_[state_];
  auto const   slice  = buffer_slice(Slice{state_, 1});
  state_--;

  switch (record.type)
  {
    case TextEditRecordType::Erase:
    {
      str
        .insert_span(record.text_pos,
                     buffer_.view().slice(slice.offset, record.erase_size))
        .unwrap();
      return Slice{record.text_pos, record.erase_size};
    }

    case TextEditRecordType::Insert:
    {
      str.erase(record.text_pos, record.insert_size);
      return Slice{record.text_pos, 0};
    }

    case TextEditRecordType::Replace:
    {
      str
        .insert_span(record.text_pos,
                     buffer_.view().slice(slice.offset, record.erase_size))
        .unwrap();
      str.erase(record.text_pos, record.insert_size);
      return Slice{record.text_pos, record.erase_size};
    }

    default:
      ASH_UNREACHABLE;
  }
}

Option<Slice> TextCompositor::redo(Vec<c32> & str)
{
  if ((state_ + 1) >= records_.size())
  {
    return none;
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
      return Slice{record.text_pos, 0};
    }

    case TextEditRecordType::Insert:
    {
      str
        .insert_span(record.text_pos,
                     buffer_.view().slice(slice.offset, record.insert_size))
        .unwrap();
      return Slice{record.text_pos, record.insert_size};
    }

    case TextEditRecordType::Replace:
    {
      str.erase(record.text_pos, record.erase_size);
      str
        .insert_span(record.text_pos,
                     buffer_.view().slice(slice.offset + record.erase_size,
                                          record.insert_size))
        .unwrap();

      return Slice{record.text_pos, record.insert_size};
    }

    default:
      ASH_UNREACHABLE;
  }
}

bool TextCompositor::erase(Vec<c32> & str, Slice slice)
{
  slice = slice(str.size());
  if (str.is_empty() || slice.is_empty())
  {
    return false;
  }

  push_record(TextEditRecordType::Erase, slice.offset, str.view().slice(slice),
              {});
  str.erase(slice);

  return true;
}

static constexpr bool is_symbol(Span<c32 const> symbols, c32 c)
{
  return !find(symbols, c).is_empty();
}

template <typename Fn>
static constexpr Option<usize> seek(Str32 text, usize pos, bool left,
                                    Fn && pred)
{
  if (pos >= text.size())
  {
    return none;
  }

  c32 const * iter    = text.pbegin() + pos;
  c32 const * end     = left ? (text.pbegin() - 1) : text.pend();
  isize       advance = left ? -1 : 1;

  while (iter != end && !pred(*iter))
  {
    iter += advance;
  }

  if (iter == end)
  {
    return none;
  }

  return iter - text.pbegin();
}

static constexpr Option<usize> seek_sym(Str32 text, usize pos, bool left,
                                        Span<c32 const> symbols)
{
  return seek(text, pos, left, [&](c32 c) { return is_symbol(symbols, c); });
}

template <typename Fn>
static constexpr Slice span_boundary(Str32 text, usize pos, Fn && pred)
{
  if (pos >= text.size())
  {
    return Slice{pos, 0};
  }

  if (pred(text[pos]))
  {
    auto neg   = [&](auto cp) { return !pred(cp); };
    auto begin = seek(text, pos, true, neg).unwrap_or(USIZE_MAX) + 1;
    auto end   = seek(text, pos, false, neg).unwrap_or(text.size());
    return Slice::from_range(begin, end);
  }
  else
  {
    auto begin = seek(text, pos, true, pred).unwrap_or(USIZE_MAX) + 1;
    auto end   = seek(text, pos, false, pred).unwrap_or(text.size());
    return Slice::from_range(begin, end);
  }
}

static constexpr Slice span_sym_boundary(Str32 text, usize pos,
                                         Span<c32 const> symbols)
{
  return span_boundary(text, pos, [&](c32 c) { return is_symbol(symbols, c); });
}

static inline Option<isize> translate_caret(TextLayout const & layout,
                                            isize              caret,
                                            CaretAlignment     alignment,
                                            isize line_displacement)
{
  auto loc = layout.locate_caret(caret);

  // [ ]
  return layout.align_caret(CaretLocation{.line = loc.line + line_displacement,
                                          .alignment = alignment});
}

// [ ] IME support
// [ ] selecting  line span resets it to the beginning
bool TextCompositor::command(RenderText & rendered, TextCommand cmd,
                             Str32 keyboard_input, ClipBoard & clipboard,
                             usize lines_per_page, usize tab_width,
                             CRect const & region, Vec2 pos, Vec2 zoom,
                             AllocatorRef scratch_allocator)
{
  u8                tmp[512];
  FallbackAllocator tmp_allocator{Arena::from(tmp), scratch_allocator};

  auto & layout = rendered.get_layout();
  auto & text   = rendered.text_;

  // [ ] normalize by caret span
  cursor_.normalizex(layout.num_carets);

  bool modified = false;

  switch (cmd)
  {
    case TextCommand::Escape:
    {
      cursor_.unselectx();
    }
    break;
    case TextCommand::Unselect:
    {
      cursor_.unselectx();
    }
    break;
    case TextCommand::BackSpace:
    {
      Slice selection;

      if (auto s = cursor_.selectionx(); !s.is_empty())
      {
        selection = s;
      }
      else
      {
        selection = cursor_.translatex(-1)
                      .span_by2x(1)
                      .normalizex(layout.num_carets)
                      .selectionx();
      }

      modified |= erase(text, layout.get_caret_selection(selection));
      cursor_.unselect_leftx();
    }
    break;
    case TextCommand::Delete:
    {
      Slice selection;

      if (auto s = cursor_.selectionx(); !s.is_empty())
      {
        selection = s;
      }
      else
      {
        selection =
          cursor_.span_by2x(1).normalizex(layout.num_carets).selectionx();
      }

      modified |= erase(text, layout.get_caret_selection(selection));
      cursor_.unselect_leftx();
    }
    break;
    case TextCommand::Paste:
    case TextCommand::InputText:
    case TextCommand::NewLine:
    case TextCommand::Tab:
    {
      Vec<c32> text32{tmp_allocator};
      Str32    input;
      Vec<c32> tabs{tmp_allocator};

      if (cmd == TextCommand::Paste)
      {
        Vec<u8> text8{tmp_allocator};
        clipboard.get(MIME_TEXT_UTF8, text8).unwrap();
        utf8_decode(text8.view().as_c8(), text32).unwrap();
        input = text32;
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
        if (auto selection = cursor_.selectionx(); !selection.is_empty())
        {
          auto text_selection = layout.get_caret_selection(selection);
          push_record(TextEditRecordType::Replace, text_selection.offset,
                      text.view().slice(text_selection), input);
          // [ ] if it doesn't accept new line, then don't accept it / !!!!!!!filter
          text.erase(text_selection);
          text.insert_span(text_selection.offset, input).unwrap();
          cursor_.unselectx()
            .translate(
              -(isize) text_selection.span)    // move to end of text actually
            .normalizex(layout.num_carets)
            .translatex(input.size());
        }
        else
        {
          auto caret = cursor_.caret();
          push_record(TextEditRecordType::Insert, caret, {}, input);
          text.insert_span(caret, input).unwrap();
          cursor_.unselect().translate(input.size()).normalize();
        }
        modified = true;
      }
    }
    break;
    case TextCommand::Left:
    {
      cursor_.translatex(-1).normalizex(layout.num_carets);
    }
    break;
    case TextCommand::Right:
    {
      cursor_.translatex(1).normalizex(layout.num_carets);
    }
    break;
    case TextCommand::WordStart:
    {
      // [ ] ???
      cursor_.move_to(
        seek_sym(text, cursor_.caret(), true, word_symbols_).unwrap_or(0));
    }
    break;
    case TextCommand::WordEnd:
    {
      // [ ] all caret indices should be converted to text positions
      cursor_.move_to(seek_sym(text, cursor_.caret(), false, word_symbols_)
                        .unwrap_or(text.size()));
    }
    break;
    case TextCommand::LineStart:
    {
      cursor_.move_to(
        seek_sym(text, cursor_.caret(), true, line_symbols_).unwrap_or(0));
    }
    break;
    case TextCommand::LineEnd:
    {
      cursor_.move_to(seek_sym(text, cursor_.caret(), false, line_symbols_)
                        .unwrap_or(text.size()));
    }
    break;
    case TextCommand::Up:
    {
      cursor_.move_to(
        translate_caret(layout, cursor_.caret(), caret_alignment_, -1)
          .unwrap_or(0));
    }
    break;
    case TextCommand::Down:
    {
      cursor_.move_to(
        translate_caret(layout, cursor_.caret(), caret_alignment_, 1)
          .unwrap_or(0));
    }
    break;
    case TextCommand::PageUp:
    {
      cursor_.move_to(translate_caret(layout, cursor_.caret(), caret_alignment_,
                                      -(isize) lines_per_page)
                        .unwrap_or(0));
    }
    break;
    case TextCommand::PageDown:
    {
      cursor_.move_to(translate_caret(layout, cursor_.caret(), caret_alignment_,
                                      (isize) lines_per_page)
                        .unwrap_or(0));
    }
    break;
    case TextCommand::SelectLeft:
    {
      cursor_.extend_selection(-1).normalize();
    }
    break;
    case TextCommand::SelectRight:
    {
      cursor_.extend_selection(1).normalize();
    }
    break;
    case TextCommand::SelectUp:
    {
      cursor_.span_to2(
        translate_caret(layout, cursor_.caret(), caret_alignment_, -1)
          .unwrap_or(0));
    }
    break;
    case TextCommand::SelectDown:
    {
      cursor_.span_to2(
        translate_caret(layout, cursor_.caret(), caret_alignment_, 1)
          .unwrap_or(0));
    }
    break;
    case TextCommand::SelectToWordStart:
    {
      cursor_.span_to2(
        seek_sym(text, cursor_.caret(), true, word_symbols_).unwrap_or(0));
    }
    break;
    case TextCommand::SelectToWordEnd:
    {
      cursor_.span_to2(seek_sym(text, cursor_.caret(), false, word_symbols_)
                         .unwrap_or(text.size()));
    }
    break;
    case TextCommand::SelectToLineStart:
    {
      cursor_.span_to2(
        seek_sym(text, cursor_.caret(), true, line_symbols_).unwrap_or(0));
    }
    break;
    case TextCommand::SelectToLineEnd:
    {
      cursor_.span_to2(seek_sym(text, cursor_.caret(), false, line_symbols_)
                         .unwrap_or(text.size()));
    }
    break;
    case TextCommand::SelectPageUp:
    {
      // [ ]
      // cursor_.span_to(translate_caret(layout, cursor_.caret().unwrap_or(0),
      //                                 caret_alignment_.unwrap_or(0),
      //                                 -(isize) lines_per_page)
      //                   .unwrap_or(0));
    }
    break;
    case TextCommand::SelectPageDown:
    {
      // [ ]
      // cursor_.span_to(translate_caret(layout, cursor_.caret().unwrap_or(0),
      //                                 caret_alignment_.unwrap_or(0),
      //                                 (isize) lines_per_page)
      //                   .unwrap_or(0));
    }
    break;
    case TextCommand::SelectCodepoint:
    {
      cursor_.span_by2(1).normalize();
    }
    break;
    case TextCommand::SelectWord:
    {
      // cursor_.select(span_symbol(text, cursor_.caret(), word_symbols_), true);
    }
    break;
    case TextCommand::SelectLine:
    {
      // [ ] get line, span it
      // cursor_.select(symbol_boundary(text, line_symbols_, cursor_.selection()));
    }
    break;
    case TextCommand::SelectAll:
    {
      cursor_.select(Slice{0, text.size()});
    }
    break;
    case TextCommand::Cut:
    {
      Vec<c8> data8{scratch_allocator};
      utf8_encode(text.view().slice(cursor_.selection()), data8).unwrap();
      erase(text, cursor_.selection());
      clipboard.set(MIME_TEXT_UTF8, data8.view().as_u8()).unwrap();
    }
    break;
    case TextCommand::Copy:
    {
      Vec<c8> data8{scratch_allocator};
      utf8_encode(text.view().slice(cursor_.selection()), data8).unwrap();
      clipboard.set(MIME_TEXT_UTF8, data8.view().as_u8()).unwrap();
    }
    break;
    case TextCommand::Undo:
    {
      undo(text).match([&](Slice inserted) {
        modified = true;
        cursor_.select(inserted);
      });
    }
    break;
    case TextCommand::Redo:
    {
      redo(text).match([&](Slice inserted) {
        modified = true;
        cursor_.select(inserted);
      });
    }
    break;
    case TextCommand::Hit:
    {
      auto hit = rendered.hit(region, pos, zoom).unwrap_or(TextHitResult{});
      caret_alignment_ = hit.column;
      cursor_.move_to(align_caret(layout, hit.line, hit.column));
    }
    break;
    case TextCommand::HitSelect:
    {
      auto hit = rendered.hit(region, pos, zoom).unwrap_or(TextHitResult{});
      caret_alignment_ = hit.column;
      cursor_.span_to2(align_caret(layout, hit.line, hit.column));
    }
    break;

    case TextCommand::None:
    default:
      break;
  }

  cursor_.normalize(text.size());

  return modified;
}

}    // namespace ash
