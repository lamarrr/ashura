

#include <tuple>
#include <utility>

#include "vlk/scheduler.h"

namespace vlk {

// take in invocables instead
namespace sched {
template <typename Fn, typename... Inputs>
auto await_all(vlk::TaskScheduler &scheduler, Fn &&task, TaskPriority priority,
               TaskTraceInfo trace_info, Future<Inputs> &&... inputs) {
  static_assert(std::is_invocable_v<Fn &, Future<Inputs> &&...>);

  using output = std::invoke_result_t<Fn &, Future<Inputs> &&...>;

  std::array<FutureAny, sizeof...(Inputs)> await_futures{FutureAny{inputs}...};

  RcFn<TaskPollStatus(nanoseconds)> readiness_fn =
      stx::make_functor_fn(scheduler.allocator, [await_futures_ =
                                                     std::move(await_futures)](
                                                    nanoseconds) {
        bool all_ready = std::all_of(
            await_futures_.begin(), await_futures_.end(),
            [](FutureAny const &future) { return future.is_done(); });

        return all_ready ? TaskPollStatus::Ready : TaskPollStatus::Awaiting;
      }).unwrap();

  std::tuple<Future<Inputs>...> args{std::move(inputs)...};

  Promise promise = stx::make_promise<output>(scheduler.allocator).unwrap();
  Future future = promise.get_future();

  RcFn<void()> fn =
      stx::make_functor_fn(scheduler.allocator, [task_ = std::move(task),
                                                 args_ = std::move(args),
                                                 promise_ = std::move(
                                                     promise)]() mutable {
        if constexpr (!std::is_void_v<output>) {
          output &&result = std::apply(task_.get(), std::move(args_));
          promise_.notify_completed(std::move(result));
        } else {
          std::apply(task_, std::move(args_));
          promise_.notify_completed();
        }
      }).unwrap();

  scheduler
      .schedule(Task{std::move(fn), std::move(readiness_fn),
                     stx::make_static_fn(no_deferred_schedule), priority,
                     std::move(trace_info)})
      .unwrap();

  return future;
}

template <typename Fn, typename... Inputs>
auto await_any(vlk::TaskScheduler &scheduler, Fn &&task, TaskPriority priority,
               TaskTraceInfo trace_info, Future<Inputs> &&... inputs) {
  static_assert(std::is_invocable_v<Fn &, Future<Inputs> &&...>);

  using output = std::invoke_result_t<Fn &, Future<Inputs> &&...>;

  std::array<FutureAny, sizeof...(Inputs)> await_futures{FutureAny{inputs}...};

  RcFn<TaskPollStatus(nanoseconds)> readiness_fn =
      stx::make_functor_fn(scheduler.allocator, [await_futures_ =
                                                     std::move(await_futures)](
                                                    nanoseconds) {
        bool any_ready = std::any_of(
            await_futures_.begin(), await_futures_.end(),
            [](FutureAny const &future) { return future.is_done(); });

        return any_ready ? TaskPollStatus::Ready : TaskPollStatus::Awaiting;
      }).unwrap();

  std::tuple<Future<Inputs>...> args{std::move(inputs)...};

  Promise promise = stx::make_promise<output>(scheduler.allocator).unwrap();
  Future future = promise.get_future();

  RcFn<void()> fn =
      stx::make_functor_fn(scheduler.allocator, [task_ = std::move(task),
                                                 args_ = std::move(args),
                                                 promise_ = std::move(
                                                     promise)]() mutable {
        if constexpr (!std::is_void_v<output>) {
          output &&result = std::apply(task_.get(), std::move(args_));
          promise_.notify_completed(std::move(result));
        } else {
          std::apply(task_, std::move(args_));
          promise_.notify_completed();
        }
      }).unwrap();

  scheduler
      .schedule(Task{std::move(fn), std::move(readiness_fn),
                     stx::make_static_fn(no_deferred_schedule), priority,
                     std::move(trace_info)})
      .unwrap();

  return future;
}

}  // namespace sched
}  // namespace vlk
