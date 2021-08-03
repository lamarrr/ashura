#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <new>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "stx/option.h"
#include "stx/result.h"
#include "vlk/utils.h"

// exception-safety: absolute zero
// we don't use exceptions and neither do we plan to support it

enum class LockStatus : uint8_t { Unlocked, Locked };

enum class CancelationStatus : uint8_t { Uncanceled, CancelRequested };
enum class CompletionStatus : uint8_t { Pending, Canceled, Completed };
enum class SuspendStatus : uint8_t { Unsuspended, SuspendRequested };

// source:
// https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
#ifdef __cpp_lib_hardware_interference_size
using std::hardware_constructive_interference_size;
using std::hardware_destructive_interference_size;
#else
// 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │
// ...
constexpr std::size_t hardware_constructive_interference_size =
    2 * sizeof(std::max_align_t);
constexpr std::size_t hardware_destructive_interference_size =
    2 * sizeof(std::max_align_t);
#endif

template <typename T>
struct CacheLineAligned {
  alignas(hardware_destructive_interference_size) T value;
};

struct CancelationState {
  CacheLineAligned<std::atomic<CancelationStatus>> status{
      CancelationStatus::Uncanceled};

  VLK_DEFAULT_CONSTRUCTOR(CancelationState)
  STX_DISABLE_COPY(CancelationState)
  VLK_DISABLE_MOVE(CancelationState)

  auto get_status() const {
    return status.value.load(std::memory_order_relaxed);
  }

  void request_cancel() {
    status.value.store(CancelationStatus::CancelRequested,
                       std::memory_order_relaxed);
  }

  bool is_cancel_requested() const {
    return status.value.load(std::memory_order_relaxed) ==
           CancelationStatus::CancelRequested;
  }
};

struct SuspendState {
  CacheLineAligned<std::atomic<SuspendStatus>> status{
      SuspendStatus::Unsuspended};

  VLK_DEFAULT_CONSTRUCTOR(SuspendState)
  STX_DISABLE_COPY(SuspendState)
  VLK_DISABLE_MOVE(SuspendState)

  auto get_status() const {
    return status.value.load(std::memory_order_relaxed);
  }

  void request_suspend() {
    status.value.store(SuspendStatus::SuspendRequested,
                       std::memory_order_relaxed);
  }

  bool is_suspend_requested() const {
    return status.value.load(std::memory_order_relaxed) ==
           SuspendStatus::SuspendRequested;
  }
};

// used for requesting cancelation of the submitted task.
//
// if cancelation has been requested before the function begins executing, the
// function is not called. and if it is called while the function is executing,
// the function can decide to respond to the cancelation request or ignore it
// and it's completion observer will be transitioned into the canceled state
// once the function returns.
//
// even if the function is run to completion
//
struct CancelationToken {
  friend struct CancelationObserver;

  CancelationToken() {}

  static CancelationToken create() {
    CancelationToken token;
    token.state = std::shared_ptr<CancelationState>(new CancelationState);

    return std::move(token);
  }

  auto get_status() const { return state->get_status(); }

  void request_cancel() const { state->request_cancel(); }

  bool is_cancel_requested() const { return state->is_cancel_requested(); }

  bool is_valid() const { return state == nullptr; }

 private:
  // this is aligned to the cache line size via heap allocation which should
  // make allocations chunked to std::max_align_t granularity. so we shouldn't
  // have cache coherence issues.
  std::shared_ptr<CancelationState> state;
};

struct CancelationObserver {
  explicit CancelationObserver(CancelationToken const& token) {
    state = token.state;
  }

  bool is_cancel_requested() const { return state->is_cancel_requested(); }

  auto get_status() const { return state->get_status(); }

  bool is_valid() const { return state == nullptr; }

 private:
  std::shared_ptr<CancelationState> state;
};

struct SuspendToken {
  friend struct SuspendObserver;

  SuspendToken() {}

  static SuspendToken create() {
    SuspendToken token;
    token.state = std::shared_ptr<SuspendState>(new SuspendState);

    return std::move(token);
  }

  auto get_status() const { return state->get_status(); }

  void request_suspend() const { state->request_suspend(); }

  bool is_suspend_requested() const { return state->is_suspend_requested(); }

  bool is_valid() const { return state == nullptr; }

 private:
  std::shared_ptr<SuspendState> state;
};

struct SuspendObserver {
  explicit SuspendObserver(SuspendToken const& token) { state = token.state; }

  bool is_suspend_requested() const {
    return get_status() == SuspendStatus::SuspendRequested;
  }

