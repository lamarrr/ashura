/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/error.h"
#include "ashura/std/range.h"

namespace ash
{

/// @brief flattened hierarchical tree node, all siblings are packed
/// sequentially to the right of the parent.
/// This only represents the parent node.
/// Since the tree is rebuilt from scratch every time, the order is preserved in
/// that parents always come before children.
/// @param depth depth of the tree this node belongs to. there's ever only one
/// node at depth 0: the root node.
struct ViewNode
{
  u32 depth        = 0;
  u32 parent       = U32_MAX;
  u32 first_child  = U32_MAX;
  u32 num_children = 0;
  u32 prev_sibling = U32_MAX;
  u32 next_sibling = U32_MAX;
};

struct ViewSystemState
{
  u64  mouse_focused_view      = 0;
  u64  mouse_hovered_view      = 0;
  u64  mouse_scroll_focus_view = 0;
  u64  drag_source_view        = 0;
  u64  drag_target_view        = 0;
  u64  keyboard_focused_view   = 0;
  bool mouse_down : 1          = false;
  bool mouse_up : 1            = false;
  bool mouse_moved : 1         = false;
  bool mouse_scrolled : 1      = false;
  bool mouse_drag_start : 1    = false;
  bool mouse_drag_end : 1      = false;
  bool mouse_dragging : 1      = false;
  bool mouse_drag_drop : 1     = false;
  bool keyboard_down : 1       = false;
  bool keyboard_up : 1         = false;
  bool text_input : 1          = false;
};

struct ViewSystem
{
  u64             frame             = 0;
  ViewSystemState state             = {};
  ViewSystemState previous_state    = {};
  u64             last_id           = 0;
  Vec<View *>     views             = {};
  Vec<ViewNode>   nodes             = {};
  Vec<Vec2>       centers           = {};
  Vec<Vec2>       extents           = {};
  Vec<CRect>      clips             = {};
  BitVec<u64>     is_hidden         = {};
  Vec<i32>        z_indices         = {};
  Vec<i32>        stacking_contexts = {};
  Vec<u32>        z_ordering        = {};

  void reset()
  {
    views.reset();
    nodes.reset();
    centers.reset();
    extents.reset();
    clips.reset();
    is_hidden.reset();
    z_indices.reset();
    stacking_contexts.reset();
    z_ordering.reset();
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

    if (state.mouse_focused_view == view.id()) [[unlikely]]
    {
      events.mouse_in   = (previous_state.mouse_focused_view != view.id());
      events.mouse_down = state.mouse_down;
      events.mouse_up   = state.mouse_up;
      events.mouse_moved =
          state.mouse_moved && (previous_state.mouse_focused_view == view.id());
    }
    else if (previous_state.mouse_focused_view == view.id()) [[unlikely]]
    {
      events.mouse_out = true;
    }

    if (state.mouse_scroll_focus_view == view.id()) [[unlikely]]
    {
      events.mouse_scroll = state.mouse_scrolled;
    }

    if (state.drag_target_view == view.id()) [[unlikely]]
    {
      events.drag_in =
          (previous_state.drag_target_view != state.drag_target_view);
      events.drop      = state.mouse_drag_drop;
      events.drag_over = state.mouse_dragging;
    }
    else if (previous_state.drag_target_view == view.id()) [[unlikely]]
    {
      events.drag_out = state.mouse_dragging;
    }

    if (state.drag_source_view == view.id()) [[unlikely]]
    {
      events.drag_start = state.mouse_drag_start;
      events.dragging   = state.mouse_dragging;
      events.drag_end   = state.mouse_drag_end;
    }

    if (state.keyboard_focused_view == view.id()) [[unlikely]]
    {
      events.focus_in   = previous_state.keyboard_focused_view != view.id();
      events.key_down   = state.keyboard_down;
      events.key_up     = state.keyboard_up;
      events.text_input = state.text_input;
    }
    else if (previous_state.keyboard_focused_view == view.id()) [[unlikely]]
    {
      events.focus_out = true;
    }

    return events;
  }

