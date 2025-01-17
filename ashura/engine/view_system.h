/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/error.h"
#include "ashura/std/range.h"

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
  u32 depth        = 0;
  u32 breadth      = 0;
  u32 parent       = U32_MAX;
  u32 first_child  = 0;
  u32 num_children = 0;
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

struct ViewSystem
{
  struct Focus
  {
    /// @brief if focusing is active
    bool active = false;

    /// @brief currently focused view
    u64 view = U64_MAX;

    /// @brief focus tree index of the view
    u32 focus_idx = U32_MAX;

    /// @brief does the view accepts text input
    bool text_input = false;

    /// @brief does the view accepts tab input
    bool tab_input = false;

    /// @brief does the view accept esc input
    bool esc_input = false;
  };

  struct State
  {
    /// @brief mouse pointed view
    Option<u64> pointed = None;

    /// @brief drag data soure view
    Option<u64> drag_src = None;

    /// @brief current cursor
    Cursor cursor = Cursor::Default;

    /// @brief focus state
    Option<Focus> focus = None;

    /// @brief grab focus state
    Option<Focus> grab_focus = None;

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

  Vec<View *>   views;
  Vec<ViewNode> nodes;

  Vec<i32>    tab_indices;
  Vec<u32>    viewports;
  BitVec<u64> is_hidden;
  BitVec<u64> is_pointable;
  BitVec<u64> is_clickable;
  BitVec<u64> is_scrollable;
  BitVec<u64> is_draggable;
  BitVec<u64> is_droppable;
  BitVec<u64> is_focusable;
  BitVec<u64> is_text_input;
  BitVec<u64> is_tab_input;
  BitVec<u64> is_esc_input;
  BitVec<u64> is_viewport;

  Vec<Vec2>    centers;
  Vec<Vec2>    extents;
  Vec<Vec2>    viewport_extents;
  Vec<Affine3> viewport_transforms;
  BitVec<u64>  is_fixed_positioned;
  Vec<Vec2>    fixed_positions;
  Vec<i32>     z_indices;
  Vec<i32>     layers;

  Vec<Affine3> transforms;
  Vec<CRect>   clips;
  Vec<u32>     z_ordering;
  Vec<u32>     focus_ordering;

  explicit ViewSystem(AllocatorImpl allocator) :
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
      is_text_input{allocator},
      is_tab_input{allocator},
      is_esc_input{allocator},
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
      focus_ordering{allocator}
  {
  }

  ViewSystem(ViewSystem const &)             = delete;
  ViewSystem(ViewSystem &&)                  = default;
  ViewSystem & operator=(ViewSystem const &) = delete;
  ViewSystem & operator=(ViewSystem &&)      = default;
  ~ViewSystem()                              = default;

  void clear()
  {
    views.clear();
    nodes.clear();

    tab_indices.clear();
    viewports.clear();
    is_hidden.clear();
    is_pointable.clear();
    is_clickable.clear();
    is_scrollable.clear();
    is_draggable.clear();
    is_droppable.clear();
    is_focusable.clear();
    is_text_input.clear();
    is_tab_input.clear();
    is_esc_input.clear();
    is_viewport.clear();

    centers.clear();
    extents.clear();
    viewport_extents.clear();
    viewport_transforms.clear();
    is_fixed_positioned.clear();
    fixed_positions.clear();
    z_indices.clear();
    layers.clear();

    transforms.clear();
    clips.clear();
    z_ordering.clear();
    focus_ordering.clear();
  }

