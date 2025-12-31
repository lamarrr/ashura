/// SPDX-License-Identifier: MIT
#include "ashura/engine/render_text.h"
#include "ashura/engine/font_system.h"
#include "ashura/engine/systems.h"
#include "ashura/std/range.h"
#include "ashura/std/text.h"
#include "ashura/std/trace.h"

namespace ash
{

TextRunsStyle TextRunsStyle::all(TextStyle const & style,
                                 FontStyle const & font)
{
  SmallVec<usize, 4>     run_indices{noop_allocator};
  SmallVec<TextStyle, 1> styles{noop_allocator};
  SmallVec<FontStyle, 1> fonts{noop_allocator};

  run_indices.append(span({0uz, USIZE_MAX})).unwrap();
  styles.push(style).unwrap();
  fonts.push(font).unwrap();

  return TextRunsStyle{std::move(run_indices), std::move(styles),
                       std::move(fonts)};
}

TextRunsStyle TextRunsStyle::make_sized(Allocator             allocator,
                                        Span<usize const>     run_sizes,
                                        Span<TextStyle const> styles,
                                        Span<FontStyle const> fonts)
{
  CHECK(!run_sizes.is_empty(), "run_sizes cannot be empty");
  CHECK(run_sizes.size() == styles.size(),
        "run_sizes and styles must have the same size");
  CHECK(run_sizes.size() == fonts.size(),
        "run_sizes and fonts must have the same size");

  auto run_indices =
    SmallVec<usize, 4>::make(run_sizes.size() + 1, allocator).unwrap();
  auto styles_vec = small_vec::copy<1>(allocator, styles).unwrap();
  auto fonts_vec  = small_vec::copy<1>(allocator, fonts).unwrap();

  run_indices.push(0uz).unwrap();
  usize run_start = 0;

  for (auto run_size : run_sizes)
  {
    auto run_end = sat_add(run_start, run_size);
    run_indices.push(run_end).unwrap();
    run_start = run_end;
  }

  if (run_indices.last() != USIZE_MAX)
  {
    run_indices.last() = USIZE_MAX;
  }

  return TextRunsStyle{std::move(run_indices), std::move(styles_vec),
                       std::move(fonts_vec)};
}

TextRunsStyle TextRunsStyle::make_indexed(Allocator             allocator,
                                          Span<usize const>     run_indices,
                                          Span<TextStyle const> styles,
                                          Span<FontStyle const> fonts)
{
  CHECK(!run_indices.is_empty(), "run_indices cannot be empty");
  CHECK(run_indices.size() >= 2, "run_indices must have at least two entries");
  CHECK(run_indices.first() == 0, "first entry of run_indices must be 0");
  CHECK(run_indices.size() - 1 == styles.size(),
        "run_indices and styles must have compatible sizes");
  CHECK(run_indices.size() - 1 == fonts.size(),
        "run_indices and fonts must have compatible sizes");

  auto run_indices_vec = small_vec::copy<4>(allocator, run_indices).unwrap();

  if (run_indices_vec.last() != USIZE_MAX)
  {
    run_indices_vec.last() = USIZE_MAX;
  }

  auto styles_vec = small_vec::copy<1>(allocator, styles).unwrap();
  auto fonts_vec  = small_vec::copy<1>(allocator, fonts).unwrap();

  return TextRunsStyle{std::move(run_indices_vec), std::move(styles_vec),
                       std::move(fonts_vec)};
}

void TextRunsStyle::update(TextStyle const & style, FontStyle const & font,
                           usize first, usize count)
{
  if (count == 0)
  {
    return;
  }

  if (run_indices_.is_empty())
  {
    run_indices_.push(0uz).unwrap();
    run_indices_.push(USIZE_MAX).unwrap();
    styles_.push(style).unwrap();
    fonts_.push(font).unwrap();
    return;
  }

  auto end = sat_add(first, count);

  auto first_run_span = binary_find(run_indices_.view(), gt, first);

  /// should never happen since there's always a USIZE_MAX run end
  CHECK(!first_run_span.is_empty(), "");

  auto last_run_span = binary_find(first_run_span, geq, end);

  /// should never happen since there's always a USIZE_MAX run end
  CHECK(!last_run_span.is_empty(), "");

  auto first_run =
    ((usize) (first_run_span.pbegin() - run_indices_.view().pbegin())) - 1;
  auto last_run =
    ((usize) (last_run_span.pbegin() - run_indices_.view().pbegin())) - 1;

  auto first_run_begin = run_indices_[first_run];
  auto last_run_end    = run_indices_[last_run + 1];

  /// run merging

  /// merge middle

  if (last_run > (first_run + 1))
  {
    auto first_erase = first_run + 1;
    auto num_erase   = last_run - first_erase;
    run_indices_.erase(first_erase + 1, num_erase);
    styles_.erase(first_erase, num_erase);
    fonts_.erase(first_erase, num_erase);
    last_run -= num_erase;
  }

  /// merge left
  if (first_run_begin == first)
  {
    auto first_erase = first_run;
    auto num_erase   = last_run - first_run;
    run_indices_.erase(first_erase + 1, num_erase);
    styles_.erase(first_erase, num_erase);
    fonts_.erase(first_erase, num_erase);
    last_run -= num_erase;
  }

  /// merge right
  if (last_run_end == end)
  {
    auto first_erase = first_run + 1;
    auto num_erase   = (last_run + 1) - first_erase;
    run_indices_.erase(first_erase + 1, num_erase);
    styles_.erase(first_erase, num_erase);
    fonts_.erase(first_erase, num_erase);
    last_run -= num_erase;
  }

  (void) last_run;

  /// run splitting
  if (first_run_begin == first && last_run_end == end)
  {
    styles_[first_run] = style;
    fonts_[first_run]  = font;
  }
  else
  {
    if (first_run_begin == first)
    {
      // split with new on left
      run_indices_.insert(first_run + 1, end).unwrap();
      styles_.insert(first_run, style).unwrap();
      fonts_.insert(first_run, font).unwrap();
    }
    else if (last_run_end == end)
    {
      // split with new on right
      run_indices_[first_run + 1] = first;
      run_indices_.insert(first_run + 1 + 1, end).unwrap();
      styles_.insert(first_run + 1, style).unwrap();
      fonts_.insert(first_run + 1, font).unwrap();
    }
    else
    {
      // split with new in the middle of the run
      run_indices_[first_run + 1] = first;
      run_indices_.insert(first_run + 1 + 1, end).unwrap();
      styles_.insert(first_run + 1, style).unwrap();
      fonts_.insert(first_run + 1, font).unwrap();
      run_indices_.insert(first_run + 2 + 1, last_run_end).unwrap();
      styles_.insert(first_run + 2, styles_[first_run]).unwrap();
      fonts_.insert(first_run + 2, fonts_[first_run]).unwrap();
    }
  }

  return;
}

RenderText & RenderText::style_runs(TextRunsStyle style)
{
  runs_style_ = std::move(style);
  hash_       = HASH_DIRTY;
  return *this;
}

TextRunsStyle const & RenderText::get_style_runs() const
{
  return runs_style_;
}

RenderText & RenderText::flush_text()
{
  hash_ = HASH_DIRTY;
  return *this;
}

RenderText & RenderText::wrap(bool wrap)
{
  wrap_ = wrap;
  return *this;
}

RenderText & RenderText::font_scale(f32 scale)
{
  font_scale_ = scale;
  return *this;
}

RenderText & RenderText::direction(TextDirection direction)
{
  if (direction_ == direction)
  {
    return *this;
  }
  direction_ = direction;
  flush_text();
  return *this;
}

RenderText & RenderText::language(Str language)
{
  if (range_eq(language_, language))
  {
    return *this;
  }
  language_ = language;
  flush_text();
  return *this;
}

RenderText & RenderText::alignment(f32 alignment)
{
  if (alignment_ == alignment)
  {
    return *this;
  }
  alignment_ = alignment;
  flush_text();
  return *this;
}

Str32 RenderText::get_text() const
{
  return text_.get();
}

RenderText & RenderText::text(Rc<Str32> utf32, TextStyle const & style,
                              FontStyle const & font)
{
  text(std::move(utf32));
  style_runs(TextRunsStyle::all(style, font));
  flush_text();
  return *this;
}

RenderText & RenderText::text(Rc<Str32> utf32)
{
  text_ = std::move(utf32);
  flush_text();
  return *this;
}

RenderText & RenderText::text_copy(Str32 utf32)
{
  auto vec    = vec::copy(allocator_, utf32).unwrap();
  auto rc_vec = rc<Vec<c32>>(allocator_, std::move(vec)).unwrap();
  auto view   = rc_vec->view().as_const();
  return text(transmute(std::move(rc_vec), view));
}

RenderText & RenderText::text_copy(Str32 utf32, TextStyle const & style,
                                   FontStyle const & font)
{
  text_copy(utf32);
  style_runs(TextRunsStyle::all(style, font));
  flush_text();
  return *this;
}

RenderText & RenderText::text_copy(Str8 utf8, TextStyle const & style,
                                   FontStyle const & font)
{
  text_copy(utf8);
  style_runs(TextRunsStyle::all(style, font));
  flush_text();
  return *this;
}

RenderText & RenderText::text_copy(Str8 utf8)
{
  Vec<c32> text32{allocator_};
  utf8_decode(utf8, text32).unwrap();
  auto rc_vec = rc<Vec<c32>>(allocator_, std::move(text32)).unwrap();
  auto view   = rc_vec->view().as_const();
  text_       = transmute(std::move(rc_vec), view);
  flush_text();
  return *this;
}

RenderText & RenderText::user_data(void * data)
{
  user_data_ = data;
  return *this;
}

usize RenderText::size() const
{
  return text_.get().size();
}

TextBlock RenderText::block() const
{
  return TextBlock{.text          = text_,
                   .run_indices   = runs_style_.run_indices_,
                   .fonts         = runs_style_.fonts_,
                   .font_scale    = font_scale_,
                   .direction     = direction_,
                   .language      = language_,
                   .wrap          = wrap_,
                   .use_kerning   = use_kerning_,
                   .use_ligatures = use_ligatures_};
}

TextBlockStyle RenderText::block_style(f32 aligned_width) const
{
  return TextBlockStyle{.alignment   = alignment_,
                        .align_width = aligned_width,
                        .user_data   = user_data_};
}

TextLayout const & RenderText::get_layout() const
{
  return layout_;
}

void RenderText::layout(f32 max_width, Allocator scratch)
{
  if (hash_ == HASH_CLEAN && max_width == layout_.max_width)
  {
    return;
  }

  sys.font->layout_text(block(), max_width, layout_, scratch);
  hash_ = HASH_CLEAN;
}

void RenderText::render(TextRenderer renderer, f32x2 center, f32 align_width,
                        f32x4x4 const & transform, CRect const & clip,
                        Span<Slice const>              highlights,
                        Span<TextHighlightStyle const> highlight_styles,
                        Span<usize const>              carets,
                        Span<CaretStyle const>         caret_styles,
                        Allocator                      scratch_allocator) const
{
  TextRenderInfo info{.center           = center,
                      .transform        = transform,
                      .clip             = clip,
                      .block            = block(),
                      .style            = block_style(align_width),
                      .runs             = runs_style_.styles_,
                      .highlights       = highlights,
                      .highlight_styles = highlight_styles,
                      .carets           = carets,
                      .caret_styles     = caret_styles};

  layout_.render(renderer, info, scratch_allocator);
}

Tuple<isize, CaretAlignment> RenderText::hit(f32x2 center, f32 align_width,
                                             f32x4x4 const & transform,
                                             f32x2 transformed_pos) const
{
  auto inv_xfm   = inverse(transform);
  auto pos       = ash::transform(inv_xfm, transformed_pos.append(0)).xy();
  auto local_pos = pos - center;
  return layout_.hit(block(), block_style(align_width), local_pos);
}

EditHistoryBuffer EditHistoryBuffer::create(Allocator allocator,
                                            usize     records_capacity)
{
  CHECK(records_capacity > 0, "");
  return EditHistoryBuffer{
    Vec<Record>::make(records_capacity, allocator).unwrap()};
}

Option<Slice> EditHistoryBuffer::redo(PieceTable32 & str)
{
  if ((current_record_ + 1) >= records_.size())
  {
    return none;
  }

  current_record_++;

  // apply changes of next record
  auto & record = records_[current_record_];

  switch (record.type)
  {
    case Record::Type::Erase:
    {
      str.erase(Slice::slice(record.pos, record.size()));
      return Slice::slice(record.pos, 0);
    }

    case Record::Type::Insert:
    {
      str.insert(record.pos, record.str.alias()).unwrap();
      return Slice::slice(record.pos, record.size());
    }

    default:
      ASH_UNREACHABLE;
  }
}

Option<Slice> EditHistoryBuffer::undo(PieceTable32 & str)
{
  if (current_record_ == 0)
  {
    return none;
  }

  // undo changes of current record
  auto & record = records_[current_record_];
  current_record_--;

  switch (record.type)
  {
    case Record::Type::Erase:
    {
      str.insert(record.pos, record.str.alias()).unwrap();
      return Slice::slice(record.pos, record.size());
    }

    case Record::Type::Insert:
    {
      str.erase(Slice::slice(record.pos, record.size()));
      return Slice::slice(record.pos, 0);
    }

    default:
      ASH_UNREACHABLE;
  }
}

void EditHistoryBuffer::add_record(Record::Type type, usize pos, Rc<Str32> str)
{
  Record record{.pos = pos, .str = std::move(str), .type = type};

  // remove all records after current record
  records_.erase(current_record_ + 1, USIZE_MAX);

  if (records_.push(within_capacity, std::move(record)))
  {
    current_record_ = records_.size() - 1;
    return;
  }

  // truncate half of the history

  records_.erase(Slice::offsets(0, max(records_.size() >> 1, 1uz)));
  records_.push(within_capacity, std::move(record)).unwrap();
  current_record_ = records_.size() - 1;
}

void EditHistoryBuffer::insert(usize pos, PieceTable32 & str,
                               Rc<Str32> insert_str)
{
  auto record_str = insert_str.alias();
  str.insert(pos, std::move(insert_str)).unwrap();
  add_record(Record::Type::Insert, pos, std::move(record_str));
}

void EditHistoryBuffer::erase(Slice selection, PieceTable32 & str)
{
  CHECK(str.num_pieces() == 1, "");

  selection   = selection(str.size());
  auto substr = std::move(str.pieces_[0].subslice(selection).buffer_);

  str.erase(selection);

  add_record(Record::Type::Erase, selection.offset, std::move(substr));
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
    return Slice::offsets(begin, end);
  }
  else
  {
    auto begin = seek(text, pos, true, pred).unwrap_or(USIZE_MAX) + 1;
    auto end   = seek(text, pos, false, pred).unwrap_or(text.size());
    return Slice::offsets(begin, end);
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

EditText::Renderer EditText::default_renderer()
{
  return static_rc<Fn<RenderText(Allocator, Rc<Str32>)>>(
    [](Allocator allocator, Rc<Str32> str) -> RenderText {
      RenderText text{allocator};
      text.text(std::move(str));
      return text;
    });
}

EditText EditText::create(Allocator allocator)
{
  return EditText{
    allocator,
    rc<State>(allocator, State{.text    = RenderText{allocator},
                               .history = EditHistoryBuffer::create(
                                 allocator, EditText::DEFAULT_RECORDS_SIZE)})
      .unwrap(),
    default_renderer()};
}

bool EditText::has_pending_edit() const
{
  return pending_result_.is_some();
}

Str32 EditText::get_text() const
{
  return state_->text.get_text();
}

TextLayout const & EditText::get_layout() const
{
  return state_->text.get_layout();
}

RenderText const & EditText::get_render_text() const
{
  return state_->text;
}

Future<Vec<c32>> EditText::copy()
{
  auto & layout = get_layout();

  auto cursor = cursor_.v;

  if (!cursor.has_selection())
  {
    auto cp = layout.get_caret_codepoint(cursor.caret());
    cursor.select(layout.lines[cp.line].carets);
  }

  auto id = action_id_++;
  cursor_ = Cursor{id, cursor};

  auto out = future<StrVec32>(allocator_).unwrap();

  action_queue_
    .push(CopyAction{.id       = id,
                     .renderer = renderer_.alias(),
                     .cursor   = cursor,
                     .output   = out.alias()})
    .unwrap();

  return out;
}

void EditText::unselect()
{
  auto cursor = cursor_.v;
  cursor.unselect();
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::left()
{
  auto cursor = cursor_.v;
  cursor.translate(-1).normalize(get_layout().num_carets);
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::right()
{
  auto cursor = cursor_.v;
  cursor.translate(1).normalize(get_layout().num_carets);
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::word_start(Span<c32 const> word_symbols)
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  auto   c      = layout.get_caret_codepoint(cursor.caret());
  auto   text   = get_text();
  cursor.move_to(layout.to_caret(
    seek_sym(text, c.codepoint + (c.after ? 1 : 0), true, word_symbols)
      .unwrap_or(),
    true));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::word_end(Span<c32 const> word_symbols)
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  auto   c      = layout.get_caret_codepoint(cursor.caret());
  cursor.move_to(layout.to_caret(
    seek_sym(get_text(), c.codepoint, false, word_symbols).unwrap_or(), true));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::line_start()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  auto   c      = layout.get_caret_codepoint(cursor.caret());
  cursor.move_to(layout.lines[c.line].carets.first());
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::line_end()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  auto   c      = layout.get_caret_codepoint(cursor.caret());
  cursor.move_to(layout.lines[c.line].carets.last());
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::up()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  cursor.move_to(translate_caret(layout, cursor.caret(), caret_alignment_, -1)
                   .unwrap_or(cursor.caret()));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::down()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  cursor.move_to(translate_caret(layout, cursor.caret(), caret_alignment_, 1)
                   .unwrap_or(cursor.caret()));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::page_up(usize lines_per_page)
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  cursor.move_to(translate_caret(layout, cursor.caret(), caret_alignment_,
                                 -(isize) lines_per_page)
                   .unwrap_or(cursor.caret()));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::page_down(usize lines_per_page)
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  cursor.move_to(translate_caret(layout, cursor.caret(), caret_alignment_,
                                 (isize) lines_per_page)
                   .unwrap_or(cursor.caret()));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_left()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  cursor.extend_selection(-1).normalize(layout.num_carets);
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_right()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  cursor.extend_selection(1).normalize(layout.num_carets);
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_up()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  cursor.span_to(translate_caret(layout, cursor.caret(), caret_alignment_, -1)
                   .unwrap_or(cursor.caret()));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_down()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  cursor.span_to(translate_caret(layout, cursor.caret(), caret_alignment_, 1)
                   .unwrap_or(cursor.caret()));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_to_word_start(Span<c32 const> word_symbols)
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  auto   c      = layout.get_caret_codepoint(cursor.caret());
  cursor.span_to(layout.to_caret(
    seek_sym(get_text(), c.codepoint, true, word_symbols).unwrap_or(), true));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_to_word_end(Span<c32 const> word_symbols)
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  auto   c      = layout.get_caret_codepoint(cursor.caret());
  cursor.span_to(layout.to_caret(
    seek_sym(get_text(), c.codepoint, false, word_symbols).unwrap_or(), true));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_to_line_start()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  auto   c      = layout.get_caret_codepoint(cursor.caret());
  cursor.span_to(layout.lines[c.line].carets.first());
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_to_line_end()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  auto   c      = layout.get_caret_codepoint(cursor.caret());
  cursor.span_to(layout.lines[c.line].carets.last());
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_page_up(usize lines_per_page)
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  cursor.span_to(translate_caret(layout, cursor.caret(), caret_alignment_,
                                 -(isize) lines_per_page)
                   .unwrap_or(cursor.caret()));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_page_down(usize lines_per_page)
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  cursor.span_to(translate_caret(layout, cursor.caret(), caret_alignment_,
                                 (isize) lines_per_page)
                   .unwrap_or(cursor.caret()));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_codepoint()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  cursor.span_by(1).normalize(layout.num_carets);
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_word(Span<c32 const> word_symbols)
{
  auto & layout    = get_layout();
  auto   cursor    = cursor_.v;
  auto   selection = span_sym_boundary(
    get_text(), layout.get_caret_codepoint(cursor.caret()).codepoint,
    word_symbols);
  cursor.select(layout.get_caret_selection(selection));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_line()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  auto   c      = layout.get_caret_codepoint(cursor.caret());
  cursor.select(layout.lines[c.line].carets);
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::select_all()
{
  auto & layout = get_layout();
  auto   cursor = cursor_.v;
  cursor.select(Slice{0, layout.num_carets});
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::hit(f32x2 center, f32 aligned_width, f32x2 pos,
                   f32x4x4 const & transform)
{
  auto cursor       = cursor_.v;
  auto [caret, loc] = state_->text.hit(center, aligned_width, transform, pos);
  caret_alignment_  = loc.x;
  cursor.move_to(state_->text.get_layout().align_caret(loc));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::hit_select(f32x2 center, f32 aligned_width, f32x2 pos,
                          f32x4x4 const & transform)
{
  auto cursor       = cursor_.v;
  auto [caret, loc] = state_->text.hit(center, aligned_width, transform, pos);
  caret_alignment_  = loc.x;
  cursor.span_to(state_->text.get_layout().align_caret(loc));
  cursor_ = Cursor{action_id_++, cursor};
}

void EditText::backspace()
{
  auto & layout = get_layout();

  auto cursor = cursor_.v;

  if (!cursor.has_selection())
  {
    cursor.translate(-1).span_by(1).normalize(layout.num_carets);
  }

  auto id = action_id_++;

  action_queue_
    .push(EraseAction{.id        = id,
                      .max_width = max_width_,
                      .renderer  = renderer_.alias(),
                      .cursor    = cursor})
    .unwrap();

  cursor.unselect_left();
  cursor_ = Cursor{id, cursor};
}

void EditText::del()
{
  auto & layout = get_layout();

  auto cursor = cursor_.v;

  if (!cursor.has_selection())
  {
    cursor.span_by(1).normalize(layout.num_carets);
  }

  auto id = action_id_++;

  action_queue_
    .push(EraseAction{.id        = id,
                      .max_width = max_width_,
                      .renderer  = renderer_.alias(),
                      .cursor    = cursor})
    .unwrap();

  cursor.unselect_left();
  cursor_ = Cursor{id, cursor};
}

void EditText::insert(Rc<Str32> input)
{
  auto cursor = cursor_.v;

  auto id = action_id_++;

  if (auto selection = cursor.selection(); !selection.is_empty())
  {
    action_queue_
      .push(EraseAction{.id        = id,
                        .max_width = max_width_,
                        .renderer  = renderer_.alias(),
                        .cursor    = cursor})
      .unwrap();
    cursor.unselect_left();
  }

  action_queue_
    .push(InsertAction{.id        = id,
                       .max_width = max_width_,
                       .renderer  = renderer_.alias(),
                       .cursor    = cursor,
                       .str       = std::move(input)})
    .unwrap();

  cursor_ = Cursor{id, cursor};
}

void EditText::new_line()
{
  return insert(static_rc(U"\n"_str));
}

Future<StrVec32> EditText::copy_cut()
{
  auto & layout = get_layout();

  auto cursor = cursor_.v;

  if (!cursor.has_selection())
  {
    auto cp = layout.get_caret_codepoint(cursor.caret());
    cursor.select(layout.lines[cp.line].carets);
  }

  auto out = future<StrVec32>(allocator_).unwrap();

  auto id = action_id_++;

  action_queue_
    .push(CopyAction{.id        = id,
                     .max_width = max_width_,
                     .renderer  = renderer_.alias(),
                     .cursor    = cursor,
                     .output    = out.alias()})
    .unwrap();
  action_queue_
    .push(EraseAction{.id        = id,
                      .max_width = max_width_,
                      .renderer  = renderer_.alias(),
                      .cursor    = cursor})
    .unwrap();

  cursor.unselect_left();
  cursor_ = Cursor{id, cursor};

  return out;
}

void EditText::cut()
{
  auto & layout = get_layout();

  auto cursor = cursor_.v;

  if (!cursor.has_selection())
  {
    auto cp = layout.get_caret_codepoint(cursor.caret());
    cursor.select(layout.lines[cp.line].carets);
  }

  auto id = action_id_++;

  action_queue_
    .push(EraseAction{.id        = id,
                      .max_width = max_width_,
                      .renderer  = renderer_.alias(),
                      .cursor    = cursor})
    .unwrap();

  cursor.unselect_left();
  cursor_ = Cursor{id, cursor};
}

void EditText::undo()
{
  auto id = action_id_++;
  action_queue_
    .push(UndoAction{
      .id = id, .max_width = max_width_, .renderer = renderer_.alias()})
    .unwrap();
}

void EditText::redo()
{
  auto id = action_id_++;
  action_queue_
    .push(RedoAction{
      .id = id, .max_width = max_width_, .renderer = renderer_.alias()})
    .unwrap();
}

void EditText::layout(f32 max_width)
{
  auto id    = action_id_++;
  max_width_ = max_width;
  action_queue_
    .push(RelayoutAction{
      .id = id, .max_width = max_width_, .renderer = renderer_.alias()})
    .unwrap();
}

void EditText::set_renderer(Renderer renderer)
{
  auto id   = action_id_++;
  renderer_ = std::move(renderer);
  action_queue_
    .push(RelayoutAction{
      .id = id, .max_width = max_width_, .renderer = renderer_.alias()})
    .unwrap();
}

void EditText::tick(nanoseconds)
{
  if (pending_result_.is_some())
  {
    auto p = pending_result_->poll();
    if (p.is_ok())
    {
      // apply new state
      auto & result = *p.unwrap();

      auto old_text = std::move(state_->text);

      *state_ = State{.text    = result->text.unwrap_or(std::move(old_text)),
                      .history = std::move(result->history)};

      for (auto & cursor : result->cursors)
      {
        if (cursor.id >= cursor_.id)
        {
          cursor_ = cursor;
          cursor_.v.normalize(state_->text.get_layout().num_carets);
        }
      }

      pending_result_ = none;
    }
  }

  if (pending_result_.is_none() && !action_queue_.is_empty())
  {
    auto actions  = std::move(action_queue_);
    action_queue_ = SmallVec<Edit, 8>{allocator_};

    pending_result_ =
      sys.sched
        ->run(
          allocator_, WorkerThread::Any,
          [allocator = allocator_, actions = std::move(actions),
           previous_state_ = state_.alias(),
           history         = std::move(state_->history)]() mutable {
            tracing::ScopeTrace trace{"EditText::tick::apply_actions"_str};
            auto                cursors = SmallVec<Cursor, 8>{allocator};

            Option<RenderText> rendered = none;

            auto rebuild = [&](PieceTable32 const & pieces, f32 max_width,
                               Renderer const & renderer) {
              StrVec32 text{allocator};
              pieces.compact(Slice::all(), text).unwrap();
              auto rc_text  = rc<StrVec32>(allocator, std::move(text)).unwrap();
              auto view     = rc_text->view().as_const();
              auto rc_str32 = transmute(std::move(rc_text), view);
              auto new_text = renderer.get()(allocator, std::move(rc_str32));
              auto * arena  = get_thread_arena();
              auto   scratch_allocator = IFallbackAllocator{arena, allocator};
              new_text.layout(max_width, scratch_allocator);
              rendered = std::move(new_text);
            };

            for (auto & action : actions)
            {
              auto &       layout = rendered.is_some() ?
                                      rendered->get_layout() :
                                      previous_state_->text.get_layout();
              auto &       str    = rendered.is_some() ? rendered->text_ :
                                                         previous_state_->text.text_;
              PieceTable32 pieces{allocator};
              pieces.insert(0, str.alias()).unwrap();

              action.match(
                [&](InsertAction & a) {
                  a.cursor.normalize(layout.num_carets);
                  auto cp        = layout.get_caret_codepoint(a.cursor.caret());
                  auto codepoint = cp.codepoint + (cp.after ? 1 : 0);
                  history.insert(codepoint, pieces, a.str.alias());
                  rebuild(pieces, a.max_width, a.renderer);
                  auto & new_layout = rendered->get_layout();
                  auto   caret =
                    new_layout.to_caret(codepoint + a.str.get().size(), true);
                  auto cursor = TextCursor{};
                  cursor.move_to(caret).normalize(new_layout.num_carets);
                  cursors.push(Cursor{.id = a.id, .v = cursor}).unwrap();
                },
                [&](EraseAction & a) {
                  a.cursor.normalize(layout.num_carets);
                  auto selection =
                    layout.get_caret_selection(a.cursor.selection());
                  history.erase(selection, pieces);
                  rebuild(pieces, a.max_width, a.renderer);
                  auto cursor = TextCursor{};
                  cursor.move_to(a.cursor.left_caret())
                    .normalize(rendered->get_layout().num_carets);
                  cursors.push(Cursor{.id = a.id, .v = cursor}).unwrap();
                },
                [&](UndoAction & a) {
                  history.undo(pieces).match(
                    [&](Slice insertion) {
                      rebuild(pieces, a.max_width, a.renderer);
                      auto & new_layout = rendered->get_layout();
                      auto selection = new_layout.to_caret_selection(insertion);
                      auto cursor    = TextCursor{};
                      cursor.select(selection);
                      cursors.push(Cursor{.id = a.id, .v = cursor}).unwrap();
                    },
                    [&] { rebuild(pieces, a.max_width, a.renderer); });
                },
                [&](RedoAction & a) {
                  history.redo(pieces).match(
                    [&](Slice insertion) {
                      rebuild(pieces, a.max_width, a.renderer);
                      auto & new_layout = rendered->get_layout();
                      auto selection = new_layout.to_caret_selection(insertion);
                      auto cursor    = TextCursor{};
                      cursor.select(selection);
                      cursors.push(Cursor{.id = a.id, .v = cursor}).unwrap();
                    },
                    [&] { rebuild(pieces, a.max_width, a.renderer); });
                },
                [&](RelayoutAction & a) {
                  rebuild(pieces, a.max_width, a.renderer);
                },
                [&](CopyAction & a) {
                  a.cursor.normalize(layout.num_carets);
                  auto selection =
                    layout.get_caret_selection(a.cursor.selection());
                  Vec<c32> output{allocator};
                  output.reserve(selection.span).unwrap();
                  pieces.compact(selection, output).unwrap();
                  a.output.yield(std::move(output)).unwrap();
                });
            }

            return rc(allocator, ActionResult{.text    = std::move(rendered),
                                              .history = std::move(history),
                                              .cursors = std::move(cursors)})
              .unwrap();
          })
        .unwrap();
  }
}

}    // namespace ash