  void build_recursive(ViewContext const &ctx, View *parent, u32 depth = 0)
  {
    u32 const first_child = views.size32();

    auto builder = [&](View &child) { views.push(&child).unwrap(); };

    parent->inner.state = parent->tick(ctx, parent->inner.region,
                                       process_events(*parent), fn(&builder));

    u32 const num_children = views.size32() - first_child;
    u32 const node_idx     = nodes.size32();
    nodes
        .push(ViewNode{.depth        = depth,
                       .first_child  = first_child,
                       .num_children = num_children})
        .unwrap();

    for (u32 c = first_child; c < (first_child + num_children); c++)
    {
      build_recursive(ctx, views[c], depth + 1);
      nodes[c].parent = node_idx;
      // [ ] next_sibling & prev_sibling
    }
  }

  void build(ViewContext const &ctx, View *root)
  {
    if (root != nullptr)
    {
      views.push(root).unwrap();
      build_recursive(ctx, root, 0);
    }
  }

  void layout(Vec2 viewport_size)
  {
    if (views.is_empty())
    {
      return;
    }

    u32 const num_views = views.size32();

    // allocate sizes to children recursively
    extents[0] = viewport_size;
    for (u32 i = 0; i < num_views; i++)
    {
      ViewNode const &node = nodes[i];
      views[i]->size(extents[i],
                     span(extents).slice(node.first_child, node.num_children));
    }

    // fit parent views along the finalized sizes of the child views and
    // assign centers to the children based on their sizes.
    for (u32 i = num_views; i != 0;)
    {
      i--;
      ViewNode const &node = nodes[i];
      extents[i]           = views[i]->fit(
          extents[i], span(extents).slice(node.first_child, node.num_children),
          span(centers).slice(node.first_child, node.num_children));
    }

    // convert from parent positions to absolute positions by recursive
    // translation
    for (u32 i = 0; i < num_views; i++)
    {
      ViewNode const &node = nodes[i];
      for (u32 c = node.first_child; c < (node.first_child + node.num_children);
           c++)
      {
        centers[c] += centers[i];
      }
    }

    // absolute re-positioning
    for (u32 i = 0; i < num_views; i++)
    {
      centers[i] =
          views[i]->position(CRect{.center = centers[i], .extent = extents[i]});
    }

    for (u32 i = 0; i < num_views; i++)
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
      View           &view = *views[i];
      bool const      hidden =
          view.inner.state.hidden || !view.inner.region.is_visible() ||
          !clips[i].is_visible() ||
          !overlaps(view.inner.region,
                    CRect{.center = {0, 0}, .extent = viewport_size}) ||
          !overlaps(view.inner.region, clips[i]);

      is_hidden.set(i, hidden || is_hidden[i]);

      // if parent requested to be hidden, make children hidden
      if (view.inner.state.hidden) [[unlikely]]
      {
        for (u32 c = node.first_child;
             c < (node.first_child + node.num_children); c++)
        {
          is_hidden.set(c, true);
        }
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
    // [ ] if widget is removed?
    // [ ] render cursor & manage cursor
    // [ ] UI tick rate (time-based/adaptive frame rate), with custom frequency
    // allowed, need to be able to merge inputs?
    //
    // [ ] can clicking be handled in the widget? i.e.
    // using ClickDetector that checks time interval and based on some
    // time or debouncing parameters
    //
    // [ ] state continuation? i.e. dragging
    // [ ] mouse/keyboard lose or gain focus

    ViewSystemState new_state;

    if (ctx.mouse.focused)
    {
      // mouse click & drag
      if (has_bits(ctx.mouse.downs, MouseButtons::Primary))
      {
        for (u32 z_i = z_ordering.size32(); z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if ((view.inner.state.clickable || view.inner.state.draggable) &&
                view.hit(view.inner.region, ctx.mouse.position))
            {
              if (view.inner.state.draggable)
              {
                new_state.mouse_dragging   = true;
                new_state.drag_source_view = view.id();
                new_state.mouse_dragging   = true;
                new_state.mouse_drag_start = true;
              }
              else
              {
                new_state.mouse_down         = true;
                new_state.mouse_focused_view = view.id();
              }
              break;
            }
          }
        }
      }
      // mouse press events
      else if ((ctx.mouse.downs != MouseButtons::None ||
                ctx.mouse.ups != MouseButtons::None) &&
               !state.mouse_dragging)
      {
        for (u32 z_i = z_ordering.size32(); z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (view.inner.state.clickable &&
                view.hit(view.inner.region, ctx.mouse.position))
            {
              new_state.mouse_down = ctx.mouse.downs != MouseButtons::None;
              new_state.mouse_up   = ctx.mouse.ups != MouseButtons::None;
              new_state.mouse_focused_view = view.id();
              break;
            }
          }
        }
      }
      // mouse dragging update event
      else if (ctx.mouse.moved && state.mouse_dragging)
      {
        new_state.drag_source_view = previous_state.drag_source_view;
        new_state.mouse_dragging   = true;
        // how is dropping accepted or rejected?
        for (u32 z_i = z_ordering.size32(); z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (view.inner.state.droppable &&
                view.hit(view.inner.region, ctx.mouse.position))
            {
              new_state.drag_target_view = view.id();
              break;
            }
          }
        }
      }
      // mouse drop event
      else if (has_bits(ctx.mouse.ups, MouseButtons::Primary) &&
               state.mouse_dragging)
      {
        // cancelation and acceptance
        // new_state.mouse_drag_drop = true;
      }
      // mouse scroll event
      else if (ctx.mouse.wheel_scrolled)
      {
        for (u32 z_i = z_ordering.size32(); z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (view.inner.state.scrollable &&
                view.hit(view.inner.region, ctx.mouse.position))
            {
              new_state.mouse_scroll_focus_view = view.id();
              new_state.mouse_scrolled          = true;
              break;
            }
          }
        }
      }
      // pointing event
      else if (!state.mouse_dragging)
      {
        for (u32 z_i = z_ordering.size32(); z_i != 0;)
        {
          z_i--;
          u32 i = z_ordering[z_i];
          if (!is_hidden[i])
          {
            View &view = *views[i];
            if (view.inner.state.pointable &&
                view.hit(view.inner.region, ctx.mouse.position))
            {
              new_state.mouse_hovered_view = view.id();
              break;
            }
          }
        }
      }
    }

    // [ ] focus model (keymap navigation Tab to move focus backwards, Shift +
    // Tab to move focus forwards) - we can follow along the tree and allow
    // widgets to specify integers of focus direction of their children
    //
    // [ ] on click or focus of focusable objects, system requests keyboard
    // input if object has a text area attribute
    // [ ] ? by default, special non-text keys will always be forwarded, to
    // reject keys, i.e. prevent tab from navigating. make PgUp have special
    // meaning within viewport, etc.
    //
    // [ ] viewport child focus and key navigation?
    // [ ] focus navigation logic
    // [ ] focus request
    // [ ] key pressed ? (is_tab : current accepts tab? , is in input mode?
    // focused widget? navigate, otherwise input)
    //

    // if only tab down, it shouldn't be an input??? it should
    new_state.keyboard_down = ctx.keyboard.down;
    new_state.keyboard_up   = ctx.keyboard.up;

    // focus nav
  }

  void tick(ViewContext const &ctx, View *root, Canvas &canvas)
  {
    views.clear();
    nodes.clear();
    build(ctx, root);
    u32 const num_views = views.size32();
    centers.resize_uninitialized(num_views).unwrap();
    extents.resize_uninitialized(num_views).unwrap();
    clips.resize_uninitialized(num_views).unwrap();
    is_hidden.resize_uninitialized(num_views).unwrap();
    fill(is_hidden.repr(), 0U);
    z_indices.resize_uninitialized(num_views).unwrap();
    stacking_contexts.resize_uninitialized(num_views).unwrap();
    z_ordering.resize_uninitialized(num_views).unwrap();
    iota(span(z_ordering), 0U);

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