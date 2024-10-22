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

struct FocusInfo
{
  u64  view       = U64_MAX;
  u32  focus_idx  = U32_MAX;
  bool text_input = false;
  bool tab_input  = false;
  bool esc_input  = false;

  constexpr bool is_empty() const
  {
    return view == U64_MAX;
  }
};

struct ViewSystemState
{
  u64       mouse_pointed_view   = 0;
  u64       mouse_scrolling_view = 0;
  u64       drag_source_view     = 0;
  u64       drag_target_view     = 0;
  FocusInfo focused              = {};
  bool      focusing : 1         = false;
  Cursor    cursor : 5           = Cursor::Default;
  bool      mouse_down : 1       = false;
  bool      mouse_up : 1         = false;
  bool      mouse_moved : 1      = false;
  bool      mouse_scrolled : 1   = false;
  bool      mouse_drag_start : 1 = false;
  bool      mouse_drag_end : 1   = false;
  bool      mouse_dragging : 1   = false;
  bool      mouse_drag_drop : 1  = false;
  bool      keyboard_down : 1    = false;
  bool      keyboard_up : 1      = false;
};

enum class FocusAction : u8
{
  None     = 0,
  Forward  = 1,
  Backward = 2
};

struct ViewSystem
{
  u64             frame         = 0;
  u64             next_id       = 0;
  ViewSystemState state[2]      = {};
  FocusInfo       focus_request = {};

  Vec<View *>   views = {};
  Vec<ViewNode> nodes = {};

  Vec<i32>    tab_indices   = {};
  Vec<u32>    viewports     = {};
  BitVec<u64> is_hidden     = {};
  BitVec<u64> is_pointable  = {};
  BitVec<u64> is_clickable  = {};
  BitVec<u64> is_scrollable = {};
  BitVec<u64> is_draggable  = {};
  BitVec<u64> is_droppable  = {};
  BitVec<u64> is_focusable  = {};
  BitVec<u64> is_text_input = {};
  BitVec<u64> is_tab_input  = {};
  BitVec<u64> is_esc_input  = {};
  BitVec<u64> is_viewport   = {};

  Vec<Vec2>       centers             = {};
  Vec<Vec2>       extents             = {};
  Vec<Vec2>       viewport_extents    = {};
  Vec<Mat3Affine> viewport_transforms = {};
  Vec<Mat3Affine> absolute_transforms = {};
  Vec<i32>        z_indices           = {};
  Vec<i32>        stacking_contexts   = {};

  Vec<Mat3Affine> transforms     = {};
  Vec<CRect>      clips          = {};
  Vec<u32>        z_ordering     = {};
  Vec<u32>        focus_ordering = {};

  void reset()
  {
    frame    = 0;
    next_id  = 0;
    state[0] = {};
    state[1] = {};

    views.reset();
    nodes.reset();

    tab_indices.reset();
    viewports.reset();
    is_hidden.reset();
    is_pointable.reset();
    is_clickable.reset();
    is_scrollable.reset();
    is_draggable.reset();
    is_droppable.reset();
    is_focusable.reset();
    is_text_input.reset();
    is_tab_input.reset();
    is_esc_input.reset();
    is_viewport.reset();

    centers.reset();
    extents.reset();
    viewport_extents.reset();
    viewport_transforms.reset();
    absolute_transforms.reset();
    z_indices.reset();
    stacking_contexts.reset();

    transforms.reset();
    clips.reset();
    z_ordering.reset();
    focus_ordering.reset();
  }

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
    absolute_transforms.clear();
    z_indices.clear();
    stacking_contexts.clear();

