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

struct ViewSystem
{
  u64           next_id           = 1;
  Vec<View *>   views             = {};
  Vec<ViewNode> nodes             = {};
  Vec<Vec2>     centers           = {};
  Vec<Vec2>     extents           = {};
  Vec<CRect>    clips             = {};
  BitVec<u64>   is_hidden         = {};
  Vec<i32>      z_indices         = {};
  Vec<i32>      stacking_contexts = {};
  Vec<u32>      z_ordering        = {};

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

  ViewEvents process_events(View *view)
  {
    ViewEvents events;

    if (view->inner.id == 0)
    {
      CHECK(next_id != U64_MAX);
      view->inner.id = next_id++;
      events.mounted = true;
    }

    // [ ] process other events

    return events;
  }

  void build_recursive(ViewContext const &ctx, View *parent, u32 depth = 0)
  {
    u32 const first_child = views.size32();

    auto builder = [&](View *sub) { views.push(sub).unwrap(); };

    parent->inner.state = parent->tick(ctx, parent->inner.region,
                                       process_events(parent), fn(&builder));

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
    u32 const num_views = views.size32();

    // allocate sizes to children recursively
    views[0]->inner.region.extent = viewport_size;
    for (u32 i = 0; i < num_views; i++)
    {
      ViewNode const &node = nodes[i];
      View           *view = views[i];
      view->size(view->inner.region.extent,
                 span(extents).slice(node.first_child, node.num_children));
      for (u32 c = node.first_child; c < (node.first_child + node.num_children);
           c++)
      {
        views[c]->inner.region.extent = extents[c];
      }
    }

    // fit parent views along the finalized sizes of the child views and
    // assign centers to the children based on their sizes.
    for (u32 i = num_views; i != 0;)
    {
      i--;
      ViewNode const &node          = nodes[i];
      views[i]->inner.region.extent = extents[i] = views[i]->fit(
          extents[i], span(extents).slice(node.first_child, node.num_children),
          span(centers).slice(node.first_child, node.num_children));
      for (u32 c = node.first_child; c < (node.first_child + node.num_children);
           c++)
      {
        views[c]->inner.region.center = centers[c];
      }
    }

    // convert from parent centers to absolute centers by recursive
    // translation
    for (u32 i = 0; i < num_views; i++)
    {
      ViewNode const &node = nodes[i];
      for (u32 c = node.first_child; c < (node.first_child + node.num_children);
           c++)
      {
        centers[c] += centers[i];
        views[c]->inner.region.center = centers[c];
      }
    }

    // absolute re-positioning
    for (u32 i = 0; i < num_views; i++)
    {
      centers[i] =
          views[i]->position(CRect{.center = centers[i], .extent = extents[i]});
      views[i]->inner.region.center = centers[i];
    }
  }

  void clip(Vec2 viewport_size)
  {
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
      View           *view = views[i];
      bool const      hidden =
          view->inner.state.hidden || !view->inner.region.is_visible() ||
          !clips[i].is_visible() ||
          !overlaps(view->inner.region,
                    CRect{.center = {0, 0}, .extent = viewport_size}) ||
          !overlaps(view->inner.region, clips[i]);

      is_hidden.set(i, hidden);

      // if parent hidden by itself. make children hidden
      if (view->inner.state.hidden) [[unlikely]]
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
        View *view = views[i];
        view->render(view->inner.region, clips[i], canvas);
      }
    }
  }

  void events(ViewContext const &ctx)
  {
    struct ViewState
    {
      u64 prev_focused  = 0;
      u64 prev_pressed  = 0;
      u64 prev_hovered  = 0;
      u64 prev_dragging = 0;
      u64 focus_out     = 0;
      u64 focused       = 0;
      u64 hovered       = 0;
      u64 pressed       = 0;
    } view;

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

  void tick(ViewContext const &ctx, View *root, Canvas &canvas,
            Vec2 viewport_size)
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
    // [ ] mouse over, mouse leave
    // [ ] view hit, view miss -
    // [ ] keyboard input: use another system?
    // [ ] clip board copy & paste with custom media format
    // [ ] text input
    // [ ] gamepad input: use another system?
    // [ ] hit testing on clipped rects
    // [ ] focus model (keymap navigation Tab to move focus backwards, Shift +
    // Tab to move focus forwards)
    // [ ] scroll on child focus for viewports
    // [ ] on click or focus of focusable objects, system requests keyboard
    // input if object has a text area attribute
    // [ ] cursor management via hit testing
    // [ ] window hit testing
    // [ ] draw on focus
    // [ ] context menu support when right-clicked?
    // [ ] non-clickable should not receive pointer events
    // [ ] ? by default, special
    // non-text keys will always be forwarded, to reject keys, i.e. prevent tab
    // from navigating. make PgUp have special meaning within viewport, etc.
    // [ ] viewport child focus and key navigation?
    //
    //
    //- process events by diffing with previous frame
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

    views.clear();
    nodes.clear();
    build(ctx, root);
    u32 const num_views = views.size32();
    centers.resize_uninitialized(num_views).unwrap();
    extents.resize_uninitialized(num_views).unwrap();
    clips.resize_uninitialized(num_views).unwrap();
    is_hidden.resize_uninitialized(num_views).unwrap();
    fill(is_hidden.repr_, 0);
    z_indices.resize_uninitialized(num_views).unwrap();
    stacking_contexts.resize_uninitialized(num_views).unwrap();
    z_ordering.resize_uninitialized(num_views).unwrap();
    iota(span(z_ordering), 0U);

    if (num_views > 0)
    {
      layout(viewport_size);
      clip(viewport_size);
      stack();
      visibility(viewport_size);
      render(canvas);
      events(ctx);
      /// [ ] mouse active ? (mouse down? click test :  hover test)
      /// [ ] key pressed ? (is_tab : current accepts tab? , is in input mode?
      /// focused widget? navigate, otherwise input) [ ] hit testing, process
      /// states, how long should states be preserved? what [ ] if widget is
      /// removed? [ ] render cursor [ ] context menus?
    }
  }
};

}        // namespace ash