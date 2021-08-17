#pragma once

#include <chrono>

#include "stx/async.h"
#include "vlk/subsystem.h"

namespace vlk {

struct SubsystemsContext;

struct SubsystemImpl : public Subsystem {
  virtual stx::FutureAny get_future() = 0;
  // called after all futures have been added
  virtual void link(SubsystemsContext const&) = 0;
  // called on every frame
  virtual void tick(std::chrono::nanoseconds) = 0;
};

};  // namespace vlk