    transforms.clear();
    clips.clear();
    z_ordering.clear();
    focus_ordering.clear();
  }

  ViewEvents process_events(View &view)
  {
    ViewEvents events;

    if (view.id() == U64_MAX) [[unlikely]]
    {
      CHECK(next_id != U64_MAX);
      view.inner.id  = next_id++;
      events.mounted = true;
    }

    events.view_hit = (view.inner.last_rendered_frame + 1) == frame;

    ViewSystemState const &s0 = state[0];
    ViewSystemState const &s1 = state[1];

    if (s1.mouse_pointed_view == view.id()) [[unlikely]]
    {
      events.mouse_in   = (s0.mouse_pointed_view != view.id());
      events.mouse_down = s1.mouse_down;
      events.mouse_up   = s1.mouse_up;
      events.mouse_moved =
          s1.mouse_moved && (s0.mouse_pointed_view == view.id());
    }
    else if (s0.mouse_pointed_view == view.id()) [[unlikely]]
    {
      events.mouse_out = true;
    }

    if (s1.mouse_scrolling_view == view.id()) [[unlikely]]
    {
      events.mouse_scroll = s1.mouse_scrolled;
    }

    if (s1.drag_target_view == view.id()) [[unlikely]]
    {
      events.drag_in   = (s0.drag_target_view != s1.drag_target_view);
      events.drop      = s1.mouse_drag_drop;
      events.drag_over = s1.mouse_dragging;
    }
    else if (s0.drag_target_view == view.id()) [[unlikely]]
    {
      events.drag_out = s1.mouse_dragging;
    }

    if (s1.drag_source_view == view.id()) [[unlikely]]
    {
      events.drag_start = s1.mouse_drag_start;
      events.dragging   = s1.mouse_dragging;
      events.drag_end   = s1.mouse_drag_end;
    }

    if (s1.focused.view == view.id() && s1.focusing) [[unlikely]]
    {
      events.focus_in   = (s0.focused.view != view.id()) || !s0.focusing;
      events.key_down   = s1.keyboard_down;
      events.key_up     = s1.keyboard_up;
      events.text_input = s1.focused.text_input;
    }
    else if (s0.focused.view == view.id()) [[unlikely]]
    {
      events.focus_out = true;
    }

    return events;
  }

  void push_view(View &view, u32 depth, u32 breadth, u32 parent)
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

  void build_children(ViewContext const &ctx, View &view, u32 idx, u32 depth,
                      i32 &tab_index, u32 viewport)
  {
    u32 const first_child  = views.size32();
    u32       num_children = 0;

    auto builder = [&](View &child) {
      push_view(child, depth + 1, num_children++, idx);
    };

    ViewState s =
        view.tick(ctx, view.inner.region, process_events(view), fn(&builder));

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
      focus_request = FocusInfo{.view       = view.id(),
                                .focus_idx  = view.inner.focus_idx,
                                .text_input = s.text_input,
                                .tab_input  = s.tab_input,
                                .esc_input  = s.esc_input};
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

  void build(ViewContext const &ctx, View *root)
  {
    if (root != nullptr)
    {
      push_view(*root, 0, 0, U32_MAX);
      i32 tab_index = 0;
      build_children(ctx, *root, 0, 0, tab_index, U32_MAX);
    }
  }

  void focus_order()
  {
    iota(span(focus_ordering), 0U);

    indirect_sort(span(focus_ordering), [&](u32 a, u32 b) {
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
      ViewNode const &node = nodes[i];
      views[i]->size(extents[i],
                     span(extents).slice(node.first_child, node.num_children));
    }

    centers[0] = Vec2::splat(0);

    // fit parent views along the finalized sizes of the child views and
    // assign centers to the children based on their sizes.
    for (u32 i = n; i != 0;)
    {
      i--;
      ViewNode const &node   = nodes[i];
      ViewLayout      layout = views[i]->fit(
          extents[i], span(extents).slice(node.first_child, node.num_children),
          span(centers).slice(node.first_child, node.num_children));
      extents[i]             = layout.extent;
      viewport_extents[i]    = layout.viewport;
      viewport_transforms[i] = layout.viewport_transform;
      absolute_transforms[i] = layout.absolute_transform;
    }

    // transform views to canvas-space

    transforms[0] = Mat3Affine::identity();

    for (u32 i = 0; i < n; i++)
    {
      ViewNode const &node = nodes[i];
      // parent-space to local viewport-space transformation matrix
      Mat3Affine viewport_transform = viewport_transforms[i];
      Mat3Affine parent_transform   = transforms[i];
      for (u32 c = node.first_child; c < (node.first_child + node.num_children);
           c++)
      {
        transforms[c] = viewport_transform * absolute_transforms[c] *
                        translate2d(centers[c]) * parent_transform;
      }
    }

    for (u32 i = 0; i < n; i++)
    {
      Mat3Affine transform = transforms[i];
      f32 const  zoom      = transform[0][0];
      centers[i]           = ash::transform(transform, centers[i]);
      extents[i]           = extents[i] * zoom;
      viewport_extents[i]  = viewport_extents[i] * zoom;
    }

    fill(span(clips), CRect{.center = {0, 0}, .extent = viewport_extent});

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
      views[i]->inner.region =
          CRect{.center = centers[i], .extent = extents[i]};
    }
  }

  void stack()
  {
    if (views.is_empty())
    {
      return;
    }

    z_indices[0] = 0;
    for (u32 i = 0; i < views.size32(); i++)
    {
      ViewNode const &node = nodes[i];
      z_indices[i]         = views[i]->z_index(
          z_indices[i],
          span(z_indices).slice(node.first_child, node.num_children));
    }

    stacking_contexts[0] = 0;
    for (u32 i = 0; i < views.size32(); i++)
    {
      ViewNode const &node = nodes[i];
      if (node.parent != U32_MAX)
      {
        stacking_contexts[i] = views[i]->stack(stacking_contexts[node.parent]);
      }
    }

    iota(span(z_ordering), 0U);

    // sort using z-index while having stacking context as higher priority
    indirect_sort(span(z_ordering), [&](u32 a, u32 b) {
      if (stacking_contexts[a] < stacking_contexts[b])
      {
        return true;
      }
      return z_indices[a] < z_indices[b];
    });
  }

  void visibility(Vec2 viewport_extent)
  {
    for (u32 i = 0; i < views.size32(); i++)
    {
      ViewNode const &node = nodes[i];

      if (is_hidden[i])
      {
        // if parent requested to be hidden, make children hidden
        for (u32 c = node.first_child;
             c < (node.first_child + node.num_children); c++)
        {
          is_hidden.set(c, true);
        }
      }
      else
      {
        CRect        region{.center = centers[i], .extent = extents[i]};
        CRect const &clip = clips[i];
        bool const   hidden =
            !overlaps(region, clip) ||
            !overlaps(region,
                      CRect{.center = {0, 0}, .extent = viewport_extent}) ||
            !region.is_visible() || !clip.is_visible();

        is_hidden.set(i, hidden);
      }
    }
  }

  void render(Canvas &canvas)
  {
    for (u32 i : z_ordering)
    {
      if (!is_hidden.get(i)) [[unlikely]]
      {
        View &view = *views[i];
        canvas.clip(clips[i]);
        view.render(canvas, view.inner.region, transforms[i][0][0], clips[i]);
        view.inner.last_rendered_frame = frame;
      }
    }
  }

  void focus_view(u32 view)
  {
    (void) view;
    // [ ] we need to know where it is located within the parent viewport, so we
    // can request the viewport focus on it
    // get position of view in parent viewport
    // scroll to that position in the parent viewport
    //
    // for all viewports the view is contained in:
    // get the position of the viewport in their respective parent viewport and
    // scroll into them.
  }

  void events(ViewContext const &ctx)
  {
    state[0]                 = state[1];
    state[1]                 = {};
    ViewSystemState &s0      = state[0];
    ViewSystemState &s1      = state[1];
    u32 const        n       = views.size32();
    FocusInfo        focused = s0.focused;
    Cursor cursor   = ctx.mouse.focused ? Cursor::Default : Cursor::None;
    bool   focusing = s0.focusing;

    if (ctx.mouse.focused)
    {
      // mouse click & drag
      if (has_bits(ctx.mouse.downs, MouseButtons::Primary))
      {
        for (u32 z_i = n; z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if ((is_clickable[i] || is_draggable[i]) &&
                view.hit(view.inner.region, transforms[i][0][0],
                         ctx.mouse.position)) [[unlikely]]
            {
              if (is_clickable[i])
              {
                s1.mouse_dragging   = true;
                s1.drag_source_view = view.id();
                s1.mouse_dragging   = true;
                s1.mouse_drag_start = true;
              }
              else
              {
                s1.mouse_down         = true;
                s1.mouse_pointed_view = view.id();
              }

              focused = FocusInfo{.view       = view.id(),
                                  .focus_idx  = view.inner.focus_idx,
                                  .text_input = is_text_input[i],
                                  .tab_input  = is_tab_input[i],
                                  .esc_input  = is_esc_input[i]};
              cursor  = view.cursor(view.inner.region, transforms[i][0][0],
                                    ctx.mouse.position);
              break;
            }
          }
        }
      }
      // mouse press events
      else if ((ctx.mouse.downs != MouseButtons::None ||
                ctx.mouse.ups != MouseButtons::None) &&
               !s0.mouse_dragging)
      {
        for (u32 z_i = n; z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (is_clickable[i] &&
                view.hit(view.inner.region, transforms[i][0][0],
                         ctx.mouse.position)) [[unlikely]]
            {
              s1.mouse_down         = ctx.mouse.downs != MouseButtons::None;
              s1.mouse_up           = ctx.mouse.ups != MouseButtons::None;
              s1.mouse_pointed_view = view.id();

              focused = FocusInfo{.view       = view.id(),
                                  .focus_idx  = view.inner.focus_idx,
                                  .text_input = is_text_input[i],
                                  .tab_input  = is_tab_input[i],
                                  .esc_input  = is_esc_input[i]};
              cursor  = view.cursor(view.inner.region, transforms[i][0][0],
                                    ctx.mouse.position);
              break;
            }
          }
        }
      }
      // mouse dragging update event
      else if (ctx.mouse.moved && s0.mouse_dragging)
      {
        s1.drag_source_view = s0.drag_source_view;
        s1.mouse_dragging   = true;
        for (u32 z_i = n; z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (is_droppable[i] &&
                view.hit(view.inner.region, transforms[i][0][0],
                         ctx.mouse.position)) [[unlikely]]
            {
              s1.drag_target_view = view.id();

              focused = FocusInfo{.view       = view.id(),
                                  .focus_idx  = view.inner.focus_idx,
                                  .text_input = is_text_input[i],
                                  .tab_input  = is_tab_input[i],
                                  .esc_input  = is_esc_input[i]};
              cursor  = view.cursor(view.inner.region, transforms[i][0][0],
                                    ctx.mouse.position);
              break;
            }
          }
        }
      }
      // mouse drop event
      else if (has_bits(ctx.mouse.ups, MouseButtons::Primary) &&
               s0.mouse_dragging)
      {
        s1.drag_source_view = s1.drag_source_view;
        s1.mouse_drag_drop  = true;
        s1.mouse_dragging   = true;
        for (u32 z_i = n; z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (is_droppable[i] &&
                view.hit(view.inner.region, transforms[i][0][0],
                         ctx.mouse.position)) [[unlikely]]
            {
              s1.drag_target_view = view.id();

              focused = FocusInfo{.view       = view.id(),
                                  .focus_idx  = view.inner.focus_idx,
                                  .text_input = is_text_input[i],
                                  .tab_input  = is_tab_input[i],
                                  .esc_input  = is_esc_input[i]};
              cursor  = view.cursor(view.inner.region, transforms[i][0][0],
                                    ctx.mouse.position);
              break;
            }
          }
        }
      }
      // mouse scroll event
      else if (ctx.mouse.wheel_scrolled)
      {
        for (u32 z_i = n; z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (is_scrollable[i] &&
                view.hit(view.inner.region, transforms[i][0][0],
                         ctx.mouse.position)) [[unlikely]]
            {
              s1.mouse_scrolling_view = view.id();
              s1.mouse_scrolled       = true;
              cursor = view.cursor(view.inner.region, transforms[i][0][0],
                                   ctx.mouse.position);
              break;
            }
          }
        }
      }
      // pointing event
      else if (!s0.mouse_dragging)
      {
        for (u32 z_i = n; z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (is_pointable[i] &&
                view.hit(view.inner.region, transforms[i][0][0],
                         ctx.mouse.position)) [[unlikely]]
            {
              s1.mouse_pointed_view = view.id();
              cursor = view.cursor(view.inner.region, transforms[i][0][0],
                                   ctx.mouse.position);
              break;
            }
          }
        }
      }
    }

    FocusAction focus_action = FocusAction::None;

    if (!focused.tab_input && ctx.key_state(KeyCode::Tab))
    {
      focus_action =
          (ctx.key_state(KeyCode::LShift) || ctx.key_state(KeyCode::RShift)) ?
              FocusAction::Backward :
              FocusAction::Forward;
      focusing = true;
    }

    if (!focused.esc_input && ctx.key_state(KeyCode::Escape))
    {
      focusing = false;
    }

    //
    // [ ] grab focus would need to scroll down to widget; would need
    // virtual scrolling support. offset based?
    //

    if (!focus_request.is_empty() && !focusing)
    {
      focused  = focus_request;
      focusing = true;
    }

    switch (focus_action)
    {
      case FocusAction::Forward:
      {
        u32 start = 0;

        // if none is focused, start from first focusable view
        if (focused.focus_idx >= n)
        {
          start = 0;
        }
        else
        {
          // start searching from the next focus point
          start = focused.focus_idx + 1;
        }

        // find next focusable view
        for (u32 f_i = start; f_i < n; f_i++)
        {
          u32 i = focus_ordering[f_i];
          if (!is_hidden[i] && is_focusable[i])
          {
            focused = FocusInfo{.view       = views[i]->id(),
                                .focus_idx  = f_i,
                                .text_input = is_text_input[i],
                                .tab_input  = is_tab_input[i],
                                .esc_input  = is_esc_input[i]};
            break;
          }
        }
      }
      break;

      case FocusAction::Backward:
      {
        u32 start = 0;

        if (focused.focus_idx >= n)
        {
          start = n;
        }
        else
        {
          // start searching from the previous focus point
          start = focused.focus_idx;
        }

        // find prev focusable view
        for (u32 f_i = start; f_i != 0;)
        {
          f_i--;
          u32 i = focus_ordering[f_i];
          if (!is_hidden[i] && is_focusable[i])
          {
            focused = FocusInfo{.view       = views[i]->id(),
                                .focus_idx  = f_i,
                                .text_input = is_text_input[i],
                                .tab_input  = is_tab_input[i],
                                .esc_input  = is_esc_input[i]};
            break;
          }
        }
      }
      break;

      default:
        break;
    }

    s1.focused       = focused;
    s1.focusing      = focusing;
    s1.cursor        = cursor;
    s1.keyboard_down = ctx.keyboard.down;
    s1.keyboard_up   = ctx.keyboard.up;
    focus_request    = {};
  }

  bool should_show_text_input() const
  {
    return state[1].focused.text_input;
  }

  void tick(ViewContext const &ctx, View *root, Canvas &canvas)
  {
    clear();
    build(ctx, root);
    u32 const n = views.size32();
    centers.resize_uninit(n).unwrap();
    extents.resize_uninit(n).unwrap();
    viewport_extents.resize_uninit(n).unwrap();
    viewport_transforms.resize_uninit(n).unwrap();
    absolute_transforms.resize_uninit(n).unwrap();
    z_indices.resize_uninit(n).unwrap();
    stacking_contexts.resize_uninit(n).unwrap();
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