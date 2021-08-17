#pragma once

#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

#include "stx/async.h"

using timepoint = std::chrono::steady_clock::time_point;
using nanoseconds = std::chrono::nanoseconds;
using namespace std::chrono_literals;
using stx::Rc;

constexpr nanoseconds INTERRUPT_PERIOD{16ms};
constexpr uint8_t STARVATION_FACTOR{4};
constexpr nanoseconds STARVATION_PERIOD = INTERRUPT_PERIOD * STARVATION_FACTOR;

enum class ThreadId : uint32_t {};

struct Timeline {
  struct Task {
    int priority;
    timepoint last_preempt_timepoint;
    stx::PromiseAny promise;
    stx::Option<ThreadId> last_used_thread;
  };

  enum class TaskStatus : uint8_t {
    Running,
    UserSuspendRequested,
    ForceSuspended,
    UserSuspended
  };

  // struct RunningTask {
  // Task task;
  // RunningTaskStatus status = RunningTaskStatus::Running;
  // };

  Timeline() {  // ____assignment_scratch.resize(num_physical_threads_);
  }

  void tick(timepoint const present, uint32_t const num_available_slots) {
    // first update all our records of the tasks' statuses
    {
      // remove the preempted tasks from the running queue
      for (auto& running_task : running_tasks) {
        auto status = running_task.get()->promise.fetch_status();
        if (status == stx::FutureStatus::Suspended ||
            status == stx::FutureStatus::ForceSuspended) {
        }
      }

      // this doesn't mean the task has completed on the thread as it still
      // needs to exit the stack??? and setting the promise immediately isn't
      // guaranteed?
      auto new_running_submissions_end = std::remove_if(
          running_submissions.begin(), running_submissions.end(),
          [](RunningTask const& task) {
            stx::FutureStatus status = task.task.promise.fetch_status();
            return status == stx::FutureStatus::Canceled ||
                   status == stx::FutureStatus::ForceCanceled ||
                   status == stx::FutureStatus::Suspended ||
                   status == stx::FutureStatus::ForceSuspended ||
                   status == stx::FutureStatus::Completed;
          });

      running_submissions.erase(new_running_submissions_end,
                                running_submissions.end());
    }

    {
      // remove done tasks (completed, canceled, force canceled) from both the
      // running and active submissions
      auto new_running_submissions_end = std::remove_if(
          running_submissions.begin(), running_submissions.end(),
          [](RunningTask const& task) { return task.task.promise.is_done(); });

      running_submissions.erase(new_running_submissions_end,
                                running_submissions.end());

      auto new_active_submissions_end = std::remove_if(
          active_submissions.begin(), active_submissions.end(),
          [](Task const& task) { return task.promise.is_done(); });

      active_submissions.erase(new_active_submissions_end,
                               active_submissions.end());
    }

    // sort into task run timeline by last preemption start time (descending)
    std::sort(active_submissions.begin(), active_submissions.end(),
              [](Task const& a, Task const& b) {
                return a.last_preempt_timepoint >= b.last_preempt_timepoint;
              });

    if (active_submissions.size() == 0) return;

    auto most_starved_timepoint = active_submissions[0].last_preempt_timepoint;

    auto starvation_window_begin = active_submissions.begin();

    //
    //
    // TODO(lamarrr): starvation period span might be more than the others or
    // not contain enough tasks
    //
    //
    auto starvation_window_end = std::find(
        active_submissions.begin(), active_submissions.end(),
        [most_starved_timepoint](Task const& task) {
          return STARVATION_PERIOD >=
                 (task.last_preempt_timepoint - most_starved_timepoint);
        });

    // TODO(lamarrr): set this into the vector
    //
    //
    starving_tasks_bin.clear();

    starving_tasks_bin.assign(starvation_window_begin,
                              starvation_window_end == active_submissions.end()
                                  ? starvation_window_end
                                  : starvation_window_end + 1);

    if ((starvation_window_end - starvation_window_begin) <=
        num_available_slots) {
      // pick more tasks to fill it up if
      // NOTE: queue could be empty after popping the previous tasks
    }

    // pick the top n tasks from here
    std::sort(
        starving_tasks_bin.begin(), starving_tasks_bin.end(),
        [](Task const& a, Task const& b) { return a.priority >= b.priority; });

    // get begin of starvation window (period) => most starved
    // auto starvation_window_begin = starving_tasks_bin.begin();

    // TODO(lamarrr): re-check this
    //
    // ***************we need to first get the most starved task and use that as
    // our anchor
    //
    //
    //
    // get end of starvation window (period)

    // sort starvation window (period) by priority (descending)
    std::sort(
        starvation_window_begin, starvation_window_end,
        [](Task const& a, Task const& b) { return a.priority >= b.priority; });

    // select n highest-priority tasks from the window (period)
    auto num_selected = starvation_window_end - starvation_window_begin;

    // we need to not suspend already selected and running tasks.
    // but suspend ones not in the running submissions.
    //
    // we need a way to propagate this information to the next tick since we
    // can't immediately suspend
    //
    for (auto& running_submission : running_submissions) {
      auto pos = std::find_if(starvation_window_begin, starvation_window_end,
                              [&running_submission](Task const& a) {
                                return running_submission.task.id == a.id;
                              });
      if (pos == starvation_window_end) {
        running_submission.task.promise.request_force_suspend();
        running_submission.status = RunningTaskStatus::SuspendRequested;
      } else {
        // could be a task that we had already requested to suspend
        // TODO(lamarrr): pos->promise.clear_force_suspension_request();
        // ...
      }
    }

    // we don't expect just-suspended tasks to suspend immediately, even if they
    // do we'll process them in the next tick.
    //
    // running submissions might not have enough to fill the queue
    // prefer last used cpu for cache reuse
    //
    // perform assignment to the physical threads.
    //
    auto num_to_start_execution =
        num_physical_threads_ - running_submissions.size();

    //
    //
    // set last_used_thread for the newly inserted tasks to the assigned thread
    //
    //
    // multiple tasks could contend for similarly used CPUs
    //
    // by how much would cache reuse affect this?
    //
  }

  // tasks scheduled for execution that are not done (canceled, force canceled,
  // completed)
  // NOTE: these are ready to execute tasks.
  std::vector<Task> starvation_timeline;
  std::vector<Task> user_suspended_tasks;
  // vector used for calculating the
  // std::vector<Task> starving_tasks_bin;
  // std::vector<task> ____assignment_scratch;

  uint32_t num_physical_threads_;
};
