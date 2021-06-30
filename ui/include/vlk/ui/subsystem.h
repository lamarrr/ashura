
#include <chrono>

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
struct Subsystem {
  virtual void init() {}

  // tick with subsystems map?
  virtual void tick(std::chrono::nanoseconds) {}

  virtual ~Subsystem() {}
};

// long-running tasks TaskSubsystem, maybe with cancelation, statistics,
// priority u64::min u64::max,
//

// only initialized once, should be present throughout program lifetime?
// pipeline.add_subsystem(identifier, std::unique_ptr<Subsystem>)
// pipeline.get_subsystem(identifier) // shared_ptr? or optional ref which will
// be guaranteed to always be valid
// subsystems can't be removed once added
