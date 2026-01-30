/// SPDX-License-Identifier: MIT
#include "ashura/engine/views/model/text.h"
#include "ashura/engine/window_system.h"
#include "ashura/std/text.h"

namespace ash
{
namespace ui
{
namespace txt
{

void Cfg::default_clipboard_setter(Str32 str)
{
    auto * clipboard = sys.win->get_clipboard();
    auto   scratch   = IFallbackAllocator{get_thread_arena(), default_allocator};
    auto   str8      = Vec<c8>::make(str.size() * 4, scratch).unwrap();
    utf8_encode(str, str8).unwrap();
    clipboard->set(MIME_TEXT_UTF8, str8.view().as_u8()).unwrap();
}

StrVec32 Cfg::default_clipboard_getter(Allocator allocator)
{
    auto * clipboard = sys.win->get_clipboard();
    auto   scratch   = IFallbackAllocator{get_thread_arena(), default_allocator};
    auto   str8      = Vec<u8>{scratch};
    clipboard->get(MIME_TEXT_UTF8, str8).unwrap();
    auto str32 = Vec<c32>::make(str8.size(), allocator).unwrap();
    utf8_decode(str8.view().as_c8(), str32).unwrap();
    return str32;
}

/// @brief Handles interaction and state updates for text views
/// It will issue actions that the text view can then execute to update its
/// internal state
static void apply_action(Cfg const & cfg, CursorAction a, State & s)
{
    auto scratch = IFallbackAllocator{get_thread_arena(), default_allocator};

    switch (a.type)
    {
        case CursorActionType::None:
        {
        }
        break;
        case CursorActionType::Unselect:
        {
            s.text_.unselect();
        }
        break;
        case CursorActionType::Escape:
        {
            s.text_.remove_cursor();
        }
        break;
        case CursorActionType::SelectLeft:
        {
            s.text_.select_left();
        }
        break;
        case CursorActionType::SelectRight:
        {
            s.text_.select_right();
        }
        break;
        case CursorActionType::SelectUp:
        {
            s.text_.select_up();
        }
        break;
        case CursorActionType::SelectDown:
        {
            s.text_.select_down();
        }
        break;
        case CursorActionType::SelectToLineStart:
        {
            s.text_.select_to_line_start();
        }
        break;
        case CursorActionType::SelectToLineEnd:
        {
            s.text_.select_to_line_end();
        }
        break;
        case CursorActionType::SelectPageUp:
        {
            s.text_.select_page_up(cfg.lines_per_page);
        }
        break;
        case CursorActionType::SelectPageDown:
        {
            s.text_.select_page_down(cfg.lines_per_page);
        }
        break;
        case CursorActionType::SelectAll:
        {
            s.text_.select_all();
        }
        break;
        case CursorActionType::SelectWord:
        {
            s.text_.select_word(DEFAULT_WORD_SYMBOLS);
        }
        break;
        case CursorActionType::SelectLine:
        {
            s.text_.select_line();
        }
        break;
        case CursorActionType::Left:
        {
            s.text_.left();
        }
        break;
        case CursorActionType::Right:
        {
            s.text_.right();
        }
        break;
        case CursorActionType::LineStart:
        {
            s.text_.line_start();
        }
        break;
        case CursorActionType::LineEnd:
        {
            s.text_.line_end();
        }
        break;
        case CursorActionType::Up:
        {
            s.text_.up();
        }
        break;
        case CursorActionType::Down:
        {
            s.text_.down();
        }
        break;
        case CursorActionType::PageUp:
        {
            s.text_.page_up(cfg.lines_per_page);
        }
        break;
        case CursorActionType::PageDown:
        {
            s.text_.page_down(cfg.lines_per_page);
        }
        break;
        case CursorActionType::Insert:
        {
            s.text_.insert(a.text.get());
        }
        break;
        case CursorActionType::Cut:
        {
            auto cut = s.text_.copy_cut(scratch);
            cfg.clipboard_setter(cut);
        }
        break;
        case CursorActionType::Copy:
        {
            auto copied = s.text_.copy(scratch);
            cfg.clipboard_setter(copied);
        }
        break;
        case CursorActionType::Paste:
        {
            auto text = cfg.clipboard_getter(scratch);
            s.text_.insert(text);
        }
        break;
        case CursorActionType::NewLine:
        {
            s.text_.new_line();
        }
        break;
        case CursorActionType::Tab:
        {
            s.text_.tab();
        }
        break;
        case CursorActionType::Backspace:
        {
            s.text_.backspace();
        }
        break;
        case CursorActionType::Delete:
        {
            s.text_.del();
        }
        break;
        case CursorActionType::Home:
        {
            s.text_.line_start();
        }
        break;
        case CursorActionType::End:
        {
            s.text_.line_end();
        }
        break;
        case CursorActionType::Hit:
        {
            s.text_.hit(a.center, a.transform, a.transformed_position);
        }
        break;
        case CursorActionType::HitSelectSpan:
        {
            s.text_.hit_select(a.center, a.transform, a.transformed_position);
        }
        break;
    }
}

static void apply_action(Cfg const & cfg, CoreAction action, State & s)
{
    switch (action.type)
    {
        case CoreActionType::None:
        {
        }
        break;

        case CoreActionType::ReplaceText:
        {
            s.text_.text(action.text.unwrap());
        }
        break;

        case CoreActionType::Undo:
        {
            warn("Undo action is not implemented yet"_str);
        }
        break;
        case CoreActionType::Redo:
        {
            warn("Redo action is not implemented yet"_str);
        }
        break;
        case CoreActionType::Submit:
        {
            auto str = s.text_.str();
            cfg.on_submit(str);
        }
        break;
    }
}

static void exec_action(Cfg const & cfg, Action action, State & s)
{
    action.match(
      [&](CursorAction & action) { apply_action(cfg, std::move(action), s); },
      [&](CoreAction & action) { apply_action(cfg, std::move(action), s); },
      [](None) {});
}

static Action input_to_action(ui::Scope const & scope, Cfg const & cfg,
                              Events const & events, Allocator allocator)
{
    auto & k = scope.key();
    auto & m = scope.mouse();

    auto shift = k.held(KeyModifiers::LeftShift) || k.held(KeyModifiers::RightShift);
    auto ctrl  = k.held(KeyModifiers::LeftCtrl) || k.held(KeyModifiers::RightCtrl);

    if (events.focus_out())
    {
        return CursorAction{.type = CursorActionType::Escape};
    }

    if (cfg.highlightable)
    {
        if (events.drag_start())
        {
            return CursorAction{
              .type   = CursorActionType::Hit,
              .center = events.hit_info_.v().canvas_region.center,
              .transform =
                transform2d_to_3d(events.hit_info_.v().canvas_transform).to_mat(),
              .transformed_position = events.hit_info_.v().canvas_hit};
        }
        else if (events.drag_update())
        {
            if (m.clicks(MouseButton::Primary) == 2)
            {
                return CursorAction{.type = CursorActionType::SelectWord};
            }
            else if (m.clicks(MouseButton::Primary) == 3)
            {
                return CursorAction{.type = CursorActionType::SelectLine};
            }
            else if (m.clicks(MouseButton::Primary) == 4)
            {
                return CursorAction{.type = CursorActionType::SelectAll};
            }
            else
            {
                return CursorAction{
                  .type   = CursorActionType::HitSelectSpan,
                  .center = events.hit_info_.v().canvas_region.center,
                  .transform =
                    transform2d_to_3d(events.hit_info_.v().canvas_transform).to_mat(),
                  .transformed_position = events.hit_info_.v().canvas_hit};
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
        return CursorAction{.type = CursorActionType::Insert, .text = std::move(text)};
    }

    if (events.key_down())
    {
        if (cfg.highlightable && cfg.enable_cursor)
        {
            if (shift && k.down(KeyCode::Left))
            {
                return CursorAction{.type = CursorActionType::SelectLeft};
            }
            else if (shift && k.down(KeyCode::Right))
            {
                return CursorAction{.type = CursorActionType::SelectRight};
            }
            else if (shift && k.down(KeyCode::Up))
            {
                return CursorAction{.type = CursorActionType::SelectUp};
            }
            else if (shift && k.down(KeyCode::Down))
            {
                return CursorAction{.type = CursorActionType::SelectDown};
            }
            else if (shift && k.down(KeyCode::Home))
            {
                return CursorAction{.type = CursorActionType::SelectToLineStart};
            }
            else if (shift && k.down(KeyCode::End))
            {
                return CursorAction{.type = CursorActionType::SelectToLineEnd};
            }
            else if (shift && k.down(KeyCode::PageUp))
            {
                return CursorAction{.type = CursorActionType::SelectPageUp};
            }
            else if (shift && k.down(KeyCode::PageDown))
            {
                return CursorAction{.type = CursorActionType::SelectPageDown};
            }
            else if (ctrl && k.down(KeyCode::A))
            {
                return CursorAction{.type = CursorActionType::SelectAll};
            }
            else if (k.down(KeyCode::Escape))
            {
                return CursorAction{.type = CursorActionType::Escape};
            }
        }

        if (cfg.editable)
        {
            if (ctrl && k.down(KeyCode::X))
            {
                return CursorAction{.type = CursorActionType::Cut};
            }
            else if (cfg.copyable && ctrl && k.down(KeyCode::C))
            {
                return CursorAction{.type = CursorActionType::Copy};
            }
            else if (ctrl && k.down(KeyCode::V))
            {
                return CursorAction{.type = CursorActionType::Paste};
            }
            else if (ctrl && k.down(KeyCode::Z))
            {
                return CoreAction{.type = CoreActionType::Undo};
            }
            else if (ctrl && k.down(KeyCode::Y))
            {
                return CoreAction{.type = CoreActionType::Redo};
            }
            else if (cfg.enable_multiline_input && !cfg.enter_submits &&
                     k.down(KeyCode::Return))
            {
                return CursorAction{.type = CursorActionType::NewLine};
            }
            else if (cfg.accept_tab_input && k.down(KeyCode::Tab))
            {
                return CursorAction{.type = CursorActionType::Tab};
            }
            else if (k.down(KeyCode::Backspace))
            {
                return CursorAction{.type = CursorActionType::Backspace};
            }
            else if (k.down(KeyCode::Delete))
            {
                return CursorAction{.type = CursorActionType::Delete};
            }
            else if (k.down(KeyCode::Left))
            {
                return CursorAction{.type = CursorActionType::Left};
            }
            else if (k.down(KeyCode::Right))
            {
                return CursorAction{.type = CursorActionType::Right};
            }
            else if (k.down(KeyCode::Home))
            {
                return CursorAction{.type = CursorActionType::LineStart};
            }
            else if (k.down(KeyCode::End))
            {
                return CursorAction{.type = CursorActionType::LineEnd};
            }
            else if (k.down(KeyCode::Up))
            {
                return CursorAction{.type = CursorActionType::Up};
            }
            else if (k.down(KeyCode::Down))
            {
                return CursorAction{.type = CursorActionType::Down};
            }
            else if (k.down(KeyCode::PageUp))
            {
                return CursorAction{.type = CursorActionType::PageUp};
            }
            else if (k.down(KeyCode::PageDown))
            {
                return CursorAction{.type = CursorActionType::PageDown};
            }
        }

        if (cfg.enter_submits && k.down(KeyCode::Return))
        {
            return CoreAction{.type = CoreActionType::Submit};
        }
    }

    return none;
}

Vec<Action> Cfg::default_input_to_actions_map(ui::Scope const & scope, Cfg const & cfg,
                                              Events const & events,
                                              Allocator      allocator)

{
    auto actions = Vec<Action>{allocator};
    actions.push(input_to_action(scope, cfg, events, allocator)).unwrap();
    return actions;
}

void Cfg::default_renderer(ui::Scope const &, Cfg const &, State const &,
                           TextRenderInfo const &    info,
                           TextPlacementInfo const & placement, Canvas canvas)
{
    canvas->text(info, placement);
}

void State::tick(ui::Scope const & scope, Cfg const & cfg, Events const & events)
{
    auto actions = cfg.input_to_actions_map(scope, cfg, events, allocator_);

    for (auto & action : actions)
    {
        exec_action(cfg, std::move(action), *this);
    }
}

ui::Layout State::fit(ui::Scope const &, f32x2 allocated, Span<f32x2 const>,
                      Span<f32x2>)
{
    auto scratch = IFallbackAllocator{get_thread_arena(), allocator_};
    text_.width(allocated.x(), allocated.x());
    text_.perform_layout(scratch);

    return ui::Layout{.extent = text_.layout().extent};
}

// TODO: info?
void State::render(ui::Scope const & scope, Cfg const & cfg, Canvas canvas,
                   ui::RenderInfo const & info)
{
    auto scratch    = IFallbackAllocator{get_thread_arena(), allocator_};
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

    auto [text_info, placement] = text_.text_.place(
      info.viewport_region.center, transform2d_to_3d(info.canvas_transform).to_mat(),
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
    cfg.renderer(scope, cfg, *this, text_info, placement_info, canvas);
}

}    // namespace txt
}    // namespace ui
}    // namespace ash
