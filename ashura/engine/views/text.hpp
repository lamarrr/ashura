/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.hpp"
#include "ashura/engine/views/model/text.hpp"
#include "ashura/std/types.hpp"

namespace ash
{

namespace ui
{

struct Text : View
{
    Allocator  allocator_;
    txt::State state_;
    txt::Cfg   cfg_;
    Frame      frame_;

    explicit Text(Allocator allocator);

    Text(Allocator allocator, Rc<Str32> text, TextStyle const & style,
         FontStyle const & font);

    Text(Allocator allocator, Str32 text, TextStyle const & style,
         FontStyle const & font);

    Text(Allocator allocator, Str8 text, TextStyle const & style,
         FontStyle const & font);

    Text(Text const &)             = delete;
    Text(Text &&)                  = default;
    Text & operator=(Text const &) = delete;
    Text & operator=(Text &&)      = default;
    virtual ~Text() override       = default;

    Text & copyable(bool v);

    Text & highlightable(bool v);

    Text & enable_cursor(bool v);

    Text & editable(bool v);

    Text & enable_undo_redo(bool v);

    Text & enable_multiline_input(bool v);

    Text & accept_tab_input(bool v);

    Text & enter_submits(bool v);

    Text & lines_per_page(u16 v);

    Text & input_to_actions_map(Rc<txt::InputToActionsMap> map);

    Text & renderer(Rc<txt::Renderer> renderer);

    Text & highlights(Span<Slice const>              highlights,
                      Span<TextHighlightStyle const> highlight_styles);

    Text & clear_highlights();

    Text & style_runs(TextRunsStyle style);

    Text & wrap(bool v);

    Text & use_kerning(bool v);

    Text & use_ligatures(bool v);

    Text & font_scale(f32 v);

    Text & direction(TextDirection v);

    Text & language(Str v);

    Text & alignment(f32 v);

    Str32 str() const;

    Text & str(Str32 str);

    Text & str(Str8 str);

    Text & str(Rc<Str32> v);

    Text & user_data(void * v);

    Text & frame(Frame frame);

    virtual ViewState tick(Scope const & scope, Events const & events,
                           Fn<void(View &)> build) override;

    virtual Layout fit(Scope const & scope, f32x2 allocated, Span<f32x2 const> sizes,
                       Span<f32x2> centers) override;

    virtual void render(Scope const & scope, Canvas canvas,
                        RenderInfo const & info) override;

    virtual Cursor cursor(Scope const & scope, f32x2 extent, f32x2 position) override;
};
}    // namespace ui

}    // namespace ash
