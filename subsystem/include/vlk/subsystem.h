

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "stx/async.h"
#include "stx/option.h"
#include "stx/resource.h"
#include "stx/span.h"
#include "vlk/utils/utils.h"

namespace vlk {
struct SubsystemsContext;

template <typename Target, typename Source>
stx::Option<Target*> upcast(Source& source) {
  Target* upcast_ptr = dynamic_cast<Target*>(&source);
  if (upcast_ptr == nullptr) return stx::None;
  return stx::Some(static_cast<Target*>(upcast_ptr));
}

struct Subsystem {
  template <typename Target>
  stx::Option<Target*> as() {
    return upcast<Target, Subsystem>(*this);
  }

  template <typename Target>
  stx::Option<Target const*> as() const {
    return upcast<Target const, Subsystem const>(*this);
  }
};

struct SubsystemImpl : public Subsystem {
  virtual stx::FutureAny get_future() = 0;
  virtual void tick(std::chrono::nanoseconds, SubsystemsContext&) {}

  virtual ~SubsystemImpl() {}
};

enum class SubsystemError : uint8_t { Exists };

struct SubsystemImplInfo {
  std::unique_ptr<SubsystemImpl> impl;
  stx::FutureAny cancelation_future;
};

// stable address map
using SubsystemsMap = std::map<std::string, SubsystemImplInfo, std::less<>>;

struct SubsystemsContext {
  STX_MAKE_PINNED(SubsystemsContext)

  friend struct SubsystemsRegistry;
  friend struct SubsystemsController;

  stx::Option<Subsystem*> get(std::string_view identifier) const {
    auto pos = map_.find(identifier);
    if (pos != map_.end()) {
      return stx::Some(static_cast<Subsystem*>(pos->second.impl.get()));
    } else {
      return stx::None;
    }
  }

  stx::Span<std::string_view const> enumerate_subsystems() const {
    return enumeration_;
  }

 private:
  SubsystemsMap map_;
  std::vector<std::string_view> enumeration_;
};

// registration only happens at startup (once)
struct SubsystemsRegistry {
  stx::Result<stx::NoneType, SubsystemError> register_subsystem(
      std::string identifier, std::unique_ptr<SubsystemImpl>&& subsystem) {
    auto cancelation_future = subsystem->get_future();
    auto [pos, was_inserted] = context_->map_.emplace(
        std::move(identifier),
        SubsystemImplInfo{std::move(subsystem), std::move(cancelation_future)});

    if (was_inserted) {
      context_->enumeration_.push_back(pos->first);
      return stx::Ok(stx::NoneType{});
    } else {
      return stx::Err(SubsystemError::Exists);
    }
  }

 private:
  SubsystemsContext* context_;
};

// registration only happens at startup (once)
struct SubsystemsController {
  void begin_shutdown() {
    for (auto& [name, info] : context_->map_) {
      info.cancelation_future.request_cancel();
    }
  }

 private:
  SubsystemsContext* context_;
};

}  // namespace vlk
