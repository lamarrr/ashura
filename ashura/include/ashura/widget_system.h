#pragma once

#include <chrono>

#include "ashura/canvas.h"
#include "ashura/layout.h"
#include "ashura/widget.h"

namespace ash {

struct WidgetDrawEntry {
  Widget* widget = nullptr;
  Widget* parent = nullptr;
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
    if (root == nullptr) return;
    launch_recursive(*root, context);
  }

  static constexpr void exit_recursive(Widget& widget, WidgetContext& context) {
    widget.on_exit(context);

    for (Widget* child : widget.get_children()) {
      exit_recursive(*child, context);
    }
  }

  constexpr void exit(WidgetContext& context) {
    if (root == nullptr) return;
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
                      Widget* parent, i64 z_index) {
    z_index = widget.get_z_index(z_index);
    draw_entries
        .push(WidgetDrawEntry{
            .widget = &widget, .parent = parent, .z_index = z_index})
        .unwrap();

    for (Widget* child : widget.get_children()) {
      push_recursive(draw_entries, *child, &widget, z_index + 1);
    }
  }

  void tick(WidgetContext& context, gfx::Canvas& canvas,
            std::chrono::nanoseconds interval, vec2 viewport_extent) {
    // TODO(lamarrr): pump gathered events
    if (root == nullptr) return;

    tick_recursive(*root, context, interval);
    perform_layout(*root,
                   rect{.offset = vec2{0, 0}, .extent = viewport_extent});
    draw_entries.clear();
    push_recursive(draw_entries, *root, nullptr, 0);
    draw_entries.span().sort(
        [](WidgetDrawEntry const& a, WidgetDrawEntry const& b) {
          return a.z_index < b.z_index;
        });

    for (WidgetDrawEntry const& entry : draw_entries) {
      if (entry.widget->get_visibility() == Visibility::Visible) {
        canvas.save();
        canvas.transform = entry.widget->get_transform() * canvas.transform;
        if (entry.parent) {
          entry.parent->pre_draw(canvas, *entry.widget);
        }
        entry.widget->draw(canvas, entry.widget->area);
        if (entry.parent) {
          entry.parent->post_draw(canvas, *entry.widget);
        }
        canvas.restore();
      }
    }
  }

  Widget* root = nullptr;
  stx::Vec<WidgetDrawEntry> draw_entries{stx::os_allocator};
};

}  // namespace ash
