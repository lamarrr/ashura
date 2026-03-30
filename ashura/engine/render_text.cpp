/// SPDX-License-Identifier: MIT
#include "ashura/engine/render_text.hpp"
#include "ashura/engine/font_system.hpp"
#include "ashura/engine/systems.hpp"
#include "ashura/std/range.hpp"
#include "ashura/std/text.hpp"

namespace ash
{

TextRunsStyle TextRunsStyle::all(TextStyle const & style, FontStyle const & font)
{
    SmallVec<usize, 2, 0>     run_indices{noop_allocator};
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
      SmallVec<usize, 2, 0>::make(run_sizes.size() + 1, allocator).unwrap();
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

    auto run_indices_vec = small_vec::copy<2, 0>(allocator, run_indices).unwrap();

    if (run_indices_vec.last() != USIZE_MAX)
    {
        run_indices_vec.last() = USIZE_MAX;
    }

    auto styles_vec = small_vec::copy<1, 0>(allocator, styles).unwrap();
    auto fonts_vec  = small_vec::copy<1, 0>(allocator, fonts).unwrap();

    return TextRunsStyle{std::move(run_indices_vec), std::move(styles_vec),
                         std::move(fonts_vec)};
}

RenderText & RenderText::runs_style(TextRunsStyle style)
{
    runs_style_ = std::move(style);
    hash_       = HASH_DIRTY;
    return *this;
}

TextRunsStyle const & RenderText::runs_style() const
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
    if (wrap_ != wrap)
    {
        flush_text();
    }

    wrap_ = wrap;
    return *this;
}

RenderText & RenderText::use_kerning(bool use_kerning)
{
    if (use_kerning_ != use_kerning)
    {
        flush_text();
    }

    use_kerning_ = use_kerning;
    return *this;
}

RenderText & RenderText::use_ligatures(bool use_ligatures)
{
    if (use_ligatures_ != use_ligatures)
    {
        flush_text();
    }
    use_ligatures_ = use_ligatures;
    return *this;
}

RenderText & RenderText::font_scale(f32 scale)
{
    if (font_scale_ != scale)
    {
        flush_text();
    }
    font_scale_ = scale;
    return *this;
}

RenderText & RenderText::direction(TextDirection direction)
{
    if (direction_ != direction)
    {
        flush_text();
    }
    direction_ = direction;
    return *this;
}

RenderText & RenderText::language(Str language)
{
    if (!span_bit_eq(language_, language))
    {
        flush_text();
    }
    language_ = language;
    return *this;
}

RenderText & RenderText::alignment(f32 alignment)
{
    if (alignment_ != alignment)
    {
        flush_text();
    }
    alignment_ = alignment;
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
    runs_style(TextRunsStyle::all(style, font));
    flush_text();
    return *this;
}

RenderText & RenderText::str(Rc<Str32> utf32)
{
    str_ = std::move(utf32);
    flush_text();
    return *this;
}

RenderText & RenderText::str_copy(Str32 utf32, Allocator allocator)
{
    auto vec    = vec::copy(allocator, utf32).unwrap();
    auto rc_vec = rc<Vec<c32>>(allocator, std::move(vec)).unwrap();
    auto view   = rc_vec->view().as_const();
    return str(transmute(std::move(rc_vec), view));
}

RenderText & RenderText::str_copy(Str32 utf32, TextStyle const & style,
                                  FontStyle const & font, Allocator allocator)
{
    str_copy(utf32, allocator);
    runs_style(TextRunsStyle::all(style, font));
    flush_text();
    return *this;
}

RenderText & RenderText::str_copy(Str8 utf8, TextStyle const & style,
                                  FontStyle const & font, Allocator allocator)
{
    str_copy(utf8, allocator);
    runs_style(TextRunsStyle::all(style, font));
    flush_text();
    return *this;
}

