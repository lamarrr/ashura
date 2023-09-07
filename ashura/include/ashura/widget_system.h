#pragma once

#include <chrono>

#include "ashura/canvas.h"
#include "ashura/uuid.h"
#include "ashura/widget.h"
#include "ashura/widget_tree.h"

namespace ash
{

struct WidgetSystem
{
  static void __assign_widget_uuids_recursive(Context &ctx, Widget &widget, PrngUuidGenerator &generator)
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

  void assign_widget_uuids(Context &ctx, Widget &root, PrngUuidGenerator &generator)
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
          case KeyAction::Press:
            if (Widget *hit_widget = tree.hit(ctx, event.position))
            {
              if (stx::Option widget_drag_data = hit_widget->on_drag_start(ctx, event.position))
              {
                drag_source = hit_widget->id.copy();
                drag_data   = std::move(widget_drag_data);
              }
              else
              {
                hit_widget->on_mouse_down(ctx, event.button, event.position, event.clicks);
              }
            }
            break;

          case KeyAction::Release:
            if (Widget *hit_widget = tree.hit(ctx, event.position))
            {
              if (drag_data.is_some())
              {
                if (!drag_source.contains(hit_widget->id.value()))
                {
                  bool accepted_drop = hit_widget->on_drop(ctx, event.position, drag_data.value());
                  if (!accepted_drop)
                  {
                    drag_source.match([&](uuid source) {  if(stx::Option source_widget = ctx.find_widget(source)){  source_widget.value()->on_drag_end(ctx, event.position  );  } }, []() {});
                  }
                }
                else
                {
                  hit_widget->on_drag_end(ctx, event.position);
                }
              }
              else
              {
                hit_widget->on_mouse_up(ctx, event.button, event.position, event.clicks);
              }
            }
            drag_data = stx::None;
            break;

          default:
            break;
        }
      }
      else if (std::holds_alternative<MouseMotionEvent>(e))
      {
        MouseMotionEvent event = std::get<MouseMotionEvent>(e);

        stx::Option<uuid> hit_widget;
        // we have to check if previous hit widget accepted the drag data
        // if it is in drag state the last_hit_widget will be a widget that accepts it

        if (drag_data.is_some() && drag_source.is_some())
        {
          ctx
              .find_widget(drag_source.value())
              .match([&](Widget *pdrag_source) { pdrag_source->on_drag_update(ctx, event.position, event.translation, drag_data.value()); }, []() {});
        }

        if (Widget *phit_widget = tree.hit(ctx, event.position))
        {
          if (last_hit_widget.is_none() || phit_widget->id.value() != last_hit_widget.value())
          {
            if (drag_data.is_some())
            {
              phit_widget->on_drag_enter(ctx, drag_data.value());
            }
            else
            {
              phit_widget->on_mouse_enter(ctx, event.position);
            }
          }
          else
          {
            if (!drag_data.is_some())
            {
              phit_widget->on_mouse_move(ctx, event.position, event.translation);
            }
          }

          hit_widget = stx::Some(phit_widget->id.copy().unwrap());
        }

        if (last_hit_widget.is_some() && (hit_widget.is_none() || hit_widget.value() != last_hit_widget.value()))
        {
          if (drag_data.is_some())
          {
            ctx
                .find_widget(last_hit_widget.value())
                .match([&](Widget *plast_hit_widget) { plast_hit_widget->on_drag_leave(ctx, stx::Some(vec2{event.position})); }, []() {});
          }
          else
          {
            ctx
                .find_widget(last_hit_widget.value())
                .match([&](Widget *plast_hit_widget) { plast_hit_widget->on_mouse_leave(ctx, stx::Some(vec2{event.position})); }, []() {});
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
            ctx
                .find_widget(last_hit_widget.value())
                .match([&](Widget *plast_hit_widget) { 
              if(drag_data.is_some())
              {
                plast_hit_widget->on_drag_leave(ctx, stx::None);
              } else{
                plast_hit_widget->on_mouse_leave(ctx, stx::None); 
               }; }, []() {});
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
  stx::Option<DragData>                                                   drag_data   = stx::None;
  stx::Option<uuid>                                                       drag_source = stx::None;
};
}        // namespace ash
