/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"

namespace ash
{

/// @brief flattened hierarchical tree node, all siblings are
/// packed sequentially. This only represents the parent node. Since the tree is
/// rebuilt from scratch every time, the order is preserved in that parents
/// always come before children.
/// @param depth depth of the tree this node belongs to. there's ever only one
/// node at depth 0: the root node.
struct ViewNode
{
  u32     depth    = 0;
  u32     breadth  = 0;
  u32     parent   = U32_MAX;
  Slice32 children = {};
};

enum class ViewHitAttributes : u32
{
  None       = 0x00,
  Pointable  = 0x01,
  Clickable  = 0x02,
  Scrollable = 0x04,
  Draggable  = 0x08,
  Droppable  = 0x10
};

ASH_BIT_ENUM_OPS(ViewHitAttributes)

enum class FocusAction : u32
{
  /// @brief stay on the current focus
  None = 0,

  /// @brief navigate forward on the focus tree
  Forward = 1,

  /// @brief navigate backwards on the focus tree
  Backward = 2
};

struct ScrollRequest
{
  Vec2       position;
  ui::ViewId view_id = ui::ViewId::None;
};

enum class HitType : u32
{
  None       = 0,
  Click      = 1,
  Drag       = 2,
  DragUpdate = 7,
  Drop       = 3,
  Release    = 4,
  Scroll     = 5,
  Point      = 6
};

enum class HitStateBase : u32
{
  None       = 0,
  Pointing   = 1,
  Clicking   = 2,
  Scrolling  = 3,
  DragStart  = 3,
  DragUpdate = 4,
  DragEnd    = 5,
  DragIn     = 7,
  DragOut,
  Drop = 6,
  DragOver,
  MouseIn,
  MouseDown,
  MouseUp,
  MouseMoved,
  MouseOut,
};

struct DragState
{
  enum class State
  {
    None   = 0,
    Start  = 1,
    Update = 2,
    End    = 3
  };

  State      state = State::None;
  ui::ViewId src   = ui::ViewId::None;
  ui::ViewId tgt   = ui::ViewId::None;
};

struct PointState
{
  enum class State
  {
    None   = 0,
    Point  = 1,
    Press  = 2,
    Scroll = 3
  };

  State      state = State::None;
  ui::ViewId tgt   = ui::ViewId::None;
};

using HitState = Enum<None, DragState, PointState>;


// [ ] prepare a list of events to be dispatched and the target views, instead of checking during view build-time
// [ ] are the passed ctx/events in sync with the state?

struct HitEvent
{
  u32     view = 0;
  HitType type = HitType::None;
};

struct ViewSystem
{
  struct Focus
  {
    /// @brief if focusing is active
    bool active = false;

    /// @brief currently focused view
    ui::ViewId view = ui::ViewId::None;

    /// @brief focus tree index of the view
    u32 focus_idx = U32_MAX;

    /// @brief does the view accepts text input
    bool input = false;

    TextInputInfo input_info = {};

    CRect region = {};
  };

  struct State
  {
    /// @brief mouse pointed view
    Option<ui::ViewId> pointed = none;

    /// @brief drag data soure view
    Option<ui::ViewId> drag_src = none;

    /// @brief current cursor
    Cursor cursor = Cursor::Default;

    /// @brief focus state
    Option<Focus> focus = none;

    /// @brief grab focus state
    Option<Focus> grab_focus = none;

    /// @brief if mouse went down on this frame
    bool mouse_down = false;

    /// @brief if mouse went up on this frame
    bool mouse_up = false;

    /// @brief if mouse moved on this frame
    bool mouse_moved = false;

    /// @brief if mouse scrolled on this frame
    bool mouse_scrolled = false;

    /// @brief if mouse dragging started on this frame
    bool drag_start = false;

    /// @brief if mouse dragging ended on this frame
    bool drag_end = false;

    /// @brief if mouse was dragging on this frame
    bool dragging = false;

    /// @brief if drag data was dropped on this frame
    bool dropped = false;

    /// @brief if key went down on the frame
    bool key_down = false;

    /// @brief if key went up on the frame
    bool key_up = false;

