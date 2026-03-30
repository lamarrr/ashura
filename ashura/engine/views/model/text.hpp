/// SPDX-License-Identifier: MIT
#include "ashura/engine/canvas.hpp"
#include "ashura/engine/render_text.hpp"
#include "ashura/engine/text.hpp"
#include "ashura/engine/view.hpp"

namespace ash
{
namespace ui
{
namespace txt
{

enum class CursorActionType : u8
{
    None              = 0,
    Unselect          = 1,
    Escape            = 2,
    SelectLeft        = 3,
    SelectRight       = 4,
    SelectUp          = 5,
    SelectDown        = 6,
    SelectToLineStart = 7,
    SelectToLineEnd   = 8,
    SelectPageUp      = 9,
    SelectPageDown    = 10,
    SelectAll         = 11,
    SelectWord        = 12,
    SelectLine        = 13,
    Left              = 14,
    Right             = 15,
    LineStart         = 16,
    LineEnd           = 17,
    Up                = 18,
    Down              = 19,
    PageUp            = 20,
    PageDown          = 21,
    Insert            = 22,
    Cut               = 23,
    Copy              = 24,
    Paste             = 25,
    NewLine           = 26,
    Tab               = 27,
    Backspace         = 28,
    Delete            = 29,
    Home              = 30,
    End               = 31,
    Hit               = 32,
    HitSelectSpan     = 33
};

struct CursorAction
{
    CursorActionType type = CursorActionType::None;

    Rc<Str32> text = static_rc(U""_s);

    f32x2 center = f32x2::zero();

    f32x4x4 transform = f32x4x4::identity();

    f32x2 transformed_position = f32x2::zero();
};

enum class CoreActionType : u8
{
    None        = 0,
    ReplaceText = 1,
    Undo        = 2,
    Redo        = 3,
    Submit      = 4
};

struct CoreAction
{
    CoreActionType type = CoreActionType::None;

    Option<RenderText> text = none;
};

using Action = Enum<CursorAction, CoreAction, None>;

struct Cfg;
struct State;

using InputToActionsMap =
  Fn<Vec<Action>(ui::Scope const &, Cfg const &, Events const &, Allocator)>;

using Renderer = Fn<void(ui::Scope const &, Cfg const &, State const &,
                         TextRenderInfo const &, TextPlacementInfo const &, Canvas)>;

struct Cfg
{
    static void default_clipboard_setter(Str32 str);

    static StrVec32 default_clipboard_getter(Allocator allocator);

    static Vec<Action> default_input_to_actions_map(ui::Scope const & scope,
                                                    Cfg const &       cfg,
                                                    Events const &    events,
                                                    Allocator         allocator);

    static void default_renderer(ui::Scope const & scope, Cfg const & cfg,
                                 State const & s, TextRenderInfo const & info,
                                 TextPlacementInfo const & placement, Canvas canvas);

    bool copyable : 1 = false;

    bool highlightable : 1 = false;

    bool enable_cursor : 1 = false;

    bool editable : 1 = false;

    bool enable_undo_redo : 1 = false;

    bool enable_multiline_input : 1 = false;

    bool accept_tab_input : 1 = true;

    bool enter_submits : 1 = false;

    u16 lines_per_page = 20;

    Rc<Fn<void(Str32)>> clipboard_setter{default_clipboard_setter, rc_noop};

    Rc<Fn<StrVec32(Allocator)>> clipboard_getter{default_clipboard_getter, rc_noop};

    Rc<Fn<void(Str32)>> on_submit{noop, rc_noop};

    Rc<InputToActionsMap> input_to_actions_map{default_input_to_actions_map, rc_noop};

    Rc<Renderer> renderer{default_renderer, rc_noop};
};

/// @brief Handles interaction and state updates for text views
/// It will issue actions that the text view can then execute to update its
/// internal state
struct State
{
    Allocator allocator_;

    TextModel text_;

    Vec<Slice> highlights_;

    Vec<TextHighlightStyle> highlight_styles_;

    State(Allocator allocator) :
      allocator_{allocator},
      text_{allocator},
      highlights_{allocator},
      highlight_styles_{allocator}
    {
    }

    void tick(ui::Scope const & scope, Cfg const & cfg, Events const & events);

    ui::Layout fit(ui::Scope const & scope, f32x2 allocated, Span<f32x2 const> sizes,
                   Span<f32x2> center);

    void render(ui::Scope const & scope, Cfg const & cfg, Canvas canvas,
                ui::RenderInfo const & info);
};

}    // namespace txt
}    // namespace ui
}    // namespace ash
