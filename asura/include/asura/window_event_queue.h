#pragma once

#include <vector>

#include "asura/event.h"

namespace asr {
namespace ui {

struct WindowEventQueue {
  std::vector<MouseButtonEvent> mouse_button_events;
  std::vector<WindowEvent> window_events;

  void add_raw(MouseButtonEvent event) { mouse_button_events.push_back(event); }
  void add_raw(WindowEvent event) { window_events.push_back(event); }

  void clear() {
    mouse_button_events.clear();
    window_events.clear();
  }
};

}  // namespace ui
}  // namespace asr