  SuspendStatus get_status() const {
    auto shared_state = state.lock();
    if (shared_state == nullptr) {
      return SuspendStatus::Unsuspended;
    } else {
      return shared_state->get_status();
    }
  }

 private:
  std::weak_ptr<SuspendState> state;
};

struct ThreadInfo {
  uint32_t index = 0;
};

struct PackagedTask {
  // function to be executed in the execution context
  std::function<void(ThreadInfo)> function;
};

// TODO(lamarrr): document weak_ptr recommendation
template <typename ReturnType>
struct Task {
  // used to store the function that needs execution. this function needs to be
  // thread-safe.
  std::function<ReturnType(CancelationObserver const&)> function =
      [](CancelationObserver const&) {};
  std::shared_ptr<std::string const> identifier;

  std::string_view get_identifier() const {
    if (identifier == nullptr) return "Unnamed Task";
    return *identifier;
  }
};

// result of one is passed to the other
template <typename... SubtaskTypes>
struct TaskChain {
  std::tuple<SubtaskTypes...> tasks;
};

// all executed sequentially without result dependency
template <typename... SubtaskTypes>
struct TaskSequence {
  std::tuple<SubtaskTypes...> tasks;
};

namespace impl {

// executed together on same thread, results are gathered
void chain();
// executed on different threads, but in the sequential order
void sequence();

void weave();

// TODO(lamarrr): we want to chain return types so we don't have to perform
// extra allocations for each task
//
// no memory allocations
template <typename ReturnType, typename... TaskTypes>
void then(Task<ReturnType>&&, TaskChain<TaskTypes...>&& chain) {}

// task to task chain
// task chain to task chain
// task to task
void then();
}  // namespace impl

struct ThreadExecutionContext {
  // must be thread-safe
  virtual void submit(PackagedTask&& task) { VLK_PANIC("Unimplemented"); }

  // must be thread-safe
  virtual uint32_t num_hardware_threads() { VLK_PANIC("Unimplemented"); }

  virtual ~ThreadExecutionContext() {}
};

enum class FutureError : uint8_t { Pending, Canceled };

template <typename T>
struct FutureInfo {
  std::aligned_storage_t<sizeof(T), alignof(T)> storage;
  std::atomic<CompletionStatus> status{CompletionStatus::Pending};
  std::atomic<CancelationStatus> cancelation_status{
      CancelationStatus::Uncanceled};
  std::atomic<SuspendStatus> suspend_status { SuspendStatus::Unsuspended }
};

template <typename T>
struct FutureState {
  VLK_DEFAULT_CONSTRUCTOR(FutureState)
  STX_DISABLE_COPY(FutureState)
  VLK_DISABLE_MOVE(FutureState)

  CacheLineAligned<FutureInfo<T>> info;

  auto get_status() const {
    return info.value.status.load(std::memory_order_acquire);
  }

  void unsafe_init(T&& value) { new (info.value.storage) T{std::move(value)}; }

  T unsafe_copy() const { return T{*laundered()}; }

  T unsafe_move() { return T{std::move(*laundered())}; }

  ~FutureState() {
    if (get_status() == CompletionStatus::Completed) {
      // completed and object was initialized by calling `unsafe_init`
      laundered()->~T();
    }
  }

 private:
  T* laundered() {
    return std::launder(reinterpret_cast<T*>(&(info.value.storage)));
  }

  T const* laundered() const {
    return std::launder(reinterpret_cast<T const*>(&(info.value.storage)));
  }
};

template <>
struct FutureState<void> {
  VLK_DEFAULT_CONSTRUCTOR(FutureState)
  STX_DISABLE_COPY(FutureState)
  VLK_DISABLE_MOVE(FutureState)

  auto get_status() const {
    return status.value.load(std::memory_order_acquire);
  }

  void mark_completed() {
    status.value.store(CompletionStatus::Completed, std::memory_order_release);
  }

  void mark_canceled() {
    status.value.store(CompletionStatus::Canceled, std::memory_order_release);
  }

  CacheLineAligned<std::atomic<CompletionStatus>> status{
      CompletionStatus::Pending};
};

