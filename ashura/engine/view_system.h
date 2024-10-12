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
  u32 first_child  = U32_MAX;
  u32 num_children = 0;
};

struct ViewSystemState
{
  u64  mouse_pointed_view   = 0;
  u64  mouse_scrolling_view = 0;
  u64  drag_source_view     = 0;
  u64  drag_target_view     = 0;
  u64  focused_view         = 0;
  u32  focus                = U32_MAX;
  bool mouse_down : 1       = false;
  bool mouse_up : 1         = false;
  bool mouse_moved : 1      = false;
  bool mouse_scrolled : 1   = false;
  bool mouse_drag_start : 1 = false;
  bool mouse_drag_end : 1   = false;
  bool mouse_dragging : 1   = false;
  bool mouse_drag_drop : 1  = false;
  bool keyboard_down : 1    = false;
  bool keyboard_up : 1      = false;
  bool text_input : 1       = false;
  bool tab_input : 1        = false;
};

enum class FocusDirection : u8
{
  None     = 0,
  Forward  = 1,
  Backward = 2
};

struct ViewSystem
{
  u64             frame             = 0;
  u64             last_id           = 0;
  ViewSystemState state[2]          = {};
  Vec<View *>     views             = {};
  Vec<ViewNode>   nodes             = {};
  Vec<Vec2>       centers           = {};
  Vec<Vec2>       extents           = {};
  Vec<CRect>      clips             = {};
  Vec<i32>        z_indices         = {};
  Vec<i32>        tab_indices       = {};
  Vec<i32>        stacking_contexts = {};
  Vec<u32>        z_ordering        = {};
  Vec<u32>        tab_ordering      = {};
  BitVec<u64>     is_hidden         = {};
  BitVec<u64>     is_pointable      = {};
  BitVec<u64>     is_clickable      = {};
  BitVec<u64>     is_scrollable     = {};
  BitVec<u64>     is_draggable      = {};
  BitVec<u64>     is_droppable      = {};
  BitVec<u64>     is_focusable      = {};
  BitVec<u64>     is_text_input     = {};
  BitVec<u64>     is_tab_input      = {};
  BitVec<u64>     is_grab_focus     = {};
  BitVec<u64>     is_lose_focus     = {};

  void reset()
  {
    views.reset();
    nodes.reset();
    centers.reset();
    extents.reset();
    clips.reset();
    z_indices.reset();
    tab_indices.reset();
    stacking_contexts.reset();
    z_ordering.reset();
    tab_ordering.reset();
    is_hidden.reset();
    is_pointable.reset();
    is_clickable.reset();
    is_scrollable.reset();
    is_draggable.reset();
    is_droppable.reset();
    is_focusable.reset();
    is_text_input.reset();
    is_tab_input.reset();
    is_grab_focus.reset();
    is_lose_focus.reset();
  }

  void clear()
  {
    views.clear();
    nodes.clear();
    centers.clear();
    extents.clear();
    clips.clear();
    z_indices.clear();
    tab_indices.clear();
    stacking_contexts.clear();
    z_ordering.clear();
    tab_ordering.clear();
    is_hidden.clear();
    is_pointable.clear();
    is_clickable.clear();
    is_scrollable.clear();
    is_draggable.clear();
    is_droppable.clear();
    is_focusable.clear();
    is_text_input.clear();
    is_tab_input.clear();
    is_grab_focus.clear();
    is_lose_focus.clear();
  }

