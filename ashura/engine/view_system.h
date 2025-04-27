/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"

namespace ash
{
namespace ui
{

struct RootView : View
{
  static constexpr u32 NODE     = 0;
  static constexpr u32 PARENT   = 0;
  static constexpr u32 VIEWPORT = 0;

  View * next = nullptr;

  constexpr virtual State tick(Ctx const &, Event const &,
                               Fn<void(View &)> build)
  {
    build(*next);
    return State{.viewport = true};
  }

  constexpr virtual void size(Vec2 allocated, Span<Vec2> sizes)
  {
    fill(sizes, allocated);
  }

  constexpr virtual Layout fit(Vec2       allocated, Span<Vec2 const>,
                               Span<Vec2> centers)
  {
    fill(centers, Vec2{0, 0});
    return Layout{.extent          = allocated,
                  .viewport_extent = allocated,
                  .viewport_center = Vec2::splat(0),
                  .viewport_zoom   = 1,
                  .fixed_center    = Vec2::splat(0)};
  }

  constexpr virtual i32 stack(i32)
  {
    return 0;
  }

  constexpr virtual i32 z_index(i32, Span<i32> indices)
  {
    fill(indices, 0);
    return 0;
  }

  constexpr virtual void render(Canvas &, CRect const &, f32, CRect const &)
  {
  }

  constexpr virtual bool hit(Vec2, Vec2)
  {
    return false;
  }

  constexpr virtual Cursor cursor(Vec2, Vec2)
  {
    return Cursor::Default;
  }
};

enum class FocusAction : u32
{
  /// @brief stay on the current focus
  None = 0,

  /// @brief navigate forward on the focus tree
  Forward = 1,

  /// @brief navigate backwards on the focus tree
  Backward = 2
};

struct DragState
{
  enum Seq
  {
    Start  = 1,
    Update = 2
  };

  Seq         seq = Start;
u32     src = 0;
 Option<u32>     tgt = none;
  MouseButton btn = MouseButton::Primary;
};

struct PointState
{
  Option<u32> tgt = none;
};

using HitState = Enum<None, DragState, PointState>;

struct FocusState
{
  /// @brief if focusing is active
  bool active = false;

  /// @brief focus tree index
  u32 focus_idx = 0;
};

// [ ] are the passed ctx/events in sync with the state?

/// @brief A compact View Hierarchy
struct System
{
  struct NodeVertex
  {
    u32 enter = 0;
    u32 exit  = 0;

    constexpr bool is_ancestor(NodeVertex const & b) const
    {
      return enter <= b.enter && exit >= b.exit;
    }
  };

  /// @brief flattened hierarchical tree node, all siblings are
  /// packed sequentially. This only represents the parent node. Since the tree is
  /// rebuilt from scratch every time, the order is preserved in that parents
  /// always come before children.
  /// @param depth depth of the tree this node belongs to. there's ever only one
  /// node at depth 0: the root node.
  struct Nodes
  {
    Vec<u32>     depth;
    Vec<u32>     parent;
    Vec<Slice32> children;

    Nodes(AllocatorRef allocator) :
      depth{allocator},
      parent{allocator},
      children{allocator}
    {
    }
  };

  /// View Attributes
  struct Attrs
  {
    Vec<i32>                   tab_idx;
    Vec<u32>                   viewports;
    BitVec<u64>                hidden;
    BitVec<u64>                pointable;
    BitVec<u64>                clickable;
    BitVec<u64>                scrollable;
    BitVec<u64>                draggable;
    BitVec<u64>                droppable;
    BitVec<u64>                focusable;
    Vec<Option<TextInputInfo>> input;
    BitVec<u64>                is_viewport;

    Attrs(AllocatorRef allocator) :
      tab_idx{allocator},
      viewports{allocator},
      hidden{allocator},
      pointable{allocator},
      clickable{allocator},
      scrollable{allocator},
      draggable{allocator},
      droppable{allocator},
      focusable{allocator},
      input{allocator},
      is_viewport{allocator}
    {
    }
  };

  struct PointerEvent
  {
    enum Type
    {
      None        = 0,
      PointerIn   = 1,
      PointerOut  = 2,
      PointerDown = 3,
      PointerUp   = 4,
      PointerOver = 5,
      Scroll      = 6,
      DragStart   = 7,
      Dragging    = 8,
      DragEnd     = 9,
      DragIn      = 10,
      DragOut     = 11,
      DragOver    = 12,
      Drop        = 13
    };

    Type type = Type::None;
    u32  dst  = 0;
  };

