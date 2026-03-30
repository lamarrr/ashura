/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/text.hpp"
#include "ashura/std/text.hpp"

namespace ash
{
namespace ui
{

Text::Text(Allocator allocator) :
  Text{allocator, static_rc(U""_s), TextStyle{}, FontStyle{}}
{
}

Text::Text(Allocator allocator, Rc<Str32> t, TextStyle const & style,
           FontStyle const & font) :
  allocator_{allocator},
  state_{allocator},
  cfg_{},
  frame_{Frame{}.rel(1, 1)}
{
    state_.text_.str(std::move(t));
    state_.text_.runs_style(TextRunsStyle::all(style, font));
}

Text::Text(Allocator allocator, Str32 t, TextStyle const & style,
           FontStyle const & font) :
  allocator_{allocator},
  state_{allocator},
  cfg_{},
  frame_{Frame{}.rel(1, 1)}
{
    // TODO: empty states? cursor behaviours? around 0 carets
    state_.text_.str(t);
    state_.text_.runs_style(TextRunsStyle::all(style, font));
}

Text::Text(Allocator allocator, Str8 t, TextStyle const & style,
           FontStyle const & font) :
  allocator_{allocator},
  state_{allocator},
  cfg_{},
  frame_{Frame{}.rel(1, 1)}
{
    state_.text_.str(t);
    state_.text_.runs_style(TextRunsStyle::all(style, font));
}

Text & Text::copyable(bool v)
{
    cfg_.copyable = v;
    return *this;
}

Text & Text::highlightable(bool v)
{
    cfg_.highlightable = v;
    return *this;
}

Text & Text::enable_cursor(bool v)
{
    cfg_.enable_cursor = v;
    return *this;
}

Text & Text::editable(bool v)
{
    cfg_.editable = v;
    return *this;
}

Text & Text::enable_undo_redo(bool v)
{
    cfg_.enable_undo_redo = v;
    return *this;
}

Text & Text::enable_multiline_input(bool v)
{
    cfg_.enable_multiline_input = v;
    return *this;
}

Text & Text::accept_tab_input(bool v)
{
    cfg_.accept_tab_input = v;
    return *this;
}

Text & Text::enter_submits(bool v)
{
    cfg_.enter_submits = v;
    return *this;
}

Text & Text::lines_per_page(u16 v)
{
    cfg_.lines_per_page = v;
    return *this;
}

Text & Text::input_to_actions_map(Rc<txt::InputToActionsMap> map)
{
    cfg_.input_to_actions_map = std::move(map);
    return *this;
}

Text & Text::renderer(Rc<txt::Renderer> renderer)
{
    cfg_.renderer = std::move(renderer);
    return *this;
}

Text & Text::highlights(Span<Slice const>              highlights,
                        Span<TextHighlightStyle const> highlight_styles)
{
    state_.highlights_       = vec::copy(state_.allocator_, highlights).unwrap();
    state_.highlight_styles_ = vec::copy(state_.allocator_, highlight_styles).unwrap();
    return *this;
}

Text & Text::clear_highlights()
{
    state_.highlights_.clear();
    state_.highlight_styles_.clear();
    return *this;
}

Text & Text::style_runs(TextRunsStyle style)
{
    state_.text_.runs_style(std::move(style));
    return *this;
}

Text & Text::wrap(bool v)
{
    state_.text_.get_render_text().wrap(v);
    return *this;
}

Text & Text::use_kerning(bool v)
{
    state_.text_.get_render_text().use_kerning(v);
    return *this;
}

Text & Text::use_ligatures(bool v)
{
    state_.text_.get_render_text().use_ligatures(v);
    return *this;
}

Text & Text::font_scale(f32 v)
{
    state_.text_.get_render_text().font_scale(v);
    return *this;
}

Text & Text::direction(TextDirection v)
{
    state_.text_.get_render_text().direction(v);
    return *this;
}

Text & Text::language(Str v)
{
    state_.text_.get_render_text().language(v);
    return *this;
}

Text & Text::alignment(f32 v)
{
    state_.text_.get_render_text().alignment(v);
    return *this;
}

Str32 Text::str() const
{
    return state_.text_.str();
}

Text & Text::str(Str32 v)
{
    state_.text_.str(v);
    return *this;
}

Text & Text::str(Str8 v)
{
    state_.text_.str(v);
    return *this;
}

Text & Text::str(Rc<Str32> v)
{
    state_.text_.str(std::move(v));
    return *this;
}

ViewState Text::tick(Scope const & scope, Events const & events, Fn<void(View &)>)
{
    state_.tick(scope, cfg_, events);

    return ViewState{.draggable = cfg_.copyable || cfg_.highlightable};
}

Layout Text::fit(Scope const & scope, f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
    return state_.fit(scope, frame_(allocated), {}, {});
}

void Text::render(Scope const & scope, Canvas canvas, RenderInfo const & info)
{
    state_.render(scope, cfg_, canvas, info);
}

Cursor Text::cursor(Scope const &, f32x2, f32x2)
{
    return cfg_.copyable ? Cursor::Text : Cursor::Default;
}

}    // namespace ui
}    // namespace ash
