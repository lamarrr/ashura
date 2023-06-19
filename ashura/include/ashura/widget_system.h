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

  static void __assign_ids_recursive(Context &context, Widget &widget, UuidGenerator &generator)
  {
    if (widget.id.is_none())
    {
      widget.id = stx::Some(generator.generate());
    }

    for (Widget *child : widget.get_children(context))
    {
      __assign_ids_recursive(context, *child, generator);
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

  static void __push_recursive(Context &context, stx::Vec<WidgetDrawEntry> &entries, Widget &widget, Widget *parent, i64 z_index)
  {
    if (widget.get_visibility(context) == Visibility::Visible)
    {
      z_index = widget.get_z_index(context, z_index);

      entries.push(WidgetDrawEntry{.widget = &widget, .z_index = z_index}).unwrap();
    }

    for (Widget *child : widget.get_children(context))
    {
      __push_recursive(context, entries, *child, &widget, z_index + 1);
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

  void assign_ids(Context &context, UuidGenerator &generator)
  {
    __assign_ids_recursive(context, *root, generator);
  }

  Widget *find_widget(Context &context, Widget *current, uuid id)
  {
    if (current->id.value() == id)
    {
      return current;
    }

    for (Widget *child : current->get_children(context))
    {
      Widget *found = find_widget(context, child, id);
      if (found != nullptr)
      {
        return found;
      }
    }

    return nullptr;
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
          if (iter->widget->transformed_area.contains(event.position))
          {
            switch (event.action)
            {
              case MouseAction::Press:
                iter->widget->on_mouse_down(context, event.button, event.position, event.clicks);
                break;

              case MouseAction::Release:
                iter->widget->on_mouse_up(context, event.button, event.position, event.clicks);
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
              iter->widget->on_mouse_enter(context, event.position);
            }
            else
            {
              iter->widget->on_mouse_move(context, event.position, event.translation);
            }
            hit_widget = stx::Some(iter->widget->id.copy().unwrap());
            break;
          }
        }

        if (last_hit_widget.is_some() && (hit_widget.is_none() || hit_widget.value() != last_hit_widget.value()))
        {
          Widget *plast_hit_widget = find_widget(context, root, last_hit_widget.value());
          if (plast_hit_widget != nullptr)
          {
            plast_hit_widget->on_mouse_leave(context, stx::Some(vec2{event.position}));
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
            Widget *plast_hit_widget = find_widget(context, root, last_hit_widget.value());
            if (plast_hit_widget != nullptr)
            {
              plast_hit_widget->on_mouse_leave(context, stx::None);
            }
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
    __push_recursive(context, entries, *root, nullptr, 0);
    entries.span().sort([](WidgetDrawEntry const &a, WidgetDrawEntry const &b) { return a.z_index < b.z_index; });
  }

  void draw_widgets(Context &context, vec2 viewport_extent, gfx::Canvas &canvas, mat4 const &global_transform)
  {
    rect viewport_rect{.offset = {0, 0}, .extent = viewport_extent};
    for (WidgetDrawEntry &entry : entries)
    {
      if (viewport_rect.contains(entry.widget->transformed_area))
      {
        canvas.reset();
        canvas.transform(entry.widget->get_transform(context));
        canvas.global_transform(global_transform);
        entry.widget->draw(context, canvas);
      }
    }
  }

  Widget                                                                 *root = nullptr;
  stx::Vec<std::variant<MouseClickEvent, MouseMotionEvent, WindowEvents>> events;
  stx::Option<uuid>                                                       last_hit_widget;
  stx::Vec<WidgetDrawEntry>                                               entries;        // sorted by z-index
};

}        // namespace ash