  ViewEvents process_events(View &view)
  {
    ViewEvents events;

    if (view.id() == 0) [[unlikely]]
    {
      CHECK(last_id != U64_MAX);
      view.inner.id  = ++last_id;
      events.mounted = true;
    }

    events.view_hit = (view.inner.last_rendered_frame + 1) == frame;

    if (state[1].mouse_pointed_view == view.id()) [[unlikely]]
    {
      events.mouse_in   = (state[0].mouse_pointed_view != view.id());
      events.mouse_down = state[1].mouse_down;
      events.mouse_up   = state[1].mouse_up;
      events.mouse_moved =
          state[1].mouse_moved && (state[0].mouse_pointed_view == view.id());
    }
    else if (state[0].mouse_pointed_view == view.id()) [[unlikely]]
    {
      events.mouse_out = true;
    }

    if (state[1].mouse_scrolling_view == view.id()) [[unlikely]]
    {
      events.mouse_scroll = state[1].mouse_scrolled;
    }

    if (state[1].drag_target_view == view.id()) [[unlikely]]
    {
      events.drag_in = (state[0].drag_target_view != state[1].drag_target_view);
      events.drop    = state[1].mouse_drag_drop;
      events.drag_over = state[1].mouse_dragging;
    }
    else if (state[0].drag_target_view == view.id()) [[unlikely]]
    {
      events.drag_out = state[1].mouse_dragging;
    }

    if (state[1].drag_source_view == view.id()) [[unlikely]]
    {
      events.drag_start = state[1].mouse_drag_start;
      events.dragging   = state[1].mouse_dragging;
      events.drag_end   = state[1].mouse_drag_end;
    }

    if (state[1].focused_view == view.id()) [[unlikely]]
    {
      events.focus_in   = state[0].focused_view != view.id();
      events.key_down   = state[1].keyboard_down;
      events.key_up     = state[1].keyboard_up;
      events.text_input = state[1].text_input;
    }
    else if (state[0].focused_view == view.id()) [[unlikely]]
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
    tab_indices.push(0).unwrap();
    is_hidden.push(false).unwrap();
    is_pointable.push(false).unwrap();
    is_clickable.push(false).unwrap();
    is_scrollable.push(false).unwrap();
    is_draggable.push(false).unwrap();
    is_droppable.push(false).unwrap();
    is_focusable.push(false).unwrap();
    is_text_input.push(false).unwrap();
    is_tab_input.push(false).unwrap();
    is_grab_focus.push(false).unwrap();
    is_lose_focus.push(false).unwrap();
  }

  void build_children(ViewContext const &ctx, View &view, u32 depth,
                      i32 &tab_index)
  {
    u32 const idx          = views.size32() - 1;
    u32 const first_child  = views.size32();
    u32       num_children = 0;

    auto builder = [&](View &child) {
      push_view(child, depth + 1, num_children++, idx);
    };

    ViewState state =
        view.tick(ctx, view.inner.region, process_events(view), fn(&builder));

    is_hidden.set(idx, state.hidden);
    is_pointable.set(idx, state.pointable);
    is_clickable.set(idx, state.clickable);
    is_scrollable.set(idx, state.scrollable);
    is_draggable.set(idx, state.draggable);
    is_droppable.set(idx, state.droppable);
    is_focusable.set(idx, state.focusable);
    is_text_input.set(idx, state.text_input);
    is_tab_input.set(idx, state.tab_input);
    is_grab_focus.set(idx, state.grab_focus);
    is_lose_focus.set(idx, state.lose_focus);
    tab_indices.set(idx, (state.tab == I32_MIN) ? tab_index : state.tab);

    nodes[idx].first_child  = first_child;
    nodes[idx].num_children = num_children;

    for (u32 c = first_child; c < (first_child + num_children); c++)
    {
      tab_index++;
      build_children(ctx, *views[c], depth + 1, tab_index);
    }
  }

  void build(ViewContext const &ctx, View *root)
  {
    if (root != nullptr)
    {
      push_view(*root, 0, 0, U32_MAX);
      i32 tab_index = 0;
      build_children(ctx, *root, 0, tab_index);
    }
  }

  void focus_order()
  {
    iota(span(tab_ordering), 0U);

    indirect_sort(span(tab_ordering), [&](u32 a, u32 b) {
      return tab_indices[a] < tab_indices[b];
    });
  }

  void layout(Vec2 viewport_size)
  {
    if (views.is_empty())
    {
      return;
    }

    u32 const n = views.size32();

    // allocate sizes to children recursively
    extents[0] = viewport_size;
    for (u32 i = 0; i < n; i++)
    {
      ViewNode const &node = nodes[i];
      views[i]->size(extents[i],
                     span(extents).slice(node.first_child, node.num_children));
    }

    // fit parent views along the finalized sizes of the child views and
    // assign centers to the children based on their sizes.
    for (u32 i = n; i != 0;)
    {
      i--;
      ViewNode const &node = nodes[i];
      extents[i]           = views[i]->fit(
          extents[i], span(extents).slice(node.first_child, node.num_children),
          span(centers).slice(node.first_child, node.num_children));
    }

    // convert from parent positions to absolute positions by recursive
    // translation
    for (u32 i = 0; i < n; i++)
    {
      ViewNode const &node = nodes[i];
      for (u32 c = node.first_child; c < (node.first_child + node.num_children);
           c++)
      {
        centers[c] += centers[i];
      }
    }

    // absolute re-positioning
    for (u32 i = 0; i < n; i++)
    {
      centers[i] =
          views[i]->position(CRect{.center = centers[i], .extent = extents[i]});
    }

    for (u32 i = 0; i < n; i++)
    {
      views[i]->inner.region.center = centers[i];
      views[i]->inner.region.extent = extents[i];
    }
  }

