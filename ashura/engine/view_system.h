/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/dict.h"

namespace ash
{

// [ ] overlap culling; occlusion rects sent to views; quadtrees
// [ ] mouse displacement for transformed/distorted views
// [ ] view click area re-targeting
// [ ] IME rect
// [ ] IME editing events
// [ ] make positions relative to center of the screen; especially in the inputstate goptten from the view

struct RootView final : ui::View
{
  static constexpr u16 NODE     = 0;
  static constexpr u16 PARENT   = 0;
  static constexpr u16 VIEWPORT = 0;

  Option<View &> next_ = none;

  constexpr RootView(Option<View &> next) : next_{next}
  {
  }

  constexpr virtual ui::ViewState tick(ui::Scope const &, ui::Events const &,
                                       Fn<void(View &)> build) override
  {
    next_.match(build);
    return ui::ViewState{.viewport = true};
  }

  constexpr virtual void size(f32x2 allocated, Span<f32x2> sizes) override
  {
    fill(sizes, allocated);
  }

  constexpr virtual ui::Layout fit(f32x2       allocated, Span<f32x2 const>,
                                   Span<f32x2> centers) override
  {
    fill(centers, f32x2{0, 0});
    return ui::Layout{.extent = allocated, .viewport_extent = allocated};
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

  constexpr virtual void render(Canvas, ui::RenderInfo const &) override
  {
  }

  constexpr virtual Cursor cursor(f32x2, f32x2) override
  {
    return Cursor::Default;
  }
};

enum class FocusAction : u8
{
  /// @brief Stay on the current focus
  None = 0,

  /// @brief Navigate forward on the focus tree
  Forward = 1,

  /// @brief Navigate backwards on the focus tree
  Backward = 2
};

typedef struct IViewSys * ViewSys;
typedef struct IEngine *  Engine;

struct ViewSysState
{
  bool                  should_continue = false;
  Option<Cursor>        cursor          = none;
  Option<TextInputInfo> input_info      = none;
};

/// @brief A compact View Hierarchy
struct IViewSys
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

  struct CrossFrameDragState
  {
    using Seq = DragState::Seq;
    using Seq::Start;
    using Seq::Update;

    Seq                seq = Start;
    Option<ui::ViewId> src = none;
    Option<ui::ViewId> tgt = none;
  };

  struct CrossFramePointState
  {
    Option<ui::ViewId> tgt = none;
  };

  using CrossFrameHitState =
    Enum<None, CrossFrameDragState, CrossFramePointState>;

  struct CrossFrameFocusState
  {
    /// @brief If focusing is active
    bool active = false;

    ui::ViewId tgt = ui::ViewId::None;
  };

  /// @brief Flattened hierarchical tree node, all siblings are
  /// packed sequentially. This only represents the parent node. Since the tree is
  /// rebuilt from scratch every time, the order is preserved in that parents
  /// always come before children.
  /// @param depth depth of the tree this node belongs to. there's ever only one
  /// node at depth 0: the root node.
  struct Nodes
  {
    Vec<ref<ui::View>> views;
    Vec<u16>           depth;
    Vec<u16>           parent;
    Vec<Slice16>       children;

    Nodes(Allocator allocator) :
      views{allocator},
      depth{allocator},
      parent{allocator},
      children{allocator}
    {
    }

    static Nodes create(Allocator allocator, usize initial_capacity);
  };

  struct Props
  {
    /// View Attributes
    Vec<i32>           tab_indices;
    Vec<u16>           viewports;
    BitVec<u64>        hidden;
    BitVec<u64>        pointable;
    BitVec<u64>        clickable;
    BitVec<u64>        scrollable;
    BitVec<u64>        draggable;
    BitVec<u64>        droppable;
    BitVec<u64>        focusable;
    Vec<TextInputInfo> input;
    BitVec<u64>        is_viewport;

    /// Computed data
    Vec<f32x2> extents;
    Vec<f32x2> centers;
    Vec<f32x2> viewport_extents;
    Vec<f32x2> viewport_centers;
    Vec<f32x2> viewport_zooms;

    /// @brief If the view is at a fixed location in the viewport
    BitVec<u64> fixed;

    /// @brief The viewport location of the views
    Vec<f32x2> fixed_centers;

    Vec<i32> z_idx;
    Vec<i32> layers;

    /// @brief Transforms from viewport-space to the canvas-space
    Vec<affinef32x3> canvas_xfm;

    /// @brief Transforms from canvas-space to viewport-space
    Vec<affinef32x3> canvas_inv_xfm;
    Vec<f32x2>       canvas_centers;
    Vec<f32x2>       canvas_extents;
    Vec<CRect>       clips;
    Vec<u16>         z_ord;

    /// @brief maps the focus tree index to the view
    Vec<u16> focus_ord;

    /// @brief maps the view to its focus index
    Vec<u16> focus_idx;

    static Props create(Allocator allocator, usize capacity);

