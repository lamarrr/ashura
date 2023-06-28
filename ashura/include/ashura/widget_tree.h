#pragma once

#include <algorithm>

#include "ashura/widget.h"

namespace ash
{

struct WidgetElement
{
  Widget                 *widget = nullptr;
  stx::Vec<vec2>          children_allocations;
  stx::Vec<vec2>          children_sizes;
  stx::Vec<vec2>          children_positions;
  stx::Vec<Visibility>    children_visibility;
  stx::Vec<i64>           children_z_indices;
  stx::Vec<rect>          children_clips;
  stx::Vec<WidgetElement> children;
};

struct WidgetRenderElement
{
  Widget *widget  = nullptr;
  i64     z_index = 0;
  rect    clip;
};

struct WidgetTree
{
  WidgetElement                 root;
  stx::Vec<WidgetRenderElement> render_elements;

  static void __build_child_recursive(Context &ctx, WidgetElement &element, Widget &widget)
  {
    stx::Span children  = widget.get_children(ctx);
    usize     nchildren = children.size();
    element.widget      = &widget;
    element.children_allocations.resize(nchildren).unwrap();
    element.children_sizes.resize(nchildren).unwrap();
    element.children_positions.resize(nchildren).unwrap();
    element.children_visibility.resize(nchildren).unwrap();
    element.children_z_indices.resize(nchildren).unwrap();
    element.children_clips.resize(nchildren).unwrap();
    element.children.resize(nchildren).unwrap();

    for (usize i = 0; i < nchildren; i++)
    {
      __build_child_recursive(ctx, element.children[i], *children[i]);
    }
  }

  static vec2 __fit_recursive(Context &ctx, WidgetElement &element, vec2 allocated_size)
  {
    stx::Span children = element.widget->get_children(ctx);
    element.widget->allocate_size(ctx, allocated_size, element.children_allocations);

    for (usize i = 0; i < element.children.size(); i++)
    {
      element.children_sizes[i] = __fit_recursive(ctx, element.children[i], element.children_allocations[i]);
    }

    return element.widget->area.extent = element.widget->fit(ctx, allocated_size, element.children_sizes, element.children_positions);
  }

  static void __absolute_position_recursive(Context &ctx, WidgetElement &element, vec2 allocated_position)
  {
    vec2 position               = element.widget->position(ctx, allocated_position);
    element.widget->area.offset = position;

    for (usize i = 0; i < element.children.size(); i++)
    {
      __absolute_position_recursive(ctx, element.children[i], position + element.children_positions[i]);
    }
  }

  void __build_render_recursive(Context &ctx, WidgetElement &element, Visibility allocated_visibility, i64 allocated_z_index, rect allocated_clip, rect view_region)
  {
    stx::Span  children   = element.children;
    Visibility visibility = element.widget->get_visibility(ctx, allocated_visibility, element.children_visibility);
    i64        z_index    = element.widget->z_stack(ctx, allocated_z_index, element.children_z_indices);
    rect       clip       = element.widget->clip(ctx, allocated_clip, element.children_clips);

    if (visibility == Visibility::Visible && clip.overlaps(view_region) && view_region.overlaps(element.widget->area))
    {
      element.widget->on_enter_view(ctx);
      render_elements.push(WidgetRenderElement{.widget = element.widget, .z_index = z_index, .clip = clip}).unwrap();
    }
    else
    {
      element.widget->on_leave_view(ctx);
    }

    for (usize i = 0; i < children.size(); i++)
    {
      __build_render_recursive(ctx, element, element.children_visibility[i], element.children_z_indices[i], element.children_clips[i], view_region);
    }
  }

  // TODO(lamarrr): figure out how viewport will work, because this
  // viewport must be centered on scale

  /// @brief
  /// @param ctx
  /// @param canvas
  /// @param root_widget
  /// @param allocated_size size allocated to the root widget
  /// @param view_region region the logical viewport is focusing on
  /// @param viewport_size the physical viewport extent
  void update(Context &ctx, gfx::Canvas &canvas, Widget &root_widget, vec2 allocated_size, rect view_region, vec2 viewport_size)
  {
    __build_child_recursive(ctx, root, root_widget);
    __fit_recursive(ctx, root, allocated_size);
    __absolute_position_recursive(ctx, root, vec2{0, 0});
    render_elements.clear();
    __build_render_recursive(ctx, root, Visibility::Visible, 0, root.widget->area, view_region);
    render_elements.span().sort([](WidgetRenderElement const &a, WidgetRenderElement const &b) { return a.z_index < b.z_index; });

    vec2 zoom2d = viewport_size / view_region.extent;
    vec3 zoom{zoom2d.x, zoom2d.y, 1};
    vec3 translation{-view_region.offset.x, -view_region.offset.y, 0};

    canvas
        .restart(viewport_size)
        .global_transform(ash::translate(translation))
        .global_transform(ash::scale(zoom));

    for (WidgetRenderElement const &element : render_elements)
    {
      canvas
          .save()
          .clip(element.clip);        // TODO(lamarrr): calculate actual clip based on viewport positioning and scale. transform clip
      element.widget->draw(ctx, canvas);
      canvas.restore();
    }
  }

  Widget *hit(Context &ctx, vec2 position) const
  {
    for (usize i = render_elements.size(); i > 0; i--)
    {
      i--;
      if (render_elements[i].widget->area.contains(position) && render_elements[i].widget->hit_test(ctx, position))
      {
        return render_elements[i].widget;
      }
    }

    return nullptr;
  }
};

}        // namespace ash
