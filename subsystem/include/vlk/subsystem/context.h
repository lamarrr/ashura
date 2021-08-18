#pragma once

#include <string_view>
#include <utility>
#include <vector>

#include "stx/option.h"
#include "stx/span.h"
#include "stx/struct.h"
#include "vlk/subsystem/impl.h"
#include "vlk/subsystem/map.h"
#include "vlk/subsystem/registry.h"

namespace vlk {

struct SubsystemsContext {
  explicit SubsystemsContext(SubsystemsRegistry&& registry)
      : map_{std::move(registry.map_)},
        enumeration_{std::move(registry.enumeration_)} {}

  stx::Option<stx::mem::Rc<Subsystem>> get(std::string_view identifier) const {
    auto pos = map_.find(identifier);
    if (pos != map_.end()) {
      return stx::Some(stx::mem::cast<Subsystem>(pos->second.impl.share()));
    } else {
      return stx::None;
    }
  }

  stx::Span<std::string_view const> enumerate_subsystems() const {
    return enumeration_;
  }

 protected:
  SubsystemsMap map_;
  std::vector<std::string_view> enumeration_;
};

}  // namespace vlk