  ViewEvents process_events(View & view)
  {
    ViewEvents events;

    if (view.id() == U64_MAX) [[unlikely]]
    {
      // should never happen
      CHECK(next_id != U64_MAX);
      view.inner.id  = next_id++;
      events.mounted = true;
    }

    u64 const id = view.id();

    events.view_hit = (view.inner.last_rendered_frame + 1) == frame;

    if (f1.pointed.contains(id)) [[unlikely]]
    {
      if (f1.dragging) [[unlikely]]
      {
        events.drag_in   = !f0.pointed.contains(id);
        events.drag_over = true;
      }

      events.drop         = f1.dropped;
      events.mouse_in     = !f0.pointed.contains(id);
      events.mouse_down   = f1.mouse_down;
      events.mouse_up     = f1.mouse_up;
      events.mouse_moved  = f1.mouse_moved;
      events.mouse_scroll = f1.mouse_scrolled;
    }
    else if (f0.pointed.contains(id)) [[unlikely]]
    {
      events.mouse_out = true;
      events.drag_out  = f0.dragging;
    }

    if (f1.drag_src.contains(id)) [[unlikely]]
    {
      events.drag_start = f1.drag_start;
      events.dragging   = f1.dragging;
      events.drag_end   = f1.drag_end;
    }

    if (f1.focus.is_some() && f1.focus.value().view == view.id() &&
        f1.focus.value().active) [[unlikely]]
    {
      events.focus_in =
          (f0.focus.value().view != view.id()) || !f0.focus.value().active;
      events.key_down   = f1.key_down;
      events.key_up     = f1.key_up;
      events.text_input = f1.text_input;
    }
    else if (f0.focus.is_some() && f0.focus.value().view == view.id() &&
             f0.focus.value().active) [[unlikely]]
    {
      events.focus_out = true;
    }

    return events;
  }

  void push_view(View & view, u32 depth, u32 breadth, u32 parent)
  {
    views.push(&view).unwrap();
    nodes
        .push(ViewNode{.depth        = depth,
                       .breadth      = breadth,
                       .parent       = parent,
                       .first_child  = 0,
                       .num_children = 0})
        .unwrap();
    tab_indices.extend_uninit(1).unwrap();
    viewports.extend_uninit(1).unwrap();
    is_hidden.extend_uninit(1).unwrap();
    is_pointable.extend_uninit(1).unwrap();
    is_clickable.extend_uninit(1).unwrap();
    is_scrollable.extend_uninit(1).unwrap();
    is_draggable.extend_uninit(1).unwrap();
    is_droppable.extend_uninit(1).unwrap();
    is_focusable.extend_uninit(1).unwrap();
    is_text_input.extend_uninit(1).unwrap();
    is_tab_input.extend_uninit(1).unwrap();
    is_esc_input.extend_uninit(1).unwrap();
    is_viewport.extend_uninit(1).unwrap();
  }

  void build_children(ViewContext const & ctx, View & view, u32 idx, u32 depth,
                      i32 & tab_index, u32 viewport)
  {
    u32 const first_child  = views.size32();
    u32       num_children = 0;

    auto builder = [&](View & child) {
      push_view(child, depth + 1, num_children++, idx);
    };

    ViewState s = view.tick(ctx, view.inner.region, view.inner.zoom,
                            process_events(view), fn(builder));

    tab_indices.set(idx, (s.tab == I32_MIN) ? tab_index : s.tab);
    viewports.set(idx, viewport);
    is_hidden.set(idx, s.hidden);
    is_pointable.set(idx, s.pointable);
    is_clickable.set(idx, s.clickable);
    is_scrollable.set(idx, s.scrollable);
    is_draggable.set(idx, s.draggable);
    is_droppable.set(idx, s.droppable);
    is_focusable.set(idx, s.focusable);
    is_text_input.set(idx, s.text_input);
    is_tab_input.set(idx, s.tab_input);
    is_esc_input.set(idx, s.esc_input);
    is_viewport.set(idx, s.viewport);

    if (!s.hidden && s.focusable && s.grab_focus) [[unlikely]]
    {
      f1.focus = Some{
          Focus{.active     = true,
                .view       = view.id(),
                .focus_idx  = view.inner.focus_idx,
                .text_input = s.text_input,
                .tab_input  = s.tab_input,
                .esc_input  = s.esc_input}
      };
    }

    nodes[idx].first_child      = first_child;
    nodes[idx].num_children     = num_children;
    u32 const children_viewport = s.viewport ? idx : viewport;

    for (u32 c = first_child; c < (first_child + num_children); c++)
    {
      tab_index++;
      build_children(ctx, *views[c], c, depth + 1, tab_index,
                     children_viewport);
    }
  }

