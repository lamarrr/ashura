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
struct ViewNode
{
  u32 depth        = 0;
  u32 parent       = U32_MAX;
  u32 first_child  = 0;
  u32 num_children = 0;
};

struct ViewSystemState
{
  u64  mouse_focused_view      = 0;
  u64  mouse_dragging_view     = 0;
  u64  mouse_hovered_view      = 0;
  u64  mouse_scroll_focus_view = 0;
  u64  keyboard_focused_view   = 0;
  bool mouse_down : 1          = false;
  bool mouse_up : 1            = false;
  bool mouse_moved : 1         = false;
  bool mouse_scrolled : 1      = false;
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

  ViewEvents process_events(ViewContext const &ctx, View &view)
  {
    ViewEvents events;

    if (view.id() == 0) [[unlikely]]
    {
      CHECK(last_id != U64_MAX);
      view.inner.id  = ++last_id;
      events.mounted = true;
    }

    events.view_hit = (view.inner.last_rendered_frame == (frame - 1));

    if (state.mouse_focused_view == view.id()) [[unlikely]]
    {
      events.mouse_in      = (previous_state.mouse_focused_view != view.id());
      events.mouse_down    = state.mouse_down;
      events.mouse_up      = state.mouse_up;
      events.mouse_pressed = previous_state.mouse_down && state.mouse_up &&
                             (previous_state.mouse_focused_view == view.id());
      events.mouse_moved =
          state.mouse_moved && (previous_state.mouse_focused_view == view.id());
    }
    else if (previous_state.mouse_focused_view == view.id()) [[unlikely]]
    {
      events.mouse_out = true;
    }

    if (state.mouse_scroll_focus_view == view.id() && state.mouse_scrolled)
        [[unlikely]]
    {
      events.mouse_scroll = true;
    }

    // [ ] drag acceptance ? drag via different buttons?
    // [ ] drag start
    // [ ] dragging
    // [ ] drag_end
    // [ ] drag_in
    // [ ] drag_out
    // [ ] drag_over
    // [ ] drop

    if (state.keyboard_focused_view == view.id()) [[unlikely]]
    {
      events.focus_in    = previous_state.keyboard_focused_view != view.id();
      events.key_down    = state.keyboard_down;
      events.key_up      = state.keyboard_up;
      events.key_pressed = previous_state.keyboard_down && state.keyboard_up &&
                           (previous_state.keyboard_focused_view == view.id());
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

    parent->inner.state = parent->tick(
        ctx, parent->inner.region, process_events(ctx, *parent), fn(&builder));

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

      // [ ] should be OR?
      is_hidden.set(i, hidden);

      // if parent hidden by itself. make children hidden
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
    /// [ ] mouse active ? (mouse down? click test :  hover test)
    /// [ ] key pressed ? (is_tab : current accepts tab? , is in input mode?
    /// focused widget? navigate, otherwise input)
    ///
    /// [ ] hit testing, process
    /// states, how long should states be preserved? what
    ///
    /// [ ] if widget is
    /// removed?
    ///
    /// [ ] render cursor
    ///
    /// [ ] context menus?

    if (ctx.mouse.out)
    {
      //
    }
    else if (ctx.mouse.pointed)
    {
      //
      if (ctx.mouse.in)
      {
      }
    }

    if (ctx.keyboard.out)
    {
      //
    }
    else if (ctx.keyboard.focused)
    {
      //
      if (ctx.keyboard.in)
      {
      }
    }
  }

  void tick(ViewContext const &ctx, View *root, Canvas &canvas)
  {
    // [ ] process events across views, hit-test, dispatch events
    //
    //
    // [ ] SDL_SetCursor()
    //     SDL_CreateSystemCursor(); - all created at startup
    //     SDL_HideCursor();
    //     SDL_ShowCursor();
    //
    // [ ] UI tick rate (time-based/adaptive frame rate)
    // [ ] click - forward directly unless draggable
    // [ ] focus and keyboard management
    // [ ] view drag & drop
    // [ ] keyboard input: use another system?
    // [ ] clip board copy & paste with custom media format
    // [ ] text input
    // [ ] gamepad input: use another system?
    // [ ] hit testing on clipped rects
    // [ ] focus model (keymap navigation Tab to move focus backwards, Shift +
    /// Tab to move focus forwards) - we can follow along the tree and allow
    /// widgets to specify integers of focus direction of their children
    ///
    // [ ] scroll on child focus for viewports
    // [ ] on click or focus of focusable objects, system requests keyboard
    // input if object has a text area attribute
    // [ ] cursor management via hit testing
    // [ ] window hit testing
    // [ ] context menu support when right-clicked?
    // [ ] non-clickable should not receive pointer events
    // [ ] ? by default, special
    // non-text keys will always be forwarded, to reject keys, i.e. prevent tab
    // from navigating. make PgUp have special meaning within viewport, etc.
    // [ ] viewport child focus and key navigation?
    //
    //
    //
    // [ ] after all are built and positioned, we need to process events based
    // on the final tree
    // [ ] need to store whether view was hit? and reset after reading.
    // need event store to store: last focused widget, last pressed widget, last
    // hovered widget, last entered widget, last dragged widget, last dropped
    // widget,
    // [ ] how will states be persisted across frames?
    //
    frame++;

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
  }
};

}        // namespace ash