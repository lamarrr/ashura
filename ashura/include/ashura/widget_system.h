#pragma once

#include <chrono>

#include "ashura/canvas.h"
#include "ashura/layout.h"
#include "ashura/uuid.h"
#include "ashura/widget.h"

namespace ash
{

struct WidgetDrawEntry
{
  Widget *widget  = nullptr;
  i64     z_index = 0;
};

// TODO(lamarrr): more window events pumping to widgets, how?
struct WidgetSystem
{
  explicit WidgetSystem(Widget &iroot) :
      root{&iroot}
  {}

  static void __assign_ids_recursive(Context &ctx, Widget &widget, UuidGenerator &generator)
  {
    if (widget.id.is_none())
    {
      widget.id = stx::Some(generator.generate());
    }

    for (Widget *child : widget.get_children(ctx))
    {
      __assign_ids_recursive(ctx, *child, generator);
    }
  }

  static void __tick_recursive(Context &ctx, Widget &widget, std::chrono::nanoseconds interval)
  {
    widget.tick(ctx, interval);
    for (Widget *child : widget.get_children(ctx))
    {
      __tick_recursive(ctx, *child, interval);
    }
  }

  static void __push_recursive(Context &ctx, stx::Vec<WidgetDrawEntry> &entries, Widget &widget, Widget *parent, i64 z_index)
  {
    if (widget.get_visibility(ctx) == Visibility::Visible)
    {
      z_index = widget.get_z_index(ctx, z_index);

      entries.push(WidgetDrawEntry{.widget = &widget, .z_index = z_index}).unwrap();
    }

    for (Widget *child : widget.get_children(ctx))
    {
      __push_recursive(ctx, entries, *child, &widget, z_index + 1);
    }
  }

  void assign_ids(Context &ctx, UuidGenerator &generator)
  {
    __assign_ids_recursive(ctx, *root, generator);
  }

  void pump_events(Context &ctx)
  {
    for (auto const &e : events)
    {
      if (std::holds_alternative<MouseClickEvent>(e))
      {
        MouseClickEvent event = std::get<MouseClickEvent>(e);

        for (WidgetDrawEntry const *iter = entries.end(); iter > entries.begin();)
        {
          iter--;
          if (iter->widget->transformed_area.contains(event.position))
          {
            switch (event.action)
            {
              case MouseAction::Press:
                iter->widget->on_mouse_down(ctx, event.button, event.position, event.clicks);
                break;

              case MouseAction::Release:
                iter->widget->on_mouse_up(ctx, event.button, event.position, event.clicks);
                break;

              default:
                break;
            }
            break;
          }
        }
      }
      else if (std::holds_alternative<MouseMotionEvent>(e))
      {
        MouseMotionEvent event = std::get<MouseMotionEvent>(e);

        stx::Option<uuid> hit_widget;

        for (WidgetDrawEntry const *iter = entries.end(); iter > entries.begin();)
        {
          iter--;
          if (iter->widget->transformed_area.contains(event.position))
          {
            if (last_hit_widget.is_none() || iter->widget->id.value() != last_hit_widget.value())
            {
              iter->widget->on_mouse_enter(ctx, event.position);
            }
            else
            {
              iter->widget->on_mouse_move(ctx, event.position, event.translation);
            }
            hit_widget = stx::Some(iter->widget->id.copy().unwrap());
            break;
          }
        }

        if (last_hit_widget.is_some() && (hit_widget.is_none() || hit_widget.value() != last_hit_widget.value()))
        {
          Widget *plast_hit_widget = find_widget(ctx, root, last_hit_widget.value());
          if (plast_hit_widget != nullptr)
          {
            plast_hit_widget->on_mouse_leave(ctx, stx::Some(vec2{event.position}));
          }
        }

        last_hit_widget = hit_widget;
      }
      else if (std::holds_alternative<WindowEvents>(e))
      {
        if ((std::get<WindowEvents>(e) & WindowEvents::MouseLeave) != WindowEvents::None)
        {
          if (last_hit_widget.is_some())
          {
            Widget *plast_hit_widget = find_widget(ctx, root, last_hit_widget.value());
            if (plast_hit_widget != nullptr)
            {
              plast_hit_widget->on_mouse_leave(ctx, stx::None);
            }
          }
        }
      }
    }

    events.clear();
  }

  void tick_widgets(Context &ctx, std::chrono::nanoseconds interval)
  {
    __tick_recursive(ctx, *root, interval);
  }

  void perform_widget_layout(Context &ctx, vec2 viewport_extent)
  {
    ash::perform_layout(ctx, *root, rect{.offset = vec2{0, 0}, .extent = viewport_extent});
  }

  void rebuild_draw_entries(Context &ctx)
  {
    entries.clear();
    __push_recursive(ctx, entries, *root, nullptr, 0);
    entries.span().sort([](WidgetDrawEntry const &a, WidgetDrawEntry const &b) { return a.z_index < b.z_index; });
  }

  void draw_widgets(Context &ctx, vec2 viewport_extent, gfx::Canvas &canvas, mat4 const &global_transform)
  {
    rect viewport_rect{.offset = {0, 0}, .extent = viewport_extent};
    for (WidgetDrawEntry &entry : entries)
    {
      if (viewport_rect.contains(entry.widget->transformed_area))
      {
        canvas.reset();
        canvas.transform(entry.widget->get_transform(ctx));
        canvas.global_transform(global_transform);
        entry.widget->draw(ctx, canvas);
      }
    }
  }

  Widget                                                                 *root = nullptr;
  stx::Vec<std::variant<MouseClickEvent, MouseMotionEvent, WindowEvents>> events;
  stx::Option<uuid>                                                       last_hit_widget;
  stx::Vec<WidgetDrawEntry>                                               entries;        // sorted by z-index
};

}        // namespace ash