// observes termination of an async operation.
//
// and ensures ordering of instructions or observation of the changes from
// another thread.
//
// this is contrary to the on-finished callback approach in which the user is
// very likely to use incorrectly due instruction re-ordering or order of
// observation of changes.
//
// any side-effects made by the callback function (reference capture or program
// state modifications) must not be observed until `is_finished` returns true or
// `await_finish` is called. this means that we don't require exclusive locking
// of the values being modified (i.e. using a mutex).
//
//
//
// the captured reference's memory address should also be aligned as
// `std::hardware_destructive_interference_size` to prevent cache coherency
// issues.
//
//
//
// this also means only one task must capture the referenced values. otherwise,
// exclusive locking is required to ensure that the values aren't being written
// to across different worker threads.
//
// for canceling a task and submitting a new one, the user has to use the
// cancelation token and call `await_finish` which will block the calling thread
// until the task's cancelation is acknowledged, or the user has to specifically
// use a mutex to lock the captured references to ensure multiple worker threads
// don't write to it at once.
//
// this helps prevent the user from writing ugly hacks like
// `std::shared_ptr<std::atomic<TaskStatus>>` which they might not even use
// correctly.
// this also prevents having the user from manually writing code to track state
// of each submitted task.
//
template <typename T>
struct Future {
  template <typename P>
  friend struct Promise;

  static auto create() {
    Future future;
    future.state = std::shared_ptr<FutureState<T>>{new FutureState<T>};
    return std::move(future);
  }

  stx::Result<T, FutureError> move() {
    switch (state->get_status()) {
      case CompletionStatus::Pending:
        return stx::Err(FutureError::Pending);
      case CompletionStatus::Canceled:
        return stx::Err(FutureError::Canceled);
      case CompletionStatus::Completed:
        return stx::Ok(state->unsafe_move());
    }
  }

  CompletionStatus get_status() const { return state->get_status(); }

  stx::Result<T, FutureError> copy() const {
    CompletionStatus status = state->info.status();
    if (status == CompletionStatus::Pending) {
      return stx::Err(FutureError::Pending);
    } else if (status == CompletionStatus::Canceled) {
      return stx::Err(FutureError::Canceled);
    } else {
      return stx::Ok(state->unsafe_copy());
    }
  }

  bool is_valid() const { return state != nullptr; }

 private:
  std::shared_ptr<FutureState<T>> state;
};

template <>
struct Future<void> {
  template <typename P>
  friend struct Promise;

  static auto create() {
    Future future;
    future.state = std::shared_ptr<FutureState<void>>{new FutureState<void>};
    return std::move(future);
  }

  CompletionStatus get_status() const { return state->get_status(); }

  bool is_valid() const { return state != nullptr; }

 private:
  std::shared_ptr<FutureState<void>> state;
};

template <typename RootTaskType, typename NewTaskType>
TaskChain<RootTaskType, NewTaskType> then(Task<RootTaskType>&& first_task,
                                          Task<NewTaskType>&& task);

// TODo(LAMARRR): separate the template parameters
template <typename... TaskTypes>
TaskChain<TaskTypes..., void> then(TaskChain<TaskTypes...>&& first_task,
                                   Task<void>&& task);

namespace impl {
inline void backoff_spin(uint64_t iteration) {
  if (iteration < 64) {
    // immediate spinning
  } else if (iteration < 128) {
    // if there are any threads that need execution, let them execute before
    // attending to us
    std::this_thread::yield();
  } else {
    // sleep for a specific amount of time
    std::this_thread::sleep_for(std::chrono::milliseconds(125));
  }
}

template <typename Predicate>
void block_on(Predicate&& predicate) {
  uint64_t uneventful_iterations = 0;
  while (!predicate()) {
    uneventful_iterations++;
    impl::backoff_spin(uneventful_iterations);
  }
}

// TODO(lamarrr): cleanup
//
// acquire exclusive access, non-blocking
inline bool try_acquire_lock(std::atomic<LockStatus>& lock) {
  LockStatus expected = LockStatus::Unlocked;
  return lock.compare_exchange_strong(expected, LockStatus::Locked,
                                      std::memory_order_acquire,
                                      std::memory_order_relaxed);
}

// acquire exclusive access, blocking
inline void blocking_acquire_lock(std::atomic<LockStatus>& lock) {
  block_on([&lock] { return try_acquire_lock(lock); });
}

// release exclusive access, must have been acquired
void release_lock(std::atomic<LockStatus>& lock) {
  lock.store(LockStatus::Unlocked, std::memory_order_release);
}

template <typename FuncSucc, typename FuncFail>
auto try_acquire_then(std::atomic<LockStatus>& lock,
                      FuncSucc&& on_acquire_succeed,
                      FuncFail&& on_acquire_failed) {
  if (try_acquire_lock(lock)) {
    auto result = static_cast<FuncSucc&&>(on_acquire_succeed)();
    release_lock(lock);
    return std::move(result);
  } else {
    return static_cast<FuncFail&&>(on_acquire_failed)();
  }
}

}  // namespace impl