    static Props none();
  };

  struct Tree
  {
    Nodes nodes;
    Props props;
  };

  struct Event
  {
    u16                    dst    = 0;
    ui::Events::Type       type   = ui::Events::PointerIn;
    Option<ui::HitInfo>    hit    = none;
    Option<ui::ScrollInfo> scroll = none;
  };

  struct FocusRequest
  {
    u16  tgt        = false;
    bool active     = false;
    bool grab_focus = false;
  };

  struct RequestQueue
  {
    Option<FocusRequest> focus       = none;
    bool                 defer_close = false;
  };

  Allocator allocator_;

  usize initial_nodes_capacity_;

  SystemState prev_frame_sys_state_;

  WindowState prev_frame_win_state_;

  ui::ViewSysScope prev_frame_sys_scope_;

  ui::Scope prev_frame_scope_;

  RootView root_view_;

  /// @brief Next view id
  u64 next_id_;

  /// @brief Id to current frame's view tree index map of hot views
  BitDict<ui::ViewId, u16> hot_ids_;

  CrossFrameHitState cross_frame_hit_state_;

  CrossFrameFocusState cross_frame_focus_state_;

  BitDict<ui::ViewId, ui::Events> event_queue_;

  IViewSys(Allocator allocator, ui::UserDataMap user_data_map) :
    allocator_{
      allocator
  },
    initial_nodes_capacity_{1'024},
    prev_frame_sys_state_{},
    prev_frame_win_state_{noop_allocator},
    prev_frame_sys_scope_{ui::default_core_theme(), std::move(user_data_map)},
    prev_frame_scope_{
      ui::InputScope{prev_frame_sys_state_, prev_frame_win_state_},
      prev_frame_sys_scope_},
    root_view_{none},
    next_id_{0},
    hot_ids_{allocator},
    cross_frame_hit_state_{none},
    cross_frame_focus_state_{},
    event_queue_{allocator}
  {
  }

  IViewSys(IViewSys const &)             = delete;
  IViewSys(IViewSys &&)                  = delete;
  IViewSys & operator=(IViewSys const &) = delete;
  IViewSys & operator=(IViewSys &&)      = delete;
  ~IViewSys()                            = default;

  void push_view_(Tree & tree, ui::View & view, u16 depth, u16 breadth,
                  u16 parent);

  ui::Events drain_events_(Tree & tree, ui::View & view, u16 idx);

  void build_children_(Tree & tree, ui::View & view, u16 idx, u16 depth,
                       u16 viewport, i32 & tab_index,
                       RequestQueue & request_queue);

  void build_(Tree & tree, RootView & root, RequestQueue & request_queue);

  void build_states_(Tree & tree);

  void focus_order_(Tree & tree);

  void layout_(Tree & tree, f32x2 viewport_extent);

  void stack_(Tree & tree);

  void visibility_(Tree & tree);

  void render_(Tree & tree, Canvas & canvas);

  /// @param active if the focus should be made an active focus, i.e. if it should
  /// be marked as an active focus. in some views, active focus may have
  /// different rendering/behavior than inactive focus.
  /// @param grab_focus if the focus is a grab focus
  void dispatch_focus_(Tree & tree, FocusRequest const & request,
                       Vec<Event> & events);

  ui::HitInfo get_hit_info_(Tree & tree, u16 view, f32x2 position) const;

  u16 navigate_focus_(Tree & tree, u16 from, bool forward) const;

  HitState none_seq_(Tree & tree, ui::InputScope const & input,
                     Vec<Event> & events, RequestQueue & request_queue);

  HitState drag_start_seq_(Tree & tree, ui::InputScope const & input,
                           Option<u16> src, Vec<Event> & events);

  HitState drag_update_seq_(Tree & tree, ui::InputScope const & input,
                            Option<u16> src, Option<u16> tgt,
                            Vec<Event> & events);

  HitState point_seq_(Tree & tree, ui::InputScope const & input,
                      Option<u16> tgt, Vec<Event> & events,
                      RequestQueue & request_queue);

  void hit_seq_(Tree & tree, ui::InputScope const & input, Vec<Event> & events,
                RequestQueue & request_queue);

  void focus_seq_(Tree & tree, ui::InputScope const & input,
                  Vec<Event> & events, RequestQueue & request_queue);

  void compose_event_(Tree & tree, ui::ViewId id, ui::Events::Type event,
                      Option<ui::HitInfo> hit, Option<ui::ScrollInfo> scroll);

  Tuple<Option<ui::FocusRect>, Option<TextInputInfo>, Cursor>
    prepare_events_(Tree & tree, ui::InputScope const & input,
                    RequestQueue & request_queue, Allocator scratch_allocator);

  ViewSysState tick(Engine engine, ui::InputScope const & input, Canvas canvas,
                    Fn<ui::View &(Engine, ui::Scope const &)> loop,
                    Allocator scratch_allocator);
};

}    // namespace ash