RenderText & RenderText::str_copy(Str8 utf8, Allocator allocator)
{
    Vec<c32> text32{allocator};
    utf8_decode(utf8, text32).unwrap();
    auto rc_vec = rc<Vec<c32>>(allocator, std::move(text32)).unwrap();
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

RenderText & RenderText::width(f32 max_width, f32 align_width)
{
    if (max_width_ != max_width || align_width_ != align_width)
    {
        hash_ = HASH_DIRTY;
    }

    max_width_   = max_width;
    align_width_ = align_width;

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

TextLayout const & RenderText::layout() const
{
    return layout_;
}

void RenderText::perform_layout()
{
    if (hash_ != HASH_CLEAN)
    {
        layout_ = sys.font->layout_text(block(), max_width_, align_width_);
    }

    hash_ = HASH_CLEAN;
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
    return layout_.hit(block(), block_style(), local_pos);
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

Str32 TextModel::str() const
{
    return text_.str();
}

TextLayout const & TextModel::layout() const
{
    return text_.layout();
}

RenderText const & TextModel::get_render_text() const
{
    return text_;
}

RenderText & TextModel::get_render_text()
{
    return text_;
}

void TextModel::text(RenderText text)
{
    text_ = std::move(text);
}

void TextModel::str(Rc<Str32> str)
{
    text_.str(std::move(str));
}

void TextModel::str(Str32 s)
{
    auto str32  = vec::copy(allocator_, s).unwrap();
    auto rc_str = rc<StrVec32>(allocator_, std::move(str32)).unwrap();
    auto view   = rc_str->view().as_const();
    str(transmute(std::move(rc_str), view));
}

void TextModel::str(Str8 s)
{
    auto str32 = Vec<c32>::make(s.size(), allocator_).unwrap();
    utf8_decode(s, str32).unwrap();
    auto rc_str = rc<StrVec32>(allocator_, std::move(str32)).unwrap();
    auto view   = rc_str->view().as_const();
    str(transmute(std::move(rc_str), view));
}

void TextModel::runs_style(TextRunsStyle runs_style)
{
    text_.runs_style(std::move(runs_style));
}

StrVec32 TextModel::copy(Allocator allocator)
{
    auto & layout = this->layout();
    auto   cursor = touch_cursor();

    if (!cursor.has_selection())
    {
        auto cp = layout.get_caret_codepoint(cursor.caret());
        cursor.select(layout.lines[cp.line].carets);
    }

    cursor.normalize(layout.num_carets);
    auto slice = layout.get_caret_selection(cursor.selection());

    return vec::copy(allocator, str().slice(slice)).unwrap();
}

void TextModel::add_cursor(TextCursor value)
{
    cursor_ = value.normalize(layout().num_carets);
}

TextCursor & TextModel::touch_cursor()
{
    if (cursor_.is_none())
    {
        cursor_ = TextCursor{};
    }

    return *cursor_;
}

void TextModel::set_cursor_style(TextHighlightStyle highlight_style,
                                 CaretStyle         caret_style)
{
    highlight_style_ = highlight_style;
    caret_style_     = caret_style;
}

void TextModel::remove_cursor()
{
    cursor_ = none;
}

void TextModel::update_cursor(TextCursor value)
{
    cursor_ = value.normalize(layout().num_carets);
}

void TextModel::unselect()
{
    auto & cursor = touch_cursor();
    cursor.unselect();
}

void TextModel::left()
{
    auto & cursor = touch_cursor();
    cursor.translate(-1).normalize(layout().num_carets);
}

void TextModel::right()
{
    auto & cursor = touch_cursor();
    cursor.translate(1).normalize(layout().num_carets);
}

void TextModel::word_start(Span<c32 const> word_symbols)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.move_to(
      l.to_caret(seek_sym(str(), cp.codepoint + (cp.after ? 1 : 0), true, word_symbols)
                   .unwrap_or(),
                 true));
}

void TextModel::word_end(Span<c32 const> word_symbols)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.move_to(
      l.to_caret(seek_sym(str(), cp.codepoint, false, word_symbols).unwrap_or(), true));
}

void TextModel::line_start()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.move_to(l.lines[cp.line].carets.first());
}