  struct Events
  {
    // mouse position?
    Option<HitEvent> hit     = none;
    Option<CRect>    focused = none;
  };

  RootView root_view;

  HitState hit;

  FocusState focus;

  /// @brief current frame id
  u64 frame = 0;

  /// @brief next view id
  u64 next_id = 0;

  /// @brief current frame input state
  InputState input;

  /// Tree Nodes

  Vec<ref<View>> views;
  Nodes          nodes;

  Attrs att;

  /// Computed data

  Vec<Vec2>   extents;
  Vec<Vec2>   centers;
  Vec<Vec2>   viewport_extents;
  Vec<Vec2>   viewport_centers;
  Vec<f32>    viewport_zooms;
  BitVec<u64> fixed;
  Vec<Vec2>   fixed_centers;
  Vec<i32>    z_idx;
  Vec<i32>    layers;

  Vec<Affine3> canvas_tx;
  Vec<Affine3> canvas_inv_tx;
  Vec<Vec2>    canvas_centers;
  Vec<Vec2>    canvas_extents;
  Vec<CRect>   clips;
  Vec<u32>     z_ord;
  Vec<u32>     focus_ord;
  Vec<u32>     focus_idx;

  /// Frame Computed Info
  bool        closing_deferred;
  Option<u32> focus_grab;
  Cursor      cursor;

  struct Active
  {
    u32    focused        = 0;
    ViewId focused_id     = ViewId::None;
    u32    pointed        = 0;
    ViewId pointed_id     = ViewId::None;
    u32    pointer_out    = 0;
    ViewId pointer_out_id = ViewId::None;
    u32    pointer_in     = 0;
    ViewId pointer_in_id  = ViewId::None;
  } actives;

  // [ ] clear at frame start
  Vec<PointerEvent> pointer_events;

  explicit System(AllocatorRef allocator) :
    root_view{},
    hit{none},
    focus{},
    frame{0},
    next_id{0},
    input{allocator},
    views{allocator},
    nodes{allocator},
    att{allocator},
    extents{allocator},
    centers{allocator},
    viewport_extents{allocator},
    viewport_centers{allocator},
    viewport_zooms{allocator},
    fixed{allocator},
    fixed_centers{allocator},
    z_idx{allocator},
    layers{allocator},
    canvas_tx{allocator},
    canvas_inv_tx{allocator},
    z_ord{allocator},
    focus_ord{allocator},
    closing_deferred{false},
    focus_grab{none},
    cursor{Cursor::Default}
  {
  }

  System(System const &)             = delete;
  System(System &&)                  = default;
  System & operator=(System const &) = delete;
  System & operator=(System &&)      = default;
  ~System()                          = default;

  void clear_frame();

  void push_view(View & view, u32 depth, u32 breadth, u32 parent);

  Event events_for(View & view);

  void build_children(Ctx const & ctx, View & view, u32 idx, u32 depth,
                      u32 viewport, i32 & tab_index);

  void build(Ctx const & ctx, RootView & root);

  void prepare_for(u32 n);

  void focus_order();

  void layout(Vec2 viewport_extent);

  void stack();

  void visibility();

  void render(Canvas & canvas);

  void focus_scroll(u32 view);

  Option<u32> hit_test(Vec2 position) const;

  template <typename Match>
  Option<u32> bubble(u32 from, Match && match) const
  {
    auto current = from;

    while (true)
    {
      if (layers[current] != layers[from])
      {
        return none;
      }

      if (match(current))
      {
        return current;
      }

      if (att.is_viewport[current])
      {
        return none;
      }

      if (current == RootView::NODE)
      {
        return none;
      }

      current = nodes.parent[current];
    }
  }

  template <typename Match>
  Option<u32> bubble_hit(Vec2 position, Match && match) const
  {
    return hit_test(position).match([&](auto i) { return bubble(i, match); });
  }

  u32 navigate_focus(u32 from, bool forward) const;

  HitState none_seq(Ctx const & ctx);

  HitState drag_start_seq(Ctx const & ctx, MouseButton btn, u32 src);

  HitState drag_update_seq(Ctx const & ctx, MouseButton btn, u32 src,
                           Option<u32> tgt);

  HitState point_seq(Ctx const & ctx, Option<u32> tgt);

  HitState hit_seq(Ctx const & ctx);

  void event_dispatch(InputState const & input);

  Option<TextInputInfo> text_input() const;

  // [ ] make positions relative to center of the screen
  bool tick(InputState const & input, View & root, Canvas & canvas,
            Fn<void(Ctx const &)> loop);
}

}    // namespace ui
}    // namespace ash
