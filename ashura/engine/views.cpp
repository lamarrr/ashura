/// SPDX-License-Identifier: MIT
#include "ashura/engine/views.hpp"
#include "ashura/engine/window_system.hpp"
#include "ashura/std/text.hpp"
#include "ashura/std/trace.hpp"

namespace ash
{
namespace ui
{

Space::Space(Frame frame) : style_{.frame = frame}
{
}

Space::Space(f32x2 extent) : style_{.frame = Frame{}.abs(extent)}
{
}

Space & Space::frame(Frame frame)
{
    style_.frame = frame;
    return *this;
}

Space & Space::extent(f32x2 extent)
{
    style_.frame = Frame{}.abs(extent);
    return *this;
}

Layout Space::fit(Scope const &, f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
    return Layout{.extent = style_.frame(allocated)};
}

void Text::Cfg::default_clipboard_setter(Str32 str)
{
    ThreadScratchScope scratch;
    auto *             clipboard = sys.win->get_clipboard();
    auto               str8      = Vec<c8>::make(str.size() * 4, scratch).unwrap();
    utf8_encode(str, str8).unwrap();
    clipboard->set(MIME_TEXT_UTF8, str8.view().as_u8()).unwrap();
}

StrVec32 Text::Cfg::default_clipboard_getter(Allocator allocator)
{
    ThreadScratchScope scratch;
    auto *             clipboard = sys.win->get_clipboard();
    auto               str8      = Vec<u8>{scratch};
    clipboard->get(MIME_TEXT_UTF8, str8).unwrap();
    auto str32 = Vec<c32>::make(str8.size(), allocator).unwrap();
    utf8_decode(str8.view().as_c8(), str32).unwrap();
    return str32;
}

/// @brief Handles interaction and state updates for text views
/// It will issue actions that the text view can then execute to update its
/// internal state
static void apply_action(Text::Cfg const & cfg, Text::CursorAction a, Text::State & s)
{
    ThreadScratchScope scratch;

    switch (a.type)
    {
        case Text::CursorActionType::None:
        {
        }
        break;
        case Text::CursorActionType::Unselect:
        {
            s.text_.unselect();
        }
        break;
        case Text::CursorActionType::Escape:
        {
            s.text_.remove_cursor();
            s.text_.unselect();
        }
        break;
        case Text::CursorActionType::SelectLeft:
        {
            s.text_.select_left();
        }
        break;
        case Text::CursorActionType::SelectRight:
        {
            s.text_.select_right();
        }
        break;
        case Text::CursorActionType::SelectUp:
        {
            s.text_.select_up();
        }
        break;
        case Text::CursorActionType::SelectDown:
        {
            s.text_.select_down();
        }
        break;
        case Text::CursorActionType::SelectToLineStart:
        {
            s.text_.select_to_line_start();
        }
        break;
        case Text::CursorActionType::SelectToLineEnd:
        {
            s.text_.select_to_line_end();
        }
        break;
        case Text::CursorActionType::SelectPageUp:
        {
            s.text_.select_page_up(cfg.lines_per_page);
        }
        break;
        case Text::CursorActionType::SelectPageDown:
        {
            s.text_.select_page_down(cfg.lines_per_page);
        }
        break;
        case Text::CursorActionType::SelectAll:
        {
            s.text_.select_all();
        }
        break;
        case Text::CursorActionType::SelectWord:
        {
            s.text_.select_word(DEFAULT_WORD_SYMBOLS);
        }
        break;
        case Text::CursorActionType::SelectLine:
        {
            s.text_.select_line();
        }
        break;
        case Text::CursorActionType::Left:
        {
            s.text_.left();
        }
        break;
        case Text::CursorActionType::Right:
        {
            s.text_.right();
        }
        break;
        case Text::CursorActionType::LineStart:
        {
            s.text_.line_start();
        }
        break;
        case Text::CursorActionType::LineEnd:
        {
            s.text_.line_end();
        }
        break;
        case Text::CursorActionType::Up:
        {
            s.text_.up();
        }
        break;
        case Text::CursorActionType::Down:
        {
            s.text_.down();
        }
        break;
        case Text::CursorActionType::PageUp:
        {
            s.text_.page_up(cfg.lines_per_page);
        }
        break;
        case Text::CursorActionType::PageDown:
        {
            s.text_.page_down(cfg.lines_per_page);
        }
        break;
        case Text::CursorActionType::Insert:
        {
            s.text_.insert(a.text.get());
        }
        break;
        case Text::CursorActionType::Cut:
        {
            auto cut = s.text_.copy_cut(scratch);
            cfg.clipboard_setter(cut);
        }
        break;
        case Text::CursorActionType::Copy:
        {
            auto copied = s.text_.copy(scratch);
            cfg.clipboard_setter(copied);
        }
        break;
        case Text::CursorActionType::Paste:
        {
            auto text = cfg.clipboard_getter(s.allocator_);
            s.text_.insert(text);
        }
        break;
        case Text::CursorActionType::NewLine:
        {
            s.text_.new_line();
        }
        break;
        case Text::CursorActionType::Tab:
        {
            s.text_.tab();
        }
        break;
        case Text::CursorActionType::Backspace:
        {
            s.text_.backspace();
        }
        break;
        case Text::CursorActionType::Delete:
        {
            s.text_.del();
        }
        break;
        case Text::CursorActionType::Home:
        {
            s.text_.line_start();
        }
        break;
        case Text::CursorActionType::End:
        {
            s.text_.line_end();
        }
        break;
        case Text::CursorActionType::Hit:
        {
            s.text_.hit(a.center, a.transform, a.transformed_position);
        }
        break;
        case Text::CursorActionType::HitSelectSpan:
        {
            s.text_.hit_select(a.center, a.transform, a.transformed_position);
        }
        break;
    }
}

static void apply_action(Text::Cfg const & cfg, Text::CoreAction action,
                         Text::State & s)
{
    switch (action.type)
    {
        case Text::CoreActionType::None:
        {
        }
        break;

        case Text::CoreActionType::ReplaceText:
        {
            s.text_.text(action.text.unwrap());
        }
        break;

        case Text::CoreActionType::Undo:
        {
            warn("Undo action is not implemented yet"_s);
        }
        break;
        case Text::CoreActionType::Redo:
        {
            warn("Redo action is not implemented yet"_s);
        }
        break;
        case Text::CoreActionType::Submit:
        {
            auto str = s.text_.str();
            cfg.on_submit(str);
        }
        break;
    }
}

static void exec_action(Text::Cfg const & cfg, Text::Action action, Text::State & s)
{
    action.match(
      [&](Text::CursorAction & action) { apply_action(cfg, std::move(action), s); },
      [&](Text::CoreAction & action) { apply_action(cfg, std::move(action), s); },
      [](None) {});
}

static Text::Action input_to_action(ui::Scope const & scope, Text::Cfg const & cfg,
                                    Events const & events, Allocator allocator)
{
    auto & k = scope.key();
    auto & m = scope.mouse();

    auto shift = k.held(KeyModifiers::LeftShift) || k.held(KeyModifiers::RightShift);
    auto ctrl  = k.held(KeyModifiers::LeftCtrl) || k.held(KeyModifiers::RightCtrl);

    if (events.focus_out())
    {
        return Text::CursorAction{.type = Text::CursorActionType::Escape};
    }

    if (cfg.highlightable)
    {
        if (events.pointer_down())
        {
            return Text::CursorAction{
              .type   = Text::CursorActionType::Hit,
              .center = events.hit_info_.v().canvas_region.center,
              .transform =
                transform2d_to_3d(events.hit_info_.v().canvas_transform).to_mat(),
              .transformed_position = events.hit_info_.v().canvas_hit};
        }

        if (events.drag_update())
        {
            auto center = events.hit_info_.v().canvas_region.center;
            auto transform =
              transform2d_to_3d(events.hit_info_.v().canvas_transform).to_mat();
            auto transformed_position = events.hit_info_.v().canvas_hit;

            return Text::CursorAction{.type   = Text::CursorActionType::HitSelectSpan,
                                      .center = center,
                                      .transform            = transform,
                                      .transformed_position = transformed_position};
        }

        if (events.click())
        {
            auto center = events.hit_info_.v().canvas_region.center;
            auto transform =
              transform2d_to_3d(events.hit_info_.v().canvas_transform).to_mat();
            auto transformed_position = events.hit_info_.v().canvas_hit;

            if (m.clicks(MouseButton::Primary) == 2)
            {
                return Text::CursorAction{.type   = Text::CursorActionType::SelectWord,
                                          .center = center,
                                          .transform            = transform,
                                          .transformed_position = transformed_position};
            }
            else if (m.clicks(MouseButton::Primary) == 3)
            {
                return Text::CursorAction{.type   = Text::CursorActionType::SelectLine,
                                          .center = center,
                                          .transform            = transform,
                                          .transformed_position = transformed_position};
            }
            else if (m.clicks(MouseButton::Primary) == 4)
            {
                return Text::CursorAction{.type   = Text::CursorActionType::SelectAll,
                                          .center = center,
                                          .transform            = transform,
                                          .transformed_position = transformed_position};
            }
        }
    }

    if (cfg.editable && events.text_input())
    {
        auto text_u8  = scope.key().input_text().unwrap();
        auto text_c32 = Vec<c32>::make(text_u8.size(), allocator).unwrap();
        utf8_decode(text_u8.view().as_c8(), text_c32).unwrap();
        auto rc_str32 = rc<StrVec32>(allocator, std::move(text_c32)).unwrap();
        auto view     = rc_str32->view().as_const();
        auto text     = transmute(std::move(rc_str32), view);
        return Text::CursorAction{.type = Text::CursorActionType::Insert,
                                  .text = std::move(text)};
    }

    if (events.key_down())
    {
        if (cfg.highlightable)
        {
            if (shift && k.down(KeyCode::Left))
            {
                return Text::CursorAction{.type = Text::CursorActionType::SelectLeft};
            }
            else if (shift && k.down(KeyCode::Right))
            {
                return Text::CursorAction{.type = Text::CursorActionType::SelectRight};
            }
            else if (shift && k.down(KeyCode::Up))
            {
                return Text::CursorAction{.type = Text::CursorActionType::SelectUp};
            }
            else if (shift && k.down(KeyCode::Down))
            {
                return Text::CursorAction{.type = Text::CursorActionType::SelectDown};
            }
            else if (shift && k.down(KeyCode::Home))
            {
                return Text::CursorAction{.type =
                                            Text::CursorActionType::SelectToLineStart};
            }
            else if (shift && k.down(KeyCode::End))
            {
                return Text::CursorAction{.type =
                                            Text::CursorActionType::SelectToLineEnd};
            }
            else if (shift && k.down(KeyCode::PageUp))
            {
                return Text::CursorAction{.type = Text::CursorActionType::SelectPageUp};
            }
            else if (shift && k.down(KeyCode::PageDown))
            {
                return Text::CursorAction{.type =
                                            Text::CursorActionType::SelectPageDown};
            }
            else if (ctrl && k.down(KeyCode::A))
            {
                return Text::CursorAction{.type = Text::CursorActionType::SelectAll};
            }
            else if (k.down(KeyCode::Escape))
            {
                return Text::CursorAction{.type = Text::CursorActionType::Escape};
            }
        }

        if (cfg.copyable && ctrl && k.down(KeyCode::C))
        {
            return Text::CursorAction{.type = Text::CursorActionType::Copy};
        }

        if (cfg.editable)
        {
            if (ctrl && k.down(KeyCode::X))
            {
                return Text::CursorAction{.type = Text::CursorActionType::Cut};
            }
            else if (ctrl && k.down(KeyCode::V))
            {
                return Text::CursorAction{.type = Text::CursorActionType::Paste};
            }
            else if (ctrl && k.down(KeyCode::Z))
            {
                return Text::CoreAction{.type = Text::CoreActionType::Undo};
            }
            else if (ctrl && k.down(KeyCode::Y))
            {
                return Text::CoreAction{.type = Text::CoreActionType::Redo};
            }
            else if (cfg.enable_multiline_input && !cfg.enter_submits &&
                     k.down(KeyCode::Return))
            {
                return Text::CursorAction{.type = Text::CursorActionType::NewLine};
            }
            else if (cfg.accept_tab_input && k.down(KeyCode::Tab))
            {
                return Text::CursorAction{.type = Text::CursorActionType::Tab};
            }
            else if (k.down(KeyCode::Backspace))
            {
                return Text::CursorAction{.type = Text::CursorActionType::Backspace};
            }
            else if (k.down(KeyCode::Delete))
            {
                return Text::CursorAction{.type = Text::CursorActionType::Delete};
            }
            else if (k.down(KeyCode::Left))
            {
                return Text::CursorAction{.type = Text::CursorActionType::Left};
            }
            else if (k.down(KeyCode::Right))
            {
                return Text::CursorAction{.type = Text::CursorActionType::Right};
            }
            else if (k.down(KeyCode::Home))
            {
                return Text::CursorAction{.type = Text::CursorActionType::LineStart};
            }
            else if (k.down(KeyCode::End))
            {
                return Text::CursorAction{.type = Text::CursorActionType::LineEnd};
            }
            else if (k.down(KeyCode::Up))
            {
                return Text::CursorAction{.type = Text::CursorActionType::Up};
            }
            else if (k.down(KeyCode::Down))
            {
                return Text::CursorAction{.type = Text::CursorActionType::Down};
            }
            else if (k.down(KeyCode::PageUp))
            {
                return Text::CursorAction{.type = Text::CursorActionType::PageUp};
            }
            else if (k.down(KeyCode::PageDown))
            {
                return Text::CursorAction{.type = Text::CursorActionType::PageDown};
            }
        }

        if (cfg.enter_submits && k.down(KeyCode::Return))
        {
            return Text::CoreAction{.type = Text::CoreActionType::Submit};
        }
    }

    return none;
}

Vec<Text::Action> Text::Cfg::default_input_to_actions_map(ui::Scope const & scope,
                                                          Cfg const &       cfg,
                                                          Events const &    events,
                                                          Allocator         allocator)

{
    auto actions = Vec<Text::Action>{allocator};
    actions.push(input_to_action(scope, cfg, events, allocator)).unwrap();
    return actions;
}

void Text::Cfg::default_renderer(ui::Scope const &, Cfg const &, State const &,
                                 TextRenderInfo const &    info,
                                 TextPlacementInfo const & placement, f32x2 center,
                                 Canvas canvas)
{
    canvas->text(info, placement, center);
}

void Text::State::tick(ui::Scope const & scope, Cfg const & cfg, Events const & events)
{
    auto actions = cfg.input_to_actions_map(scope, cfg, events, allocator_);

    for (auto & action : actions)
    {
        exec_action(cfg, std::move(action), *this);
    }
}

ui::Layout Text::State::fit(ui::Scope const &, f32x2 allocated, Span<f32x2 const>,
                            Span<f32x2>)
{
    text_.width(allocated.x(), allocated.x());
    text_.perform_layout();

    return ui::Layout{.extent = text_.layout().extent};
}

void Text::State::render(ui::Scope const & scope, Cfg const & cfg, Canvas canvas,
                         ui::RenderInfo const & info)
{
    ASH_TRACE_SCOPE;
    ThreadScratchScope scratch;

    auto highlights = Vec<Slice>{scratch};
    highlights.append(highlights_).unwrap();
    auto highlight_styles = Vec<TextHighlightStyle>{scratch};
    highlight_styles.append(highlight_styles_).unwrap();

    if (text_.cursor_.is_some())
    {
        if (text_.cursor_->has_selection())
        {
            highlights.push(text_.cursor_->selection()).unwrap();
            highlight_styles.push(text_.highlight_style_).unwrap();
        }
    }

    auto carets       = Vec<usize>{scratch};
    auto caret_styles = Vec<CaretStyle>{scratch};

    if (text_.cursor_.is_some())
    {
        auto caret = text_.cursor_->caret();
        carets.push((usize) caret).unwrap();
        caret_styles.push(text_.caret_style_).unwrap();
    }

    auto [text_info, placement] =
      text_.text_.place(transform2d_to_3d(info.canvas_transform).to_mat(), info.clip,
                        highlights, highlight_styles, carets, caret_styles, scratch);

    auto placement_info = TextPlacementInfo{
      .blocks         = placement.blocks,
      .lines          = placement.lines,
      .backgrounds    = placement.backgrounds,
      .glyph_shadows  = placement.glyph_shadows,
      .glyphs         = placement.glyphs,
      .underlines     = placement.underlines,
      .strikethroughs = placement.strikethroughs,
      .highlights     = placement.highlights,
      .carets         = placement.carets,
    };

    cfg.renderer(scope, cfg, *this, text_info, placement_info,
                 info.canvas_region.center, canvas);
}

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

Text & Text::input_to_actions_map(Rc<InputToActionsMap> map)
{
    cfg_.input_to_actions_map = std::move(map);
    return *this;
}

Text & Text::renderer(Rc<Renderer> renderer)
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

Text & Text::style(TextRunsStyle style)
{
    state_.text_.runs_style(std::move(style));
    return *this;
}

Text & Text::style(TextStyle style, FontStyle font)
{
    return this->style(TextRunsStyle::all(std::move(style), std::move(font)));
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

Text & Text::frame(Frame const & f)
{
    frame_ = f;
    return *this;
}

ViewState Text::tick(Scope const & scope, Events const & events, Fn<void(View &)>)
{
    state_.tick(scope, cfg_, events);
    auto interactable = cfg_.copyable || cfg_.highlightable || cfg_.editable;
    return ViewState{
      .text{.enabled     = interactable,
            .type        = TextInputType::Text,
            .multiline   = interactable && cfg_.enable_multiline_input,
            .esc_input   = false,
            .tab_input   = interactable && cfg_.accept_tab_input,
            .cap         = TextCapitalization::None,
            .autocorrect = false},
      .clickable = interactable,
      .draggable = interactable,
      .focusable = interactable
    };
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

Image::Image(ImageId src) : style_{}, source_{src}, state_{}
{
    state_.info = sys.image->get(src);
}

Image & Image::source(ImageId src)
{
    source_     = src;
    state_.info = sys.image->get(src);
    return *this;
}

Image & Image::aspect_ratio(f32x2 ratio)
{
    style_.aspect_ratio =
      (ratio.x() == 0 || ratio.y() == 0) ? 1 : (ratio.x() / ratio.y());
    return *this;
}

Image & Image::aspect_ratio(Option<f32> ratio)
{
    style_.aspect_ratio = ratio;
    return *this;
}

Image & Image::frame(Frame const & frame)
{
    style_.frame = frame;
    return *this;
}

Image & Image::radii(CornerRadii const & radii)
{
    style_.radii = radii;
    return *this;
}

Image & Image::tint(ColorGradient const & color)
{
    style_.tint = color;
    return *this;
}

Image & Image::fit(Fit fit)
{
    style_.fit = fit;
    return *this;
}

Image & Image::align(f32x2 a)
{
    style_.alignment = a;
    return *this;
}

Layout Image::fit(Scope const &, f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
    auto frame = style_.frame(allocated);
    return Layout{
      .extent = style_.aspect_ratio.map([&](f32 ar) { return with_aspect(frame, ar); })
                  .unwrap_or(frame)};
}

static Tuple<f32x2, f32x2, f32x2> fit_image(f32x2 image_extent, f32x2 region_extent,
                                            Image::Fit fit)
{
    auto ar = image_extent.x() / image_extent.y();

    switch (fit)
    {
        case Image::Fit::Crop:
        {
            auto dst_ar    = region_extent.x() / region_extent.y();
            auto uv_extent = with_aspect(f32x2{ar, 1}, dst_ar) / ar;
            auto space     = (1 - uv_extent) * 0.5F;
            return {region_extent, space, 1 - space};
        }
        case Image::Fit::Stretch:
        {
            return {
              region_extent, {0, 0},
               {1, 1}
            };
        }
        default:
        case Image::Fit::Contain:
        {
            return {
              with_aspect(region_extent, ar), {0, 0},
                {1, 1}
            };
        }
    }
}

void Image::render(Scope const &, Canvas canvas, RenderInfo const & info)
{
    auto [extent, uv0, uv1] = fit_image(state_.info.info.extent.xy().to<f32>(),
                                        info.canvas_region.extent, style_.fit);

    auto center = space_align(info.canvas_region.extent, extent, style_.alignment);

    canvas->rrect(Shape{
      .world_transform = f32x4x4::identity(),
      .uv_transform    = f32x4x4::identity(),
      .area{info.canvas_region.center + center, extent},
      .bbox_extent = extent,
      .radii       = style_.radii,
      .shade       = ShadeType::Flood,
      .feather     = 0,
      .tint        = style_.tint,
      .sampler     = SamplerIndex::LinearEdgeClampBlackFloat,
      .texture_set = sampled_textures,
      .map         = state_.info.textures[0],
      .sdf_sampler = SamplerIndex::LinearRepeatWhiteFloat,
      .sdf_map     = TextureIndex::White,
    });
}

Flex::Flex(Allocator allocator) : allocator_{allocator}, items_{allocator}
{
}

Flex & Flex::axis(Axis a)
{
    style_.axis = a;
    return *this;
}

Flex & Flex::wrap(bool w)
{
    style_.wrap = w;
    return *this;
}

Flex & Flex::main_align(MainAlign align)
{
    style_.main_align = align;
    return *this;
}

Flex & Flex::cross_align(f32 align)
{
    style_.cross_align = align;
    return *this;
}

Flex & Flex::frame(Frame const & f)
{
    style_.frame = f;
    return *this;
}

Flex & Flex::item_frame(Frame const & f)
{
    style_.item_frame = f;
    return *this;
}

Flex & Flex::item_frame(Span<Frame const> frames)
{
    ASH_CHECK(frames.size() == items_.size(), "");
    style_.item_frame = vec::copy(allocator_, frames).unwrap();
    return *this;
}

Flex & Flex::item_frame(InitList<Frame> frames)
{
    ASH_CHECK(frames.size() == items_.size(), "");
    return item_frame(span(frames));
}

Flex & Flex::items(InitList<ref<View>> list)
{
    return items(span(list));
}

Flex & Flex::items(Span<ref<View> const> list)
{
    items_.append(list).unwrap();
    return *this;
}

ViewState Flex::tick(Scope const &, Events const &, Fn<void(View &)> build)
{
    for (ref item : items_)
    {
        build(item);
    }

    return ViewState{};
}

void Flex::size(Scope const &, f32x2 allocated, Span<f32x2> sizes)
{
    auto box = style_.frame(allocated);
    style_.item_frame.match([&](Frame const & frame) { fill(sizes, frame(box)); },
                            [&](Vec<Frame> const & frames) {
                                for (auto [size, frame] : zip(sizes, frames))
                                {
                                    size = frame(box);
                                }
                            });
}

Layout Flex::fit(Scope const &, f32x2 allocated, Span<f32x2 const> sizes,
                 Span<f32x2> centers)
{
    auto  n            = sizes.size();
    auto  frame        = style_.frame(allocated);
    auto  main_axis    = (style_.axis == Axis::X) ? 0 : 1;
    auto  cross_axis   = (style_.axis == Axis::X) ? 1 : 0;
    f32x2 span         = {};
    f32   cross_cursor = 0;

    for (usize i = 0; i < n;)
    {
        auto first        = i++;
        f32  main_extent  = sizes[first][main_axis];
        f32  cross_extent = sizes[first][cross_axis];
        f32  main_spacing = 0;

        while (i < n &&
               !(style_.wrap && (main_extent + sizes[i][main_axis]) > frame[main_axis]))
        {
            main_extent += sizes[i][main_axis];
            cross_extent = max(cross_extent, sizes[i][cross_axis]);
            i++;
        }

        auto count = i - first;

        if (style_.main_align != MainAlign::Start)
        {
            main_spacing = max(frame[main_axis] - main_extent, 0.0F);
        }

        for (auto [center, size] :
             zip(centers.slice(first, count), sizes.slice(first, count)))
        {
            auto pos = space_align(cross_extent, size[cross_axis], style_.cross_align);
            center[cross_axis] = cross_cursor + cross_extent * 0.5F + pos;
        }

        switch (style_.main_align)
        {
            case MainAlign::Start:
            {
                f32 main_spacing_cursor = 0;
                for (auto [center, size] :
                     zip(centers.slice(first, count), sizes.slice(first, count)))
                {
                    center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
                    main_spacing_cursor += size[main_axis];
                }
            }
            break;

            case MainAlign::SpaceAround:
            {
                f32 spacing             = main_spacing / (count * 2);
                f32 main_spacing_cursor = 0;
                for (auto [center, size] :
                     zip(centers.slice(first, count), sizes.slice(first, count)))
                {
                    main_spacing_cursor += spacing;
                    center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
                    main_spacing_cursor += size[main_axis] + spacing;
                }
            }
            break;

            case MainAlign::SpaceBetween:
            {
                f32 spacing             = main_spacing / (count - 1);
                f32 main_spacing_cursor = 0;
                for (auto [center, size] :
                     zip(centers.slice(first, count), sizes.slice(first, count)))
                {
                    center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
                    main_spacing_cursor += size[main_axis] + spacing;
                }
            }
            break;

            case MainAlign::SpaceEvenly:
            {
                f32 spacing             = main_spacing / (count + 1);
                f32 main_spacing_cursor = spacing;
                for (auto [center, size] :
                     zip(centers.slice(first, count), sizes.slice(first, count)))
                {
                    center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
                    main_spacing_cursor += size[main_axis] + spacing;
                }
            }
            break;

            case MainAlign::End:
            {
                f32 main_spacing_cursor = main_spacing;
                for (auto [center, size] :
                     zip(centers.slice(first, count), sizes.slice(first, count)))
                {
                    center[main_axis] = main_spacing_cursor + size[main_axis] * 0.5F;
                    main_spacing_cursor += size[main_axis];
                }
            }
            break;

            default:
                break;
        }

        cross_cursor += cross_extent;

        span[main_axis]  = max(span[main_axis], main_extent + main_spacing);
        span[cross_axis] = cross_cursor;
    }

    // convert from cursor space [0, w] to parent space [-0.5w, 0.5w]
    for (auto & center : centers)
    {
        center -= span * 0.5F;
    }

    return {.extent = span};
}

Stack::Stack(Allocator allocator) : allocator_{allocator}, style_{}, items_{allocator}
{
}

Stack & Stack::reverse(bool r)
{
    if (!r)
    {
        return *this;
    }

    style_.stack_order.match([](i32 & order) { order *= -1; },
                             [](Vec<i32> & orders) {
                                 for (auto & order : orders)
                                 {
                                     order *= -1;
                                 }
                             });
    return *this;
}

Stack & Stack::stack_order(Span<i32 const> order)
{
    style_.stack_order = vec::copy(allocator_, order).unwrap();
    return *this;
}

Stack & Stack::stack_order(InitList<i32> order)
{
    style_.stack_order = vec::copy(allocator_, span(order)).unwrap();
    return *this;
}

Stack & Stack::align(f32x2 a)
{
    style_.alignment = a;
    return *this;
}

Stack & Stack::align(Span<f32x2 const> a)
{
    ASH_CHECK(a.size() == items_.size(), "");
    style_.alignment = vec::copy(allocator_, a).unwrap();
    return *this;
}

Stack & Stack::align(InitList<f32x2> a)
{
    ASH_CHECK(a.size() == items_.size(), "");
    style_.alignment = vec::copy(allocator_, span(a)).unwrap();
    return *this;
}

Stack & Stack::items(InitList<ref<View>> list)
{
    items(span(list));
    return *this;
}

Stack & Stack::items(Span<ref<View> const> list)
{
    items_.append(span(list)).unwrap();
    return *this;
}

Stack & Stack::frame(Frame const & frame)
{
    style_.frame = frame;
    return *this;
}

Stack & Stack::item_frame(Frame const & frame)
{
    style_.item_frame = frame;
    return *this;
}

Stack & Stack::item_frame(InitList<Frame> list)
{
    style_.item_frame = vec::copy(allocator_, span(list)).unwrap();
    return *this;
}

Stack & Stack::item_frame(Span<Frame const> list)
{
    style_.item_frame = vec::copy(allocator_, list).unwrap();
    return *this;
}

ViewState Stack::tick(Scope const &, Events const &, Fn<void(View &)> build)
{
    for (ref item : items_)
    {
        build(item);
    }

    return ViewState{};
}

void Stack::size(Scope const &, f32x2 allocated, Span<f32x2> sizes)
{
    auto box = style_.frame(allocated);
    style_.item_frame.match([&](Frame const & frame) { fill(sizes, frame(box)); },
                            [&](Vec<Frame> const & frames) {
                                for (auto [size, frame] : zip(sizes, frames))
                                {
                                    size = frame(box);
                                }
                            });
}

Layout Stack::fit(Scope const &, f32x2, Span<f32x2 const> sizes, Span<f32x2> centers)
{
    f32x2 span;

    for (auto size : sizes)
    {
        span.x() = max(span.x(), size.x());
        span.y() = max(span.y(), size.y());
    }

    for (auto [center, size, i] : zip(centers, sizes, range(sizes.size())))
    {
        center =
          space_align(span, size,
                      style_.alignment.match(
                        [&](f32x2 const & alignment) { return alignment; },
                        [&](Vec<f32x2> const & alignments) { return alignments[i]; }));
    }

    return {.extent = span};
}

i32 Stack::z_index(Scope const &, i32 allocated, Span<i32> indices)
{
    for (auto [i, stack_index] : enumerate<i32>(indices))
    {
        stack_index = style_.stack_order.match(
          [&](i32 const & order) {
              if (order > 0)
              {
                  return allocated + i;
              }
              else
              {
                  return allocated + (static_cast<i32>(indices.size()) - 1) - i;
              }
          },
          [&](Vec<i32> const & orders) { return allocated + orders[i]; });
    }

    return allocated;
}

Button & Button::disable(bool disable)
{
    state_.disabled = disable;
    return *this;
}

Button & Button::color(ColorGradient const & color)
{
    style_.idle_color     = color;
    style_.disabled_color = color;
    style_.hovered_color  = color;
    style_.pressed_color  = color;
    return *this;
}

Button & Button::idle_color(ColorGradient const & color)
{
    style_.idle_color = color;
    return *this;
}

Button & Button::disabled_color(ColorGradient const & color)
{
    style_.disabled_color = color;
    return *this;
}

Button & Button::hovered_color(ColorGradient const & color)
{
    style_.hovered_color = color;
    return *this;
}

Button & Button::pressed_color(ColorGradient const & color)
{
    style_.pressed_color = color;
    return *this;
}

Button & Button::focus_ring_color(ColorGradient const & color)
{
    style_.focus_ring_color = color;
    return *this;
}

Button & Button::rrect(CornerRadii const & radii)
{
    style_.corner_radii = radii;
    style_.shape        = Shape::RRect;
    return *this;
}

Button & Button::squircle(f32 degree)
{
    style_.corner_radii = CornerRadii{degree, degree, degree, degree};
    style_.shape        = Shape::Squircle;
    return *this;
}

Button & Button::stroke(bool stroke)
{
    style_.stroke = stroke;
    return *this;
}

Button & Button::thickness(f32 thickness)
{
    style_.thickness = thickness;
    return *this;
}

Button & Button::padding(Padding const & p)
{
    style_.padding = p;
    return *this;
}

Button & Button::focus_ring_thickness(f32 thickness)
{
    style_.focus_ring_thickness = thickness;
    return *this;
}

Button & Button::item_frame(Frame const & f)
{
    style_.item_frame = f;
    return *this;
}

Button & Button::frame(Frame const & f)
{
    style_.frame = f;
    return *this;
}

Button & Button::on_click(Dyn<Fn<void(u32)>> f)
{
    callbacks_.on_click = std::move(f);
    return *this;
}

Button & Button::on_click(Fn<void(u32)> f)
{
    callbacks_.on_click = Dyn<Fn<void(u32)>>{f, dyn_noop};
    return *this;
}

Button & Button::on_hold(Dyn<Fn<void()>> f)
{
    callbacks_.on_hold = std::move(f);
    return *this;
}

Button & Button::on_hold(Fn<void()> f)
{
    callbacks_.on_hold = Dyn<Fn<void()>>{f, dyn_noop};
    return *this;
}

Button & Button::on_release(Dyn<Fn<void()>> f)
{
    callbacks_.on_release = std::move(f);
    return *this;
}

Button & Button::on_release(Fn<void()> f)
{
    callbacks_.on_release = Dyn<Fn<void()>>{f, dyn_noop};
    return *this;
}

Button & Button::on_hover(Dyn<Fn<void()>> fn)
{
    callbacks_.on_hover = std::move(fn);
    return *this;
}

Button & Button::on_hover(Fn<void()> fn)
{
    callbacks_.on_hover = Dyn<Fn<void()>>{fn, dyn_noop};
    return *this;
}

Button & Button::on_blur(Dyn<Fn<void()>> fn)
{
    callbacks_.on_blur = std::move(fn);
    return *this;
}

Button & Button::on_blur(Fn<void()> fn)
{
    callbacks_.on_blur = Dyn<Fn<void()>>{fn, dyn_noop};
    return *this;
}

Button & Button::on_focus_in(Dyn<Fn<void()>> fn)
{
    callbacks_.on_focus_in = std::move(fn);
    return *this;
}

Button & Button::on_focus_in(Fn<void()> fn)
{
    callbacks_.on_focus_in = Dyn<Fn<void()>>{fn, dyn_noop};
    return *this;
}

Button & Button::on_focus_out(Dyn<Fn<void()>> fn)
{
    callbacks_.on_focus_out = std::move(fn);
    return *this;
}

Button & Button::on_focus_out(Fn<void()> fn)
{
    callbacks_.on_focus_out = Dyn<Fn<void()>>{fn, dyn_noop};
    return *this;
}

Button & Button::item(Option<View &> v)
{
    item_ = v;
    return *this;
}

Button & Button::shadow(Option<Shadow> s)
{
    style_.shadow = s;
    return *this;
}

ViewState Button::tick(Scope const & scope, Events const & events,
                       Fn<void(View &)> build)
{
    auto & mouse = scope.mouse();
    auto & key   = scope.key();

    item_.match(build);

    if (state_.disabled)
    {
        state_ = State{.disabled = true};
    }
    else
    {
        if (events.click() || (events.focus_over() && key.down(KeyCode::Return)))
        {
            callbacks_.on_click(events.click() ? mouse.clicks(MouseButton::Primary) :
                                                 1);
            state_.held = false;
            callbacks_.on_release();
        }

        if (events.pointer_down())
        {
            state_.held = true;
            callbacks_.on_hold();
        }

        if (events.pointer_over())
        {
            state_.pointed = true;
        }

        if (events.pointer_out())
        {
            state_.held = false;
            callbacks_.on_release();
        }

        if (events.pointer_in())
        {
            state_.pointed = true;
            callbacks_.on_hover();
        }

        if (events.pointer_out())
        {
            state_.pointed = false;
            callbacks_.on_blur();
        }

        if (events.focus_in())
        {
            state_.focused = true;
            callbacks_.on_focus_in();
        }

        if (events.focus_out())
        {
            state_.focused = false;
            callbacks_.on_focus_out();
        }
    }

    return ViewState{.pointable = !state_.disabled,
                     .clickable = !state_.disabled,
                     .focusable = !state_.disabled};
}

void Button::size(Scope const &, f32x2 allocated, Span<f32x2> sizes)
{
    fill(sizes, style_.item_frame(allocated));
}

Layout Button::fit(Scope const &, f32x2, Span<f32x2 const> sizes, Span<f32x2> centers)
{
    fill(centers, f32x2{0, 0});
    auto size   = sizes.is_empty() ? f32x2{0, 0} : sizes[0];
    auto padded = style_.frame(size) + style_.padding.axes();

    if (style_.shape == Shape::Squircle)
    {
        padded = f32x2::splat(padded.max());
    }

    return {.extent = padded};
}

void Button::render(Scope const &, Canvas canvas, RenderInfo const & info)
{
    ColorGradient tint;

    if (state_.disabled)
    {
        tint = style_.disabled_color;
    }
    else if (state_.hovered)
    {
        tint = style_.hovered_color;
    }
    else if (state_.held)
    {
        tint = style_.pressed_color;
    }
    else if (state_.pointed)
    {
        tint = style_.hovered_color;
    }
    else
    {
        tint = style_.idle_color;
    }

    auto draw = [&](bool stroke, ColorGradient tint) {
        auto shape = ash::Shape{.world_transform = f32x4x4::identity(),
                                .uv_transform    = f32x4x4::identity(),
                                .area            = info.canvas_region,
                                .bbox_extent     = info.canvas_region.extent,
                                .radii           = style_.corner_radii,
                                .shade = stroke ? ShadeType::Stroked : ShadeType::Flood,
                                .feather     = stroke ? style_.thickness : 0,
                                .tint        = tint,
                                .sampler     = SamplerIndex::LinearEdgeClampBlackFloat,
                                .texture_set = sampled_textures,
                                .map         = TextureIndex::White,
                                .sdf_sampler = SamplerIndex::LinearRepeatWhiteFloat,
                                .sdf_map     = TextureIndex::White};

        switch (style_.shape)
        {
            case Shape::RRect:
                canvas->rrect(shape);
                break;
            case Shape::Squircle:
                canvas->squircle(shape);
                break;
            default:
                break;
        }
    };

    style_.shadow.match([&](Shadow const & shadow) {
        auto shape = ash::Shape{
          .world_transform = f32x4x4::identity(),
          .uv_transform    = f32x4x4::identity(),
          .area{info.canvas_region.center + shadow.offset,
                info.canvas_region.extent + shadow.spread},
          .bbox_extent = info.canvas_region.extent + shadow.spread + 2 * shadow.feather,
          .radii       = style_.corner_radii,
          .shade       = ShadeType::Feathered,
          .feather     = shadow.feather,
          .tint        = shadow.tint,
          .sampler     = SamplerIndex::LinearEdgeClampTransparentFloat,
          .texture_set = sampled_textures,
          .map         = TextureIndex::White,
          .sdf_sampler = SamplerIndex::LinearRepeatWhiteFloat,
          .sdf_map     = TextureIndex::White,
        };

        switch (style_.shape)
        {
            case Shape::RRect:
                canvas->rrect(shape);
                break;
            case Shape::Squircle:
                canvas->squircle(shape);
                break;
            default:
                break;
        }
    });

    draw(style_.stroke, tint);

    if (state_.focused)
    {
        draw(true, style_.focus_ring_color);
    }
}

Cursor Button::cursor(Scope const &, f32x2, f32x2)
{
    return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

Box & Box::item(Option<View &> item)
{
    item_ = item;
    return *this;
}

Box & Box::background_image(Option<ImageId> image, ColorGradient const & tint)
{
    style_.background_image = image.map([&](ImageId id) { return sys.image->get(id); });
    style_.background_image_tint = tint;
    return *this;
}

Box & Box::background_color(ColorGradient const & color)
{
    style_.background_color = color;
    return *this;
}

Box & Box::border_thickness(f32 thickness)
{
    style_.border_thickness = thickness;
    return *this;
}

Box & Box::border_color(ColorGradient const & color)
{
    style_.border_color = color;
    return *this;
}

Box & Box::radii(CornerRadii const & radii)
{
    style_.radii = radii;
    return *this;
}

Box & Box::padding(Padding const & padding)
{
    style_.padding = padding;
    return *this;
}

Box & Box::background_blur(f32 blur)
{
    style_.background_blur = blur;
    return *this;
}

Box & Box::frame(Frame const & frame)
{
    style_.frame = frame;
    return *this;
}

Box & Box::item_frame(Frame const & frame)
{
    style_.item_frame = frame;
    return *this;
}

Box & Box::shadow(Option<Shadow> shadow)
{
    style_.shadow = shadow;
    return *this;
}

ViewState Box::tick(Scope const &, Events const &, Fn<void(View &)> build)
{
    item_.match(build);
    return ViewState{};
}

void Box::size(Scope const &, f32x2 allocated, Span<f32x2> sizes)
{
    fill(sizes, style_.frame(allocated));
}

Layout Box::fit(Scope const &, f32x2, Span<f32x2 const> sizes, Span<f32x2> centers)
{
    auto item_size = sizes.is_empty() ? f32x2{0, 0} : sizes[0];
    auto size      = style_.frame(item_size) + style_.padding.axes();

    fill(centers, f32x2{0, 0});

    return Layout{.extent = size};
}

void Box::render(Scope const &, Canvas canvas, RenderInfo const & info)
{
    style_.shadow.match([&](Shadow const & shadow) {
        auto shape = Shape{
          .world_transform = f32x4x4::identity(),
          .uv_transform    = f32x4x4::identity(),
          .area{info.canvas_region.center + shadow.offset,
                info.canvas_region.extent + shadow.spread},
          .bbox_extent = info.canvas_region.extent + shadow.spread + 2 * shadow.feather,
          .radii       = style_.radii,
          .shade       = ShadeType::Feathered,
          .feather     = shadow.feather,
          .tint        = shadow.tint,
          .sampler     = SamplerIndex::LinearEdgeClampTransparentFloat,
          .texture_set = sampled_textures,
          .map         = TextureIndex::White,
          .sdf_sampler = SamplerIndex::LinearRepeatWhiteFloat,
          .sdf_map     = TextureIndex::White,
        };

        canvas->rrect(shape);
    });

    if (style_.background_blur > 0)
    {
        auto shape = Shape{
          .world_transform = f32x4x4::identity(),
          .uv_transform    = f32x4x4::identity(),
          .area            = info.canvas_region,
          .bbox_extent     = info.canvas_region.extent,
          .radii           = style_.radii,
          .shade           = ShadeType::Flood,
          .feather         = style_.background_blur,
          .tint            = colors::WHITE,
          .sampler         = SamplerIndex::LinearEdgeClampTransparentFloat,
          .texture_set     = sampled_textures,
          .map             = TextureIndex::White,
          .sdf_sampler     = SamplerIndex::LinearRepeatWhiteFloat,
          .sdf_map         = TextureIndex::White,
        };

        canvas->blur(shape);
    }
    else if (style_.background_image.is_some())
    {
        auto shape = Shape{
          .world_transform = f32x4x4::identity(),
          .uv_transform    = f32x4x4::identity(),
          .area            = info.canvas_region,
          .bbox_extent     = info.canvas_region.extent,
          .radii           = style_.radii,
          .shade           = ShadeType::Flood,
          .feather         = 0,
          .tint            = style_.background_image_tint,
          .sampler         = SamplerIndex::LinearEdgeClampTransparentFloat,
          .texture_set     = sampled_textures,
          .map             = style_.background_image->textures[0],
          .sdf_sampler     = SamplerIndex::LinearRepeatWhiteFloat,
          .sdf_map         = TextureIndex::White,
        };
        canvas->rrect(shape);
    }

    if (!style_.background_color.is_transparent())
    {
        auto shape = Shape{
          .world_transform = f32x4x4::identity(),
          .uv_transform    = f32x4x4::identity(),
          .area            = info.canvas_region,
          .bbox_extent     = info.canvas_region.extent,
          .radii           = style_.radii,
          .shade           = ShadeType::Flood,
          .feather         = 0,
          .tint            = style_.background_color,
          .sampler         = SamplerIndex::LinearEdgeClampTransparentFloat,
          .texture_set     = sampled_textures,
          .map             = TextureIndex::White,
          .sdf_sampler     = SamplerIndex::LinearRepeatWhiteFloat,
          .sdf_map         = TextureIndex::White,
        };
        canvas->rrect(shape);
    }

    if (style_.border_thickness > 0)
    {
        auto shape = Shape{
          .world_transform = f32x4x4::identity(),
          .uv_transform    = f32x4x4::identity(),
          .area            = info.canvas_region,
          .bbox_extent     = info.canvas_region.extent,
          .radii           = style_.radii,
          .shade           = ShadeType::Stroked,
          .feather         = style_.border_thickness,
          .tint            = style_.border_color,
          .sampler         = SamplerIndex::LinearEdgeClampBlackFloat,
          .texture_set     = sampled_textures,
          .map             = TextureIndex::White,
          .sdf_sampler     = SamplerIndex::LinearRepeatWhiteFloat,
          .sdf_map         = TextureIndex::White,
        };
        canvas->rrect(shape);
    }
}

CheckBox::CheckBox(Allocator allocator) :
  allocator_{allocator},
  style_{},
  state_{},
  callbacks_{},
  checked_text_{allocator, static_rc(U"check_box"_s), TextStyle{}, FontStyle{}},
  unchecked_text_{allocator, static_rc(U"check_box_outline_blank"_s), TextStyle{},
                  FontStyle{}}
{
}

}    // namespace ui
}    // namespace ash
