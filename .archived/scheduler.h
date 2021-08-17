#pragma once

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>
#include <variant>

#include "stx/async.h"
#include "stx/fn.h"
#include "stx/mem.h"
#include "stx/option.h"
#include "stx/str.h"
#include "stx/task/chain.h"
#include "stx/task/priority.h"
#include "stx/vec.h"
#include "stx/void.h"
#include "vlk/scheduler/thread_pool.h"
#include "vlk/subsystem/impl.h"
#include "vlk/utils.h"

namespace vlk {

using namespace std::chrono_literals;
using std::string_view;
using std::chrono::nanoseconds;
using stx::Chain;
using stx::Fn;
using stx::Future;
using stx::FutureAny;
using stx::FutureStatus;
using stx::Promise;
using stx::PromiseAny;
using stx::Rc;
using stx::RcFn;
using stx::RcStr;
using stx::TaskPriority;
using stx::Vec;
using stx::Void;

// rc but doesn't really need to be shared across threads.
//
// TODO(lamarrr): should the static string be in the string handling package or
// the mem package?
//
// stx::str::make_static - with no-op
// stx::str::make_static_rc - with no-op
//
//
//
struct TaskTraceInfo {
  RcStr content = stx::str::make_static_rc("[Unspecified Context]");
  RcStr purpose = stx::str::make_static_rc("[Unspecified Purpose]");
};

enum class TaskPollStatus { Ready, Awaiting, Cancel };

struct TaskScheduler;

// cancel itself???? i.e. via if statements
//
// # Join
//
// this will accept a transform method that transforms the forked
// futures into a single type, the type is used as the output of
// the fork.
//
// i.e.
// i32 fn(Future<f32>&&, Future<f64>&&) => Future<i32>
//
//
// # Fork
//
// this will accept a callback that transforms a single input future into
// multiple futures.
//
// i.e.
// tuple<i32, f32> fn(Future<i32>&&) ==> tuple<Future<i32>,
// Future<f32>>
//
// each future can then be used to do whatever
//
//
// we really want to be able to delegate and hop off to another executor
//
//
//
struct Task {
  // this is the final task to be executed on **another thread**.
  // must only be invoked by one thread at a point in time.
  RcFn<void()> fn;

  // how will this be awaited??? or should this be part of task and it should
  // have a type tag for forking????
  //
  //
  // used for dynamic scheduling i.e. scheduling more tasks after the present
  // task has finished.
  //
  // always called on the main scheduler thread once the task is done. it will
  // always be executed even if the task is canceled or the executor begins
  // shutdown.
  //
  // typically used for fork/join.
  //
  //
  //
  // used for mapping the output of a future onto another ??? i.e. wanting to
  // submit tasks from the task itself.
  // i.e. fork/self-split on receiving inputs?
  // how will this work????
  //
  // will
  //
  // has a few advantages, shutdown is handled properly
  //
  // can be used to extend itself??? what about if it dynamically wants to
  // schedule on another executor??? will it be able to make that decision on
  // the executor
  //
  // can other executors do same?? i.e. if we want to do same for http executor
  //
  //
  // it's associated futures are pre-created and type-erased since we can't
  // figure that out later on
  //
  //
  // this can be used for implementing generators, though it'd probably need a
  // collection mechanism.
  //
  RcFn<void(TaskScheduler &)> deferred_schedule;  // dynamic fork/join

  // used to ask if the task is ready for execution.
  // called on scheduler thread.
  //
  // argument is time since schedule.
  //
  // this is used for deferred execution: deferred cancelation (timeouts),
  // awaiting of futures.
  //
  RcFn<TaskPollStatus(nanoseconds)> poll_ready;

  TaskPriority priority{};

  TaskTraceInfo trace_info{};
};

struct TaskData {
  Task task;
  // result output
  //
  // used to observe terminal state of the task by the scheduler,
  //
  // this is used for deferred_schedule and removing the task from the queue.
  //
  // shared across threads and needs to be captured by the packaged_task, thus
  // requiring it to be placed in a different address space from the
  // packaged_task.
  //
  //
  // we also shouldn't be relying on this future as a source of truth?
  FutureAny future;

  FutureStatus status_capture = FutureStatus::Scheduled;
};

enum class TaskEntryState {
  Scheduled,
  // executing on the execution unit
  Executing,
  // suspended by user
  Suspended,
  // forced to suspension due to scheduling
  ForceSuspended,
  // canceled by the user
  Canceled,
  // forced to cancel due to shutdown of executor/system
  ForceCanceled
};

// Scheduler Requirements
//
// - Priority-based scheduling
// - we want the highest priority tasks to run first
// - but they could run for potentially long periods of time and starve other
// threads so we want to be able to preempt them once a certain limit is reached
// so other low priority tasks can be executed
//
// - Round Robin scheduling?
//
struct TaskEntryTracking {
  TaskEntryState state{TaskEntryState::Scheduled};
  // ======= HEURISTICS DATA ===========//

  // relative to scheduler initialization timepoint
  //
  //
  // or from program start point?========== to sync with other tracers
  //
  //
  nanoseconds schedule_timepoint;
  // time since last execution
  nanoseconds last_execution_timepoint;
  nanoseconds preempt_start;
  nanoseconds last_execution_interval;
  /* is_shutting_down * {priority} *
   * is_ready * {submit_timepoint>starvation}  */
  // preempt timepoint
};

// helper methods that ask for allocators

namespace sched {
// helper functions
};

//
//
//
// scheduler should be simple and just collect the task struct
//
// one single method => schedule( ... )
//
//
//
struct TaskScheduler final : public SubsystemImpl {
  //
  //
  // if task is a ready one, add it to the schedule timeline immediately. this
  // should probably be renamed to the execution timeline.
  //
  //
  // if possible model everything into a single queue
  //
  //
  // if it is a deferred one.... add it to the deferred queue.
  //
  //
  void schedule(Task &&task);

  TaskScheduler(std::chrono::steady_clock::time_point reference_timepoint);

  stx::FutureAny get_future() final {
    return stx::FutureAny{cancelation_promise.get_future()};
  }

  void link(SubsystemsContext const &) override {}
  void tick(nanoseconds) override {
    // if cancelation requested,
    // begin shutdown sequence
    // cancel non-critical tasks
  }

  Vec<Task> entries;
  Promise<void> cancelation_promise;
};

}  // namespace vlk
