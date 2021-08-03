#pragma once

#include <cinttypes>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "stx/mem.h"
#include "stx/result.h"
#include "vlk/subsystem/map.h"

namespace vlk {

enum class SubsystemError : uint8_t { Exists };

// registration only happens at startup (once)
struct SubsystemsRegistry {
  friend struct SubsystemsContext;

  template <typename SubSystemType>
  stx::Result<stx::NoneType, SubsystemError> register_subsystem(
      std::string identifier, stx::mem::Rc<SubSystemType> const& subsystem) {
    static_assert(std::is_base_of_v<SubsystemImpl, SubSystemType>);
    auto subsystem_impl = stx::mem::cast<SubsystemImpl>(subsystem);
    auto cancelation_future = subsystem_impl.get()->get_future();
    auto [pos, was_inserted] =
        map_.emplace(std::move(identifier),
                     SubsystemImplInfo{std::move(subsystem_impl),
                                       std::move(cancelation_future)});

    if (was_inserted) {
      enumeration_.push_back(pos->first);
      return stx::Ok(stx::NoneType{});
    } else {
      return stx::Err(SubsystemError::Exists);
    }
  }

 private:
  SubsystemsMap map_;
  std::vector<std::string_view> enumeration_;
};

}  // namespace vlk
