/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/render_text.h"
#include "ashura/engine/text_compositor.h"
#include "ashura/engine/view.h"
#include "ashura/std/types.h"

namespace ash
{

namespace ui
{

// TODO: scroll and clip text if region isn't large enough
// -  wrapping to the next line if not large enough
// -  no wrap
// -  max-len
// -  filter/transform function
// -  secret text input
struct InputCfg
{
    bool wrappable     : 1 = false;
    bool submittable   : 1 = false;
    bool multiline     : 1 = false;
    bool enter_submits : 1 = false;
    bool tab_input     : 1 = false;

    Fn<void(Vec<c32> &, Str32)> insert;
};

// TODO: renderer hooks for regions
struct Input : View
{
    enum class Cmd : u8
    {
        None = 0,

        Escape = 1,

        /// Cursor State
        Unselect = 2,

        /// Editing
        BackSpace = 3,
        Delete    = 4,
        InputText = 5,

        /// Cursor Positioning
        Left      = 6,
        Right     = 7,
        WordStart = 8,
        WordEnd   = 9,
        LineStart = 10,
        LineEnd   = 11,
        Up        = 12,
        Down      = 13,
        PageUp    = 14,
        PageDown  = 15,

        /// Cursor Selection
        SelectLeft        = 16,
        SelectRight       = 17,
        SelectUp          = 18,
        SelectDown        = 19,
        SelectToWordStart = 20,
        SelectToWordEnd   = 21,
        SelectToLineStart = 22,
        SelectToLineEnd   = 23,
        SelectPageUp      = 24,
        SelectPageDown    = 25,

        /// Semantic Selection
        SelectCodepoint = 26,
        SelectWord      = 27,
        SelectLine      = 28,
        SelectAll       = 29,

        /// ClipBoard
        Cut   = 30,
        Copy  = 31,
        Paste = 32,

        /// Redo/Undo
        Undo = 33,
        Redo = 34,

        /// Mouse Selection (Visual)
        Hit       = 35,
        HitSelect = 36,

        /// Insert new line
        NewLine = 37,
        Tab     = 38,

        Submit = 39
    };

    struct State
    {
        bool disabled : 1 = false;

        bool editing : 1 = false;

        bool submit : 1 = false;

        bool multiline : 1 = false;

        bool enter_submits : 1 = false;

        bool tab_input : 1 = false;
    } state_;

    struct Style
    {
        TextHighlightStyle highlight = {.color        = theme.highlight,
                                        .corner_radii = f32x4::splat(0)};
        CaretStyle         caret{.color = theme.caret, .thickness = 1.0F};
        usize              lines_per_page = 40;
        usize              tab_width      = 1;
    } style_;

    struct Callbacks
    {
        Fn<void()> edit = noop;

        Fn<void()> submit = noop;

        Fn<void()> focus_in = noop;

        Fn<void()> focus_out = noop;
    } cb;

    Allocator allocator_;

    RenderText content_;

    RenderText stub_;

    TextCompositor compositor_;

    Input(Str32             stub      = U""_s,
          TextStyle const & style     = TextStyle{.color = theme.on_surface},
          FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                                  .height      = theme.body_font_height,
                                                  .line_height = theme.line_height},
          Allocator         allocator = default_allocator);

    Input(Str8 stub, TextStyle const & style = TextStyle{.color = theme.on_surface},
          FontStyle const & font      = FontStyle{.font        = theme.body_font,
                                                  .height      = theme.body_font_height,
                                                  .line_height = theme.line_height},
          Allocator         allocator = default_allocator);

    Input(Input const &)             = delete;
    Input(Input &&)                  = default;
    Input & operator=(Input const &) = delete;
    Input & operator=(Input &&)      = default;
    virtual ~Input() override        = default;

    Input & disable(bool disable);

    Input & multiline(bool enable);

    Input & enter_submits(bool enable);

    Input & tab_input(bool enable);

    Input & on_edit(Fn<void()> fn);

    Input & on_submit(Fn<void()> fn);

    Input & on_focus_in(Fn<void()> fn);

    Input & on_focus_out(Fn<void()> fn);

    Input & content(Str8 text);

    Input & content(Str32 text);

    Input & content_run(TextStyle const & style, FontStyle const & font,
                        auto first = 0uz, usize count = USIZE_MAX);

    Input & stub(Str8 text);

    Input & stub(Str32 text);

    Input & stub_run(TextStyle const & style, FontStyle const & font, auto first = 0uz,
                     usize count = USIZE_MAX);

    virtual ui::State tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual Layout fit(f32x2 allocated, Span<f32x2 const>, Span<f32x2>) override;

    virtual void render(Canvas & canvas, RenderInfo const & info) override;

    virtual Cursor cursor(f32x2 extent, f32x2 position) override;
};

}    // namespace ui

}    // namespace ash
