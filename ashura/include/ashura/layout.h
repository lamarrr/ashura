#pragma once

#include <algorithm>

#include "ashura/widget.h"

namespace ash
{

struct LayoutEntry
{
  vec2            extent;
  stx::Span<vec2> children_positions;
  stx::Span<vec2> children_sizes;
};

struct LayoutTree
{};

struct RenderEntry
{
  Widget *widget  = nullptr;
  i64     z_index = 0;
};

struct RenderTree
{
  stx::Vec<RenderEntry> entries;
};

inline vec2 perform_layout(Context &ctx, Widget &widget, vec2 allocated_size)
{
  stx::Span<Widget *const> children = widget.get_children(ctx);

  stx::Vec<vec2> children_allocations;
  children_allocations.resize(children.size()).unwrap();

  stx::Vec<vec2> children_sizes;
  children_sizes.resize(children.size()).unwrap();

  stx::Vec<vec2> children_positions;
  children_positions.resize(children.size()).unwrap();

  widget.allocate_size(ctx, allocated_size, children_allocations);

  for (usize i = 0; i < children.size(); i++)
  {
    children_sizes[i] = perform_layout(ctx, *children[i], children_allocations[i]);
  }

  vec2 extent = widget.layout(ctx, allocated_size, children_sizes, children_positions);

  // use children_positions
  // for (Widget *const child : children)
  // {
  //   perform_layout(ctx, *child, child->area);
  //   child->transformed_area = ash::transform(child->get_transform(ctx), child->area);
  // }

  return extent;
}
}        // namespace ash
