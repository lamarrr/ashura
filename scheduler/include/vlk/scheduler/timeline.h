#pragma once

#include <algorithm>
#include <chrono>
#include <thread>
#include <utility>

#include "stx/async.h"
#include "stx/task/id.h"
#include "stx/task/priority.h"
#include "stx/vec.h"
#include "vlk/scheduler/thread_slot.h"

namespace vlk {

using timepoint = std::chrono::steady_clock::time_point;
using namespace std::chrono_literals;
using std::chrono::nanoseconds;
using stx::Allocator;
using stx::CancelRequest;
using stx::FutureStatus;
using stx::Option;
using stx::PromiseAny;
using stx::Rc;
using stx::RcFn;
using stx::RequestedCancelState;
using stx::RequestSource;
using stx::Span;
using stx::TaskId;
using stx::TaskPriority;
using stx::Vec;

enum class ThreadId : uint32_t {};

struct ScheduleTimeline {
  static constexpr nanoseconds INTERRUPT_PERIOD{16ms};
  static constexpr uint8_t STARVATION_FACTOR{4};
  static constexpr nanoseconds STARVATION_PERIOD =
      INTERRUPT_PERIOD * STARVATION_FACTOR;

  struct Task {
    RcFn<void()> fn;
    TaskId id{};
    TaskPriority priority = stx::NORMAL_PRIORITY;
    PromiseAny promise;

    // scheduling parameters
    // must be initialized to the timepoint the task became ready
    timepoint last_preempt_timepoint{};
    FutureStatus last_status_poll = FutureStatus::Scheduled;
    RequestedCancelState last_requested_cancel_state_poll =
        RequestedCancelState::None;
  };

  explicit ScheduleTimeline(Allocator allocator)
      : starvation_timeline{allocator}, thread_slots_capture{allocator} {}

  auto add_task(RcFn<void()> fn, TaskId id, TaskPriority priority,
                PromiseAny promise, timepoint present_timepoint) {
    return starvation_timeline.push(Task{
        std::move(fn), id, priority, std::move(promise), present_timepoint});
  }

  void remove_done_and_canceled_tasks() {
    auto new_starvation_timeline_end =
        std::remove_if(starvation_timeline.begin(), starvation_timeline.end(),
                       [](Task const& task) {
                         FutureStatus status = task.last_status_poll;
                         return status == FutureStatus::Completed ||
                                status == FutureStatus::Canceled ||
                                status == FutureStatus::ForceCanceled;
                       });

    starvation_timeline.erase(new_starvation_timeline_end,
                              starvation_timeline.end());
  }

  void update_records(timepoint present_timepoint) {
    // update all our record of the tasks' statuses

    //
    //
    // NOTE: the task could still be running whilst cancelation was requested.
    // it just means we get to remove it from taking part in future scheduling.
    //
    // if the task is already running, it either has to attend to the
    // cancel request, attend to the suspend request, or complete.
    // if it makes modifications to the terminal state, after we have made
    // changes to it, its changes are ignored. and if it has reached a terminal
    // state before we attend to the request, our changes are ignored.
    //
    //
    //
    std::for_each(
        starvation_timeline.begin(), starvation_timeline.end(),
        [present_timepoint](Task& task) {
          CancelRequest const cancel_request =
              task.promise.fetch_cancel_request();

          task.last_requested_cancel_state_poll = cancel_request.state;

          if (cancel_request.state == RequestedCancelState::Canceled) {
            if (cancel_request.source == RequestSource::Executor) {
              task.promise.notify_force_canceled();
            } else {
              task.promise.notify_user_canceled();
            }
          }

          // the status could have been modified in another thread, so we need
          // to fetch the status
          FutureStatus new_status = task.promise.fetch_status();

          task.last_status_poll = new_status;

          // TODO(lamarrr): forward events to scheduler trace system.
          // though this would mean we won't be able to observe the task's state
          // immediately after the event happens?? but that's okay????
          //
          //

          // if preempt timepoint not already updated, update it
          if (task.last_status_poll != FutureStatus::ForceSuspended &&
              new_status == FutureStatus::ForceSuspended) {
            task.last_preempt_timepoint = present_timepoint;
          }
        });

    remove_done_and_canceled_tasks();
  }