void TextModel::line_end()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.move_to(l.lines[cp.line].carets.last());
}

void TextModel::up()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.move_to(translate_caret(l, cursor.caret(), caret_alignment_, -1)
                     .unwrap_or(cursor.caret()));
}

void TextModel::down()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.move_to(translate_caret(l, cursor.caret(), caret_alignment_, 1)
                     .unwrap_or(cursor.caret()));
}

void TextModel::page_up(usize lines_per_page)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.move_to(
      translate_caret(l, cursor.caret(), caret_alignment_, -(isize) lines_per_page)
        .unwrap_or(cursor.caret()));
}

void TextModel::page_down(usize lines_per_page)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.move_to(
      translate_caret(l, cursor.caret(), caret_alignment_, (isize) lines_per_page)
        .unwrap_or(cursor.caret()));
}

void TextModel::select_left()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.extend_selection(-1).normalize(l.num_carets);
}

void TextModel::select_right()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.extend_selection(1).normalize(l.num_carets);
}

void TextModel::select_up()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.span_to(translate_caret(l, cursor.caret(), caret_alignment_, -1)
                     .unwrap_or(cursor.caret()));
}

void TextModel::select_down()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.span_to(translate_caret(l, cursor.caret(), caret_alignment_, 1)
                     .unwrap_or(cursor.caret()));
}

void TextModel::select_to_word_start(Span<c32 const> word_symbols)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.span_to(
      l.to_caret(seek_sym(str(), cp.codepoint, true, word_symbols).unwrap_or(), true));
}

void TextModel::select_to_word_end(Span<c32 const> word_symbols)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.span_to(
      l.to_caret(seek_sym(str(), cp.codepoint, false, word_symbols).unwrap_or(), true));
}

void TextModel::select_to_line_start()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.span_to(l.lines[cp.line].carets.first());
}

void TextModel::select_to_line_end()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.span_to(l.lines[cp.line].carets.last());
}

void TextModel::select_page_up(usize lines_per_page)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.span_to(
      translate_caret(l, cursor.caret(), caret_alignment_, -(isize) lines_per_page)
        .unwrap_or(cursor.caret()));
}

void TextModel::select_page_down(usize lines_per_page)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.span_to(
      translate_caret(l, cursor.caret(), caret_alignment_, (isize) lines_per_page)
        .unwrap_or(cursor.caret()));
}

void TextModel::select_codepoint()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.span_by(1).normalize(l.num_carets);
}

void TextModel::select_word(Span<c32 const> word_symbols)
{
    auto & l         = layout();
    auto & cursor    = touch_cursor();
    auto   selection = span_sym_boundary(
      str(), l.get_caret_codepoint(cursor.caret()).codepoint, word_symbols);
    cursor.select(l.get_caret_selection(selection));
}

void TextModel::select_line()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    auto   cp     = l.get_caret_codepoint(cursor.caret());
    cursor.select(l.lines[cp.line].carets);
}

void TextModel::select_all()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();
    cursor.select(Slice{0, l.num_carets});
}

void TextModel::hit(f32x2 center, f32x4x4 const & transform, f32x2 pos)
{
    auto & l          = layout();
    auto & cursor     = touch_cursor();
    auto [caret, loc] = text_.hit(center, transform, pos);
    caret_alignment_  = loc.x;
    cursor.move_to(l.align_caret(loc));
}

void TextModel::hit_select(f32x2 center, f32x4x4 const & transform, f32x2 pos)
{
    auto & l          = layout();
    auto & cursor     = touch_cursor();
    auto [caret, loc] = text_.hit(center, transform, pos);
    caret_alignment_  = loc.x;
    cursor.span_to(l.align_caret(loc));
}

