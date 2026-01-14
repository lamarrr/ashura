/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/text.h"
#include "ashura/std/math.h"
#include "ashura/std/piece_table.h"
#include "ashura/std/types.h"

namespace ash
{

struct [[nodiscard]] TextRunsStyle
{
    static auto default_run_indices(Allocator allocator = noop_allocator)
    {
        static constexpr usize data[] = {0uz, USIZE_MAX};
        return small_vec::copy<4, 0>(allocator, span(data)).unwrap();
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

    SmallVec<usize, 4, 0>     run_indices_;
    SmallVec<TextStyle, 1, 0> styles_;
    SmallVec<FontStyle, 1, 0> fonts_;

    TextRunsStyle(SmallVec<usize, 4, 0> run_indices, SmallVec<TextStyle, 1, 0> styles,
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

    Allocator allocator_;

    usize hash_;

    bool wrap_ : 1;

    bool use_kerning_ : 1;

    bool use_ligatures_ : 1;

    TextDirection direction_ : 2;

    f32 alignment_;

    f32 font_scale_;

    Rc<Str32> str_;

    TextRunsStyle runs_style_;

    Str language_;

    TextLayout layout_;

    void * user_data_;

    constexpr RenderText(Allocator allocator) :
      allocator_{allocator},
      hash_{HASH_DIRTY},
      wrap_{true},
      use_kerning_{true},
      use_ligatures_{true},
      direction_{TextDirection::LeftToRight},
      alignment_{ALIGNMENT_LEFT},
      font_scale_{1},
      str_{static_rc(U""_str)},
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

    RenderText & style_runs(TextRunsStyle style);

    TextRunsStyle const & get_style_runs() const;

    RenderText & flush_text();

    RenderText & wrap(bool wrap);

    RenderText & font_scale(f32 scale);

    RenderText & direction(TextDirection direction);

    RenderText & language(Str language);

    RenderText & alignment(f32 alignment);

    Str32 str() const;

    RenderText & str(Rc<Str32> utf32, TextStyle const & style, FontStyle const & font);

    RenderText & str(Rc<Str32> utf32);

    RenderText & str_copy(Str32 utf32);

    RenderText & str_copy(Str32 utf32, TextStyle const & style,
                           FontStyle const & font);

    RenderText & str_copy(Str8 utf8, TextStyle const & style, FontStyle const & font);

    RenderText & str_copy(Str8 utf8);

    RenderText & user_data(void * data);

    usize size() const;

    TextBlock block() const;

    TextBlockStyle block_style(f32 aligned_width) const;

    TextLayout const & get_layout() const;

    void layout(f32 max_width, Allocator scratch);

    /// @brief Render the laid out text
    /// @param center canvas-space region of the text to place the text on
    /// @param align_width the width to align the text to
    /// @param transform the canvas-space transform to apply to the text block
    /// @param clip the canvas-space clip rectangle
    /// @param highlights text highlights to render
    /// @param highlight_styles styles for each of the highlights
    /// @param carets carets to render
    /// @param caret_styles styles for each of the carets
    /// @param scratch_allocator allocator to use for temporary allocations
    void render(TextRenderer renderer, f32x2 center, f32 align_width,
                f32x4x4 const & transform, CRect const & clip,
                Span<Slice const>              highlights,
                Span<TextHighlightStyle const> highlight_styles,
                Span<usize const> carets, Span<CaretStyle const> caret_styles,
                Allocator scratch_allocator) const;

    /// @brief Perform hit test on the laid-out text
    /// @param center canvas-space region the text was placed on
    /// @param align_width the width the text was aligned to
    /// @returns .v0: caret index, .v1: caret location
    Tuple<isize, CaretAlignment> hit(f32x2 center, f32 align_width,
                                     f32x4x4 const & transform,
                                     f32x2           transformed_pos) const;
};

struct [[nodiscard]] EditHistoryBuffer
{
    enum class RecordType : u8
    {
        Erase  = 0,
        Insert = 1
    };

    struct Record
    {
        usize pos = 0;

        Rc<Str32> str = static_rc(U""_str);

        RecordType type = RecordType::Insert;

        constexpr usize size() const
        {
            return str.get().size();
        }
    };

    Vec<Record> records_;

    /// @brief Record representing the current text composition state;
    /// the base state is index 0
    usize current_record_;

    explicit constexpr EditHistoryBuffer(Vec<Record> buffer) :
      records_{std::move(buffer)},
      current_record_{0}
    {
    }

    constexpr EditHistoryBuffer(EditHistoryBuffer const &)             = delete;
    constexpr EditHistoryBuffer & operator=(EditHistoryBuffer const &) = delete;
    constexpr EditHistoryBuffer(EditHistoryBuffer &&)                  = default;
    constexpr EditHistoryBuffer & operator=(EditHistoryBuffer &&)      = default;
    constexpr ~EditHistoryBuffer()                                     = default;

    static EditHistoryBuffer create(Allocator allocator, usize records_capacity);

    /// @brief Apply changes of next record
    /// @param str  piece table to apply the redo changes to
    /// @returns selection slice after redo, if any
    Option<Slice> redo(PieceTable32 & str);

    /// @brief Undo changes of current record
    /// @param str  piece table to apply the undo changes to
    /// @returns selection slice after undo, if any
    Option<Slice> undo(PieceTable32 & str);

    void add_record(RecordType type, usize pos, Rc<Str32> str);

    void insert(usize pos, PieceTable32 & str, Rc<Str32> insert_str);

    void erase(Slice selection, PieceTable32 & str);
};

static constexpr c32 DEFAULT_WORD_SYMBOLS[] = {U' ', U'\t'};

struct [[nodiscard]] InteractText
{
    struct Cursor
    {
        u64 action_id = 0;

        TextCursor v = {};
    };

    struct State
    {
        RenderText text;

        EditHistoryBuffer history;
    };

    struct ActionResult
    {
        Option<RenderText> text;

        EditHistoryBuffer history;

        SmallVec<Cursor, 8, 0> cursors;
    };

    using Renderer = Rc<Fn<RenderText(Allocator, Rc<Str32>)>>;

    struct InsertAction
    {
        u64 id = 0;

        f32 max_width = 0.0f;

        Renderer renderer;

        TextCursor cursor = {};

        Rc<Str32> str = static_rc(U""_str);
    };

    struct EraseAction
    {
        u64 id = 0;

        f32 max_width = 0.0f;

        Renderer renderer;

        TextCursor cursor = {};
    };

    struct UndoAction
    {
        u64 id = 0;

        f32 max_width = 0.0f;

        Renderer renderer;
    };

    struct RedoAction
    {
        u64 id = 0;

        f32 max_width = 0.0f;

        Renderer renderer;
    };

    struct RelayoutAction
    {
        u64 id = 0;

        f32 max_width = 0.0f;

        Renderer renderer;
    };

    struct CopyAction
    {
        u64 id = 0;

        f32 max_width = 0.0f;

        Renderer renderer;

        TextCursor cursor = {};

        Future<Vec<c32>> output;
    };

    using Edit = Enum<InsertAction, EraseAction, UndoAction, RedoAction, RelayoutAction,
                      CopyAction>;

    static constexpr usize DEFAULT_RECORDS_SIZE = 2'048;

    static constexpr c32 DEFAULT_WORD_SYMBOLS[] = {U' ', U'\t'};

    Allocator allocator_;

    Rc<State *> state_;

    Option<Future<Rc<ActionResult *>>> pending_result_;

    Renderer renderer_;

    /// @brief action queue of mutations to be performed on the text.
    /// this is queued up whilst an async text edit is being performed.
    /// after each edit action, the text is re-laid out.
    SmallVec<Edit, 8, 0> action_queue_;

    Cursor cursor_;

    CaretXAlignment caret_alignment_;

    f32 max_width_;

    u64 action_id_;

    constexpr EditText(Allocator allocator, Rc<State *> state, Renderer renderer) :
      allocator_{allocator},
      state_{std::move(state)},
      pending_result_{none},
      renderer_{std::move(renderer)},
      action_queue_{allocator},
      cursor_{},
      caret_alignment_{CaretXAlignment::Start},
      max_width_{0},
      action_id_{0}
    {
    }

    constexpr EditText(EditText const &)             = delete;
    constexpr EditText(EditText &&)                  = default;
    constexpr EditText & operator=(EditText const &) = delete;
    constexpr EditText & operator=(EditText &&)      = default;
    constexpr ~EditText()                            = default;

    static Renderer default_renderer();

    static EditText create(Allocator allocator);

    bool has_pending_edit() const;

    Str32 get_text() const;

    Rc<Span<c32 const>> create_copy(Span<c32 const> str);

    Rc<Span<c32 const>> create_copy(Span<c8 const> str);

    TextLayout const & get_layout() const;

    RenderText const & get_render_text() const;

    void set_renderer(Renderer renderer);

    Future<Vec<c32>> copy();

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

    void hit(f32x2 center, f32 aligned_width, f32x2 pos, f32x4x4 const & transform);

    void hit_select(f32x2 center, f32 aligned_width, f32x2 pos,
                    f32x4x4 const & transform);

    void backspace();

    void del();

    void insert(Rc<Str32> input);

    void new_line();

    Future<Vec<c32>> copy_cut();

    void cut();

    void undo();

    void redo();

    void layout(f32 max_width);

    void tick(nanoseconds dt);
};

}    // namespace ash