  void build(ViewContext const & ctx, View & root)
  {
    push_view(root, 0, 0, U32_MAX);
    i32 tab_index = 0;
    build_children(ctx, root, 0, 0, tab_index, U32_MAX);
  }

  void focus_order()
  {
    iota(focus_ordering.view(), 0U);

    indirect_sort(focus_ordering.view(), [&](u32 a, u32 b) {
      return tab_indices[a] < tab_indices[b];
    });

    for (u32 i = 0; i < views.size32(); i++)
    {
      views[focus_ordering[i]]->inner.focus_idx = i;
    }
  }

  void layout(Vec2 viewport_extent)
  {
    if (views.is_empty())
    {
      return;
    }

    u32 const n = views.size32();

    // allocate sizes to children recursively
    extents[0] = viewport_extent;
    for (u32 i = 0; i < n; i++)
    {
      ViewNode const & node = nodes[i];
      views[i]->size(extents[i],
                     extents.view().slice(node.first_child, node.num_children));
    }

    centers[0] = Vec2::splat(0);

    // fit parent views along the finalized sizes of the child views and
    // assign centers to the children based on their sizes.
    for (u32 i = n; i != 0;)
    {
      i--;
      ViewNode const & node   = nodes[i];
      ViewLayout       layout = views[i]->fit(
          extents[i], extents.view().slice(node.first_child, node.num_children),
          centers.view().slice(node.first_child, node.num_children));
      extents[i]             = layout.extent;
      viewport_extents[i]    = layout.viewport_extent;
      viewport_transforms[i] = layout.viewport_transform;
      is_fixed_positioned.set(i, layout.fixed_position.is_some());
      if (layout.fixed_position.is_some()) [[unlikely]]
      {
        fixed_positions[i] = layout.fixed_position.value();
      }
    }

    // transform views to canvas-space

    transforms[0] = Affine3::identity();

    for (u32 i = 0; i < n; i++)
    {
      ViewNode const & node               = nodes[i];
      // parent-space to local viewport-space transformation matrix
      Affine3 const &  viewport_transform = viewport_transforms[i];
      // accumulated transform of all ancestors, determines position until this
      // parent
      Affine3 const &  ancestor_transform = transforms[i];
      for (u32 c = node.first_child; c < (node.first_child + node.num_children);
           c++)
      {
        transforms[c] =
            // apply viewport-space transform
            viewport_transform
            // apply parent-space transform
            * translate2d(centers[c])
            // first use accumulated ancestor transform
            * ancestor_transform;
      }
    }

    // convert to [0, viewport_extent] space
    for (u32 i = 0; i < n; i++)
    {
      Affine3 const & transform = transforms[i];
      f32 const       zoom      = transform[0][0];
      centers[i] =
          ash::transform(transform, Vec2{0, 0}) + viewport_extent * 0.5F;
      extents[i]          = extents[i] * zoom;
      viewport_extents[i] = viewport_extents[i] * zoom;
    }

    for (u32 i = 0; i < n; i++)
    {
      if (is_fixed_positioned[i]) [[unlikely]]
      {
        centers[i] = fixed_positions[i];
      }
    }

    fill(clips.view(), CRect::from_offset({0, 0}, viewport_extent));

    /// recursive view clipping
    for (u32 i = 0; i < n; i++)
    {
      u32 const viewport = viewports[i];
      if (is_viewport[i]) [[unlikely]]
      {
        CRect const clip{.center = centers[i], .extent = extents[i]};
        if (viewport != U32_MAX) [[likely]]
        {
          clips[i] = intersect(clip, clips[viewport]);
        }
        else
        {
          clips[i] = clip;
        }
      }
    }

    /// assign viewport clips to contained views
    for (u32 i = 0; i < n; i++)
    {
      u32 const viewport = viewports[i];
      if (!is_viewport[i] && viewport != U32_MAX) [[likely]]
      {
        clips[i] = clips[viewport];
      }
    }

    for (u32 i = 0; i < n; i++)
    {
      View & view       = *views[i];
      view.inner.region = CRect{.center = centers[i], .extent = extents[i]};
      view.inner.zoom   = transforms[i][0][0];
    }
  }

