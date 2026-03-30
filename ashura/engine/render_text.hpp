/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/text.hpp"
#include "ashura/std/math.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

struct [[nodiscard]] TextRunsStyle
{
    static auto default_run_indices(Allocator allocator = noop_allocator)
    {
        static constexpr usize data[] = {0uz, USIZE_MAX};
        return small_vec::copy<2, 0>(allocator, span(data)).unwrap();
    }

    static auto default_styles(Allocator allocator = noop_allocator)
    {
        static constexpr TextStyle data[] = {{}};
        return small_vec::copy<1, 0>(allocator, span(data)).unwrap();
    }

    static auto default_fonts(Allocator allocator = noop_allocator)
    {
        static constexpr FontStyle data[] = {{}};
        return small_vec::copy<1, 0>(allocator, span(data)).unwrap();
    }

    SmallVec<usize, 2, 0>     run_indices_;
    SmallVec<TextStyle, 1, 0> styles_;
    SmallVec<FontStyle, 1, 0> fonts_;

    TextRunsStyle(SmallVec<usize, 2, 0> run_indices, SmallVec<TextStyle, 1, 0> styles,
                  SmallVec<FontStyle, 1, 0> fonts) :
      run_indices_{std::move(run_indices)},
      styles_{std::move(styles)},
      fonts_{std::move(fonts)}
    {
    }

    TextRunsStyle(Allocator allocator) :
      run_indices_{default_run_indices(allocator)},
      styles_{default_styles(allocator)},
      fonts_{default_fonts(allocator)}
    {
    }

    static TextRunsStyle all(TextStyle const & style, FontStyle const & font);

    /// @brief Creates a TextRunsStyle from run sizes, styles, and fonts.
    /// @param allocator allocator to use for memory allocations
    /// @param run_sizes sizes of each run of text the styles and fonts correspond
    /// to
    /// @param styles styles for each run
    /// @param fonts fonts for each run
    static TextRunsStyle make_sized(Allocator allocator, Span<usize const> run_sizes,
                                    Span<TextStyle const> styles,
                                    Span<FontStyle const> fonts);

    /// @brief Creates a TextRunsStyle from run end indices, styles, and fonts.
    /// @param allocator allocator to use for memory allocations
    /// @param run_indices run end indices of each run of text the styles and
    /// fonts correspond to
    /// @param styles styles for each run
    static TextRunsStyle make_indexed(Allocator             allocator,
                                      Span<usize const>     run_indices,
                                      Span<TextStyle const> styles,
                                      Span<FontStyle const> fonts);

    TextRunsStyle copy(Allocator allocator) const;

    Span<usize const> run_indices() const
    {
        return run_indices_.view();
    }

    Span<TextStyle const> styles() const
    {
        return styles_.view();
    }

    Span<FontStyle const> fonts() const
    {
        return fonts_.view();
    }
};

/// @brief Controls and manages GUI text state for rendering
/// - manages runs and run styling
/// - manages and checks for text layout invalidation
/// - recalculates text layout when it changes and if necessary
/// - renders the text using the computed style information
/// @param runs  Run-End encoded sequences of the runs
struct [[nodiscard]] RenderText
{
    static constexpr usize HASH_CLEAN = USIZE_MAX;
    static constexpr usize HASH_DIRTY = 0z;

    usize hash_;

    bool wrap_ : 1;

    bool use_kerning_ : 1;

    bool use_ligatures_ : 1;

    TextDirection direction_ : 2;

    f32 alignment_;

    f32 font_scale_;

    f32 max_width_;

    f32 align_width_;

    Rc<Str32> str_;

    TextRunsStyle runs_style_;

    Str language_;

    TextLayout layout_;

    void * user_data_;

    constexpr RenderText(Allocator allocator) :
      hash_{HASH_DIRTY},
      wrap_{true},
      use_kerning_{true},
      use_ligatures_{true},
      direction_{TextDirection::LeftToRight},
      alignment_{ALIGNMENT_LEFT},
      font_scale_{1},
      max_width_{1024.0F},
      align_width_{1024.0F},
      str_{static_rc(U""_s)},
      runs_style_{noop_allocator},
      language_{},
      layout_{.glyphs{allocator},
              .runs{allocator},
              .lines{allocator},
              .paragraphs{allocator}},
      user_data_{nullptr}
    {
    }

    constexpr RenderText(RenderText const &)             = delete;
    constexpr RenderText & operator=(RenderText const &) = delete;
    constexpr RenderText(RenderText &&)                  = default;
    constexpr RenderText & operator=(RenderText &&)      = default;
    constexpr ~RenderText()                              = default;

    RenderText & runs_style(TextRunsStyle style);

    TextRunsStyle const & runs_style() const;

    RenderText & flush_text();

    RenderText & wrap(bool wrap);

    RenderText & use_kerning(bool use_kerning);

    RenderText & use_ligatures(bool use_ligatures);

    RenderText & font_scale(f32 scale);

    RenderText & direction(TextDirection direction);

