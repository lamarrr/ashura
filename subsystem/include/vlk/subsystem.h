#pragma once

#include <chrono>

#include "stx/async.h"
#include "stx/option.h"

namespace vlk {

struct SubsystemsContext;

struct Subsystem {
  template <typename Target>
  stx::Option<Target*> as() {
    Target* upcast_ptr = dynamic_cast<Target*>(this);
    if (upcast_ptr == nullptr) {
      return stx::None;
    } else {
      return stx::Some(static_cast<Target*>(upcast_ptr));
    }
  }

  // used for cancelation (shutdown)
  virtual stx::FutureAny get_future() = 0;

  // used to fetch a subsystem dependency
  virtual void link(SubsystemsContext const&) = 0;

  // called on every frame
  virtual void tick(std::chrono::nanoseconds) = 0;

  virtual ~Subsystem() {}
};

}  // namespace vlk
