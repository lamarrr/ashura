#pragma once

#include <algorithm>
#include <utility>

#include "vlk/subsystem/context.h"
#include "vlk/subsystem/registry.h"

namespace vlk {

struct SubsystemsContextImpl : public SubsystemsContext {
  SubsystemsContextImpl(SubsystemsRegistry&& registry)
      : SubsystemsContext{std::move(registry)} {}

  void begin_shutdown() const {
    for (auto const& [identifier, info] : map_) {
      info.cancelation_future.request_cancel();
    }
  }

  bool all_shutdown() const {
    return std::all_of(map_.begin(), map_.end(), [](auto const& entry) {
      return entry.second.cancelation_future.is_done();
    });
  }
};

}  // namespace vlk