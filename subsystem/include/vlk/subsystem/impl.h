#pragma once

#include <chrono>

#include "stx/async.h"
#include "stx/mem.h"
#include "stx/span.h"
#include "vlk/subsystem.h"

namespace vlk {

struct SubsystemsContext;

struct SubsystemImpl : public Subsystem {
  virtual stx::FutureAny get_future() = 0;
  virtual void link(SubsystemsContext const&);
  virtual void tick(std::chrono::nanoseconds) = 0;
  virtual ~SubsystemImpl() = 0;
};

};  // namespace vlk
