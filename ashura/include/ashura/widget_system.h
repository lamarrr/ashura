#pragma once

#include <chrono>

#include "ashura/canvas.h"
#include "ashura/layout.h"
#include "ashura/widget.h"

namespace ash {

struct WidgetDrawEntry {
  Widget* widget = nullptr;
  i64 z_index = 0;
};

struct WidgetSystem {
  static constexpr void launch_recursive(Widget& widget,
                                         WidgetContext& context) {
    widget.on_launch(context);

    for (Widget* child : widget.get_children()) {
      launch_recursive(*child, context);
    }
  }

  constexpr void launch(WidgetContext& context) {
    launch_recursive(*root, context);
  }

  static constexpr void exit_recursive(Widget& widget, WidgetContext& context) {
    widget.on_exit(context);

    for (Widget* child : widget.get_children()) {
      exit_recursive(*child, context);
    }
  }

  constexpr void exit(WidgetContext& context) {
    exit_recursive(*root, context);
  }

  static constexpr void tick_recursive(Widget& widget, WidgetContext& context,
                                       std::chrono::nanoseconds interval) {
    widget.tick(context, interval);
    for (Widget* child : widget.get_children()) {
      tick_recursive(*child, context, interval);
    }
  }

  void push_recursive(stx::Vec<WidgetDrawEntry>& draw_entries, Widget& widget,
                      i64 z_index) {
    z_index = widget.get_z_index(z_index);
    draw_entries.push(WidgetDrawEntry{.widget = &widget, .z_index = z_index})
        .unwrap();

    for (Widget* child : widget.get_children()) {
      push_recursive(draw_entries, *child, z_index + 1);
    }
  }

  constexpr void tick(WidgetContext& context, gfx::Canvas& canvas,
                      std::chrono::nanoseconds interval, vec2 viewport_extent) {
    // TODO(lamarrr): pump gathered events

    tick_recursive(*root, context, interval);
    perform_layout(*root,
                   rect{.offset = vec2{0, 0}, .extent = viewport_extent});
    draw_entries.clear();
    push_recursive(draw_entries, *root, 0);
    draw_entries.span().sort(
        [](WidgetDrawEntry const& a, WidgetDrawEntry const& b) {
          return a.z_index < b.z_index;
        });

    for (WidgetDrawEntry const& entry : draw_entries) {
      if (entry.widget->get_visibility() == Visibility::Visible) {
        entry.widget->draw(canvas, entry.widget->area);
      }
    }
  }

  Widget* root = nullptr;
  stx::Vec<WidgetDrawEntry> draw_entries{stx::os_allocator};
};

}  // namespace ash
