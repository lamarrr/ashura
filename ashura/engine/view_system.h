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
  enum class Seq
  {
    Start  = 1,
    Update = 2
  };

  Seq         seq = Seq::Start;
  ViewId      src = ViewId::None;
  ViewId      tgt = ViewId::None;
  MouseButton btn = MouseButton::Primary;
};

struct PointState
{
  ViewId tgt = ViewId::None;
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

/// @brief flattened hierarchical tree node, all siblings are
/// packed sequentially. This only represents the parent node. Since the tree is
/// rebuilt from scratch every time, the order is preserved in that parents
/// always come before children.
/// @param depth depth of the tree this node belongs to. there's ever only one
/// node at depth 0: the root node.
struct Node
{
  u32     depth    = 0;
  u32     breadth  = 0;
  u32     parent   = RootView::NODE;
  Slice32 children = {};
};

/// @warning: internal-only class
struct System
{
  /// System state

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
  Vec<Node>      nodes;

  /// Tree/View data

  Vec<i32>                   tab_indices;
  Vec<u32>                   viewports;
  BitVec<u64>                is_hidden;
  BitVec<u64>                is_pointable;
  BitVec<u64>                is_clickable;
  BitVec<u64>                is_scrollable;
  BitVec<u64>                is_draggable;
  BitVec<u64>                is_droppable;
  BitVec<u64>                is_focusable;
  Vec<Option<TextInputInfo>> input_infos;
  BitVec<u64>                is_viewport;

  /// Computed data

  Vec<Vec2>   extents;
  Vec<Vec2>   centers;
  Vec<Vec2>   viewport_extents;
  Vec<Vec2>   viewport_centers;
  Vec<f32>    viewport_zooms;
  BitVec<u64> is_fixed_centered;
  Vec<Vec2>   fixed_centers;
  Vec<i32>    z_indices;
  Vec<i32>    layers;

  Vec<Affine3> canvas_transforms;
  Vec<Affine3> canvas_inverse_transforms;
  Vec<Vec2>    canvas_centers;
  Vec<Vec2>    canvas_extents;
  Vec<CRect>   clips;
  Vec<u32>     z_ordering;
  Vec<u32>     focus_ordering;
  Vec<u32>     focus_indices;

  /// Frame Computed Info
  bool                      closing_deferred;
  Option<u32>               focus_grab;
  Cursor                    cursor;
  Vec<Tuple<ViewId, Event>> events;

  explicit System(AllocatorRef allocator) :
    root_view{},
    hit{none},
    focus{},
    frame{0},
    next_id{0},
    input{allocator},
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
    input_infos{allocator},
    is_viewport{allocator},
    extents{allocator},
    centers{allocator},
    viewport_extents{allocator},
    viewport_centers{allocator},
    viewport_zooms{allocator},
    is_fixed_centered{allocator},
    fixed_centers{allocator},
    z_indices{allocator},
    layers{allocator},
    canvas_transforms{allocator},
    canvas_inverse_transforms{allocator},
    z_ordering{allocator},
    focus_ordering{allocator},
    closing_deferred{false},
    focus_grab{none},
    cursor{Cursor::Default},
    events{allocator}
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
                      i32 & tab_index, u32 viewport);

  void build(Ctx const & ctx, RootView & root);

  void prepare_for(u32 n);

  void focus_order();

  void layout(Vec2 viewport_extent);

  void stack();

  void visibility();

  void render(Canvas & canvas);

  void focus_scroll(u32 view);

  template <typename Filter, typename Match>
  Option<Tuple<u32, HitEvent>> hit_views(Option<Vec2> position,
                                         Filter && filter, Match && match) const
  {
    if (!position)
    {
      return none;
    }

    auto p = position.value();

    // find in reverse z-order
    for (auto z = views.size(); z != 0;)
    {
      z--;

      auto const i = z_ordering[z];

      // find first non-hidden view that overlaps the hit position
      if (!is_hidden[i] && filter(i) &&
          CRect{.center = canvas_centers[i], .extent = canvas_extents[i]}
            .contains(p)) [[unlikely]]
      {
        auto result = hit_test(i, p);
        if (result) [[unlikely]]
        {
          if (match(i)) [[unlikely]]
          {
            return Tuple{i, result.value()};
          }
          else
          {
            // the filtering failed: obstructed by view that doesn't match the accept
            return none;
          }
        }
      }
    }

    return none;
  }

  Option<HitEvent> hit_test(u32 view, Vec2 position) const;

  u32 navigate_focus(u32 from, bool forward) const;

  void event_dispatch(InputState const & input);

  Option<TextInputInfo> text_input() const;

  HitState none_seq(Ctx const & ctx);

  HitState drag_start_seq(Ctx const & ctx, MouseButton btn, ViewId src);

  HitState drag_update_seq(Ctx const & ctx, MouseButton btn, ViewId src,
                           ViewId tgt);

  HitState point_seq(Ctx const & ctx, ViewId tgt);

  HitState hit_seq(Ctx const & ctx);

  // [ ] make positions relative to center of the screen
  bool tick(InputState const & input, View & root, Canvas & canvas,
            Fn<void(Ctx const &)> loop);
};

}    // namespace ui
}    // namespace ash