  /// @brief compares the z-order of `a` and `b`
  static constexpr Ordering z_order_cmp(i32 a_layer, i32 a_z_index, u32 a_depth,
                                        i32 b_layer, i32 b_z_index, u32 b_depth)
  {
    // cmp stacking context/layer first
    Ordering ord = cmp(a_layer, b_layer);

    if (ord != Ordering::Equal)
    {
      return ord;
    }

    // cmp z_index
    ord = cmp(a_z_index, b_z_index);

    if (ord != Ordering::Equal)
    {
      return ord;
    }

    // cmp depth in the view tree
    return cmp(a_depth, b_depth);
  }

  void stack()
  {
    u32 const n = views.size32();

    if (n == 0)
    {
      return;
    }

    z_indices[0] = 0;

    for (u32 i = 0; i < n; i++)
    {
      ViewNode const & node = nodes[i];
      z_indices[i]          = views[i]->z_index(
          z_indices[i],
          z_indices.view().slice(node.first_child, node.num_children));
    }

    layers[0] = 0;
    for (u32 i = 0; i < n; i++)
    {
      ViewNode const & node = nodes[i];
      if (node.parent != U32_MAX)
      {
        layers[i] = views[i]->stack(layers[node.parent]);
      }
    }

    iota(z_ordering.view(), 0U);

    // sort layers
    indirect_sort(z_ordering.view(), [&](u32 a, u32 b) {
      return z_order_cmp(layers[a], z_indices[a], nodes[a].depth, layers[b],
                         z_indices[b], nodes[b].depth) == Ordering::Less;
    });
  }

  void visibility(Vec2 viewport_extent)
  {
    for (u32 i = 0; i < views.size32(); i++)
    {
      ViewNode const & node = nodes[i];

      if (is_hidden[i])
      {
        // if parent requested to be hidden, make children hidden
        for (u32 c = node.first_child;
             c < (node.first_child + node.num_children); c++)
        {
          is_hidden.set_bit(c);
        }
      }
      else
      {
        CRect         region{.center = centers[i], .extent = extents[i]};
        CRect const & clip = clips[i];
        bool const    hidden =
            !overlaps(region, clip) ||
            !overlaps(region, CRect::from_offset({0, 0}, viewport_extent)) ||
            !region.is_visible() || !clip.is_visible();

        is_hidden.set(i, hidden);
      }
    }
  }

  void render(Canvas & canvas)
  {
    for (u32 i : z_ordering)
    {
      if (!is_hidden.get(i)) [[unlikely]]
      {
        View & view = *views[i];
        canvas.clip(clips[i]);
        view.render(canvas, view.inner.region, view.inner.zoom, clips[i]);
        view.inner.last_rendered_frame = frame;
      }
    }
  }

  void focus_view(u32 view)
  {
    (void) view;
    // [ ] grab focus would need to scroll down to widget; would need
    // virtual scrolling support. offset based?
    // [ ] we need to know where it is located within the parent viewport, so we
    // can request the viewport focus on it
    // get position of view in parent viewport
    // scroll to that position in the parent viewport
    //
    // for all viewports the view is contained in:
    // get the position of the viewport in their respective parent viewport and
    // scroll into them.
  }

  static constexpr bool hit_test(View & view, Vec2 position)
  {
    return contains(view.inner.region, position) &&
           view.hit(view.inner.region, view.inner.zoom, position);
  }

  Option<u32> hit_views(Vec2 mouse_position, ViewHitAttributes match) const
  {
    u32 const n = views.size32();

    // find in reverse z-order
    for (u32 z = n; z != 0;)
    {
      z--;

      u32 const i    = z_ordering[z];
      View &    view = *views[i];

      bool matches = false;

      // avoids reading from the bit arrays if not needed

      if (has_bits(match, ViewHitAttributes::Pointable))
      {
        matches |= is_pointable[i];
      }

      if (has_bits(match, ViewHitAttributes::Clickable))
      {
        matches |= is_clickable[i];
      }

      if (has_bits(match, ViewHitAttributes::Scrollable))
      {
        matches |= is_scrollable[i];
      }

      if (has_bits(match, ViewHitAttributes::Draggable))
      {
        matches |= is_draggable[i];
      }

      if (has_bits(match, ViewHitAttributes::Droppable))
      {
        matches |= is_droppable[i];
      }

      if (!is_hidden[i] && matches && hit_test(view, mouse_position))
          [[unlikely]]
      {
        return Some{i};
      }
    }

    return None;
  }