    RenderText & language(Str language);

    RenderText & alignment(f32 alignment);

    Str32 str() const;

    RenderText & str(Rc<Str32> utf32, TextStyle const & style, FontStyle const & font);

    RenderText & str(Rc<Str32> utf32);

    RenderText & str_copy(Str32 utf32, Allocator allocator);

    RenderText & str_copy(Str32 utf32, TextStyle const & style, FontStyle const & font,
                          Allocator allocator);

    RenderText & str_copy(Str8 utf8, TextStyle const & style, FontStyle const & font,
                          Allocator allocator);

    RenderText & str_copy(Str8 utf8, Allocator allocator);

    RenderText & user_data(void * data);

    RenderText & width(f32 max_width, f32 align_width);

    usize size() const;

    TextBlock block() const;

    TextBlockStyle block_style() const;

    TextLayout const & layout() const;

    void perform_layout();

    /// @brief Generate the placement rectangles for the laid-out text
    /// @param center canvas-space region of the text to place the text on
    /// @param transform the canvas-space transform to apply to the text block
    /// @param clip the canvas-space clip rectangle
    /// @param highlights text highlights to render
    /// @param highlight_styles styles for each of the highlights
    /// @param carets carets to render
    /// @param caret_styles styles for each of the carets
    /// @param allocator allocator to use for the placement allocations
    Tuple<TextRenderInfo, TextPlacement>
      place(f32x2 center, f32x4x4 const & transform, CRect const & clip,
            Span<Slice const>              highlights,
            Span<TextHighlightStyle const> highlight_styles, Span<usize const> carets,
            Span<CaretStyle const> caret_styles, Allocator allocator) const;

    /// @brief Perform hit test on the laid-out text
    /// @param center canvas-space region used as the center of the text block
    /// @param align_width the width the text block was aligned to
    /// @param transform the canvas-space transform applied to the text block
    /// @param transformed_pos the canvas-space position to hit test
    /// @returns .v0: caret index, .v1: caret location
    Tuple<isize, CaretAlignment> hit(f32x2 center, f32x4x4 const & transform,
                                     f32x2 transformed_pos) const;
};

static constexpr c32 DEFAULT_WORD_SYMBOLS[] = {U' ', U'\t'};

/// @brief Model for interactive text views
struct [[nodiscard]] TextModel
{
    // TODO: type-erased undo/redo

    Allocator allocator_;

    RenderText text_;

    Option<TextCursor> cursor_;

    CaretXAlignment caret_alignment_;

    TextHighlightStyle highlight_style_;

    CaretStyle caret_style_;

    constexpr TextModel(Allocator allocator) :
      allocator_{allocator},
      text_{allocator},
      cursor_{none},
      caret_alignment_{CaretXAlignment::Start},
      highlight_style_{},
      caret_style_{}
    {
    }

    constexpr TextModel(TextModel const &)             = delete;
    constexpr TextModel(TextModel &&)                  = default;
    constexpr TextModel & operator=(TextModel const &) = delete;
    constexpr TextModel & operator=(TextModel &&)      = default;
    constexpr ~TextModel()                             = default;

    Str32 str() const;

    TextLayout const & layout() const;

    RenderText const & get_render_text() const;

    RenderText & get_render_text();

    void text(RenderText text);

    void str(Rc<Str32> str);

    void str(Str32 str);

    void str(Str8 str);

    void runs_style(TextRunsStyle runs_style);

    StrVec32 copy(Allocator allocator);

    void add_cursor(TextCursor value);

    TextCursor & touch_cursor();

    void set_cursor_style(TextHighlightStyle highlight_style, CaretStyle caret_style);

    void remove_cursor();

    void update_cursor(TextCursor value);

    void unselect();

    void left();

    void right();

    void word_start(Span<c32 const> word_symbols);

    void word_end(Span<c32 const> word_symbols);

    void line_start();

    void line_end();

    void up();

    void down();

    void page_up(usize lines_per_page);

    void page_down(usize lines_per_page);

    void select_left();

    void select_right();

    void select_up();

    void select_down();

    void select_to_word_start(Span<c32 const> word_symbols);

    void select_to_word_end(Span<c32 const> word_symbols);

    void select_to_line_start();

    void select_to_line_end();

    void select_page_up(usize lines_per_page);

    void select_page_down(usize lines_per_page);

    void select_codepoint();

    void select_word(Span<c32 const> word_symbols);

    void select_line();

    void select_all();

    void hit(f32x2 center, f32x4x4 const & transform, f32x2 pos);

    void hit_select(f32x2 center, f32x4x4 const & transform, f32x2 pos);

    void erase_at(Slice selection);

    void insert_at(usize pos, Str32 str);

    void backspace();

    void del();

    void insert(Str32 input);

    void insert(Str8 input);

    void new_line();

    void tab();

    StrVec32 copy_cut(Allocator allocator);

    void cut();

    void width(f32 max_width, f32 align_width);

    void perform_layout();
};

}    // namespace ash
