/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/dict.h"

namespace ash
{
namespace ui
{

struct RootView : View
{
  static constexpr u16 NODE     = 0;
  static constexpr u16 PARENT   = 0;
  static constexpr u16 VIEWPORT = 0;

  Option<View &> next_ = none;

  constexpr RootView(Option<View &> next) : next_{next}
  {
  }

  constexpr virtual State tick(Ctx const &      ctx, Events const &,
                               Fn<void(View &)> build) override
  {
    next_.match(build);
    return State{.viewport = true};
  }

  constexpr virtual void size(f32x2 allocated, Span<f32x2> sizes) override
  {
    fill(sizes, allocated);
  }

  constexpr virtual Layout fit(f32x2       allocated, Span<f32x2 const>,
                               Span<f32x2> centers) override
  {
    fill(centers, f32x2{0, 0});
    return Layout{.extent = allocated, .viewport_extent = allocated};
  }

  constexpr virtual i32 layer(i32, Span<i32> indices) override
  {
    fill(indices, 0);
    return 0;
  }

  constexpr virtual i32 z_index(i32, Span<i32> indices) override
  {
    fill(indices, 0);
    return 0;
  }

  constexpr virtual void render(Canvas &           canvas,
                                RenderInfo const & info) override
  {
    // [ ] Body
    canvas.rect(ShapeInfo{
      .area = info.canvas_region, .tint = mdc::GRAY_900, .clip = info.clip});
  }

  constexpr virtual Cursor cursor(f32x2, f32x2) override
  {
    return Cursor::Default;
  }
};

enum class FocusAction : u8
{
  /// @brief stay on the current focus
  None = 0,

  /// @brief navigate forward on the focus tree
  Forward = 1,

  /// @brief navigate backwards on the focus tree
  Backward = 2
};

// [ ] overlap culling; occlusion rects sent to views; quadtrees
// [ ] mouse displacement for transformed/distorted views
// [ ] view click area re-targeting

/// @brief A compact View Hierarchy
struct System
{
  struct DragState
  {
    enum Seq : u8
    {
      Start  = 0,
      Update = 1
    };

    Seq         seq = Start;
    Option<u16> src = none;
    Option<u16> tgt = none;
  };

  struct PointState
  {
    Option<u16> tgt = none;
  };

  using HitState = Enum<None, DragState, PointState>;

  struct FocusState
  {
    /// @brief if focusing is active
    bool active = false;

    u16 tgt = 0;
  };

  struct XFrameDragState
  {
    using Seq = DragState::Seq;
    using Seq::Start;
    using Seq::Update;

    Seq            seq = Start;
    Option<ViewId> src = none;
    Option<ViewId> tgt = none;
  };

  struct XFramePointState
  {
    Option<ViewId> tgt = none;
  };

  using XFrameHitState = Enum<None, XFrameDragState, XFramePointState>;

  struct XFrameFocusState
  {
    /// @brief if focusing is active
    bool active = false;

    ViewId tgt = ViewId::None;
  };

  /// @brief flattened hierarchical tree node, all siblings are
  /// packed sequentially. This only represents the parent node. Since the tree is
  /// rebuilt from scratch every time, the order is preserved in that parents
  /// always come before children.
  /// @param depth depth of the tree this node belongs to. there's ever only one
  /// node at depth 0: the root node.
  struct Nodes
  {
    Vec<u16>     depth;
    Vec<u16>     parent;
    Vec<Slice16> children;

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
    Vec<u16>                   viewports;
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

  struct Event
  {
    u16                dst    = 0;
    Events::Type       type   = Events::PointerIn;
    Option<HitInfo>    hit    = none;
    Option<ScrollInfo> scroll = none;
  };

  /// @brief id to current frame's view tree index map of hot views

  RootView root_view;

  /// @brief current frame id
  u64 frame = 0;

  /// @brief next view id
  u64 next_id = 0;

  /// @brief Build context for views
  Ctx ctx;

  /// Tree Nodes

  Vec<ref<View>>       views;
  Nodes                nodes;
  BitDict<ViewId, u16> ids;

  Attrs att;

  /// Computed data

