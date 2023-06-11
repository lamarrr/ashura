#pragma once

#include <chrono>

#include "ashura/canvas.h"
#include "ashura/layout.h"
#include "ashura/widget.h"

namespace ash
{

struct WidgetDrawEntry
{
  Widget   *widget  = nullptr;
  i64       z_index = 0;
  ash::quad quad;
};

// TODO(lamarrr): more window events pumping to widgets, how?
struct WidgetSystem
{
  explicit WidgetSystem(Widget &iroot) :
      root{&iroot}
  {}

  static void __assign_ids_recursive(Context &context, Widget &widget, u64 &id)
  {
    widget.id = id;

    for (Widget *child : widget.get_children(context))
    {
      id++;
      __assign_ids_recursive(context, *child, id);
    }
  }

  static void __startup_recursive(Context &context, Widget &widget)
  {
    widget.on_startup(context);

    for (Widget *child : widget.get_children(context))
    {
      __startup_recursive(context, *child);
    }
  }

  static void __exit_recursive(Context &context, Widget &widget)
  {
    widget.on_exit(context);

    for (Widget *child : widget.get_children(context))
    {
      __exit_recursive(context, *child);
    }
  }

  static void __tick_recursive(Context &context, Widget &widget, std::chrono::nanoseconds interval)
  {
    widget.tick(context, interval);
    for (Widget *child : widget.get_children(context))
    {
      __tick_recursive(context, *child, interval);
    }
  }

  static void __push_recursive(Context &context, stx::Vec<WidgetDrawEntry> &entries, Widget const *last_hit_widget,
                               bool &last_hit_widget_is_alive, Widget &widget, Widget *parent, i64 z_index)
  {
    if (widget.get_visibility(context) == Visibility::Visible)
    {
      z_index = widget.get_z_index(context, z_index);

      entries.push(WidgetDrawEntry{.widget = &widget, .z_index = z_index, .quad = quad{}}).unwrap();
    }

    if (last_hit_widget == &widget)
    {
      last_hit_widget_is_alive = true;
    }

    for (Widget *child : widget.get_children(context))
    {
      __push_recursive(context, entries, last_hit_widget, last_hit_widget_is_alive, *child, &widget, z_index + 1);
    }
  }

  void on_startup(Context &context)
  {
    __startup_recursive(context, *root);
  }

  void exit(Context &context)
  {
    __exit_recursive(context, *root);
  }

  void assign_ids(Context &context)
  {
    u64 id = 0;
    __assign_ids_recursive(context, *root, id);
  }

  void pump_events(Context &context)
  {
    for (auto const &e : events)
    {
      if (std::holds_alternative<MouseClickEvent>(e))
      {
        MouseClickEvent event = std::get<MouseClickEvent>(e);

        for (WidgetDrawEntry const *iter = entries.end(); iter > entries.begin();)
        {
          iter--;
          if (iter->quad.contains(event.position))
          {
            switch (event.action)
            {
              case MouseAction::Press:
                iter->widget->on_mouse_down(context, event.button, event.position, event.clicks, iter->quad);
                break;

              case MouseAction::Release:
                iter->widget->on_mouse_up(context, event.button, event.position, event.clicks, iter->quad);
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

        Widget *hit_widget = nullptr;

        for (WidgetDrawEntry const *iter = entries.end(); iter > entries.begin();)
        {
          iter--;
          if (iter->quad.contains(event.position))
          {
            if (iter->widget != last_hit_widget)
            {
              iter->widget->on_mouse_enter(context, event.position, iter->quad);
            }
            else
            {
              iter->widget->on_mouse_move(context, event.position, event.translation, iter->quad);
            }
            hit_widget = iter->widget;
            break;
          }
        }

        if (last_hit_widget != nullptr && last_hit_widget_is_alive && last_hit_widget != hit_widget)
        {
          last_hit_widget->on_mouse_leave(context, stx::Some(vec2{event.position}));
        }

        last_hit_widget          = hit_widget;
        last_hit_widget_is_alive = hit_widget != nullptr;
      }
      else if (std::holds_alternative<WindowEvents>(e))
      {
        if ((std::get<WindowEvents>(e) & WindowEvents::MouseLeave) != WindowEvents::None)
        {
          if (last_hit_widget != nullptr && last_hit_widget_is_alive)
          {
            last_hit_widget->on_mouse_leave(context, stx::None);
            last_hit_widget = nullptr;
          }
        }
      }
    }

    events.clear();
  }

  void tick_widgets(Context &context, std::chrono::nanoseconds interval)
  {
    __tick_recursive(context, *root, interval);
  }

  void perform_widget_layout(Context &context, vec2 viewport_extent)
  {
    ash::perform_layout(context, *root, rect{.offset = vec2{0, 0}, .extent = viewport_extent});
  }

  void rebuild_draw_entries(Context &context)
  {
    entries.clear();
    __push_recursive(context, entries, last_hit_widget, last_hit_widget_is_alive, *root, nullptr, 0);
    entries.span().sort([](WidgetDrawEntry const &a, WidgetDrawEntry const &b) { return a.z_index < b.z_index; });
  }

  void draw_widgets(Context &context, vec2 viewport_extent, gfx::Canvas &canvas, mat4 const &global_transform)
  {
    rect viewport_rect{.offset = {0, 0}, .extent = viewport_extent};
    for (WidgetDrawEntry &entry : entries)
    {
      mat4 widget_transform = entry.widget->get_transform(context);
      entry.quad            = transform(global_transform * widget_transform, entry.widget->area);

      if (viewport_rect.contains(entry.quad))
      {
        canvas.reset();
        canvas.transform(widget_transform);
        canvas.global_transform(global_transform);
        entry.widget->draw(context, canvas, entry.widget->area);
      }
    }
  }

  Widget                                                                 *root = nullptr;
  stx::Vec<std::variant<MouseClickEvent, MouseMotionEvent, WindowEvents>> events;
  Widget                                                                 *last_hit_widget          = nullptr;
  bool                                                                    last_hit_widget_is_alive = false;
  // sorted by z-index
  stx::Vec<WidgetDrawEntry> entries;
};

}        // namespace ash
