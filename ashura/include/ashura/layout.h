#pragma once

#include <algorithm>

#include "ashura/widget.h"

namespace ash
{

struct LayoutEntry
{
  Widget               *widget = nullptr;        /// widget represented by this entry
  stx::Vec<vec2>        children_allocations;
  stx::Vec<vec2>        children_sizes;
  stx::Vec<vec2>        children_positions;
  stx::Vec<LayoutEntry> children;
};

struct RenderEntry
{
  Widget *widget  = nullptr;
  i64     z_index = 0;
  rect    clip;        // TODO(lamarrr): figure out how viewport scaling will work, because this 
};

struct WidgetTree
{
  LayoutEntry           layout_root;
  stx::Vec<RenderEntry> render_entries;

  void __build_layout_recursive(Context &ctx, LayoutEntry &entry, Widget &widget)
  {
    entry.widget        = &widget;
    stx::Span children  = widget.get_children(ctx);
    usize     nchildren = children.size();
    entry.children_allocations.resize(nchildren).unwrap();
    entry.children_sizes.resize(nchildren).unwrap();
    entry.children_positions.resize(nchildren).unwrap();
    entry.children.resize(nchildren).unwrap();

    for (usize i = 0; i < nchildren; i++)
    {
      __build_layout_recursive(ctx, entry.children[i], *children[i]);
    }
  }

  vec2 __fit_recursive(Context &ctx, LayoutEntry &entry, vec2 allocated_size)
  {
    entry.widget->allocate_size(ctx, allocated_size, entry.children_allocations);
    stx::Span children = entry.widget->get_children(ctx);

    for (usize i = 0; i < entry.children.size(); i++)
    {
      entry.children_sizes[i] = __fit_recursive(ctx, entry.children[i], entry.children_allocations[i]);
    }

    vec2 extent = entry.widget->fit(ctx, allocated_size, entry.children_sizes, entry.children_positions);

    //   stx::Span children =  entry.widget->get_children(ctx);
    // for(usize i = 0; i < entry.children.size(); i++){
    // }

    // use children_positions
    // for (Widget *const child : children)
    // {
    //   perform_layout(ctx, *child, child->area);
    //   child->transformed_area = ash::transform(child->get_transform(ctx), child->area);
    // }

    entry.widget->area.extent = extent;
    return extent;
  }

  void __position_recursive(Context &ctx, LayoutEntry &entry, vec2 position)
  {
    entry.widget->area.offset = position;
    for (usize i = 0; i < entry.children.size(); i++)
    {
      __position_recursive(ctx, entry.children[i], position + entry.children_positions[i]);
    }
  }

 void __build_render_recursive(Context &ctx,RenderEntry & entry, LayoutEntry & layout_entry  ){
  if(layout_entry.widget->g)

  }

  void layout(Context &ctx, Widget &root, vec2 position, vec2 allocated_size)
  {
    __build_layout_recursive(ctx, layout_root, root);
    __fit_recursive(ctx, layout_root, allocated_size);
    __position_recursive(ctx, layout_root, vec2{0, 0});
  }
};

}        // namespace ash
