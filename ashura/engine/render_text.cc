/// SPDX-License-Identifier: MIT
#include "ashura/engine/render_text.h"
#include "ashura/engine/font_system.h"
#include "ashura/engine/systems.h"
#include "ashura/std/range.h"
#include "ashura/std/text.h"
#include "ashura/std/trace.h"

namespace ash
{

TextRunsStyle TextRunsStyle::all(TextStyle const & style, FontStyle const & font)
{
    SmallVec<usize, 4, 0>     run_indices{noop_allocator};
    SmallVec<TextStyle, 1, 0> styles{noop_allocator};
    SmallVec<FontStyle, 1, 0> fonts{noop_allocator};

    run_indices.append(span({0uz, USIZE_MAX})).unwrap();
    styles.push(style).unwrap();
    fonts.push(font).unwrap();

    return TextRunsStyle{std::move(run_indices), std::move(styles), std::move(fonts)};
}

TextRunsStyle TextRunsStyle::make_sized(Allocator             allocator,
                                        Span<usize const>     run_sizes,
                                        Span<TextStyle const> styles,
                                        Span<FontStyle const> fonts)
{
    ASH_CHECK(!run_sizes.is_empty(), "run_sizes cannot be empty");
    ASH_CHECK(run_sizes.size() == styles.size(),
              "run_sizes and styles must have the same size");
    ASH_CHECK(run_sizes.size() == fonts.size(),
              "run_sizes and fonts must have the same size");

    auto run_indices =
      SmallVec<usize, 4, 0>::make(run_sizes.size() + 1, allocator).unwrap();
    auto styles_vec = small_vec::copy<1, 0>(allocator, styles).unwrap();
    auto fonts_vec  = small_vec::copy<1, 0>(allocator, fonts).unwrap();

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

TextRunsStyle TextRunsStyle::copy(Allocator allocator) const
{
    return TextRunsStyle::make_indexed(allocator, run_indices_.view(), styles_.view(),
                                       fonts_.view());
}

TextRunsStyle TextRunsStyle::make_indexed(Allocator             allocator,
                                          Span<usize const>     run_indices,
                                          Span<TextStyle const> styles,
                                          Span<FontStyle const> fonts)
{
    ASH_CHECK(!run_indices.is_empty(), "run_indices cannot be empty");
    ASH_CHECK(run_indices.size() >= 2, "run_indices must have at least two entries");
    ASH_CHECK(run_indices.first() == 0, "first entry of run_indices must be 0");
    ASH_CHECK(run_indices.size() - 1 == styles.size(),
              "run_indices and styles must have compatible sizes");
    ASH_CHECK(run_indices.size() - 1 == fonts.size(),
              "run_indices and fonts must have compatible sizes");

    auto run_indices_vec = small_vec::copy<4, 0>(allocator, run_indices).unwrap();

    if (run_indices_vec.last() != USIZE_MAX)
    {
        run_indices_vec.last() = USIZE_MAX;
    }

    auto styles_vec = small_vec::copy<1, 0>(allocator, styles).unwrap();
    auto fonts_vec  = small_vec::copy<1, 0>(allocator, fonts).unwrap();

    return TextRunsStyle{std::move(run_indices_vec), std::move(styles_vec),
                         std::move(fonts_vec)};
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
    flush_text();
    return *this;
}

RenderText & RenderText::use_kerning(bool use_kerning)
{
    use_kerning_ = use_kerning;
    flush_text();
    return *this;
}

RenderText & RenderText::use_ligatures(bool use_ligatures)
{
    use_ligatures_ = use_ligatures;
    flush_text();
    return *this;
}

RenderText & RenderText::font_scale(f32 scale)
{
    font_scale_ = scale;
    flush_text();
    return *this;
}

RenderText & RenderText::direction(TextDirection direction)
{
    direction_ = direction;
    flush_text();
    return *this;
}

RenderText & RenderText::language(Str language)
{
    language_ = language;
    flush_text();
    return *this;
}

RenderText & RenderText::alignment(f32 alignment)
{
    alignment_ = alignment;
    flush_text();
    return *this;
}

Str32 RenderText::str() const
{
    return str_.get();
}

RenderText & RenderText::str(Rc<Str32> utf32, TextStyle const & style,
                             FontStyle const & font)
{
    str(std::move(utf32));
    style_runs(TextRunsStyle::all(style, font));
    flush_text();
    return *this;
}

RenderText & RenderText::str(Rc<Str32> utf32)
{
    str_ = std::move(utf32);
    flush_text();
    return *this;
}

RenderText & RenderText::str_copy(Str32 utf32)
{
    auto vec    = vec::copy(allocator_, utf32).unwrap();
    auto rc_vec = rc<Vec<c32>>(allocator_, std::move(vec)).unwrap();
    auto view   = rc_vec->view().as_const();
    return str(transmute(std::move(rc_vec), view));
}

RenderText & RenderText::str_copy(Str32 utf32, TextStyle const & style,
                                  FontStyle const & font)
{
    str_copy(utf32);
    style_runs(TextRunsStyle::all(style, font));
    flush_text();
    return *this;
}

RenderText & RenderText::str_copy(Str8 utf8, TextStyle const & style,
                                  FontStyle const & font)
{
    str_copy(utf8);
    style_runs(TextRunsStyle::all(style, font));
    flush_text();
    return *this;
}

RenderText & RenderText::str_copy(Str8 utf8)
{
    Vec<c32> text32{allocator_};
    utf8_decode(utf8, text32).unwrap();
    auto rc_vec = rc<Vec<c32>>(allocator_, std::move(text32)).unwrap();
    auto view   = rc_vec->view().as_const();
    str_        = transmute(std::move(rc_vec), view);
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
    return str_.get().size();
}

TextBlock RenderText::block() const
{
    return TextBlock{.str           = str_.get(),
                     .run_indices   = runs_style_.run_indices_,
                     .fonts         = runs_style_.fonts_,
                     .font_scale    = font_scale_,
                     .direction     = direction_,
                     .language      = language_,
                     .wrap          = wrap_,
                     .use_kerning   = use_kerning_,
                     .use_ligatures = use_ligatures_};
}

TextBlockStyle RenderText::block_style() const
{
    return TextBlockStyle{.alignment = alignment_, .user_data = user_data_};
}

TextLayout const & RenderText::get_layout() const
{
    return layout_;
}

void RenderText::layout(f32 max_width, f32 align_width, Allocator scratch)
{
    if (hash_ == HASH_CLEAN && max_width == layout_.max_width &&
        align_width == layout_.align_width)
    {
        return;
    }

    layout_ = sys.font->layout_text(block(), max_width, align_width, scratch);
    hash_   = HASH_CLEAN;
}

Tuple<TextRenderInfo, TextPlacement>
  RenderText::place(f32x2 center, f32x4x4 const & transform, CRect const & clip,
                    Span<Slice const>              highlights,
                    Span<TextHighlightStyle const> highlight_styles,
                    Span<usize const> carets, Span<CaretStyle const> caret_styles,
                    Allocator allocator) const
{
    TextRenderInfo info{.center           = center,
                        .transform        = transform,
                        .clip             = clip,
                        .block            = block(),
                        .style            = block_style(),
                        .runs             = runs_style_.styles_,
                        .highlights       = highlights,
                        .highlight_styles = highlight_styles,
                        .carets           = carets,
                        .caret_styles     = caret_styles};

    return {info, layout_.place(info, allocator)};
}

Tuple<isize, CaretAlignment> RenderText::hit(f32x2 center, f32x4x4 const & transform,
                                             f32x2 transformed_pos) const
{
    auto inv_xfm   = inverse(transform);
    auto pos       = ash::transform(inv_xfm, transformed_pos.append(0)).xy();
    auto local_pos = pos - center;
    return layout_.hit(block(), block_style(align_width), local_pos);
}

EditHistoryBuffer EditHistoryBuffer::create(Allocator allocator, usize records_capacity)
{
    ASH_CHECK(records_capacity > 0, "");
    return EditHistoryBuffer{Vec<Record>::make(records_capacity, allocator).unwrap()};
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
        case RecordType::Erase:
        {
            str.erase(Slice::slice(record.pos, record.size()));
            return Slice::slice(record.pos, 0);
        }

        case RecordType::Insert:
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
        case RecordType::Erase:
        {
            str.insert(record.pos, record.str.alias()).unwrap();
            return Slice::slice(record.pos, record.size());
        }

        case RecordType::Insert:
        {
            str.erase(Slice::slice(record.pos, record.size()));
            return Slice::slice(record.pos, 0);
        }

        default:
            ASH_UNREACHABLE;
    }
}

void EditHistoryBuffer::add_record(RecordType type, usize pos, Rc<Str32> str)
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

void EditHistoryBuffer::insert(usize pos, PieceTable32 & str, Rc<Str32> insert_str)
{
    auto record_str = insert_str.alias();
    str.insert(pos, std::move(insert_str)).unwrap();
    add_record(RecordType::Insert, pos, std::move(record_str));
}

void EditHistoryBuffer::erase(Slice selection, PieceTable32 & str)
{
    ASH_CHECK(str.num_pieces() == 1, "");

    selection   = selection(str.size());
    auto substr = std::move(str.pieces_[0].subslice(selection).buffer_);

    str.erase(selection);

    add_record(RecordType::Erase, selection.offset, std::move(substr));
}

static constexpr bool is_symbol(Span<c32 const> symbols, c32 c)
{
    return !find(symbols, c).is_empty();
}

template <typename Fn>
static constexpr Option<usize> seek(Str32 text, usize pos, bool left, Fn && pred)
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

static constexpr Option<usize> seek_sym(Str32 str, usize pos, bool left,
                                        Span<c32 const> symbols)
{
    return seek(str, pos, left, [&](c32 c) { return is_symbol(symbols, c); });
}

template <typename Fn>
static constexpr Slice span_boundary(Str32 str, usize pos, Fn && pred)
{
    if (pos >= str.size())
    {
        return Slice{pos, 0};
    }

    if (pred(str[pos]))
    {
        auto neg   = [&](auto cp) { return !pred(cp); };
        auto begin = seek(str, pos, true, neg).unwrap_or(USIZE_MAX) + 1;
        auto end   = seek(str, pos, false, neg).unwrap_or(str.size());
        return Slice::offsets(begin, end);
    }
    else
    {
        auto begin = seek(str, pos, true, pred).unwrap_or(USIZE_MAX) + 1;
        auto end   = seek(str, pos, false, pred).unwrap_or(str.size());
        return Slice::offsets(begin, end);
    }
}

static constexpr Slice span_sym_boundary(Str32 text, usize pos, Span<c32 const> symbols)
{
    return span_boundary(text, pos, [&](c32 c) { return is_symbol(symbols, c); });
}

static inline Option<isize> translate_caret(TextLayout const & layout, isize caret,
                                            CaretXAlignment alignment,
                                            isize           line_displacement)
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

InteractableText InteractableText::create(Allocator allocator, bool use_async)
{
    return InteractableText{
      allocator, use_async,
      rc<State>(allocator, State{.text    = RenderText{allocator},
                                 .history = EditHistoryBuffer::create(
                                   allocator, InteractableText::DEFAULT_RECORDS_SIZE)})
        .unwrap()};
}

bool InteractableText::has_pending_edit() const
{
    return pending_result_.is_some();
}

Str32 InteractableText::str() const
{
    return state_->text.str();
}

TextLayout const & InteractableText::get_layout() const
{
    return state_->text.get_layout();
}

RenderText const & InteractableText::get_render_text() const
{
    return state_->text;
}

Vec<c32> InteractableText::copy(Allocator allocator)
{
    auto & layout = get_layout();
    auto   cursor = cursor_.v().indices;

    if (!cursor.has_selection())
    {
        auto cp = layout.get_caret_codepoint(cursor.caret());
        cursor.select(layout.lines[cp.line].carets);
    }

    cursor.normalize(layout.num_carets);
    auto slice = layout.get_caret_selection(cursor.selection());

    return vec::copy(allocator, str().slice(slice)).unwrap();
}

void InteractableText::add_cursor(TextCursor         value,
                                  TextHighlightStyle cursor_highlight_style,
                                  CaretStyle         caret_style)
{
    cursor_ = CursorData{.action_stamp = next_action_stamp_++, .indices = value};
    cursor_highlight_style_ = cursor_highlight_style;
    cursor_caret_style_     = caret_style;
}

void InteractableText::remove_cursor()
{
    cursor_ = none;
}

// [ ] need to correctly handle adjusting cursors if in non-async mode; they need to be completed before the next action.
void InteractableText::update_cursor(TextCursor         value,
                                     TextHighlightStyle cursor_highlight_style,
                                     CaretStyle         caret_style)
{
    cursor_.v() = CursorData{.action_stamp = next_action_stamp_++, .indices = value};
    cursor_highlight_style_ = cursor_highlight_style;
    cursor_caret_style_     = caret_style;
}

void InteractableText::unselect()
{
    auto & cursor = cursor_.v();
    cursor.indices.unselect();
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::left()
{
    auto & cursor = cursor_.v();
    cursor.indices.translate(-1).normalize(get_layout().num_carets);
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::right()
{
    auto & cursor = cursor_.v();
    cursor.indices.translate(1).normalize(get_layout().num_carets);
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::word_start(Span<c32 const> word_symbols)
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    auto   cp     = layout.get_caret_codepoint(cursor.indices.caret());
    cursor.indices.move_to(layout.to_caret(
      seek_sym(str(), cp.codepoint + (cp.after ? 1 : 0), true, word_symbols)
        .unwrap_or(),
      true));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::word_end(Span<c32 const> word_symbols)
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    auto   cp     = layout.get_caret_codepoint(cursor.indices.caret());
    cursor.indices.move_to(layout.to_caret(
      seek_sym(str(), cp.codepoint, false, word_symbols).unwrap_or(), true));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::line_start()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    auto   cp     = layout.get_caret_codepoint(cursor.indices.caret());
    cursor.indices.move_to(layout.lines[cp.line].carets.first());
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::line_end()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    auto   cp     = layout.get_caret_codepoint(cursor.indices.caret());
    cursor.indices.move_to(layout.lines[cp.line].carets.last());
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::up()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    cursor.indices.move_to(
      translate_caret(layout, cursor.indices.caret(), caret_alignment_, -1)
        .unwrap_or(cursor.indices.caret()));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::down()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    cursor.indices.move_to(
      translate_caret(layout, cursor.indices.caret(), caret_alignment_, 1)
        .unwrap_or(cursor.indices.caret()));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::page_up(usize lines_per_page)
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    cursor.indices.move_to(translate_caret(layout, cursor.indices.caret(),
                                           caret_alignment_, -(isize) lines_per_page)
                             .unwrap_or(cursor.indices.caret()));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::page_down(usize lines_per_page)
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    cursor.indices.move_to(translate_caret(layout, cursor.indices.caret(),
                                           caret_alignment_, (isize) lines_per_page)
                             .unwrap_or(cursor.indices.caret()));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_left()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    cursor.indices.extend_selection(-1).normalize(layout.num_carets);
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_right()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    cursor.indices.extend_selection(1).normalize(layout.num_carets);
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_up()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    cursor.indices.span_to(
      translate_caret(layout, cursor.indices.caret(), caret_alignment_, -1)
        .unwrap_or(cursor.indices.caret()));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_down()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    cursor.indices.span_to(
      translate_caret(layout, cursor.indices.caret(), caret_alignment_, 1)
        .unwrap_or(cursor.indices.caret()));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_to_word_start(Span<c32 const> word_symbols)
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    auto   cp     = layout.get_caret_codepoint(cursor.indices.caret());
    cursor.indices.span_to(layout.to_caret(
      seek_sym(str(), cp.codepoint, true, word_symbols).unwrap_or(), true));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_to_word_end(Span<c32 const> word_symbols)
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    auto   cp     = layout.get_caret_codepoint(cursor.indices.caret());
    cursor.indices.span_to(layout.to_caret(
      seek_sym(str(), cp.codepoint, false, word_symbols).unwrap_or(), true));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_to_line_start()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    auto   cp     = layout.get_caret_codepoint(cursor.indices.caret());
    cursor.indices.span_to(layout.lines[cp.line].carets.first());
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_to_line_end()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    auto   cp     = layout.get_caret_codepoint(cursor.indices.caret());
    cursor.indices.span_to(layout.lines[cp.line].carets.last());
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_page_up(usize lines_per_page)
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    cursor.indices.span_to(translate_caret(layout, cursor.indices.caret(),
                                           caret_alignment_, -(isize) lines_per_page)
                             .unwrap_or(cursor.indices.caret()));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_page_down(usize lines_per_page)
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    cursor.indices.span_to(translate_caret(layout, cursor.indices.caret(),
                                           caret_alignment_, (isize) lines_per_page)
                             .unwrap_or(cursor.indices.caret()));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_codepoint()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    cursor.indices.span_by(1).normalize(layout.num_carets);
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_word(Span<c32 const> word_symbols)
{
    auto & layout    = get_layout();
    auto & cursor    = cursor_.v();
    auto   selection = span_sym_boundary(
      str(), layout.get_caret_codepoint(cursor.indices.caret()).codepoint,
      word_symbols);
    cursor.indices.select(layout.get_caret_selection(selection));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_line()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    auto   cp     = layout.get_caret_codepoint(cursor.indices.caret());
    cursor.indices.select(layout.lines[cp.line].carets);
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::select_all()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();
    cursor.indices.select(Slice{0, layout.num_carets});
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::hit(f32x2 center, f32x4x4 const & transform, f32x2 pos)
{
    auto & cursor     = cursor_.v();
    auto [caret, loc] = state_->text.hit(center, transform, pos);
    caret_alignment_  = loc.x;
    cursor.indices.move_to(state_->text.get_layout().align_caret(loc));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::hit_select(f32x2 center, f32x4x4 const & transform, f32x2 pos)
{
    auto & cursor     = cursor_.v();
    auto [caret, loc] = state_->text.hit(center, transform, pos);
    caret_alignment_  = loc.x;
    cursor.indices.span_to(state_->text.get_layout().align_caret(loc));
    cursor.action_stamp = next_action_stamp_++;
}

void InteractableText::backspace()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();

    if (!cursor.indices.has_selection())
    {
        cursor.indices.translate(-1).span_by(1).normalize(layout.num_carets);
    }

    auto action_stamp = next_action_stamp_++;
    auto indices      = cursor.indices;

    cursor.indices.unselect_left();
    cursor.action_stamp = action_stamp;

    run_action_(EraseAction{
      .max_width = max_width_, .align_width = align_width_, .indices = indices});
}

void InteractableText::del()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();

    if (!cursor.indices.has_selection())
    {
        cursor.indices.span_by(1).normalize(layout.num_carets);
    }

    auto action_stamp = next_action_stamp_++;
    auto indices      = cursor.indices;

    cursor.indices.unselect_left();
    cursor.action_stamp = action_stamp;

    run_action_(EraseAction{
      .max_width = max_width_, .align_width = align_width_, .indices = indices});
}

void InteractableText::insert(Rc<Str32> input)
{
    auto & cursor       = cursor_.v();
    auto   action_stamp = next_action_stamp_++;
    cursor.action_stamp = action_stamp;

    if (auto selection = cursor.indices.selection(); !selection.is_empty())
    {
        auto erase = cursor.indices;
        cursor.indices.unselect_left();
        run_action_(EraseAction{
          .max_width = max_width_, .align_width = align_width_, .indices = erase});
    }

    run_action_(InsertAction{.max_width   = max_width_,
                             .align_width = align_width_,
                             .indices     = cursor.indices,
                             .str         = std::move(input)});
}

void InteractableText::new_line()
{
    return insert(static_rc(U"\n"_str));
}

void InteractableText::tab()
{
    return insert(static_rc(U"\t"_str));
}

StrVec32 InteractableText::copy_cut(Allocator allocator)
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();

    if (!cursor.indices.has_selection())
    {
        auto cp = layout.get_caret_codepoint(cursor.indices.caret());
        cursor.indices.select(layout.lines[cp.line].carets)
          .normalize(layout.num_carets);
    }

    auto action_stamp = next_action_stamp_++;
    auto selection    = cursor.indices.selection();
    auto out          = vec::copy(allocator, str().slice(selection)).unwrap();
    auto indices      = cursor.indices;
    cursor.indices.unselect_left();
    cursor.action_stamp = action_stamp;

    run_action_(EraseAction{
      .max_width = max_width_, .align_width = align_width_, .indices = indices});

    return out;
}

void InteractableText::cut()
{
    auto & layout = get_layout();
    auto & cursor = cursor_.v();

    if (!cursor.indices.has_selection())
    {
        auto cp = layout.get_caret_codepoint(cursor.indices.caret());
        cursor.indices.select(layout.lines[cp.line].carets);
    }

    auto action_stamp = next_action_stamp_++;
    auto indices      = cursor.indices;

    cursor.indices.unselect_left();
    cursor.action_stamp = action_stamp;

    run_action_(EraseAction{
      .max_width = max_width_, .align_width = align_width_, .indices = indices});
}

void InteractableText::undo()
{
    next_action_stamp_++;
    run_action_(UndoAction{.max_width = max_width_, .align_width = align_width_});
}

void InteractableText::redo()
{
    next_action_stamp_++;
    run_action_(RedoAction{.max_width = max_width_, .align_width = align_width_});
}

void InteractableText::layout(f32 max_width)
{
    next_action_stamp_++;
    max_width_ = max_width;
    run_action_(RelayoutAction{.max_width = max_width_, .renderer = renderer_.alias()});
}

void InteractableText::set_renderer(Rc<Renderer> renderer)
{
    next_action_stamp_++;
    renderer_ = std::move(renderer);
    run_action_(RelayoutAction{.max_width = max_width_, .renderer = renderer_.alias()});
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
        action_queue_ = SmallVec<Edit, 8, 0>{allocator_};

        pending_result_ =
          sys.sched
            ->run(
              allocator_, WorkerThread::Any,
              [allocator = allocator_, actions = std::move(actions),
               previous_state_ = state_.alias(),
               history         = std::move(state_->history)]() mutable {
                  tracing::ScopeTrace trace{"EditText::tick::apply_actions"_str};

                  auto               cursors = SmallVec<Cursor, 8, 0>{allocator};
                  Option<RenderText> text    = none;

                  auto render = [&](PieceTable32 const & pieces, f32 max_width,
                                    Renderer const & renderer) {
                      StrVec32 str{allocator};
                      pieces.compact(Slice::all(), str).unwrap();
                      auto rc_strvec = rc<StrVec32>(allocator, std::move(str)).unwrap();
                      auto view      = rc_strvec->view().as_const();
                      auto rc_str32  = transmute(std::move(rc_strvec), view);
                      auto text      = renderer(allocator, std::move(rc_str32));
                      auto scratch = IFallbackAllocator{get_thread_arena(), allocator};
                      text.layout(max_width, scratch);
                      return text;
                  };

                  for (auto & action : actions)
                  {
                      auto & layout = text.is_some() ?
                                        text->get_layout() :
                                        previous_state_->text.get_layout();
                      auto & str =
                        text.is_some() ? text->text_ : previous_state_->text.text_;
                      PieceTable32 pieces{allocator};
                      pieces.insert(0, str.alias()).unwrap();

                      action.match(
                        [&](InsertAction & a) {
                            a.cursor.normalize(layout.num_carets);
                            auto cp = layout.get_caret_codepoint(a.cursor.caret());
                            auto codepoint = cp.codepoint + (cp.after ? 1 : 0);
                            history.insert(codepoint, pieces, a.str.alias());
                            text              = render(pieces, a.max_width, a.renderer);
                            auto & new_layout = text->get_layout();
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
                            text        = render(pieces, a.max_width, a.renderer);
                            auto cursor = TextCursor{};
                            cursor.move_to(a.cursor.left_caret())
                              .normalize(text->get_layout().num_carets);
                            cursors.push(Cursor{.id = a.id, .v = cursor}).unwrap();
                        },
                        [&](UndoAction & a) {
                            history.undo(pieces).match(
                              [&](Slice insertion) {
                                  text = render(pieces, a.max_width, a.renderer);
                                  auto & new_layout = text->get_layout();
                                  auto   selection =
                                    new_layout.to_caret_selection(insertion);
                                  auto cursor = TextCursor{};
                                  cursor.select(selection);
                                  cursors.push(Cursor{.id = a.id, .v = cursor})
                                    .unwrap();
                              },
                              [&] { text = render(pieces, a.max_width, a.renderer); });
                        },
                        [&](RedoAction & a) {
                            history.redo(pieces).match(
                              [&](Slice insertion) {
                                  text = render(pieces, a.max_width, a.renderer);
                                  auto & new_layout = text->get_layout();
                                  auto   selection =
                                    new_layout.to_caret_selection(insertion);
                                  auto cursor = TextCursor{};
                                  cursor.select(selection);
                                  cursors.push(Cursor{.id = a.id, .v = cursor})
                                    .unwrap();
                              },
                              [&] { text = render(pieces, a.max_width, a.renderer); });
                        },
                        [&](RelayoutAction & a) {
                            text = render(pieces, a.max_width, a.renderer);
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

                  return rc(allocator, ActionResult{.text    = std::move(text),
                                                    .history = std::move(history),
                                                    .cursors = std::move(cursors)})
                    .unwrap();
              })
            .unwrap();
    }
}

}    // namespace ash
