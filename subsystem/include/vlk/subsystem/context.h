#pragma once

#include <map>
#include <string_view>
#include <utility>
#include <vector>

#include "stx/option.h"
#include "stx/span.h"
#include "stx/struct.h"
#include "vlk/subsystem.h"

namespace vlk {

enum class SubsystemError : uint8_t { Exists };

struct SubsystemsContext {
  SubsystemsContext() = default;

  stx::Option<stx::Rc<Subsystem*>> get(std::string_view identifier) const {
    auto pos = map_.find(identifier);
    if (pos != map_.end()) {
      return stx::Some(pos->second.share());
    } else {
      return stx::None;
    }
  }

  stx::Span<std::string_view const> enumerate_subsystems() const {
    return enumeration_;
  }

  void __link() const {
    for (auto const& [name, subsys] : map_) {
      subsys.handle->link(*this);
    }
  }

  void __begin_shutdown() const {
    for (auto const& [identifier, info] : map_) {
      info.handle->get_future().request_cancel();
    }
  }

  bool __is_all_shutdown() const {
    return std::all_of(map_.begin(), map_.end(), [](auto const& entry) {
      return entry.second.handle->get_future().is_done();
    });
  }

  void __tick(std::chrono::nanoseconds interval) {
    for (auto& [name, subsys] : map_) {
      subsys.handle->tick(interval);
    }
  }

  template <typename SubSystemType>
  stx::Result<stx::NoneType, SubsystemError> __register_subsystem(
      std::string identifier, stx::Rc<SubSystemType*> subsystem) {
    static_assert(std::is_base_of_v<Subsystem, SubSystemType>);
    auto subsystem_impl = stx::cast<Subsystem*>(std::move(subsystem));
    auto [pos, was_inserted] =
        map_.emplace(std::move(identifier), std::move(subsystem_impl));

    if (was_inserted) {
      enumeration_.push_back(pos->first);
      return stx::Ok(stx::NoneType{});
    } else {
      return stx::Err(SubsystemError::Exists);
    }
  }

 private:
  std::map<std::string, stx::Rc<Subsystem*>, std::less<>> map_;
  std::vector<std::string_view> enumeration_;
};

}  // namespace vlk
