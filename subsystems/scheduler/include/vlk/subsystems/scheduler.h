#pragma once

#include "stx/scheduler.h"
#include "vlk/subsystem.h"
#include "vlk/subsystem/context.h"
#include "vlk/subsystems/scheduler.h"

namespace vlk {

struct TaskScheduler : public Subsystem {
  virtual void tick(std::chrono::nanoseconds interval) override {
    scheduler.tick(interval);
  }

  virtual void link(SubsystemsContext const&) override {}

  virtual stx::FutureAny get_future() override {
    return stx::FutureAny{scheduler.cancelation_promise.get_future()};
  }

  TaskScheduler(stx::Allocator allocator,
                std::chrono::steady_clock::time_point tp)
      : scheduler{allocator, tp} {}

  virtual ~TaskScheduler() override {}

  stx::TaskScheduler scheduler;
};

}  // namespace vlk