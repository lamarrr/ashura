/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/engine/view.h"
#include "ashura/std/error.h"
#include "ashura/std/range.h"

namespace ash
{

/// @brief flattened hierarchical tree node, all siblings are packed
/// sequentially.
/// This only represents the parent node.
/// Since the tree is rebuilt from scratch every time, the order is preserved in
/// that parents always come before children.
struct ViewNode
{
  u32 first_child  = 0;
  u32 num_children = 0;
};

struct ViewSystem
{
  ViewContext    ctx           = {};
  uid            next_id       = 0;
  Vec<View *>    views         = {};
  Vec<ViewState> states        = {};
  Vec<ViewNode>  nodes         = {};
  Vec<Vec2>      sizes         = {};
  Vec<Vec2>      positions     = {};
  Vec<CRect>     clips         = {};
  Vec<i32>       z_indices     = {};
  Vec<u32>       layered       = {};
  Vec2           viewport_size = {};

  void insert_subview(View *parent)
  {
    u32 const first_child  = views.size32();
    u32       num_children = 0;
    while (true)
    {
      View *child = parent->child(num_children);
      if (child == nullptr)
      {
        break;
      }
      views.push(child).unwrap();
      num_children++;
      CHECK(num_children < U32_MAX);
    }
    nodes
        .push(
            ViewNode{.first_child = first_child, .num_children = num_children})
        .unwrap();

    for (u32 i = 0; i < num_children; i++)
    {
      insert_subview(views[first_child + i]);
    }
  }

  void allocate_ids()
  {
    for (View *w : views)
    {
      if (w->inner.id == UID_MAX)
      {
        w->inner.id = next_id++;
        // [ ] MOUNT
      }
    }
    CHECK(next_id != UID_MAX);
  }

  void tick(nanoseconds dt)
  {
    // TODO(lamarrr)
    // process events across views, hit-test, dispatch events
    //
    //
    //
    //
    // [ ] SDL_SetCursor()
    //     SDL_CreateSystemCursor(); - all created at startup
    //     SDL_HideCursor();
    //     SDL_ShowCursor();
    //
    //
    // [ ] Update timestamp and dt
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
    for (u32 i = 0; i < views.size32(); i++)
    {
      states[i] = views[i]->tick(
          ctx, CRect{.center = positions[i], .extent = sizes[i]}, ViewEvents{});
    }
  }

  void layout()
  {
    sizes[0]            = viewport_size;
    positions[0]        = Vec2{0, 0};
    u32 const num_views = views.size32();
    for (u32 i = 0; i < num_views; i++)
    {
      // allocate sizes to children
      ViewNode const &node = nodes[i];
      views[i]->size(sizes[i],
                     span(sizes).slice(node.first_child, node.num_children));
    }

    for (u32 i = 0; i < num_views; i++)
    {
      // fit parent views along the finalized sizes of the child views and
      // assign positions to the children based on their sizes.
      ViewNode const &node = nodes[i];
      sizes[i]             = views[i]->fit(
          sizes[i], span(sizes).slice(node.first_child, node.num_children),
          span(positions).slice(node.first_child, node.num_children));
    }

    for (u32 i = 0; i < num_views; i++)
    {
      // convert from parent positions to absolute positions by recursive
      // translation
      ViewNode const &node = nodes[i];
      for (Vec2 &pos :
           span(positions).slice(node.first_child, node.num_children))
      {
        pos += positions[i];
      }
    }

    // allow views to pop out of their parents
    for (u32 i = 0; i < num_views; i++)
    {
      positions[i] =
          views[i]->position(CRect{.center = positions[i], .extent = sizes[i]});
    }
  }

  void stack()
  {
    z_indices[0] = 0;
    for (u32 i = 0; i < views.size32(); i++)
    {
      ViewNode const &node = nodes[i];
      z_indices[i]         = views[i]->stack(
          z_indices[i],
          span(z_indices).slice(node.first_child, node.num_children));
    }
  }

  void clip()
  {
    clips[0] = CRect{.center = {0, 0}, .extent = viewport_size};
    for (u32 i = 0; i < views.size32(); i++)
    {
      ViewNode const &node = nodes[i];
      clips[i]             = views[i]->clip(
          CRect{.center = positions[i], .extent = sizes[i]}, clips[i]);
      fill(span(clips).slice(node.first_child, node.num_children), clips[i]);
    }
  }

  void sort_layers()
  {
    iota(span(layered), 0U);
    indirect_sort(span(z_indices), span(layered),
                  [](i32 za, i32 zb) { return za < zb; });
  }

  void visibility()
  { /*
     for (u32 i = 0; i < views.size32(); i++)
     {
       ViewNode const &node = nodes[i];
       // if parent not visibile. make children not visible
       if (!has_bits(attributes[i], ViewAttributes::Visible))
       {
         for (ViewAttributes &attr :
              span(attributes).slice(node.first_child, node.num_children))
         {
           attr = attr & ~ViewAttributes::Visible;
         }
       }
     }*/
  }

  void render(Canvas &canvas)
  {
    /*
    for (u32 i : layered)
    {
      canvas.clip(clips[i]);
      if (has_bits(attributes[i], ViewAttributes::Visible))
      {
        views[i]->render(CRect{.center = positions[i], .extent = sizes[i]},
                         canvas);
      }
    }*/
  }

  void frame(View *root, Canvas &canvas, nanoseconds dt)
  {
    views.clear();
    if (root != nullptr)
    {
      views.push(root).unwrap();
      insert_subview(root);
    }
    allocate_ids();
    u32 const num_views = views.size32();
    states.resize_uninitialized(num_views).unwrap();
    sizes.resize_uninitialized(num_views).unwrap();
    positions.resize_uninitialized(num_views).unwrap();
    clips.resize_uninitialized(num_views).unwrap();
    z_indices.resize_uninitialized(num_views).unwrap();
    layered.resize_uninitialized(num_views).unwrap();

    // child()
    // - hashmap of ids to node and pointers need to be built every frame
    // tick() => S
    // size()          => S
    // fit()           => S
    // position()      => S
    // stack() => S
    // clip()          => S
    // render()        => S
    // hit()
    //

    tick(dt);
    layout();
    stack();
    clip();
    sort_layers();
    visibility();
    render(canvas);
  }
};

}        // namespace ash