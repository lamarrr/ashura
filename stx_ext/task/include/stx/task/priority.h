#pragma once

#include "stx/limits.h"

namespace stx {

/// Some tasks can take an unreasonably long time to complete. and we can't wait
/// for them to complete, we therefore need these priorities along with a
/// cancelation mechanism.
/// this will enable us make smart decisions about graceful shutdown and task
/// prioritization.
///

// used to notify executors of the task's priority. the executor must conform to
// the required properties of each task priority.
//
enum class TaskPriority : uint8_t {
  // can be force-canceled and suspended. involves tasks that need to be run in
  // the background. i.e. checking wifi status, polling peripherral status, etc.
  //
  // these tasks need not be executed immediately but can be executed once there
  // is no important tasks that need execution.
  //
  //
  Background,
  // can be force-canceled and suspended. involves tasks that the user needs to
  // observe its result as soon as possible. i.e. image loading and decoding,
  // texture loading, offscreen rendering, etc.
  //
  // these tasks can be terminated without consequences.
  //
  Interactive,
  // once execution of this task begins, the executor must ensure it is
  // completed before shutting down and must not be force-canceled in the
  // process.
  //
  // If the executor is shut down before the task arrives, the
  // executor must mark the task as force-canceled.
  //
  // Critical tasks can involve tasks saving user data. i.e. backing up user
  // data, saving changes to disk, etc.
  //
  // this is similar but simpler and more intuitive than taking a `shared_ptr`
  // or `ExecutorKeepAlive` token.
  //
  //
  Critical = stx::u8_max
};

}  // namespace stx
