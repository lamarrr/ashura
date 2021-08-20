
#pragma once

#include <utility>

#include "stx/task/forloop.h"
#include "stx/task/loop.h"
#include "vlk/scheduler.h"

namespace vlk {

using stx::For;
using stx::Loop;
using stx::Promise;
using stx::RequestProxy;
using stx::RequestSource;
using stx::ServiceToken;

namespace sched {

template <typename Fn>
auto fn(TaskScheduler &scheduler, Fn &&fn_task, TaskPriority priority,
        TaskTraceInfo trace_info) {
  static_assert(std::is_invocable_v<Fn &>);

  using output = std::invoke_result_t<Fn &>;

  Promise promise = stx::make_promise<output>(scheduler.allocator).unwrap();
  Future future = promise.get_future();

  RcFn<void()> sched_fn =
      stx::make_functor_fn(scheduler.allocator, [fn_task_ = std::move(fn_task),
                                                 promise_ =
                                                     std::move(promise)]() {
        if constexpr (std::is_void_v<std::invoke_result_t<Fn &>>) {
          fn_task_();
          promise_.notify_completed();
        } else {
          promise_.notify_completed(fn_task_());
        }
      }).unwrap();

  scheduler
      .schedule(Task{std::move(sched_fn), stx::make_static_fn(always_ready),
                     stx::make_static_fn(no_deferred_schedule), priority,
                     std::move(trace_info)})
      .unwrap();

  return future;
}

template <typename Fn>
auto loop(TaskScheduler &scheduler, Loop<Fn> &&loop, TaskPriority priority,
          TaskTraceInfo trace_info) {
  Promise promise = stx::make_promise<void>(scheduler.allocator).unwrap();
  Future future = promise.get_future();

  RcFn<void()> fn =
      stx::make_functor_fn(scheduler.allocator, [state_ = stx::LoopState{},
                                                 loop_ = std::move(loop),
                                                 promise_ = std::move(
                                                     promise)]() mutable {
        RequestProxy proxy{promise_};

        loop_.resume(state_, proxy);

        // suspended or canceled
        if (state_.serviced) {
          ServiceToken service_token = state_.service_token;
          if (service_token.source == RequestSource::Executor) {
            if (service_token.type == stx::RequestType::Cancel) {
              promise_.notify_force_canceled();
            } else {
              promise_.notify_force_suspended();
            }
          } else {
            if (service_token.type == stx::RequestType::Cancel) {
              promise_.notify_user_canceled();
            } else {
              promise_.notify_user_suspended();
            }
          }
        } else {
          // will really never happen
          // completed
          promise_.notify_completed();
        }
      }).unwrap();

  scheduler
      .schedule(Task{std::move(fn), stx::make_static_fn(always_ready),
                     stx::make_static_fn(no_deferred_schedule), priority,
                     std::move(trace_info)})
      .unwrap();

  return future;
}

template <typename Fn>
auto forloop(TaskScheduler &scheduler, For<Fn> &&loop, TaskPriority priority,
             TaskTraceInfo trace_info) {
  Promise promise = stx::make_promise<void>(scheduler.allocator).unwrap();
  Future future = promise.get_future();

  RcFn<void()> fn =
      stx::make_functor_fn(scheduler.allocator, [state_ = stx::ForState{},
                                                 loop_ = std::move(loop),
                                                 promise_ = std::move(
                                                     promise)]() mutable {
        RequestProxy proxy{promise_};

        loop_.resume(state_, proxy);

        // suspended or canceled
        if (state_.next < loop_.end) {
          ServiceToken service_token = state_.service_token;
          if (service_token.source == RequestSource::Executor) {
            if (service_token.type == stx::RequestType::Cancel) {
              promise_.notify_force_canceled();
            } else {
              promise_.notify_force_suspended();
            }
          } else {
            if (service_token.type == stx::RequestType::Cancel) {
              promise_.notify_user_canceled();
            } else {
              promise_.notify_user_suspended();
            }
          }
        } else {
          // completed
          promise_.notify_completed();
        }
      }).unwrap();

  scheduler
      .schedule(Task{std::move(fn), stx::make_static_fn(always_ready),
                     stx::make_static_fn(no_deferred_schedule), priority,
                     std::move(trace_info)})
      .unwrap();

  return future;
}

// returns future of invoke result
template <typename Fn, typename... OtherFns>
auto chain(TaskScheduler &scheduler, Chain<Fn, OtherFns...> &&chain,
           TaskPriority priority, TaskTraceInfo trace_info) {
  using result_type = typename Chain<Fn, OtherFns...>::last_phase_result_type;
  using stack_type = typename Chain<Fn, OtherFns...>::stack_type;
  static constexpr auto num_phases = Chain<Fn, OtherFns...>::num_phases;

  Promise promise =
      stx::make_promise<result_type>(scheduler.allocator).unwrap();

  Future future = promise.get_future();

  RcFn<void()> fn =
      stx::make_functor_fn(
          scheduler.allocator,
          [state_ = stx::ChainState{}, stack_ = stack_type{stx::Void{}},
           chain_ = std::move(chain), promise_ = std::move(promise)]() mutable {
            RequestProxy proxy{promise_};

            chain_.resume(stack_, state_, proxy);

            // suspended or canceled
            if (state_.next_phase_index < num_phases) {
              ServiceToken service_token = state_.service_token;
              if (service_token.source == RequestSource::Executor) {
                if (service_token.type == stx::RequestType::Cancel) {
                  promise_.notify_force_canceled();
                } else {
                  promise_.notify_force_suspended();
                }
              } else {
                if (service_token.type == stx::RequestType::Cancel) {
                  promise_.notify_user_canceled();
                } else {
                  promise_.notify_user_suspended();
                }
              }
            } else {
              // completed
              result_type &&result = std::move(std::get<result_type>(stack_));
              promise_.notify_completed(std::move(result));
            }
          })
          .unwrap();

  scheduler
      .schedule(Task{std::move(fn), stx::make_static_fn(always_ready),
                     stx::make_static_fn(no_deferred_schedule), priority,
                     std::move(trace_info)})
      .unwrap();

  return future;
}

}  // namespace sched

}  // namespace vlk
