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
            s.text_.hit(a.position);
        }
        break;
        case Text::CursorActionType::HitSelectSpan:
        {
            s.text_.hit_select(a.position);
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

    auto get_text_pos = [&] {
        auto hit           = events.hit_info_.unwrap();
        auto transform     = transform2d_to_3d(hit.canvas_transform) *
                             translate3d(hit.viewport_region.center.append(0));
        auto inv_transform = inverse(transform.to_mat());
        auto local_pos     = ::ash::transform(inv_transform, hit.canvas_hit.append(0));
        return local_pos.xy();
    };

    if (cfg.highlightable)
    {
        if (events.pointer_down())
        {
            return Text::CursorAction{.type     = Text::CursorActionType::Hit,
                                      .position = get_text_pos()};
        }

        if (events.drag_update())
        {
            return Text::CursorAction{.type     = Text::CursorActionType::HitSelectSpan,
                                      .position = get_text_pos()};
        }

        if (events.click())
        {
            if (m.clicks(MouseButton::Primary) == 2)
            {
                return Text::CursorAction{.type = Text::CursorActionType::SelectWord,
                                          .position = get_text_pos()};
            }
            else if (m.clicks(MouseButton::Primary) == 3)
            {
                return Text::CursorAction{.type = Text::CursorActionType::SelectLine,
                                          .position = get_text_pos()};
            }
            else if (m.clicks(MouseButton::Primary) == 4)
            {
                return Text::CursorAction{.type     = Text::CursorActionType::SelectAll,
                                          .position = get_text_pos()};
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
                                 TextPlacementInfo const & placement,
                                 f32x4x4 const & transform, Canvas canvas)
{
    canvas->text(info, placement, transform);
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

    // TODO: properly handle clip

    auto [text_info, placement] = text_.text_.place(
      info.clip, highlights, highlight_styles, carets, caret_styles, scratch);

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

    auto transform = transform2d_to_3d(info.canvas_transform) *
                     translate3d(info.viewport_region.center.append(0));

    cfg.renderer(scope, cfg, *this, text_info, placement_info, transform.to_mat(),
                 canvas);
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

Text & Text::hidden(bool v)
{
    state_.hidden_ = v;
    return *this;
}

ViewState Text::tick(Scope const & scope, Events const & events, Fn<void(View &)>)
{
    state_.tick(scope, cfg_, events);
    auto interactable =
      !state_.hidden_ && (cfg_.copyable || cfg_.highlightable || cfg_.editable);
    return ViewState{
      .text{.enabled     = interactable,
            .type        = TextInputType::Text,
            .multiline   = interactable && cfg_.enable_multiline_input,
            .esc_input   = false,
            .tab_input   = interactable && cfg_.accept_tab_input,
            .cap         = TextCapitalization::None,
            .autocorrect = false},
      .hidden    = state_.hidden_,
      .clickable = interactable,
      .draggable = interactable,
      .focusable = interactable,
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

Button & Button::pointed_color(ColorGradient const & color)
{
    style_.pointed_color = color;
    return *this;
}

Button & Button::pressed_color(ColorGradient const & color)
{
    style_.pressed_color = color;
    return *this;
}

Button & Button::focus_ring(FocusRing const & focus_ring)
{
    style_.focus_ring = focus_ring;
    return *this;
}

Button & Button::rrect(CornerRadii const & radii)
{
    style_.radii = radii;
    style_.shape = Shape::RRect;
    return *this;
}

Button & Button::squircle(f32 degree)
{
    style_.radii = CornerRadii{degree, degree, degree, degree};
    style_.shape = Shape::Squircle;
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

Button & Button::on_click(Fn<void(u32)> fn)
{
    callbacks_.on_click = fn;
    return *this;
}

Button & Button::on_hold(Fn<void()> fn)
{
    callbacks_.on_hold = fn;
    return *this;
}

Button & Button::on_release(Fn<void()> fn)
{
    callbacks_.on_release = fn;
    return *this;
}

Button & Button::on_pointer_in(Fn<void()> fn)
{
    callbacks_.on_pointer_in = fn;
    return *this;
}

Button & Button::on_pointer_out(Fn<void()> fn)
{
    callbacks_.on_pointer_out = fn;
    return *this;
}

Button & Button::on_focus_in(Fn<void()> fn)
{
    callbacks_.on_focus_in = fn;
    return *this;
}

Button & Button::on_focus_out(Fn<void()> fn)
{
    callbacks_.on_focus_out = fn;
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

        if (events.pointer_out())
        {
            state_.held    = false;
            state_.pointed = false;
            callbacks_.on_pointer_out();
        }

        if (events.pointer_in())
        {
            state_.pointed = true;
            callbacks_.on_pointer_in();
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
    fill(centers, f32x2::zero());
    auto size   = sizes.is_empty() ? f32x2::zero() : sizes[0];
    auto padded = style_.frame(size) + style_.padding.axes();

    if (style_.shape == Shape::Squircle)
    {
        padded = f32x2::splat(padded.max());
    }

    return {.extent = padded};
}

void draw_focus_ring(Canvas canvas, FocusRing const & ring, Button::Shape shape,
                     CRect const & area, f32x4 radii)
{
    auto extent = area.extent + ring.thickness;

    auto shape_info = Shape{
      .world_transform = f32x4x4::identity(),
      .uv_transform    = f32x4x4::identity(),
      .area{area.center, extent},
      .bbox_extent = extent + 2 * ring.feather,
      .radii       = radii,
      .shade       = ShadeType::Stroked,
      .feather     = ring.feather,
      .tint        = ring.tint,
      .sampler     = SamplerIndex::LinearEdgeClampBlackFloat,
      .texture_set = sampled_textures,
      .map         = TextureIndex::White,
      .sdf_sampler = SamplerIndex::LinearRepeatWhiteFloat,
      .sdf_map     = TextureIndex::White
    };

    switch (shape)
    {
        case Button::Shape::RRect:
            canvas->rrect(shape_info);
            break;
        case Button::Shape::Squircle:
            canvas->squircle(shape_info);
            break;
        default:
            break;
    }
}

void Button::render(Scope const &, Canvas canvas, RenderInfo const & info)
{
    ColorGradient tint;

    if (state_.disabled)
    {
        tint = style_.disabled_color;
    }
    else if (state_.held)
    {
        tint = style_.pressed_color;
    }
    else if (state_.pointed)
    {
        tint = style_.pointed_color;
    }
    else
    {
        tint = style_.idle_color;
    }

    style_.shadow.match([&](Shadow const & shadow) {
        auto shape = ash::Shape{
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

    auto shape = ash::Shape{.world_transform = f32x4x4::identity(),
                            .uv_transform    = f32x4x4::identity(),
                            .area            = info.canvas_region,
                            .bbox_extent     = info.canvas_region.extent,
                            .radii           = style_.radii,
                            .shade           = ShadeType::Flood,
                            .feather         = 0,
                            .tint            = tint,
                            .sampler         = SamplerIndex::LinearEdgeClampBlackFloat,
                            .texture_set     = sampled_textures,
                            .map             = TextureIndex::White,
                            .sdf_sampler     = SamplerIndex::LinearRepeatWhiteFloat,
                            .sdf_map         = TextureIndex::White};

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

    if (state_.focused)
    {
        draw_focus_ring(canvas, style_.focus_ring, style_.shape, info.canvas_region,
                        style_.radii);
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
    auto item_size = sizes.is_empty() ? f32x2::zero() : sizes[0];
    auto size      = style_.frame(item_size) + style_.padding.axes();

    fill(centers, f32x2::zero());

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

    // TODO: noise overlay
}

CheckBox::CheckBox(Allocator allocator) :
  allocator_{
    allocator
},
  style_{},
  state_{},
  callbacks_{},
  check_text_{allocator, static_rc(U"check"_s), TextStyle{},
              FontStyle{.font = FontId::Icons, .line_height = 1.0F}}
{
}

CheckBox & CheckBox::frame(Frame const & frame)
{
    style_.frame = frame;
    return *this;
}

CheckBox & CheckBox::text_frame(Frame const & frame)
{
    style_.text_frame = frame;
    return *this;
}

CheckBox & CheckBox::padding(Padding const & padding)
{
    style_.padding = padding;
    return *this;
}

CheckBox & CheckBox::radii(CornerRadii const & radii)
{
    style_.radii = radii;
    return *this;
}

CheckBox & CheckBox::text_color(ColorGradient const & color)
{
    check_text_.state_.text_.text_.runs_style_.styles_[0].foreground = color;
    return *this;
}

CheckBox & CheckBox::color(ColorGradient const & color)
{
    style_.color = color;
    return *this;
}

CheckBox & CheckBox::disabled_color(ColorGradient const & color)
{
    style_.disabled_color = color;
    return *this;
}

CheckBox & CheckBox::pointed_color(ColorGradient const & color)
{
    style_.pointed_color = color;
    return *this;
}

CheckBox & CheckBox::pressed_color(ColorGradient const & color)
{
    style_.pressed_color = color;
    return *this;
}

CheckBox & CheckBox::focus_ring(FocusRing const & focus_ring)
{
    style_.focus_ring = focus_ring;
    return *this;
}

CheckBox & CheckBox::disable(bool disable)
{
    state_.disabled = disable;
    return *this;
}

CheckBox & CheckBox::on_change(Fn<void(bool)> fn)
{
    callbacks_.on_change = fn;
    return *this;
}

CheckBox & CheckBox::on_pointer_in(Fn<void()> fn)
{
    callbacks_.on_pointer_in = fn;
    return *this;
}

CheckBox & CheckBox::on_pointer_out(Fn<void()> fn)
{
    callbacks_.on_pointer_out = fn;
    return *this;
}

CheckBox & CheckBox::check(bool check)
{
    if (state_.checked != check)
    {
        state_.checked = check;
        callbacks_.on_change(static_cast<bool>(state_.checked));
    }
    return *this;
}

ViewState CheckBox::tick(Scope const & scope, Events const & events,
                         Fn<void(View &)> build)
{
    auto & key = scope.key();

    check_text_.hidden(!state_.checked);
    build(check_text_);

    if (state_.disabled)
    {
        state_.focused = false;
        state_.pointed = false;
        state_.held    = false;
    }
    else
    {
        if (events.click() || (events.focus_over() && key.down(KeyCode::Return)))
        {
            state_.checked = !state_.checked;
            callbacks_.on_change(static_cast<bool>(state_.checked));
            state_.held = false;
        }

        if (events.pointer_down())
        {
            state_.held = true;
        }

        if (events.pointer_up())
        {
            state_.held = false;
        }

        if (events.pointer_out())
        {
            state_.held    = false;
            state_.pointed = false;
            callbacks_.on_pointer_out();
        }

        if (events.pointer_in())
        {
            state_.pointed = true;
            callbacks_.on_pointer_in();
        }

        if (events.focus_in())
        {
            state_.focused = true;
        }

        if (events.focus_out())
        {
            state_.focused = false;
        }
    }

    return ViewState{.pointable = !state_.disabled,
                     .clickable = !state_.disabled,
                     .focusable = !state_.disabled};
}

void CheckBox::size(Scope const &, f32x2 allocated, Span<f32x2> sizes)
{
    fill(sizes, style_.frame(allocated));
}

Layout CheckBox::fit(Scope const &, f32x2, Span<f32x2 const> sizes, Span<f32x2> centers)
{
    fill(centers, f32x2::zero());
    auto item_size = sizes[0];
    auto size      = style_.frame(item_size) + style_.padding.axes();
    auto padded    = style_.frame(size) + style_.padding.axes();
    return Layout{.extent = padded};
}

void CheckBox::render(Scope const &, Canvas canvas, RenderInfo const & info)
{
    ColorGradient tint;

    if (state_.disabled)
    {
        tint = style_.disabled_color;
    }
    else if (state_.held)
    {
        tint = style_.pressed_color;
    }
    else if (state_.pointed)
    {
        tint = style_.pointed_color;
    }
    else
    {
        tint = style_.color;
    }

    auto shape =
      ash::Shape{.world_transform = f32x4x4::identity(),
                 .uv_transform    = f32x4x4::identity(),
                 .area            = info.canvas_region,
                 .bbox_extent     = info.canvas_region.extent,
                 .radii           = style_.radii,
                 .shade       = state_.checked ? ShadeType::Flood : ShadeType::Stroked,
                 .feather     = state_.checked ? 0.0F : 1.5F,
                 .tint        = tint,
                 .sampler     = SamplerIndex::LinearEdgeClampBlackFloat,
                 .texture_set = sampled_textures,
                 .map         = TextureIndex::White,
                 .sdf_sampler = SamplerIndex::LinearRepeatWhiteFloat,
                 .sdf_map     = TextureIndex::White};

    canvas->rrect(shape);

    if (state_.focused)
    {
        draw_focus_ring(canvas, style_.focus_ring, Button::Shape::RRect,
                        info.canvas_region, style_.radii);
    }
}

Cursor CheckBox::cursor(Scope const &, f32x2, f32x2)
{
    return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

Switch::Switch(Allocator allocator) :
  allocator_{
    allocator
},
  style_{},
  state_{},
  callbacks_{},
  check_text_{allocator, static_rc(U"check"_s), TextStyle{},
              FontStyle{.font = FontId::Icons, .line_height = 1.0F}}
{
}

Switch & Switch::frame(Frame const & frame)
{
    style_.frame = frame;
    return *this;
}

Switch & Switch::text_frame(Frame const & frame)
{
    style_.text_frame = frame;
    return *this;
}

Switch & Switch::padding(Padding const & padding)
{
    style_.padding = padding;
    return *this;
}

Switch & Switch::knob_on_color(ColorGradient const & color)
{
    style_.knob_on_color = color;
    return *this;
}

Switch & Switch::knob_off_color(ColorGradient const & color)
{
    style_.knob_off_color = color;
    return *this;
}

Switch & Switch::track_on_color(ColorGradient const & color)
{
    style_.track_on_color = color;
    return *this;
}

Switch & Switch::track_off_color(ColorGradient const & color)
{
    style_.track_off_color = color;
    return *this;
}

Switch & Switch::focus_ring(FocusRing const & focus_ring)
{
    style_.focus_ring = focus_ring;
    return *this;
}

Switch & Switch::on_change(Fn<void(bool)> fn)
{
    callbacks_.on_change = fn;
    return *this;
}

Switch & Switch::on(bool on)
{
    state_.on = on;
    return *this;
}

ViewState Switch::tick(Scope const & scope, Events const & events,
                       Fn<void(View &)> build)
{
    return ViewState{};
}

void Switch::size(Scope const & scope, f32x2 allocated, Span<f32x2> sizes)
{
}

Layout Switch::fit(Scope const & scope, f32x2 allocated, Span<f32x2 const> sizes,
                   Span<f32x2> centers)
{
}

void Switch::render(Scope const & scope, Canvas canvas, RenderInfo const & info)
{
}

Cursor Switch::cursor(Scope const & scope, f32x2 extent, f32x2 position)
{
    return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

Slider & Slider::disable(bool disable)
{
    state_.disabled = disable;
    return *this;
}

Slider & Slider::snaps(Option<u32> snaps)
{
    style_.snaps = snaps;
    return *this;
}

Slider & Slider::track_color(ColorGradient const & color)
{
    style_.track_color = color;
    return *this;
}

Slider & Slider::progress_color(ColorGradient const & color)
{
    style_.progress_color = color;
    return *this;
}

Slider & Slider::snaps_color(ColorGradient const & color)
{
    style_.snaps_color = color;
    return *this;
}

Slider & Slider::disabled_color(ColorGradient const & color)
{
    style_.disabled_color = color;
    return *this;
}

Slider & Slider::slider_thickness(f32 thickness)
{
    style_.slider_thickness = thickness;
    return *this;
}

Slider & Slider::track_thickness(f32 thickness)
{
    style_.track_thickness = thickness;
    return *this;
}

Slider & Slider::track_corner_radius(f32 radius)
{
    style_.track_corner_radius = radius;
    return *this;
}

Slider & Slider::knob_radius(f32 radius)
{
    style_.knob_radius = radius;
    return *this;
}

Slider & Slider::knob_focus_ring_radius(f32 radius)
{
    style_.knob_focus_ring_radius = radius;
    return *this;
}

Slider & Slider::knob_color(ColorGradient const & color)
{
    style_.knob_color = color;
    return *this;
}

Slider & Slider::extent(Size extent)
{
    style_.extent = extent;
    return *this;
}

Slider & Slider::horizontal(bool horizontal)
{
    state_.horizontal = horizontal;
    return *this;
}

Slider & Slider::on_changed(Fn<void(f32)> fn)
{
    callbacks_.on_changed = fn;
    return *this;
}

Slider & Slider::on_hold(Fn<void()> fn)
{
    callbacks_.on_hold = fn;
    return *this;
}

Slider & Slider::on_release(Fn<void()> fn)
{
    callbacks_.on_release = fn;
    return *this;
}

Slider & Slider::on_pointer_in(Fn<void()> fn)
{
    callbacks_.on_pointer_in = fn;
    return *this;
}

Slider & Slider::on_pointer_out(Fn<void()> fn)
{
    callbacks_.on_pointer_out = fn;
    return *this;
}

Slider & Slider::on_focus_in(Fn<void()> fn)
{
    callbacks_.on_focus_in = fn;
    return *this;
}

Slider & Slider::on_focus_out(Fn<void()> fn)
{
    callbacks_.on_focus_out = fn;
    return *this;
}

ViewState Slider::tick(Scope const & scope, Events const & events, Fn<void(View &)>)
{
    auto & key = scope.key();

    if (state_.disabled)
    {
        state_.focused = false;
        state_.pointed = false;
        state_.held    = false;
    }
    else
    {
        if (events.pointer_down() || events.drag_update())
        {
            auto info = events.hit_info().unwrap();
            auto x0   = info.viewport_region.begin().x();
            auto x1   = info.viewport_region.end().x();
            auto x    = info.viewport_hit.x();
            auto t    = unlerp(x0, x1, x);
            t         = style_.snaps.match(
              [&](u32 snaps) {
                  snaps = snaps < 2 ? 2 : snaps;
                  return grid_snap(t, 1.0F / (snaps - 1));
              },
              [&] { return t; });
            state_.value = clamp(t, 0.0F, 1.0F);
        }

        if (events.pointer_down() || events.drag_start())
        {
            state_.held = true;
            callbacks_.on_hold();
        }

        if (events.pointer_up() || events.drag_end())
        {
            state_.held    = false;
            state_.pointed = false;
            callbacks_.on_release();
        }

        if (events.focus_over() &&
            (key.down(KeyCode::Left) || key.down(KeyCode::Right)))
        {
            bool left = key.down(KeyCode::Left);
            auto t    = style_.snaps.match(
              [&](u32 snaps) {
                  snaps      = snaps < 2 ? 2 : snaps;
                  auto count = snaps - 1;
                  auto value = state_.value;
                  auto step  = 1.0F / count;

                  if (left)
                  {
                      value -= step;
                  }
                  else
                  {
                      value += step;
                  }

                  return value;
              },
              [&] {
                  auto default_step = 0.1F;
                  auto value        = state_.value;

                  if (left)
                  {
                      value -= default_step;
                  }
                  else
                  {
                      value += default_step;
                  }

                  return value;
              });

            state_.value = clamp(t, 0.0F, 1.0F);
            callbacks_.on_changed(state_.value);
        }

        if (events.pointer_out())
        {
            state_.pointed = false;
            callbacks_.on_pointer_out();
        }

        if (events.pointer_in())
        {
            state_.pointed = true;
            callbacks_.on_pointer_in();
        }

        if (events.focus_in())
        {
            state_.focused = true;
        }

        if (events.focus_out())
        {
            state_.focused = false;
        }
    }

    return ViewState{.pointable = !state_.disabled,
                     .clickable = !state_.disabled,
                     .draggable = !state_.disabled,
                     .focusable = !state_.disabled};
}

Layout Slider::fit(Scope const &, f32x2 allocated, Span<f32x2 const>, Span<f32x2>)
{
    auto extent = f32x2::zero();

    auto main  = state_.horizontal ? 0 : 1;
    auto cross = state_.horizontal ? 1 : 0;

    extent[main]  = style_.extent(allocated[main]);
    extent[cross] = style_.slider_thickness;

    return Layout{.extent = extent};
}

void Slider::render(Scope const &, Canvas canvas, RenderInfo const & info)
{
    auto main  = state_.horizontal ? 0 : 1;
    auto cross = state_.horizontal ? 1 : 0;

    {
        f32x2 track_extent{};
        track_extent[main]  = info.canvas_region.extent[main];
        track_extent[cross] = style_.track_thickness;
        auto track_shape    = Shape{
          .world_transform = f32x4x4::identity(),
          .uv_transform    = f32x4x4::identity(),
          .area            = {info.canvas_region.center, track_extent},
          .bbox_extent     = track_extent,
          .radii           = f32x4::splat(style_.track_corner_radius),
          .shade           = ShadeType::Flood,
          .feather         = 0,
          .tint        = state_.disabled ? style_.disabled_color : style_.track_color,
          .sampler     = SamplerIndex::LinearEdgeClampBlackFloat,
          .texture_set = sampled_textures,
          .map         = TextureIndex::White,
          .sdf_sampler = SamplerIndex::LinearRepeatWhiteFloat,
          .sdf_map     = TextureIndex::White
        };

        canvas->rrect(track_shape);

        f32x2 progress_extent{};
        progress_extent[main]  = info.canvas_region.extent[main] * state_.value;
        progress_extent[cross] = style_.track_thickness;
        auto progress_center_main =
          info.canvas_region.begin()[main] + 0.5F * progress_extent[main];
        auto progress_center =
          f32x2{progress_center_main, info.canvas_region.center[cross]};
        auto progress_shape = Shape{
          .world_transform = f32x4x4::identity(),
          .uv_transform    = f32x4x4::identity(),
          .area            = {progress_center, progress_extent},
          .bbox_extent     = progress_extent,
          .radii           = f32x4::splat(style_.track_corner_radius),
          .shade           = ShadeType::Flood,
          .feather         = 0,
          .tint    = state_.disabled ? style_.disabled_color : style_.progress_color,
          .sampler = SamplerIndex::LinearEdgeClampBlackFloat,
          .texture_set = sampled_textures,
          .map         = TextureIndex::White,
          .sdf_sampler = SamplerIndex::LinearRepeatWhiteFloat,
          .sdf_map     = TextureIndex::White
        };

        canvas->rrect(progress_shape);

        style_.snaps.match([&](u32 snaps) {
            if (snaps > 2)
            {
                auto count = snaps - 1;
                auto unit  = info.canvas_region.extent[main] / count;

                for (u32 i : range(1U, count))
                {
                    f32x2 center;

                    center[main]  = info.canvas_region.begin()[main] + i * unit;
                    center[cross] = info.canvas_region.center[cross];

                    f32x2 extent;
                    extent[main]  = 4.0F;
                    extent[cross] = style_.track_thickness;

                    auto shape = Shape{
                      .world_transform = f32x4x4::identity(),
                      .uv_transform    = f32x4x4::identity(),
                      .area{.center = center, .extent = extent},
                      .bbox_extent = extent,
                      .radii       = {},
                      .shade       = ShadeType::Flood,
                      .feather     = 0,
                      .tint        = style_.snaps_color,
                      .sampler     = SamplerIndex::LinearEdgeClampBlackFloat,
                      .texture_set = sampled_textures,
                      .map         = TextureIndex::White,
                      .sdf_sampler = SamplerIndex::LinearRepeatWhiteFloat,
                      .sdf_map     = TextureIndex::White
                    };

                    canvas->rrect(shape);
                }
            }
        });
    }

    {
        f32x2 center;

        center[main] =
          lerp(info.canvas_region.begin()[main], info.canvas_region.end()[main],
               clamp(state_.value, 0.0F, 1.0F));
        center[cross] = info.canvas_region.center[cross];
        auto extent   = f32x2::splat(style_.knob_radius * 2.0F);
        auto shape    = Shape{
          .world_transform = f32x4x4::identity(),
          .uv_transform    = f32x4x4::identity(),
          .area            = {center, extent},
          .bbox_extent     = extent,
          .radii           = f32x4::splat(style_.knob_radius),
          .shade           = ShadeType::Flood,
          .feather         = 0,
          .tint            = style_.knob_color,
          .sampler         = SamplerIndex::LinearEdgeClampBlackFloat,
          .texture_set     = sampled_textures,
          .map             = TextureIndex::White,
          .sdf_sampler     = SamplerIndex::LinearRepeatWhiteFloat,
          .sdf_map         = TextureIndex::White
        };

        canvas->rrect(shape);

        //  TODO: separate active and inactive focus
        if (state_.focused || state_.pointed || state_.held)
        {
            auto shape = Shape{
              .world_transform = f32x4x4::identity(),
              .uv_transform    = f32x4x4::identity(),
              .area{.center = center,
                    .extent = f32x2::splat(2 * style_.knob_focus_ring_radius)},
              .bbox_extent = f32x2::splat(2 * style_.knob_focus_ring_radius),
              .radii       = f32x4::splat(style_.knob_focus_ring_radius),
              .shade       = ShadeType::Flood,
              .feather     = 0,
              .tint =
                ColorGradient{style_.knob_color.top() * f32x4{1, 1, 1, 0.35F},
                    style_.knob_color.bottom() * f32x4{1, 1, 1, 0.35F},
                    style_.knob_color.angle(), style_.knob_color.center()},
              .sampler     = SamplerIndex::LinearEdgeClampBlackFloat,
              .texture_set = sampled_textures,
              .map         = TextureIndex::White,
              .sdf_sampler = SamplerIndex::LinearRepeatWhiteFloat,
              .sdf_map     = TextureIndex::White
            };
            canvas->rrect(shape);
        }
    }
}

Cursor Slider::cursor(Scope const &, f32x2, f32x2)
{
    return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

ScrollBox::Bar & ScrollBox::Bar::thickness(f32 value)
{
    style_.thickness = value;
    return *this;
}

ScrollBox::Bar & ScrollBox::Bar::corner_radius(f32 value)
{
    style_.corner_radius = value;
    return *this;
}

ScrollBox::Bar & ScrollBox::Bar::track_color(ColorGradient const & value)
{
    style_.track_color = value;
    return *this;
}

ScrollBox::Bar & ScrollBox::Bar::idle_color(ColorGradient const & value)
{
    style_.idle_color = value;
    return *this;
}

ScrollBox::Bar & ScrollBox::Bar::pointed_color(ColorGradient const & value)
{
    style_.pointed_color = value;
    return *this;
}

ScrollBox::Bar & ScrollBox::Bar::pressed_color(ColorGradient const & value)
{
    style_.pressed_color = value;
    return *this;
}

ScrollBox::Bar & ScrollBox::Bar::disabled_color(ColorGradient const & value)
{
    style_.disabled_color = value;
    return *this;
}

ScrollBox::Bar & ScrollBox::Bar::focus_ring(FocusRing const & value)
{
    style_.focus_ring = value;
    return *this;
}

ScrollBox::Bar & ScrollBox::Bar::snaps(Option<u32> value)
{
    style_.snaps = value;
    return *this;
}

ScrollBox::Bar & ScrollBox::Bar::horizontal(bool horizontal)
{
    state_.horizontal = horizontal;
    return *this;
}

ScrollBox::Bar & ScrollBox::Bar::disable(bool disable)
{
    state_.disabled = disable;
    return *this;
}

ScrollBox::Bar & ScrollBox::Bar::hidden(bool hidden)
{
    state_.hidden = hidden;
    return *this;
}

ViewState ScrollBox::Bar::tick(Scope const & scope, Events const & events,
                               Fn<void(View &)>)
{
    auto & key  = scope.key();
    auto   main = state_.horizontal ? 0 : 1;

    if (state_.disabled)
    {
        state_.focused = false;
        state_.pointed = false;
        state_.held    = false;
    }
    else
    {
        // TODO: scroll wheel interaction
        // TODO: drag interaction should be based on drag delta rather than hit position
        if (events.pointer_down() || events.drag_update())
        {
            auto info              = events.hit_info().unwrap();
            auto x0                = info.viewport_region.begin()[main];
            auto x1                = info.viewport_region.end()[main];
            auto track_extent_main = info.viewport_region.extent[main];
            auto hit               = info.viewport_hit[main];
            auto visibility_ratio =
              (*state_.viewport_extent)[main] / (*state_.content_extent)[main];
            auto knob_extent_main  = visibility_ratio * track_extent_main;
            auto knob_offset_begin = x0 + knob_extent_main * 0.5F;
            auto knob_offset_end   = x1 - knob_extent_main * 0.5F;
            auto t                 = unlerp(knob_offset_begin, knob_offset_end, hit);
            t                      = clamp(t, 0.0F, 1.0F);
            t                      = style_.snaps.match(
              [&](u32 snaps) {
                  snaps      = snaps < 2 ? 2 : snaps;
                  auto count = snaps - 1;
                  auto value = (*state_.offset)[main];
                  auto step  = 1.0F / count;

                  value = grid_snap(value, step);
                  return clamp(value, 0.0F, 1.0F);
              },
              [&] { return t; });
            (*state_.offset)[main] = t;
            callbacks_.on_scroll();
        }

        if (events.pointer_down() || events.drag_start())
        {
            state_.held = true;
        }

        if (events.pointer_up() || events.drag_end())
        {
            state_.held    = false;
            state_.pointed = false;
        }

        if (events.focus_over() && state_.horizontal ?
              (key.down(KeyCode::Left) || key.down(KeyCode::Right)) :
              (key.down(KeyCode::Up) || key.down(KeyCode::Down)))
        {
            bool left = key.down(KeyCode::Left);
            auto t    = style_.snaps.match(
              [&](u32 snaps) {
                  snaps      = snaps < 2 ? 2 : snaps;
                  auto count = snaps - 1;
                  auto value = (*state_.offset)[main];
                  auto step  = 1.0F / count;

                  if (left)
                  {
                      value -= step;
                  }
                  else
                  {
                      value += step;
                  }

                  return value;
              },
              [&] {
                  auto default_step = 0.1F;
                  auto value        = (*state_.offset)[main];

                  if (left)
                  {
                      value -= default_step;
                  }
                  else
                  {
                      value += default_step;
                  }

                  return value;
              });

            t = clamp(t, 0.0F, 1.0F);

            (*state_.offset)[main] = t;

            callbacks_.on_scroll();
        }

        if (events.pointer_out())
        {
            state_.pointed = false;
        }

        if (events.pointer_in())
        {
            state_.pointed = true;
        }

        if (events.focus_in())
        {
            state_.focused = true;
        }

        if (events.focus_out())
        {
            state_.focused = false;
        }
    }

    bool hidden = state_.hidden;

    if ((*state_.viewport_extent)[main] >= (*state_.content_extent)[main])
    {
        hidden = true;
    }

    bool interactable = !state_.disabled && !hidden;

    return ViewState{.hidden    = hidden,
                     .pointable = interactable,
                     .clickable = interactable,
                     .draggable = interactable,
                     .focusable = interactable};
}

Layout ScrollBox::Bar::fit(Scope const &, f32x2 allocated, Span<f32x2 const>,
                           Span<f32x2>)
{
    auto main  = state_.horizontal ? 0 : 1;
    auto cross = state_.horizontal ? 1 : 0;

    f32x2 extent;
    extent[main]  = allocated[main];
    extent[cross] = 2 * style_.thickness;
    return Layout{.extent = extent};
}

void ScrollBox::Bar::render(Scope const &, Canvas canvas, RenderInfo const & info)
{
    auto main              = state_.horizontal ? 0 : 1;
    auto cross             = state_.horizontal ? 1 : 0;
    auto x0                = info.viewport_region.begin()[main];
    auto x1                = info.viewport_region.end()[main];
    auto track_extent_main = info.viewport_region.extent[main];
    auto visibility_ratio =
      (*state_.viewport_extent)[main] / (*state_.content_extent)[main];
    auto knob_extent_main  = visibility_ratio * track_extent_main;
    auto knob_offset_begin = x0 + knob_extent_main * 0.5F;
    auto knob_offset_end   = x1 - knob_extent_main * 0.5F;

    auto track_shape = Shape{.world_transform = f32x4x4::identity(),
                             .uv_transform    = f32x4x4::identity(),
                             .area            = info.canvas_region,
                             .bbox_extent     = info.canvas_region.extent,
                             .radii           = f32x4::splat(style_.corner_radius),
                             .shade           = ShadeType::Flood,
                             .feather         = 0,
                             .tint            = style_.track_color,
                             .sampler         = SamplerIndex::LinearEdgeClampBlackFloat,
                             .texture_set     = sampled_textures,
                             .map             = TextureIndex::White,
                             .sdf_sampler     = SamplerIndex::LinearRepeatWhiteFloat,
                             .sdf_map         = TextureIndex::White};

    canvas->rrect(track_shape);

    auto knob_center_main =
      lerp(knob_offset_begin, knob_offset_end, (*state_.offset)[main]);
    f32x2 knob_center;
    knob_center[main]  = knob_center_main;
    knob_center[cross] = info.canvas_region.center[cross];
    f32x2 knob_extent;
    knob_extent[main]  = knob_extent_main;
    knob_extent[cross] = info.canvas_region.extent[cross];

    ColorGradient knob_color;

    if (state_.disabled)
    {
        knob_color = style_.disabled_color;
    }
    else if (state_.held)
    {
        knob_color = style_.pressed_color;
    }
    else if (state_.pointed)
    {
        knob_color = style_.pointed_color;
    }
    else
    {
        knob_color = style_.idle_color;
    }

    auto knob_shape = Shape{
      .world_transform = f32x4x4::identity(),
      .uv_transform    = f32x4x4::identity(),
      .area{.center = knob_center, .extent = knob_extent},
      .bbox_extent = knob_extent,
      .radii       = f32x4::splat(style_.corner_radius),
      .shade       = ShadeType::Flood,
      .feather     = 0,
      .tint        = knob_color,
      .sampler     = SamplerIndex::LinearEdgeClampBlackFloat,
      .texture_set = sampled_textures,
      .map         = TextureIndex::White,
      .sdf_sampler = SamplerIndex::LinearRepeatWhiteFloat,
      .sdf_map     = TextureIndex::White
    };

    canvas->rrect(knob_shape);

    if (state_.focused)
    {
        draw_focus_ring(canvas, style_.focus_ring, Button::Shape::RRect,
                        info.canvas_region, f32x4::splat(style_.corner_radius));
    }
}

Cursor ScrollBox::Bar::cursor(Scope const &, f32x2, f32x2)
{
    return state_.disabled ? Cursor::Default : Cursor::Pointer;
}

ViewState ScrollBox::Viewport::tick(Scope const & scope, Events const & events,
                                    Fn<void(View &)> build)
{
    auto & win   = scope.window();
    auto & mouse = scope.mouse();

    item_.match(build);

    if (state_.disabled)
    {
    }
    else
    {
        if (events.wheel_scroll())
        {
            auto delta =
              -1 * win.scroll_delta() * mouse.wheel_translation().v().to<f32>();
            auto diff = delta / state_.content_extent;
            state_.offset += diff;
            state_.offset = state_.offset.clamp(0.0F, 1.0F);
            callbacks_.on_scroll();
        }

        events.scroll_to().match([&](ScrollFocus const & s) {
            auto pos      = s.center / state_.content_extent;
            state_.offset = pos.clamp(0.0F, 1.0F);
            callbacks_.on_scroll();
        });
    }

    return ViewState{.scrollable = !state_.disabled, .viewport = true};
}

void ScrollBox::Viewport::size(Scope const &, f32x2 allocated, Span<f32x2> sizes)
{
    auto extent = style_.viewport_frame(allocated);
    if (!sizes.is_empty())
    {
        sizes[0] = style_.content_frame(extent);
    }
}

Layout ScrollBox::Viewport::fit(Scope const &, f32x2 allocated, Span<f32x2 const> sizes,
                                Span<f32x2> centers)
{
    auto extent         = style_.viewport_frame(allocated);
    auto content_extent = sizes.is_empty() ? f32x2::zero() : sizes[0];
    content_extent      = content_extent.max(extent);

    if (!centers.is_empty())
    {
        centers[0] = f32x2::zero();
    }

    // TODO: smooth scrolling
    state_.viewport_extent = extent;
    state_.content_extent  = content_extent;

    auto begin = 0.5F * state_.viewport_extent;
    auto end   = state_.content_extent - 0.5F * state_.viewport_extent;

    auto content_center = lerp(begin, end, state_.offset);
    content_center -= 0.5F * state_.content_extent;

    return Layout{.extent         = extent,
                  .content_extent = content_extent,
                  .content_center = content_center,
                  .content_zoom   = f32x2::splat(1.0f)};
}

ScrollBox::ScrollBox()
{
    x_bar_.horizontal(true);
    y_bar_.horizontal(false);
}

ScrollBox & ScrollBox::disable(bool disabled)
{
    viewport_.state_.disabled = disabled;
    x_bar_.state_.disabled    = disabled;
    y_bar_.state_.disabled    = disabled;
    return *this;
}

ScrollBox & ScrollBox::viewport_frame(Frame const & frame)
{
    viewport_.style_.viewport_frame = frame;
    return *this;
}

ScrollBox & ScrollBox::content_frame(Frame const & frame)
{
    viewport_.style_.content_frame = frame;
    return *this;
}

ScrollBox & ScrollBox::on_scroll(Fn<void()> callback)
{
    viewport_.callbacks_.on_scroll = callback;
    return *this;
}

ScrollBox & ScrollBox::item(Option<View &> item)
{
    viewport_.item_ = item;
    return *this;
}

ScrollBox::Bar & ScrollBox::x_bar()
{
    return x_bar_;
}

ScrollBox::Bar & ScrollBox::y_bar()
{
    return y_bar_;
}

i32 ScrollBox::layer(Scope const &, i32 allocated, Span<i32> indices)
{
    indices[0] = allocated + 1;
    indices[1] = allocated + 1;
    indices[2] = allocated;

    return allocated;
}

ViewState ScrollBox::tick(Scope const &, Events const &, Fn<void(View &)> build)
{
    x_bar_.state_.offset = y_bar_.state_.offset = &viewport_.state_.offset;
    x_bar_.state_.viewport_extent               = y_bar_.state_.viewport_extent =
      &viewport_.state_.viewport_extent;
    x_bar_.state_.content_extent = y_bar_.state_.content_extent =
      &viewport_.state_.content_extent;
    x_bar_.callbacks_.on_scroll = y_bar_.callbacks_.on_scroll =
      viewport_.callbacks_.on_scroll;

    build(x_bar_);
    build(y_bar_);
    build(viewport_);

    return ViewState{};
}

void ScrollBox::size(Scope const &, f32x2 allocated, Span<f32x2> sizes)
{
    auto extent = viewport_.style_.viewport_frame(allocated);
    sizes[0]    = extent;
    sizes[1]    = extent;
    sizes[2]    = allocated;
}

Layout ScrollBox::fit(Scope const &, f32x2 allocated, Span<f32x2 const> sizes,
                      Span<f32x2> centers)
{
    auto extent = viewport_.style_.viewport_frame(allocated);
    centers[0]  = f32x2{0, 0.5F * extent.y() - 0.5F * sizes[0].y()};
    centers[1]  = f32x2{0.5F * extent.x() - 0.5F * sizes[1].x(), 0};
    centers[2]  = f32x2::zero();

    return Layout{.extent = extent};
}

}    // namespace ui
}    // namespace ash
