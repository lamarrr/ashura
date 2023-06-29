#pragma once

#include <chrono>

#include "ashura/canvas.h"
#include "ashura/uuid.h"
#include "ashura/widget.h"
#include "ashura/widget_tree.h"

namespace ash
{

// TODO(lamarrr): more window events pumping to widgets, how?
struct WidgetSystem
{
  static void __assign_widget_uuids_recursive(Context &ctx, Widget &widget, UuidGenerator &generator)
  {
    if (widget.id.is_none())
    {
      widget.id = stx::Some(generator.generate());
    }

    for (Widget *child : widget.get_children(ctx))
    {
      __assign_widget_uuids_recursive(ctx, *child, generator);
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

  void assign_widget_uuids(Context &ctx, Widget &root, UuidGenerator &generator)
  {
    __assign_widget_uuids_recursive(ctx, root, generator);
  }

  void pump_widget_events(WidgetTree &tree, Context &ctx)
  {
    for (auto const &e : events)
    {
      if (std::holds_alternative<MouseClickEvent>(e))
      {
        MouseClickEvent event = std::get<MouseClickEvent>(e);

        switch (event.action)
        {
          case MouseAction::Press:
            if (Widget *hit_widget = tree.hit(ctx, event.position))
            {
              hit_widget->on_mouse_down(ctx, event.button, event.position, event.clicks);
            }
            break;

          case MouseAction::Release:
            if (Widget *hit_widget = tree.hit(ctx, event.position))
            {
              hit_widget->on_mouse_up(ctx, event.button, event.position, event.clicks);
            }
            break;

          default:
            break;
        }
      }
      else if (std::holds_alternative<MouseMotionEvent>(e))
      {
        MouseMotionEvent event = std::get<MouseMotionEvent>(e);

        stx::Option<uuid> hit_widget;

        if (Widget *phit_widget = tree.hit(ctx, event.position))
        {
          if (last_hit_widget.is_none() || phit_widget->id.value() != last_hit_widget.value())
          {
            phit_widget->on_mouse_enter(ctx, event.position);
          }
          else
          {
            phit_widget->on_mouse_move(ctx, event.position, event.translation);
          }
          hit_widget = stx::Some(phit_widget->id.copy().unwrap());
        }

        if (last_hit_widget.is_some() && (hit_widget.is_none() || hit_widget.value() != last_hit_widget.value()))
        {
          ctx.find_widget(last_hit_widget.value()).match([&](Widget *plast_hit_widget) { plast_hit_widget->on_mouse_leave(ctx, stx::Some(vec2{event.position})); }, []() {});
        }

        last_hit_widget = hit_widget;
      }
      else if (std::holds_alternative<WindowEvents>(e))
      {
        if ((std::get<WindowEvents>(e) & WindowEvents::MouseLeave) != WindowEvents::None)
        {
          if (last_hit_widget.is_some())
          {
            ctx.find_widget(last_hit_widget.value()).match([&](Widget *plast_hit_widget) { plast_hit_widget->on_mouse_leave(ctx, stx::None); }, []() {});
            last_hit_widget = stx::None;
          }
        }
      }
    }

    events.clear();
  }

  void tick_widgets(Context &ctx, Widget &root, std::chrono::nanoseconds interval)
  {
    __tick_recursive(ctx, root, interval);
  }

  stx::Vec<std::variant<MouseClickEvent, MouseMotionEvent, WindowEvents>> events;
  stx::Option<uuid>                                                       last_hit_widget;
};
}        // namespace ash
