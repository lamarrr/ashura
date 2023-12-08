#pragma once

#include <algorithm>

#include "ashura/canvas.h"
#include "ashura/uuid.h"
#include "ashura/widget.h"

namespace ash
{

struct WidgetElement
{
  Widget                 *widget = nullptr;
  stx::Vec<Vec2>          children_allocations;
  stx::Vec<Vec2>          children_sizes;
  stx::Vec<Vec2>          children_positions;
  stx::Vec<Visibility>    children_visibility;
  stx::Vec<i32>           children_z_indices;
  stx::Vec<Rect>          children_clips;
  stx::Vec<WidgetElement> children;
};

struct WidgetRenderElement
{
  Widget *widget  = nullptr;
  i32     z_index = 0;
  Rect    clip;
};

struct WidgetTree
{
  WidgetElement                 root;
  stx::Vec<WidgetRenderElement> render_elements;

  static void __build_child_recursive(Context &ctx, WidgetElement &element,
                                      Widget &widget)
  {
    // NOTE: we are trying to re-use the widget tree memory allocations
    stx::Span children  = widget.get_children(ctx);
    usize     nchildren = children.size();
    element.widget      = &widget;
    element.children_allocations.resize(nchildren).unwrap();
    element.children_sizes.resize(nchildren).unwrap();
    element.children_positions.resize(nchildren).unwrap();
    element.children_visibility.resize(nchildren).unwrap();
    element.children_z_indices.resize(nchildren).unwrap();
    element.children_clips.resize(nchildren).unwrap();
    if (element.children.size() > nchildren)
    {
      element.children.erase(element.children.span().slice(nchildren));
    }
    else
    {
      usize additional = nchildren - element.children.size();
      for (usize i = 0; i < additional; i++)
      {
        element.children.push(WidgetElement{}).unwrap();
      }
    }

    for (usize i = 0; i < nchildren; i++)
    {
      __build_child_recursive(ctx, element.children[i], *children[i]);
    }
  }

  static Vec2 __fit_recursive(Context &ctx, WidgetElement &element,
                              Vec2 allocated_size)
  {
    stx::Span children = element.widget->get_children(ctx);
    element.widget->allocate_size(ctx, allocated_size,
                                  element.children_allocations);

    for (usize i = 0; i < element.children.size(); i++)
    {
      element.children_sizes[i] = __fit_recursive(
          ctx, element.children[i], element.children_allocations[i]);
    }

    return element.widget->area.extent = element.widget->fit(
               ctx, allocated_size, element.children_allocations,
               element.children_sizes, element.children_positions);
  }

  static void __absolute_position_recursive(Context       &ctx,
                                            WidgetElement &element,
                                            Vec2           allocated_position)
  {
    Vec2 position = element.widget->position(ctx, allocated_position);
    element.widget->area.offset = position;

    for (usize i = 0; i < element.children.size(); i++)
    {
      __absolute_position_recursive(ctx, element.children[i],
                                    position + element.children_positions[i]);
    }
  }

  void __build_render_recursive(Context &ctx, WidgetElement &element,
                                Visibility allocated_visibility,
                                i32 allocated_z_index, Rect allocated_clip,
                                Rect view_region)
  {
    stx::Span  children   = element.children;
    Visibility visibility = element.widget->get_visibility(
        ctx, allocated_visibility, element.children_visibility);
    i32  z_index = element.widget->z_stack(ctx, allocated_z_index,
                                           element.children_z_indices);
    Rect clip =
        element.widget->clip(ctx, allocated_clip, element.children_clips);

    if (visibility == Visibility::Visible && clip.overlaps(view_region) &&
        view_region.overlaps(element.widget->area))
    {
      element.widget->on_view_hit(ctx);
      render_elements
          .push(WidgetRenderElement{
              .widget = element.widget, .z_index = z_index, .clip = clip})
          .unwrap();
    }
    else
    {
      element.widget->on_view_miss(ctx);
    }

    for (usize i = 0; i < children.size(); i++)
    {
      __build_render_recursive(ctx, element.children[i],
                               element.children_visibility[i],
                               element.children_z_indices[i],
                               element.children_clips[i], view_region);
    }
  }

  /// @brief
  /// @param ctx
  /// @param canvas
  /// @param root_widget
  /// @param allocated_size size allocated to the root widget
  /// @param view_region region the logical viewport is focusing on
  /// @param viewport_size the physical viewport extent
  void build(Context &ctx, Widget &root_widget)
  {
    __build_child_recursive(ctx, root, root_widget);
  }

  void layout(Context &ctx, Vec2 allocated_size)
  {
    __fit_recursive(ctx, root, allocated_size);
    __absolute_position_recursive(ctx, root, Vec2{0, 0});
  }

  void render(Context &ctx, gfx::Canvas &canvas, Rect view_region,
              Vec2 viewport_size)
  {
    render_elements.clear();
    __build_render_recursive(ctx, root, Visibility::Visible, 0,
                             root.widget->area, view_region);
    render_elements.span().sort(
        [](WidgetRenderElement const &a, WidgetRenderElement const &b) {
          return a.z_index < b.z_index;
        });

    Vec2 scale       = viewport_size / view_region.extent;
    Vec2 translation = 0 - view_region.offset;

    canvas.restart(viewport_size)
        .global_translate(translation.x, translation.y)
        .global_scale(scale.x, scale.y);

    for (WidgetRenderElement const &element : render_elements)
    {
      Rect scissor;
      scissor.offset = (element.clip.offset - view_region.offset) * scale;
      scissor.extent = element.clip.extent * scale;
      canvas.save().scissor(scissor);
      element.widget->draw(ctx, canvas);
      canvas.restore();
    }
  }

  Widget *hit(Context &ctx, Vec2 position) const
  {
    for (usize i = render_elements.size(); i > 0;)
    {
      i--;
      if (render_elements[i].widget->area.contains(position) &&
          render_elements[i].widget->hit_test(ctx, position))
      {
        return render_elements[i].widget;
      }
    }

    return nullptr;
  }
};

}        // namespace ash
