/// SPDX-License-Identifier: MIT
#include "ashura/engine/text_compositor.h"
#include "ashura/std/error.h"
#include "ashura/std/mem.h"
#include "ashura/std/range.h"
#include "ashura/std/text.h"

namespace ash
{

TextCompositor TextCompositor::create(AllocatorRef allocator, usize buffer_size,
                                      usize records_size, Str32 word_symbols)
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

  return TextCompositor{std::move(buffer), std::move(records), word_symbols};
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
    return Slice::range(begin, end);
  }
  else
  {
    auto begin = seek(text, pos, true, pred).unwrap_or(USIZE_MAX) + 1;
    auto end   = seek(text, pos, false, pred).unwrap_or(text.size());
    return Slice::range(begin, end);
  }
}

static constexpr Slice span_sym_boundary(Str32 text, usize pos,
                                         Span<c32 const> symbols)
{
  return span_boundary(text, pos, [&](c32 c) { return is_symbol(symbols, c); });
}

static inline Option<isize> translate_caret(TextLayout const & layout,
                                            isize              caret,
                                            CaretXAlignment    alignment,
                                            isize line_displacement)
{
  if (layout.lines.is_empty())
  {
    return none;
  }

  auto loc = layout.get_caret_codepoint(caret);

  auto line = clamp((isize) loc.line + line_displacement, (isize) 0,
                    (isize) layout.lines.size());

  return layout.align_caret(
    CaretAlignment{.x = alignment, .y = static_cast<CaretYAlignment>(line)});
}

