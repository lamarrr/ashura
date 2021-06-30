#pragma once

#include <vector>

#include "vlk/ui/event.h"

namespace vlk {
namespace ui {

struct WindowEventQueue {
  std::vector<MouseButtonEvent> mouse_button_events;

  void add_raw(MouseButtonEvent event) { mouse_button_events.push_back(event); }

  void clear() { mouse_button_events.clear(); }
};

}  // namespace ui
}  // namespace vlk