template <typename ReturnType>
void block_on(Future<ReturnType> const& future) {
  impl::block_on([&future]() {
    switch (future.get_status()) {
      case CompletionStatus::Completed:
        return true;
      case CompletionStatus::Canceled:
        return true;
      case CompletionStatus::Pending:
        return false;
    }
  });
}

template <typename T>
struct Promise {
  explicit Promise(Future<T> const& future) { state = future.state; }

  void finish(CancelationStatus status, T&& value) const {
    auto shared_state = state->lock();

    if (shared_state == nullptr) {
      // the user is no longer interested in the result as they've discarded the
      // future and the future is no longer existing
      return;
    } else {
      switch (status) {
        case CancelationStatus::Proceed: {
          shared_state->unsafe_init(std::move(value));
          shared_state->mark_completed();
        } break;
        case CancelationStatus::CancelRequested: {
          shared_state->mark_canceled();
        } break;
      }
    }
  }

  void finish_canceled() const {
    auto shared_state = state->lock();

    if (shared_state == nullptr) {
      // the user is no longer interested in the result as they've discarded the
      // future and the future is no longer existing
      return;
    } else {
      shared_state->state.mark_canceled();
    }
  }

 private:
  std::weak_ptr<FutureState<T>> state;
};

template <>
struct Promise<void> {
  explicit Promise(Future<void> const& future) { state = future.state; }

  void finish(CancelationStatus status) const {
    auto shared_state = state.lock();

    if (shared_state == nullptr) {
      // the user is no longer interested in the result as they've discarded the
      // future and the future is no longer existing
      return;
    } else {
      switch (status) {
        case CancelationStatus::Uncanceled: {
          shared_state->mark_completed();
        } break;
        case CancelationStatus::CancelRequested: {
          shared_state->mark_canceled();
        } break;
      }
    }
  }

  void finish_canceled() const {
    auto shared_state = state.lock();

    if (shared_state == nullptr) {
      // the user is no longer interested in the result as they've discarded the
      // future and the future is no longer existing
      return;
    } else {
      shared_state->mark_canceled();
    }
  }

 private:
  std::weak_ptr<FutureState<void>> state;
};

// TODO(lamarrr): package task function

// TODO(lamarrr): how do we handle this scenario? submit task, cancel, submit
// another, we don't want to have to wait till the canceled one finishes as it
// could be modifying a value and not respond to the cancelation request.
//
//
// All async operations in the application should be cancelable, the async
// operations should try to be cancelable but are not enforced to.
//
//
// on handling cancelation: if we were to return a future with a result, we
// would need to
//
//
// once scheduler is dropped, all pending tasks would be requested to cancel
//
struct ThreadTaskScheduler {
  //
  //
  // even if the user requests for cancelation and we are unable to service the
  // request, we still have to
  //
  //
  //
  // we can't stop the function from running to completion even if a cancelation
  // is requested
  //
  // tasks are submitted for execution in a FIFO order
  // TODO(lamarrr): need to be able to submit from multiple threads
  // should support submitting one without completion observer or cancelation?
  // a fire-and-forget-style is almost impossible to use
  //
  // discarding the completion observer means you are no longer interested in
  // the result of the computation
  // TODO(lamarrr): this should be a specialization of one that returns void

  template <typename T>
  std::pair<Future<T>, CancelationToken> schedule(Task<T>&& task);

  std::pair<Future<void>, CancelationToken> schedule(Task<void>&& task) {
    auto future = Future<void>::create();
    auto cancelation_token = CancelationToken::create();

    execution_context_->submit(PackagedTask{
        [cancelation_observer = CancelationObserver{cancelation_token},
         promise = Promise<void>(future),
         task_moved = std::move(task)](ThreadInfo info) {
          if (cancelation_observer.is_cancel_requested()) {
            promise.finish_canceled();
          }
          task_moved.function(cancelation_observer);
          promise.finish(cancelation_observer.get_status());
          // TODO(lamarrr): use task_moved.identifier for tracing
          // we need a weak ptr to the trace context needed for logging so in
          // case the scheduler is destroyed, we'll be able to detect
        }});

    return std::pair{std::move(future), std::move(cancelation_token)};
  }

  // of-course, the long names are meant to discourage you from using them
  //
  // the executor will still have cancelation tokens to execute on the worker
  // threads
  //