void TextCompositor::command(RenderText & rendered, TextCommand cmd,
                             Str32 keyboard_input, ClipBoard & clipboard,
                             usize lines_per_page, usize tab_width, Vec2 center,
                             f32 aligned_width, Vec2 pos,
                             Mat4 const & transform,
                             AllocatorRef scratch_allocator)
{
  u8                tmp[512];
  FallbackAllocator tmp_allocator{Arena::from(tmp), scratch_allocator};

  auto & layout = rendered.get_layout();
  auto & text   = rendered.text_;

  auto perform_layout = [&]() {
    rendered.flush_text();
    rendered.layout(rendered.get_layout().max_width);
  };

  cursor_.normalize(layout.num_carets);

  switch (cmd)
  {
    case TextCommand::Escape:
    {
      cursor_.unselect();
    }
    break;
    case TextCommand::Unselect:
    {
      cursor_.unselect();
    }
    break;
    case TextCommand::BackSpace:
    {
      if (!cursor_.has_selection())
      {
        cursor_.translate(-1).span_by(1).normalize(layout.num_carets);
      }

      Slice carets = cursor_.selection();
      erase(text, layout.get_caret_selection(carets));
      perform_layout();
      cursor_.unselect_left();
    }
    break;
    case TextCommand::Delete:
    {
      if (!cursor_.has_selection())
      {
        cursor_.span_by(1).normalize(layout.num_carets);
      }

      Slice carets = cursor_.selection();
      erase(text, layout.get_caret_selection(carets));
      perform_layout();
      cursor_.unselect_left();
    }
    break;
    case TextCommand::InputText:
    case TextCommand::NewLine:
    case TextCommand::Tab:
    case TextCommand::Paste:
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
        if (auto carets = cursor_.selection(); !carets.is_empty())
        {
          // [ ] process replace correctly
          // [ ] hit span starts with the last hit, sometimes not ideal
          auto selection = layout.get_caret_selection(carets);
          push_record(TextEditRecordType::Replace, selection.offset,
                      text.view().slice(selection), input);
          text.erase(selection);
          text.insert_span(selection.offset, input).unwrap();
          perform_layout();
          cursor_
            .move_to(layout.to_caret(selection.offset + input.size(), true))
            .normalize(layout.num_carets);
        }
        else
        {
          auto cp        = layout.get_caret_codepoint(cursor_.caret());
          auto codepoint = cp.codepoint + (cp.after ? 1 : 0);
          push_record(TextEditRecordType::Insert, codepoint, {}, input);
          text.insert_span(codepoint, input).unwrap();
          perform_layout();
          cursor_.unselect()
            .move_to(layout.to_caret(codepoint + input.size(), true))
            .normalize(layout.num_carets);
        }
      }
    }
    break;
    case TextCommand::Left:
    {
      cursor_.translate(-1).normalize(layout.num_carets);
    }
    break;
    case TextCommand::Right:
    {
      cursor_.translate(1).normalize(layout.num_carets);
    }
    break;
    case TextCommand::WordStart:
    {
      auto c = layout.get_caret_codepoint(cursor_.caret());
      cursor_.move_to(layout.to_caret(
        seek_sym(text, c.codepoint + (c.after ? 1 : 0), true, word_symbols_)
          .unwrap_or(),
        true));
    }
    break;
    case TextCommand::WordEnd:
    {
      auto c = layout.get_caret_codepoint(cursor_.caret());
      cursor_.move_to(layout.to_caret(
        seek_sym(text, c.codepoint + (c.after ? 1 : 0), false, word_symbols_)
          .unwrap_or(layout.num_codepoints),
        true));
    }
    break;
    case TextCommand::LineStart:
    {
      auto c = layout.get_caret_codepoint(cursor_.caret());
      cursor_.move_to(layout.lines[c.line].carets.first());
    }
    break;
    case TextCommand::LineEnd:
    {
      auto c = layout.get_caret_codepoint(cursor_.caret());
      cursor_.move_to(layout.lines[c.line].carets.last());
    }
    break;
    case TextCommand::Up:
    {
      cursor_.move_to(
        translate_caret(layout, cursor_.caret(), caret_alignment_, -1)
          .unwrap_or(cursor_.caret()));
    }
    break;
    case TextCommand::Down:
    {
      cursor_.move_to(
        translate_caret(layout, cursor_.caret(), caret_alignment_, 1)
          .unwrap_or(cursor_.caret()));
    }
    break;
    case TextCommand::PageUp:
    {
      cursor_.move_to(translate_caret(layout, cursor_.caret(), caret_alignment_,
                                      -(isize) lines_per_page)
                        .unwrap_or(cursor_.caret()));
    }
    break;
    case TextCommand::PageDown:
    {
      cursor_.move_to(translate_caret(layout, cursor_.caret(), caret_alignment_,
                                      (isize) lines_per_page)
                        .unwrap_or(cursor_.caret()));
    }
    break;
    case TextCommand::SelectLeft:
    {
      cursor_.extend_selection(-1).normalize(layout.num_carets);
    }
    break;
    case TextCommand::SelectRight:
    {
      cursor_.extend_selection(1).normalize(layout.num_carets);
    }
    break;
    case TextCommand::SelectUp:
    {
      cursor_.span_to(
        translate_caret(layout, cursor_.caret(), caret_alignment_, -1)
          .unwrap_or(cursor_.caret()));
    }
    break;
    case TextCommand::SelectDown:
    {
      cursor_.span_to(
        translate_caret(layout, cursor_.caret(), caret_alignment_, 1)
          .unwrap_or(cursor_.caret()));
    }
    break;
    case TextCommand::SelectToWordStart:
    {
      auto c = layout.get_caret_codepoint(cursor_.caret());
      cursor_.span_to(layout.to_caret(
        seek_sym(text, c.codepoint, true, word_symbols_).unwrap_or(), true));
    }
    break;
    case TextCommand::SelectToWordEnd:
    {
      auto c = layout.get_caret_codepoint(cursor_.caret());
      cursor_.span_to(
        layout.to_caret(seek_sym(text, c.codepoint, false, word_symbols_)
                          .unwrap_or(layout.num_codepoints),
                        true));
    }
    break;
    case TextCommand::SelectToLineStart:
    {
      auto c = layout.get_caret_codepoint(cursor_.caret());
      cursor_.span_to(layout.lines[c.line].carets.first());
    }
    break;
    case TextCommand::SelectToLineEnd:
    {
      auto c = layout.get_caret_codepoint(cursor_.caret());
      cursor_.span_to(layout.lines[c.line].carets.last());
    }
    break;
    case TextCommand::SelectPageUp:
    {
      cursor_.span_to(translate_caret(layout, cursor_.caret(), caret_alignment_,
                                      -(isize) lines_per_page)
                        .unwrap_or(cursor_.caret()));
    }
    break;
    case TextCommand::SelectPageDown:
    {
      cursor_.span_to(translate_caret(layout, cursor_.caret(), caret_alignment_,
                                      (isize) lines_per_page)
                        .unwrap_or(cursor_.caret()));
    }
    break;
    case TextCommand::SelectCodepoint:
    {
      cursor_.span_by(1).normalize(layout.num_carets);
    }
    break;
    case TextCommand::SelectWord:
    {
      auto selection = span_sym_boundary(
        text, layout.get_caret_codepoint(cursor_.caret()).codepoint,
        word_symbols_);
      cursor_.select(layout.get_caret_selection(selection));
    }
    break;
    case TextCommand::SelectLine:
    {
      auto c = layout.get_caret_codepoint(cursor_.caret());
      cursor_.select(layout.lines[c.line].carets);
    }
    break;
    case TextCommand::SelectAll:
    {
      cursor_.select(Slice{0, layout.num_carets});
    }
    break;
    case TextCommand::Cut:
    {
      Vec<c8> data8{scratch_allocator};
      if (!cursor_.has_selection())
      {
        auto cp = layout.get_caret_codepoint(cursor_.caret());
        cursor_.select(layout.lines[cp.line].carets);
      }

      auto selection = layout.get_caret_selection(cursor_.selection());
      utf8_encode(text.view().slice(selection), data8).unwrap();
      erase(text, selection);
      perform_layout();
      clipboard.set(MIME_TEXT_UTF8, data8.view().as_u8()).unwrap();
    }
    break;
    case TextCommand::Copy:
    {
      Vec<c8> data8{scratch_allocator};
      if (!cursor_.has_selection())
      {
        auto cp = layout.get_caret_codepoint(cursor_.caret());
        cursor_.select(layout.lines[cp.line].carets);
      }

      auto selection = layout.get_caret_selection(cursor_.selection());
      utf8_encode(text.view().slice(selection), data8).unwrap();
      clipboard.set(MIME_TEXT_UTF8, data8.view().as_u8()).unwrap();
    }
    break;
    case TextCommand::Undo:
    {
      undo(text).match([&](Slice inserted) {
        cursor_.select(layout.to_caret_selection(inserted));
        perform_layout();
      });
    }
    break;
    case TextCommand::Redo:
    {
      redo(text).match([&](Slice inserted) {
        cursor_.select(layout.to_caret_selection(inserted));
        perform_layout();
      });
    }
    break;
    case TextCommand::Hit:
    {
      auto [caret, loc] = rendered.hit(center, aligned_width, transform, pos);
      caret_alignment_  = loc.x;
      cursor_.move_to(layout.align_caret(loc));
    }
    break;
    case TextCommand::HitSelect:
    {
      auto [caret, loc] = rendered.hit(center, aligned_width, transform, pos);
      caret_alignment_  = loc.x;
      cursor_.span_to(layout.align_caret(loc));
    }
    break;

    case TextCommand::None:
    default:
      break;
  }

  cursor_.normalize(layout.num_carets);
}

}    // namespace ash
