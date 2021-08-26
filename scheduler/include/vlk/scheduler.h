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
using timepoint = std::chrono::steady_clock::time_point;
using stx::AllocError;
using stx::Result;

struct TaskTraceInfo {
  RcStr content = stx::str::rc::make_static("[Unspecified Context]");
  RcStr purpose = stx::str::rc::make_static("[Unspecified Purpose]");
};

enum class TaskReady { Yes, No };

constexpr TaskReady task_is_ready(nanoseconds) { return TaskReady::Yes; }

// NOTE: scheduler isn't thread-safe. don't submit tasks to them from the
// tasks.
//
struct Task {
  // this is the final task to be executed on **another thread**.
  // must only be invoked by one thread at a point in time.
  //
  RcFn<void()> fn;

  // used to ask if the task is ready for execution.
  // called on scheduler thread.
  //
  // argument is time since schedule.
  //
  // this is used for awaiting of futures or events.
  //
  RcFn<TaskReady(nanoseconds)> poll_ready;

  TaskPriority priority = stx::NORMAL_PRIORITY;

  TaskTraceInfo trace_info;
};

// used for:
//
// - conditional deferred scheduling i.e. if  a future is canceled, propagate
// the cancelation down the chain, or if an image decode task fails, propagate
// the error and don't schedule for loading on the GPU.
// - dynamic scheduling i.e. scheduling more tasks after a task has finished
//
// presents an advantage: shutdown is handled properly if all tasks are provided
// ahead of time.
//
// TODO(lamarrr): system cancelation??? coordination by the widgets??
//
struct DeferredTask {
  // always called on the main scheduler thread once the task is done. it will
  // always be executed even if the task is canceled or the executor begins
  // shutdown.
  //
  // used for mapping the output of a future onto another ??? i.e. wanting to
  // submit tasks from the task itself.
  //
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
  //
  //
  RcFn<void()> schedule;
  RcFn<TaskReady(nanoseconds)> poll_ready;
};

/*struct TaskData {
  Task task;
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
  // FutureAny future;

  // FutureStatus status_capture = FutureStatus::Scheduled;
};
*/

// TODO(lamarrr): scheduler just dispatches to the timeline once the tasks are
// ready
struct TaskScheduler final : public SubsystemImpl {
  // if task is a ready one, add it to the schedule timeline immediately. this
  // should probably be renamed to the execution timeline.
  //
  TaskScheduler(timepoint ireference_timepoint, Allocator iallocator)
      : reference_timepoint{ireference_timepoint},
        entries{iallocator},
        deferred_entries{iallocator},
        cancelation_promise{stx::make_promise<void>(iallocator).unwrap()},
        allocator{iallocator} {}

  FutureAny get_future() final {
    return FutureAny{cancelation_promise.get_future()};
  }

  void link(SubsystemsContext const &) override {}
  void tick(nanoseconds) override {
    // if cancelation requested,
    // begin shutdown sequence
    // cancel non-critical tasks
  }

  timepoint reference_timepoint;
  Vec<Task> entries;
  Vec<DeferredTask> deferred_entries;
  Promise<void> cancelation_promise;
  Allocator allocator;
};

}  // namespace vlk