    /// @brief if text input was received on this frame
    bool text_input = false;
  };

  /// @brief current frame id
  u64 frame = 0;

  /// @brief next view id
  u64 next_id = 0;

  /// @brief previous frame state
  State f0 = {};

  /// @brief current frame state
  State f1 = {};

  /// @brief current frame input state
  InputState s1;

  Vec<ref<ui::View>> views;
  Vec<ViewNode>      nodes;

  Vec<i32>    tab_indices;
  Vec<u32>    viewports;
  BitVec<u64> is_hidden;
  BitVec<u64> is_pointable;
  BitVec<u64> is_clickable;
  BitVec<u64> is_scrollable;
  BitVec<u64> is_draggable;
  BitVec<u64> is_droppable;
  BitVec<u64> is_focusable;

  BitVec<u64>             is_input;
  Vec<TextInputType>      input_type;
  BitVec<u64>             is_multiline_input;
  BitVec<u64>             is_esc_input;
  BitVec<u64>             is_tab_input;
  Vec<TextCapitalization> input_cap;
  BitVec<u64>             input_autocorrect;

  BitVec<u64>  is_viewport;
  Vec<Vec2>    centers;
  Vec<Vec2>    extents;
  Vec<Vec2>    viewport_extents;
  Vec<Affine3> viewport_transforms;
  BitVec<u64>  is_fixed_positioned;
  Vec<Vec2>    fixed_positions;
  Vec<i32>     z_indices;
  Vec<i32>     layers;

  Vec<Affine3>       transforms;
  Vec<CRect>         clips;
  Vec<u32>           z_ordering;
  Vec<u32>           focus_ordering;
  Vec<ScrollRequest> scrolls;
  bool               closing_deferred;

  explicit ViewSystem(AllocatorRef allocator) :
    s1{allocator},
    views{allocator},
    nodes{allocator},
    tab_indices{allocator},
    viewports{allocator},
    is_hidden{allocator},
    is_pointable{allocator},
    is_clickable{allocator},
    is_scrollable{allocator},
    is_draggable{allocator},
    is_droppable{allocator},
    is_focusable{allocator},
    is_input{allocator},
    input_type{allocator},
    is_multiline_input{allocator},
    is_esc_input{allocator},
    is_tab_input{allocator},
    input_cap{allocator},
    input_autocorrect{allocator},
    is_viewport{allocator},
    centers{allocator},
    extents{allocator},
    viewport_extents{allocator},
    viewport_transforms{allocator},
    is_fixed_positioned{allocator},
    fixed_positions{allocator},
    z_indices{allocator},
    layers{allocator},
    transforms{allocator},
    clips{allocator},
    z_ordering{allocator},
    focus_ordering{allocator},
    scrolls{allocator},
    closing_deferred{false}
  {
  }

  ViewSystem(ViewSystem const &)             = delete;
  ViewSystem(ViewSystem &&)                  = default;
  ViewSystem & operator=(ViewSystem const &) = delete;
  ViewSystem & operator=(ViewSystem &&)      = default;
  ~ViewSystem()                              = default;

  void clear();

  ui::ViewEvents process_events(ui::View & view);

  void push_view(ui::View & view, u32 depth, u32 breadth, u32 parent);

  void build_children(ui::ViewContext const & ctx, ui::View & view, u32 idx,
                      u32 depth, i32 & tab_index, u32 viewport);

  void build(ui::ViewContext const & ctx, ui::View & root);

  void focus_order();

  void layout(Vec2 viewport_extent);

  void stack();

  void visibility();

  void render(Canvas & canvas);

  void scroll_to(u32 view);

  Option<u32> hit_views(Vec2 mouse_position, ViewHitAttributes match) const;

  Option<u32> navigate_focus(u32 from, bool forward) const;

  void handle_hit();

  void dispatch_hit(ui::ViewContext const & ctx, HitEvent event);

  void events(ui::ViewContext const & ctx);

  Cursor cursor() const;

  Option<TextInputInfo> text_input() const;

  bool tick(InputState const & input, ui::View & root, Canvas & canvas,
            Fn<void(ui::ViewContext const &)> loop);
};

}    // namespace ash