  template <typename T>
  CancelationToken schedule_forget(Task<T>&& task);
  CancelationToken schedule_forget(Task<void>&& task);

  template <typename T, typename DurationRep, typename DurationPeriod>
  std::pair<Future<T>, CancelationToken> schedule_every(
      Task<T>&& task,
      std::chrono::duration<DurationRep, DurationPeriod> duration);

  template <typename DurationRep, typename DurationPeriod>
  CancelationToken schedule_forget_every(
      Task<void>&& task,
      std::chrono::duration<DurationRep, DurationPeriod> duration);

  // TODO(lamarrr): how to manage lifetimes, we need to shutdown scheduler and
  // ensure none of the tasks are running at the moment we want to shutdown.
  // we'd need a shared_ptr queue of pending taskss
  void attach_execution_context(
      std::shared_ptr<ThreadExecutionContext> execution_context) {}

  bool has_execution_context() const { return execution_context_ != nullptr; }

  void check_execution_context() const {
    VLK_ENSURE(
        has_execution_context(),
        "Task execution context has not been attached to task scheduler");
  }

  // cancel pending when destruction requested
  // store or track cancelation tokens? we should give the physical executor
  // cancelation tokens as part of the packaged task, ensures cancelation is
  // possible as long as the task is active and executing

 private:
  std::shared_ptr<ThreadExecutionContext> execution_context_;
};

struct ThreadTaskExecutor {};

// we need to be able to notify the worker thread to shutdown
struct WorkerThreadInfo {
  std::thread thread;
  CancelationToken cancelation_token;
  Future<void> task;
};

// TODO(lamarrr): think about shutting down process of the pipeline with
// execution contexts. be sure to prevent cyclic references
struct DefaultThreadExecutionContext {
  struct TaskQueueState {
    VLK_DEFAULT_CONSTRUCTOR(TaskQueueState)
    STX_DISABLE_COPY(TaskQueueState)
    VLK_DISABLE_MOVE(TaskQueueState)

    std::queue<PackagedTask> task_queue;
    std::atomic<LockStatus> lock_status = LockStatus::Unlocked;
  };

  struct TaskQueue {
    CacheLineAligned<TaskQueueState> state;

    // producer
    bool try_push(PackagedTask&& task) {
      impl::try_acquire_then(
          state.value.lock_status,
          [&task, this] {
            state.value.task_queue.push(std::move(task));
            return true;
          },
          [] { return false; });
    }

    // consumer - worker threads, locking and unlocking the task queue mutex
    // should be relatively fast and should only be used for push and try_pop.
    // the worker threads must not hold on to them while executing the tasks. we
    // also want to be able to send cancelation requests to the worker threads
    auto try_pop() {
      return impl::try_acquire_then(
          state.value.lock_status,
          [this]() -> stx::Option<PackagedTask> {
            PackagedTask task = std::move(state.value.task_queue.front());
            state.value.task_queue.pop();
            return stx::Some(std::move(task));
          },
          []() -> stx::Option<PackagedTask> { return stx::None; });
    }
  };

  // create threads
  //
  // request cancelation, mark all pending tasks as canceled, this would mean
  // packaged task would need cancelation tokens for each of the tasks
  //
  // join them
  //
  //
  // handling more submitted tasks?
  //
  std::weak_ptr<ThreadTaskExecutor> executor;

  // we don't need mutex???? for a scheduler that is
  // multithreaded trace context

  std::vector<WorkerThreadInfo> worker_threads_;

  TaskQueue task_queue_;

  void shutdown_worker_threads() {
    for (WorkerThreadInfo& worker_thread : worker_threads_) {
      worker_thread.cancelation_token.request_cancel();
      block_on(worker_thread.task);
    }
  }

  void launch_worker_threads() {
    uint32_t num_threads = std::thread::hardware_concurrency();
  }

  void worker_thread_task(MultiThreadTraceContext& execution_trace_context,
                          CancelationObserver cancelation_observer,
                          ThreadInfo info) {
    uint64_t taskless_iterations = 0;
    // sleeping procedure if no task available
    while (!cancelation_observer.is_cancel_requested()) {
      PackagedTask task;
      {
        task_queue_mutex_.lock();
        if (task_queue_.empty()) {
          task_queue_mutex_.unlock();
          // TODO(lamarrr): need to sleep and release lock here
          // exponential sleep

          continue;
        }
        task = std::move(task_queue_.front());
        task_queue_.pop();
      }
    }
  }
};
