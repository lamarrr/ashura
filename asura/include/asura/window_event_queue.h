#pragma once

#include "asura/event.h"
#include "stx/vec.h"

namespace asr {

struct WindowEventQueue {
  stx::Vec<MouseButtonEvent> mouse_button_events;
  stx::Vec<WindowEvent> window_events;

  void add_raw(MouseButtonEvent event) {
    mouse_button_events.push_inplace(event).unwrap();
  }

  void add_raw(WindowEvent event) {
    window_events.push_inplace(event).unwrap();
  }

  void clear() {
    mouse_button_events.clear();
    window_events.clear();
  }
};

}  // namespace asr