  Option<u32> navigate_focus(u32 from, bool forward) const
  {
    u32 const n = views.size32();

    if (n == 0)
    {
      return None;
    }

    if (from > n)
    {
      from = n - 1;
    }

    if (forward)
    {
      u32 f = (from + 1) % n;

      while (f != from)
      {
        u32 const i = focus_ordering[f];
        if (!is_hidden[i] && is_focusable[i])
        {
          break;
        }

        f = (f + 1) % n;
      }

      return Some{f};
    }
    else
    {
      u32 start = (n - 1) - from;
      u32 f     = (start + 1) % n;

      while (f != start)
      {
        u32 const i = focus_ordering[(n - 1) - f];
        if (!is_hidden[i] && is_focusable[i])
        {
          break;
        }

        f = (f + 1) % n;
      }

      return Some{(n - 1) - f};
    }
  }

  void events(ViewContext const & ctx)
  {
    f0 = f1;
    f1 = State{};

    f1.mouse_down     = ctx.mouse.any_down;
    f1.mouse_up       = ctx.mouse.any_up;
    f1.mouse_moved    = ctx.mouse.moved;
    f1.mouse_scrolled = ctx.mouse.wheel_scrolled;
    f1.key_down       = ctx.key.any_down;
    f1.key_up         = ctx.key.any_up;
    f1.text_input     = ctx.text_input;

    bool const esc_input = ctx.key_down(KeyCode::Escape);
    bool const tab_input = ctx.key_down(KeyCode::Tab);

    // use grab focus request if any, otherwise persist previous frame's focus
    f1.focus = f0.grab_focus ? f0.grab_focus : f0.focus;

    // [ ] start and end text input on mobile platforms

    // mouse click & drag
    if (f1.mouse_down)
    {
      hit_views(ctx.mouse.position,
                ViewHitAttributes::Clickable | ViewHitAttributes::Draggable)
          .match(
              [&](u32 i) {
                View & view = *views[i];

                f1.pointed = Some{view.id()};

                if (ctx.mouse_down(MouseButton::Primary) && is_draggable[i])
                {
                  f1.dragging   = true;
                  f1.drag_src   = Some{view.id()};
                  f1.drag_start = true;
                }

                f1.cursor = view.cursor(view.inner.region, view.inner.zoom,
                                        ctx.mouse.position);

                f1.focus = Some{
                    Focus{.active     = false,
                          .view       = view.id(),
                          .focus_idx  = view.inner.focus_idx,
                          .text_input = is_text_input[i],
                          .tab_input  = is_tab_input[i],
                          .esc_input  = is_esc_input[i]}
                };
              },
              [&]() { f1.focus = None; });
    }
    // mouse drop event
    else if ((f0.dragging && ctx.mouse_up(MouseButton::Primary)) || ctx.dropped)
    {
      f1.drag_src = f0.drag_src;
      f1.dropped  = true;
      f1.dragging = false;

      hit_views(ctx.mouse.position, ViewHitAttributes::Droppable)
          .match([&](u32 i) {
            View & view = *views[i];
            f1.pointed  = Some{view.id()};
            f1.cursor   = view.cursor(view.inner.region, view.inner.zoom,
                                      ctx.mouse.position);
          });
    }
    // mouse dragging update event
    else if (f0.dragging || ctx.drop_hovering)
    {
      f1.drag_src = f0.drag_src;
      f1.dragging = true;

      hit_views(ctx.mouse.position, ViewHitAttributes::Droppable)
          .match([&](u32 i) {
            View & view = *views[i];
            f1.pointed  = Some{view.id()};
            f1.cursor   = view.cursor(view.inner.region, view.inner.zoom,
                                      ctx.mouse.position);
          });
    }
    // mouse release event
    else if (f1.mouse_up)
    {
      hit_views(ctx.mouse.position, ViewHitAttributes::Clickable)
          .match([&](u32 i) {
            View & view = *views[i];
            f1.pointed  = Some{view.id()};
            f1.cursor   = view.cursor(view.inner.region, view.inner.zoom,
                                      ctx.mouse.position);
          });
    }
    // mouse scroll event
    else if (ctx.mouse.wheel_scrolled)
    {
      hit_views(ctx.mouse.position, ViewHitAttributes::Scrollable)
          .match([&](u32 i) {
            View & view = *views[i];
            f1.pointed  = Some{view.id()};
            f1.cursor   = view.cursor(view.inner.region, view.inner.zoom,
                                      ctx.mouse.position);
          });
    }
    // mouse pointing event
    else
    {
      hit_views(ctx.mouse.position, ViewHitAttributes::Pointable |
                                        ViewHitAttributes::Clickable |
                                        ViewHitAttributes::Draggable |
                                        ViewHitAttributes::Scrollable)
          .match([&](u32 i) {
            View & view = *views[i];
            f1.pointed  = Some{view.id()};
            f1.cursor   = view.cursor(view.inner.region, view.inner.zoom,
                                      ctx.mouse.position);
          });
    }

    // determine focus navigation direction
    FocusAction focus_action = FocusAction::None;

    // navigate focus request
    if (tab_input)
    {
      if (!(f1.focus.is_some() && f1.focus.value().tab_input))
      {
        if (ctx.key_state(KeyCode::LShift) || ctx.key_state(KeyCode::RShift))
        {
          focus_action = FocusAction::Backward;
        }
        else
        {
          focus_action = FocusAction::Forward;
        }
      }
    }

    // clear focus request
    if (esc_input)
    {
      if (!(f1.focus.is_some() && f1.focus.value().esc_input))
      {
        f1.focus = None;
      }
    }

    switch (focus_action)
    {
      case FocusAction::Forward:
      case FocusAction::Backward:
      {
        u32 from = 0;
        if (f1.focus.is_some())
        {
          from = f1.focus.value().focus_idx;
        }
        else
        {
          from = 0;
        }

        f1.focus = navigate_focus(from, focus_action == FocusAction::Forward)
                       .map([&](u32 focus_idx) {
                         u32 const i = focus_ordering[focus_idx];
                         return Focus{.active     = true,
                                      .view       = views[i]->id(),
                                      .focus_idx  = focus_idx,
                                      .text_input = is_text_input[i],
                                      .tab_input  = is_tab_input[i],
                                      .esc_input  = is_esc_input[i]};
                       });
      }
      break;
      default:
      case FocusAction::None:
        break;
    }

    // [ ] call focus_view() once a new focus navigation has occured
  }

  // bool should_show_text_input() const
  // {
  //   return state[1].focus.text_input;
  // }

  void tick(ViewContext const & ctx, View & root, Canvas & canvas)
  {
    clear();
    build(ctx, root);
    u32 const n = views.size32();
    centers.resize_uninit(n).unwrap();
    extents.resize_uninit(n).unwrap();
    viewport_extents.resize_uninit(n).unwrap();
    viewport_transforms.resize_uninit(n).unwrap();
    is_fixed_positioned.resize_uninit(n).unwrap();
    fixed_positions.resize_uninit(n).unwrap();
    z_indices.resize_uninit(n).unwrap();
    layers.resize_uninit(n).unwrap();
    transforms.resize_uninit(n).unwrap();
    clips.resize_uninit(n).unwrap();
    z_ordering.resize_uninit(n).unwrap();
    focus_ordering.resize_uninit(n).unwrap();

    focus_order();
    layout(ctx.viewport_extent);
    stack();
    visibility(ctx.viewport_extent);
    events(ctx);
    render(canvas);

    frame++;
  }
};

}        // namespace ash