  // returns the end of the starving tasks (non-suspended)
  auto sort_and_partition_timeline() {
    // split into ready to execute (pre-empted or running) and user-suspended
    // partition
    auto const starvation_timeline_starving_end = std::partition(
        starvation_timeline.begin(), starvation_timeline.end(),
        [](Task const& task) {
          return task.last_status_poll != FutureStatus::Suspended;
        });

    // sort by preemption/starvation duration (descending)
    std::sort(starvation_timeline.begin(), starvation_timeline_starving_end,
              [](Task const& a, Task const& b) {
                return a.last_preempt_timepoint > b.last_preempt_timepoint;
              });

    return starvation_timeline_starving_end;
  }

  // returns end of selections (sorted by priority in descending order)
  auto select_tasks_for_slots(size_t num_slots) {
    auto const begin = starvation_timeline.begin();
    auto timeline_selection_it = begin;

    // select only starving and ready tasks
    auto starving_end = sort_and_partition_timeline();

    timepoint const most_starved_task_timepoint =
        starvation_timeline[0].last_preempt_timepoint;

    nanoseconds selection_period_span = STARVATION_PERIOD;

    while (timeline_selection_it < starving_end) {
      if ((timeline_selection_it->last_preempt_timepoint -
           most_starved_task_timepoint) <= selection_period_span) {
        // add to timeline selection
      } else if (static_cast<size_t>(timeline_selection_it - begin) <
                 num_slots) {
        // if there's not enough tasks to fill up all the slots, extend
        // the starvation period span
        selection_period_span += STARVATION_PERIOD;
      } else {
        break;
      }

      timeline_selection_it++;
    }

    // sort selection span by priority
    std::sort(
        starvation_timeline.begin(), timeline_selection_it,
        [](Task const& a, Task const& b) { return a.priority > b.priority; });

    size_t num_selected = timeline_selection_it - starvation_timeline.begin();

    return std::min(num_slots, num_selected);
  }

  void tick(Span<Rc<ThreadSlot*> const> slots, timepoint present_timepoint) {
    // cancelation and suspension isn't handled in here, it doesn't really make
    // sense to handle here. if the task is fine-grained enough, it'll be
    // canceled as soon as its first phase finishes execution. this has the
    // advantage that we don't waste scheduling efforts.

    size_t const num_slots = slots.size();

    stx::vec::resize(thread_slots_capture, num_slots).unwrap();

    // fetch the status of each thread slot
    std::transform(slots.begin(), slots.end(), thread_slots_capture.begin(),
                   [](Rc<ThreadSlot*> const& rc_slot) {
                     return rc_slot.handle->slot.query();
                   });

    update_records(present_timepoint);

    if (starvation_timeline.size() == 0) return;

    size_t const num_selected = select_tasks_for_slots(num_slots);

    // request suspend of non-selected tasks,
    //
    // we only do this if the task is not already force suspended since it could
    // be potentially expensive if the promises are very far apart in memory.
    //
    // we don't expect just-suspended tasks to suspend immediately, even if
    // they do we'll process them in the next tick and our we account for that.
    //
    for (Task const& task :
         Span<Task const>{starvation_timeline}.subspan(num_selected)) {
      if (task.last_status_poll != FutureStatus::ForceSuspended) {
        task.promise.request_force_suspend();
      }
    }

    // add tasks to slot if not already on the slots
    size_t next_slot = 0;

    // push the tasks onto the task slots if the task is not already on any of
    // the slots.
    //
    // the selected tasks might not get slots assigned to them. i.e. if tasks
    // are still using some of the slots. tasks that don't get assigned to slots
    // here will get assigned in the next tick
    //
    for (Task const& task :
         Span<Task const>{starvation_timeline}.subspan(0, num_selected)) {
      auto task_pos =
          std::find_if(thread_slots_capture.begin(), thread_slots_capture.end(),
                       [&task](ThreadSlot::Query const& query) {
                         return query.executing_task.contains(task.id) ||
                                query.pending_task.contains(task.id);
                       });

      bool has_slot = task_pos != thread_slots_capture.end();

      while (next_slot < num_slots && !has_slot) {
        if (thread_slots_capture[next_slot].can_push) {
          task.promise.clear_force_suspension_request();
          slots[next_slot].handle->slot.push_task(
              ThreadSlot::Task{task.fn.share(), task.id});
          has_slot = true;
        }

        next_slot++;
      }
    }
  }

  // pending tasks
  // TODO(lamarrr):  newly ready tasks must be marked as suspended
  //
  Vec<Task> starvation_timeline;
  Vec<ThreadSlot::Query> thread_slots_capture;
};


// TODO(lamarrr): move execution of the timeline to the worker threads
// they go to the slots and sort them, satisfy pending streams if any
//
// they need to sleep if no task is available
//


}  // namespace vlk