  Vec<f32x2> extents;
  Vec<f32x2> centers;
  Vec<f32x2> viewport_extents;
  Vec<f32x2> viewport_centers;
  Vec<f32x2> viewport_zooms;

  /// @brief if the view is at a fixed location in the viewport
  BitVec<u64> fixed;

  /// @brief the viewport location of the views
  Vec<f32x2> fixed_centers;

  Vec<i32> z_idx;
  Vec<i32> layers;

  /// @brief transforms from viewport-space to the canvas-space
  Vec<affinef32x3> canvas_xfm;

  /// @brief transforms from canvas-space to viewport-space
  Vec<affinef32x3> canvas_inv_xfm;
  Vec<f32x2>       canvas_centers;
  Vec<f32x2>       canvas_extents;
  Vec<CRect>       clips;
  Vec<u16>         z_ord;

  // maps the focus tree index to the view
  Vec<u16> focus_ord;

  // maps the view to its focus index
  Vec<u16> focus_idx;

  /// Frame Computed Info
  bool        closing_deferred;
  Option<u16> focus_grab_tgt;

  XFrameHitState   xframe_hit_state;
  XFrameFocusState xframe_focus_state;

  HitState   hit_state;
  FocusState focus_state;

  Vec<Event> events;

  BitDict<ViewId, Events> event_queue;

  Option<FocusRect>     focus_rect;
  Option<TextInputInfo> input_info;
  Option<Cursor>        cursor;
  f32                   scroll_delta;

  explicit System(AllocatorRef allocator) :
    root_view{none},
    frame{0},
    next_id{0},
    ctx{allocator, nullptr},
    views{allocator},
    nodes{allocator},
    ids{allocator},
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
    canvas_xfm{allocator},
    canvas_inv_xfm{allocator},
    z_ord{allocator},
    focus_ord{allocator},
    focus_idx{allocator},
    closing_deferred{false},
    focus_grab_tgt{none},
    xframe_hit_state{none},
    xframe_focus_state{},
    hit_state{none},
    focus_state{},
    events{allocator},
    event_queue{allocator},
    focus_rect{none},
    cursor{Cursor::Default},
    scroll_delta{100}
  {
  }

  System(System const &)             = delete;
  System(System &&)                  = default;
  System & operator=(System const &) = delete;
  System & operator=(System &&)      = default;
  ~System()                          = default;

  void clear_frame();

  void push_view(View & view, u16 depth, u16 breadth, u16 parent);

  Events drain_events(View & view, u16 idx);

  void build_children(Ctx const & ctx, View & view, u16 idx, u16 depth,
                      u16 viewport, i32 & tab_index);

  void build(Ctx const & ctx, RootView & root);

  void prepare_for(u16 n);

  void focus_order();

  void layout(f32x2 viewport_extent);

  void stack();

  void visibility();

  void render(Canvas & canvas);

  void focus_on(u16 view, bool active, bool grab_focus);

  Option<u16> hit_test(f32x2 position) const;

  HitInfo get_hit_info(u16 view, f32x2 position) const;

  template <typename Match>
  Option<u16> bubble(u16 from, Match && match) const
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
  Option<u16> bubble_hit(f32x2 position, Match && match) const
  {
    return hit_test(position).and_then(
      [&](auto i) { return bubble(i, match); });
  }

  u16 navigate_focus(u16 from, bool forward) const;

  HitState none_seq(Ctx const & ctx);

  HitState drag_start_seq(Ctx const & ctx, Option<u16> src);

  HitState drag_update_seq(Ctx const & ctx, Option<u16> src, Option<u16> tgt);

  HitState point_seq(Ctx const & ctx, Option<u16> tgt);

  void hit_seq(Ctx const & ctx);

  void focus_seq(Ctx const & ctx);

  void compose_event(ViewId id, Events::Type event, Option<HitInfo> hit,
                     Option<ScrollInfo> scroll);

  void process_input(Ctx const & ctx);

  // [ ] IME rect
  // [ ] IME editing events
  Option<TextInputInfo> text_input() const;

  // [ ] make positions relative to center of the screen; especially in the inputstate goptten from the view
  bool tick(InputState const & input, View & root, Canvas & canvas,
            Fn<void(Ctx const &)> loop);
};

}    // namespace ui
}    // namespace ash