void TextModel::erase_at(Slice selection)
{
    auto str  = text_.str();
    selection = selection(str.size());
    auto res  = StrVec32::make(str.size() - selection.span, allocator_).unwrap();
    res.append(str.slice(0, selection.begin())).unwrap();
    res.append(str.slice(selection.end())).unwrap();
    auto rc_resv = rc<StrVec32>(allocator_, std::move(res)).unwrap();
    auto view    = rc_resv->view().as_const();
    text_.str(transmute(std::move(rc_resv), view));
}

void TextModel::insert_at(usize pos, Str32 in)
{
    auto str = text_.str();
    auto res = StrVec32::make(str.size() + in.size(), allocator_).unwrap();
    res.append(str.slice(0, pos)).unwrap();
    res.append(in).unwrap();
    res.append(str.slice(pos)).unwrap();
    auto rc_resv = rc<StrVec32>(allocator_, std::move(res)).unwrap();
    auto view    = rc_resv->view().as_const();
    text_.str(transmute(std::move(rc_resv), view));
}

void TextModel::backspace()
{
    auto & cursor = touch_cursor();

    {
        auto & l = layout();
        if (!cursor.has_selection())
        {
            if (cursor.caret() == 0)
            {
                return;
            }
            cursor.translate(-1).span_by(1).normalize(l.num_carets);
        }

        erase_at(l.get_caret_selection(cursor.selection()));
    }

    cursor.unselect_left().normalize(layout().num_carets);
}

void TextModel::del()
{
    auto & cursor = touch_cursor();

    {
        auto & l = layout();

        if (!cursor.has_selection())
        {
            if (l.num_carets == 0 || cursor.caret() == ((isize) l.num_carets - 1))
            {
                return;
            }
            cursor.span_by(1).normalize(l.num_carets);
        }

        erase_at(l.get_caret_selection(cursor.selection()));
    }

    cursor.unselect_left().normalize(layout().num_carets);
}

void TextModel::insert(Str32 input)
{
    auto & cursor = touch_cursor();

    {
        auto & l = layout();
        if (auto selection = cursor.selection(); !selection.is_empty())
        {
            cursor.unselect_left();
            erase_at(l.get_caret_selection(selection));
        }
    }

    {
        auto & l = layout();
        cursor.normalize(l.num_carets);
        auto cp        = l.get_caret_codepoint(cursor.caret());
        auto codepoint = cp.codepoint + (cp.after ? 1 : 0);
        insert_at(codepoint, input);
        auto caret = l.to_caret(codepoint + input.size(), true);
        cursor.move_to(caret).normalize(l.num_carets);
    }
}

void TextModel::insert(Str8 input)
{
    ScratchScope scratch{allocator_};
    Vec<c32>     utf32{scratch};
    utf8_decode(input, utf32).unwrap();
    insert(utf32);
}

void TextModel::new_line()
{
    return insert(U"\n"_s);
}

void TextModel::tab()
{
    return insert(U"\t"_s);
}

StrVec32 TextModel::copy_cut(Allocator allocator)
{
    auto & l      = layout();
    auto & cursor = touch_cursor();

    if (!cursor.has_selection())
    {
        auto cp = l.get_caret_codepoint(cursor.caret());
        cursor.select(l.lines[cp.line].carets);
    }

    cursor.normalize(l.num_carets);
    auto slice = l.get_caret_selection(cursor.selection());
    auto copy  = vec::copy(allocator, str().slice(slice)).unwrap();

    backspace();

    return copy;
}

void TextModel::cut()
{
    auto & l      = layout();
    auto & cursor = touch_cursor();

    if (!cursor.has_selection())
    {
        auto cp = l.get_caret_codepoint(cursor.caret());
        cursor.select(l.lines[cp.line].carets);
    }

    backspace();
}

void TextModel::width(f32 max_width, f32 align_width)
{
    text_.width(max_width, align_width);
}

void TextModel::perform_layout()
{
    text_.perform_layout();
    cursor_.match(
      [&](TextCursor & cursor) { cursor.normalize(text_.layout().num_carets); }, [] {});
}

}    // namespace ash
