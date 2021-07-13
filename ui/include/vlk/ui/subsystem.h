
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "stx/option.h"
#include "vlk/utils/utils.h"
// subsystems can have different threads? or do we need a different tasking
// system?
//
//
// Guidelines
// - Long running, non-deterministic or variable-timed tasks should be submitted
// to the tasking interface
//
//
// we need an overridable tasking system to submit tasks to and should be
// multi-threaded and fully configurable, we want to minimize context-switching
// costs
//
struct SubsystemContext;

struct Subsystem {
  // tick with subsystems map?
  virtual void tick(std::chrono::nanoseconds, SubsystemContext&) {}

  template <typename Target>
  stx::Option<stx::Ref<Target>> as() {
    return upcast<Target>(*this);
  }

  template <typename Target>
  stx::Option<stx::ConstRef<Target>> as() const {
    return upcast<Target>(*this);
  }

  virtual ~Subsystem() {}
};

enum class SubsystemContextError : uint8_t { Exists };

struct SubsystemContext {
  stx::Result<stx::NoneType, SubsystemContextError> add(
      std::string name, std::unique_ptr<Subsystem>&& subsystem);

  stx::Option<stx::MutRef<Subsystem>> get(std::string_view name);

  stx::Option<stx::ConstRef<Subsystem>> get(std::string_view name) const;

 private:
  std::map<std::string, std::unique_ptr<Subsystem>, std::less<>> subsystems_;
};

// long-running tasks TaskSubsystem, maybe with cancelation, statistics,
// priority u64::min u64::max,
//

// only initialized once, should be present throughout program lifetime?
// pipeline.add_subsystem(identifier, std::unique_ptr<Subsystem>)
// pipeline.get_subsystem(identifier) // shared_ptr? or optional ref which will
// be guaranteed to always be valid
// subsystems can't be removed once added
