#pragma once

#include <chrono>

#include "SDL.h"
#include "stx/vec.h"
#include "stx/fn.h"
#include "vlk/subsystem.h"

namespace vlk {

struct Keyboard : public Subsystem {
  virtual void tick(std::chrono::nanoseconds interval);

  void listen(stx::RcFn<void()> on_event);

  stx::Vec<stx::RcFn<void()>> listeners;
};

}  // namespace vlk
