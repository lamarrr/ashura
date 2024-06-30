#pragma once

#include "ashura/engine/widget.h"
#include "ashura/std/error.h"
#include "ashura/std/range.h"

namespace ash
{

/// @brief flattened hierarchical tree node, all siblings are packed
/// sequentially.
/// This only represents the parent node.
/// Since the tree is rebuilt from scratch every time, the order is preserved in
/// that parents always come before children.
struct WidgetNode
{
  u32 first_child  = 0;
  u32 num_children = 0;
};

struct WidgetSystem
{
  WidgetContext         ctx           = {};
  uid                   next_id       = 0;
  Vec<Widget *>         widgets       = {};
  Vec<WidgetAttributes> attributes    = {};
  Vec<WidgetNode>       nodes         = {};
  Vec<Vec2>             sizes         = {};
  Vec<Vec2>             positions     = {};
  Vec<CRect>            clips         = {};
  Vec<i32>              z_indices     = {};
  Vec<u32>              layered       = {};
  Vec2                  viewport_size = {};

  void insert_children(Widget *parent)
  {
    u32 const first_child  = (u32) widgets.size();
    u32       num_children = 0;
    while (true)
    {
      Widget *child = parent->child(num_children);
      if (child == nullptr)
      {
        break;
      }
      CHECK(widgets.push(child));
      num_children++;
      CHECK(num_children < U32_MAX);
    }
    CHECK(nodes.push(
        WidgetNode{.first_child = first_child, .num_children = num_children}));

    for (u32 i = 0; i < num_children; i++)
    {
      insert_children(widgets[first_child + i]);
    }
  }

  void allocate_ids()
  {
    for (Widget *w : widgets)
    {
      if (w->id == UID_MAX)
      {
        w->id = next_id++;
      }
    }
    CHECK(next_id != UID_MAX);
  }

  void attribute()
  {
    for (u32 i = 0; i < (u32) widgets.size(); i++)
    {
      attributes[i] = widgets[i]->attributes();
    }
  }

  void layout()
  {
    sizes[0]              = viewport_size;
    positions[0]          = Vec2{0, 0};
    u32 const num_widgets = (u32) widgets.size();
    for (u32 i = 0; i < num_widgets; i++)
    {
      // allocate sizes to children
      WidgetNode const &node = nodes[i];
      widgets[i]->size(
          sizes[i], to_span(sizes).slice(node.first_child, node.num_children));
    }

    for (u32 i = 0; i < num_widgets; i++)
    {
      // fit parent widgets along the finalized sizes of the child widgets and
      // assign positions to the children based on their sizes.
      WidgetNode const &node = nodes[i];
      sizes[i]               = widgets[i]->fit(
          sizes[i], to_span(sizes).slice(node.first_child, node.num_children),
          to_span(positions).slice(node.first_child, node.num_children));
    }

    for (u32 i = 0; i < num_widgets; i++)
    {
      // convert from parent positions to absolute positions by recursive
      // translation
      WidgetNode const &node = nodes[i];
      for (Vec2 &pos :
           to_span(positions).slice(node.first_child, node.num_children))
      {
        pos += positions[i];
      }
    }

    // allow widgets to pop out of their parents
    for (u32 i = 0; i < num_widgets; i++)
    {
      positions[i] = widgets[i]->position(
          CRect{.center = positions[i], .extent = sizes[i]});
    }
  }

  void stack()
  {
    z_indices[0] = 0;
    for (u32 i = 0; i < (u32) widgets.size(); i++)
    {
      WidgetNode const &node = nodes[i];
      z_indices[i]           = widgets[i]->stack(
          z_indices[i],
          to_span(z_indices).slice(node.first_child, node.num_children));
    }
  }

  void clip()
  {
    clips[0] = CRect{.center = {0, 0}, .extent = viewport_size};
    for (u32 i = 0; i < (u32) widgets.size(); i++)
    {
      WidgetNode const &node = nodes[i];
      clips[i]               = widgets[i]->clip(
          CRect{.center = positions[i], .extent = sizes[i]}, clips[i]);
      fill(to_span(clips).slice(node.first_child, node.num_children), clips[i]);
    }
  }

  void sort_layers()
  {
    iota(to_span(layered), 0U);
    indirect_sort(to_span(z_indices), to_span(layered),
                  [](i32 za, i32 zb) { return za < zb; });
  }

  void visibility()
  {
    for (u32 i = 0; i < (u32) widgets.size(); i++)
    {
      WidgetNode const &node = nodes[i];
      // if parent not visibile. make children not visible
      if (!has_bits(attributes[i], WidgetAttributes::Visible))
      {
        for (WidgetAttributes &attr :
             to_span(attributes).slice(node.first_child, node.num_children))
        {
          attr = attr & ~WidgetAttributes::Visible;
        }
      }
    }
  }

  void render(Canvas &canvas)
  {
    for (u32 i : layered)
    {
      canvas.clip(clips[i]);
      if (has_bits(attributes[i], WidgetAttributes::Visible))
      {
        widgets[i]->render(CRect{.center = positions[i], .extent = sizes[i]},
                           canvas);
      }
    }
  }

  void tick(nanoseconds dt)
  {
    // TODO(lamarrr)
    // process events across widgets, hit-test, dispatch events
    //
    // click - forward directly unless draggable
    // touch -
    // widget drag & drop
    // mouse over, mouse leave
    // view hit, view miss -
    // keyboard input: use another system?
    // clip board copy & paste with custom media format
    // text input
    // gamepad input: use another system?
    //
    for (u32 i = 0; i < (u32) widgets.size(); i++)
    {
      widgets[i]->tick(ctx, CRect{.center = positions[i], .extent = sizes[i]},
                       dt, WidgetEventTypes::None);
    }
  }

  void frame(Widget *root, Canvas &canvas, nanoseconds dt)
  {
    widgets.clear();
    if (root != nullptr)
    {
      CHECK(widgets.push(root));
      insert_children(root);
    }
    allocate_ids();
    u32 const num_widgets = (u32) widgets.size();
    CHECK(attributes.resize_uninitialized(num_widgets));
    CHECK(sizes.resize_uninitialized(num_widgets));
    CHECK(positions.resize_uninitialized(num_widgets));
    CHECK(clips.resize_uninitialized(num_widgets));
    CHECK(z_indices.resize_uninitialized(num_widgets));
    CHECK(layered.resize_uninitialized(num_widgets));

    attribute();
    layout();
    stack();
    clip();
    sort_layers();
    visibility();
    render(canvas);
    tick(dt);
  }
};

}        // namespace ash