  void clip(Vec2 viewport_size)
  {
    if (views.is_empty())
    {
      return;
    }

    clips[0] = CRect{.center = {0, 0}, .extent = viewport_size};
    for (u32 i = 0; i < views.size32(); i++)
    {
      ViewNode const &node = nodes[i];
      clips[i]             = views[i]->clip(
          CRect{.center = centers[i], .extent = extents[i]}, clips[i]);
      fill(span(clips).slice(node.first_child, node.num_children), clips[i]);
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

  void visibility(Vec2 viewport_size)
  {
    for (u32 i = 0; i < views.size32(); i++)
    {
      ViewNode const &node = nodes[i];
      View const     &view = *views[i];

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
        bool const hidden =
            !view.inner.region.is_visible() || !clips[i].is_visible() ||
            !overlaps(view.inner.region,
                      CRect{.center = {0, 0}, .extent = viewport_size}) ||
            !overlaps(view.inner.region, clips[i]);

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
        view.render(view.inner.region, clips[i], canvas);
        view.inner.last_rendered_frame = frame;
      }
    }
  }

  void events(ViewContext const &ctx)
  {
    // [ ] render cursor & manage cursor
    // [ ] mouse/keyboard lose or gain focus

    state[0] = state[1];
    state[1] = {};

    u32 const n = views.size32();

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
                view.hit(view.inner.region, ctx.mouse.position)) [[unlikely]]
            {
              if (is_clickable[i])
              {
                state[1].mouse_dragging   = true;
                state[1].drag_source_view = view.id();
                state[1].mouse_dragging   = true;
                state[1].mouse_drag_start = true;
              }
              else
              {
                state[1].mouse_down         = true;
                state[1].mouse_pointed_view = view.id();
              }
              break;
            }
          }
        }
      }
      // mouse press events
      else if ((ctx.mouse.downs != MouseButtons::None ||
                ctx.mouse.ups != MouseButtons::None) &&
               !state[0].mouse_dragging)
      {
        for (u32 z_i = n; z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (is_clickable[i] &&
                view.hit(view.inner.region, ctx.mouse.position)) [[unlikely]]
            {
              state[1].mouse_down = ctx.mouse.downs != MouseButtons::None;
              state[1].mouse_up   = ctx.mouse.ups != MouseButtons::None;
              state[1].mouse_pointed_view = view.id();
              break;
            }
          }
        }
      }
      // mouse dragging update event
      else if (ctx.mouse.moved && state[0].mouse_dragging)
      {
        state[1].drag_source_view = state[0].drag_source_view;
        state[1].mouse_dragging   = true;
        for (u32 z_i = n; z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (is_droppable[i] &&
                view.hit(view.inner.region, ctx.mouse.position)) [[unlikely]]
            {
              state[1].drag_target_view = view.id();
              break;
            }
          }
        }
      }
      // mouse drop event
      else if (has_bits(ctx.mouse.ups, MouseButtons::Primary) &&
               state[0].mouse_dragging)
      {
        state[1].drag_source_view = state[1].drag_source_view;
        state[1].mouse_drag_drop  = true;
        state[1].mouse_dragging   = true;
        for (u32 z_i = n; z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (is_droppable[i] &&
                view.hit(view.inner.region, ctx.mouse.position)) [[unlikely]]
            {
              state[1].drag_target_view = view.id();
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
                view.hit(view.inner.region, ctx.mouse.position)) [[unlikely]]
            {
              state[1].mouse_scrolling_view = view.id();
              state[1].mouse_scrolled       = true;
              break;
            }
          }
        }
      }
      // pointing event
      else if (!state[0].mouse_dragging)
      {
        for (u32 z_i = n; z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (is_pointable[i] &&
                view.hit(view.inner.region, ctx.mouse.position)) [[unlikely]]
            {
              state[1].mouse_pointed_view = view.id();
              break;
            }
          }
        }
      }
    }

    for (u32 i = 0; i < n; i++)
    {
      View const &view = *views[i];
      if (is_grab_focus[i])
      {
        state[1].focused_view = view.id();
        state[1].text_input   = is_text_input[i];
        state[1].tab_input    = is_tab_input[i];
        break;
      }
      else if (view.id() == state[1].focused_view && is_lose_focus[i])
      {
        state[1].focused_view = 0;
        state[1].text_input   = false;
        state[1].tab_input    = false;
        break;
      }
    }

    state[1].keyboard_down = ctx.keyboard.down;
    state[1].keyboard_up   = ctx.keyboard.up;

    // [ ] focus widget on clicked

    // [ ] grab attention would need to scroll down to widget; would need
    // virtual scrolling support. offset based?
    // [ ] action on click of a focusable widget, f
    //
    // [ ] on click or focus of focusable objects, system requests keyboard
    // input if object has a text area attribute
    //
    // [ ] viewport child focus and key navigation?
    //
    //
    // [ ] if a click occured; and clicked widget is not the currently
    // keyboard focused widget then lose keyboard focus for the widget; or
    // esc key is pressed.
    //
    // [ ] we need to preserve tab index upon focus removal (i.e. esc or outer
    // click)
    //
    //
    state[1].focused_view;

    FocusDirection focus_direction = FocusDirection::None;

    if (!state[1].tab_input && ctx.key_state(KeyCode::Tab))
    {
      focus_direction =
          (ctx.key_state(KeyCode::LShift) || ctx.key_state(KeyCode::RShift)) ?
              FocusDirection::Backward :
              FocusDirection::Forward;
    }

    switch (focus_direction)
    {
      case FocusDirection::None:
      {
        // if currently focused widget is removed or hidden, change focus to
        // None
        if (state[0].focus >= n || is_hidden[tab_ordering[state[0].focus]])
        {
          state[1].focus = U32_MAX;
        }
      }
      break;

      case FocusDirection::Forward:
      {
        // if none is focused, move to first focusable widget
        if (state[0].focus == U32_MAX || state[0].focus >= n ||
            is_hidden[tab_ordering[state[0].focus]])
        {
          state[1].focus = U32_MAX;
          for (u32 i = 0; i < n; i++)
          {
            if (!is_hidden[tab_ordering[i]] && is_focusable[tab_ordering[i]])
            {
              state[1].focus = i;
              break;
            }
          }
        }
        else
        {
          // if widget is focused and visible; move to next/prev if any,
          // otherwise stay
          for (u32 i = state[0].focus + 1; i < n; i++)
          {
            if (!is_hidden[tab_ordering[i]] && is_focusable[tab_ordering[i]])
            {
              state[1].focus = i;
              break;
            }
          }
        }
      }
      break;

      case FocusDirection::Backward:
      {
        if (state[0].focus == U32_MAX || state[0].focus >= n ||
            is_hidden[tab_ordering[state[0].focus]])
        {
          state[1].focus = U32_MAX;
          for (u32 i = n; i > 0;)
          {
            i--;
            if (!is_hidden[tab_ordering[i]] && is_focusable[tab_ordering[i]])
            {
              state[1].focus = i;
              break;
            }
          }
        }
        else
        {
          for (u32 i = state[0].focus; i > 0;)
          {
            i--;
            if (!is_hidden[tab_ordering[i]] && is_focusable[tab_ordering[i]])
            {
              state[1].focus = i;
              break;
            }
          }
        }
      }
      break;

      default:
        break;
    }
  }

  void tick(ViewContext const &ctx, View *root, Canvas &canvas)
  {
    clear();
    build(ctx, root);
    u32 const n = views.size32();
    centers.resize_uninit(n).unwrap();
    extents.resize_uninit(n).unwrap();
    clips.resize_uninit(n).unwrap();
    z_indices.resize_uninit(n).unwrap();
    stacking_contexts.resize_uninit(n).unwrap();
    z_ordering.resize_uninit(n).unwrap();
    tab_ordering.resize_uninit(n).unwrap();

    focus_order();
    layout(ctx.viewport_size);
    clip(ctx.viewport_size);
    stack();
    visibility(ctx.viewport_size);
    render(canvas);
    events(ctx);

    frame++;
  }
};

}        // namespace ash