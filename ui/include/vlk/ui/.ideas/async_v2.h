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
#include "vlk/utils/utils.h"

// exception-safety: absolute zero
// we don't use exceptions and neither do we plan to support it

namespace vlk {
enum class LockStatus: uint8_t { Unlocked, Locked };

enum class CancelationStatus : uint8_t {
  // no cancelation requests have been received
  Uncanceled,
  // the async operation has been requested to cancel
  CancelRequested,
  // the async operation is presently being canceled
  Canceling
};

enum class CompletionStatus : uint8_t {
  // the async operation is pending completion
  Pending,
  // the async operation has completed successfully
  Completed,
  // the async operation has been canceled
  Canceled
};

enum class SuspendStatus : uint8_t {
  // the async operation is in progress
  Resumed,
  // the async operation has been suspended
  Suspended,
  // the async operation has been requested to resume execution
  ResumeRequested,
  // the async operation has been requested to resume execution
  SuspendRequested
};

enum class FutureStatus : uint8_t {
  // the async operation has been submitted to the executor for execution
  Submitted,
  // the async operation is now being executed by the executor
  Executing,
  // the async operation is now being canceled
  Canceling,
  // the async operation has been canceled
  Canceled,
  // the async operation has been suspended
  Suspended,
  // the async operation's cancelation has been requested
  CancelRequested,
  // the async operation's resumption has been requested
  ResumeRequested,
  // the async operation's suspension has been requested
  SuspendRequested
};
// Submitted
//
//
//

// requests are mutually exclusive, i.e. no two can exist at once.
// if the requests are not acknowledged, the intended effect will not happen on
// the async operation
enum class RequestStatus : uint8_t {
  // the last submitted request has been acknowledged and the effect is about to
  // happen to the async operation
  Acknowledged,
  // the async operation has been requested to cancel
  CancelRequested,
  // the async operation has been requested to resume
  ResumeRequested,
  // the async operation has been requested to suspend
  SuspendRequested
};

enum class FutureError : uint8_t { Pending, Canceled };

// TODO(primitives)

struct Counter {
  void increment();
  void add();
  void load();
  void fetch();
};

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

// each core has its cache line, so we need to ensure we are not performing
// false sharing across these cores. False sharing leads to excessive cache
// flushes and thus reduces performances as the CPU now has to read from main
// memory which is the slowest read path.
template <typename T>
struct CacheLineAligned {
  alignas(hardware_destructive_interference_size) T value;
};

struct SharedBaseState {
  VLK_DEFAULT_CONSTRUCTOR(SharedBaseState)
  STX_DISABLE_COPY(SharedBaseState)
  VLK_DISABLE_MOVE(SharedBaseState)

  CacheLineAligned<std::atomic<CompletionStatus>> completion_status{
      CompletionStatus::Pending};
  CacheLineAligned<std::atomic<CancelationStatus>> cancelation_status{
      CancelationStatus::Uncanceled};
  CacheLineAligned<std::atomic<SuspendStatus>> suspend_status{
      SuspendStatus::Resumed};

  auto acquire_completion_status() const {
    return completion_status.value.load(std::memory_order_acquire);
  }

  auto fetch_cancelation_status() const {
    return cancelation_status.value.load(std::memory_order_relaxed);
  }

  auto fetch_suspend_status() const {
    return suspend_status.value.load(std::memory_order_relaxed);
  }

  void request_cancel() {
    // successfully request for cancelation only if it is not already canceled
    // or already being canceled
    CancelationStatus expected = CancelationStatus::Uncanceled;
    CancelationStatus target = CancelationStatus::CancelRequested;

    cancelation_status.value.compare_exchange_strong(expected, target,
                                                     std::memory_order_relaxed);
  }

  void request_suspend() {
    // SuspendStatus::ResumeRequested = the
    SuspendStatus expected = SuspendStatus::Resumed;
    SuspendStatus target = SuspendStatus::SuspendRequested;

    suspend_status.value.compare_exchange_strong(expected, target,
                                                 std::memory_order_relaxed);
  }

  void request_resume() {
    // successfully request for resumption only if it is already suspended
    SuspendStatus expected = SuspendStatus::SuspendAcknowledged;
    SuspendStatus target = SuspendStatus::ResumeRequested;

    suspend_status.value.compare_exchange_strong(expected, target,
                                                 std::memory_order_relaxed);
  }

  void release_completed_state() {
    completion_status.value.store(CompletionStatus::Completed,
                                  std::memory_order_release);
  }

  void release_canceled_state() {
    completion_status.value.store(CompletionStatus::Canceled,
                                  std::memory_order_release);
  }

  [[nodiscard]] bool try_acknowledge_cancel() {
    CancelationStatus expected = CancelationStatus::CancelRequested;
    CancelationStatus target = CancelationStatus::Canceling;
    return cancelation_status.value.compare_exchange_strong(
        expected, target, std::memory_order_relaxed);
  }

  [[nodiscard]] bool try_acknowledge_suspend() {
    SuspendStatus expected = SuspendStatus::SuspendRequested;
    SuspendStatus target = SuspendStatus::SuspendAcknowledged;
    return suspend_status.value.compare_exchange_strong(
        expected, target, std::memory_order_relaxed);
  }

  [[nodiscard]] bool try_acknowledge_resume() {
    SuspendStatus expected = SuspendStatus::ResumeRequested;
    SuspendStatus target = SuspendStatus::Resumed;
    return suspend_status.value.compare_exchange_strong(
        expected, target, std::memory_order_relaxed);
  }
};

template <typename T>
struct FutureState : public SharedBaseState {
  std::aligned_storage_t<sizeof(T), alignof(T)> storage;

  void unsafe_init(T&& value) { new (&storage) T{std::move(value)}; }

  T unsafe_copy() const { return T{laundered()}; }

  T unsafe_move() { return T{std::move(laundered())}; }

  ~FutureState() {
    switch (acquire_completion_status()) {
      case CompletionStatus::Canceled:
      case CompletionStatus::Completed: {
        laundered().~T();
        return;
      }
      case CompletionStatus::Pending:
        return;
    }
  }

 private:
  T& laundered() { return *std::launder(reinterpret_cast<T*>(&storage)); }

  T const& laundered() const {
    return *std::launder(reinterpret_cast<T const*>(&storage));
  }
};

template <>
struct FutureState<void> : public SharedBaseState {};

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

  friend struct FutureToken;
  friend struct CancelationProxy;

  static auto create() {
    Future future;
    future.state = std::shared_ptr<FutureState<T>>{new FutureState<T>};
    return std::move(future);
  }

  CompletionStatus acquire_completion_status() const {
    return state->acquire_completion_status();
  }

  CancelationStatus fetch_cancelation_status() const {
    return state->fetch_cancelation_status();
  }

  SuspendStatus fetch_suspend_status() const {
    return state->fetch_suspend_status();
  }

  void request_cancel() const { state->request_cancel(); }

  void request_suspend() const { state->request_suspend(); }

  void request_resume() const { state->request_resume(); }

  stx::Result<T, FutureError> copy() const {
    return acquire_completed_state(
        [this]() -> T { return state->unsafe_copy(); });
  }

  stx::Result<T, FutureError> move() {
    return acquire_completed_state(
        [this]() -> T { return state->unsafe_move(); });
  }

  bool is_done() const {
    switch (acquire_completion_status()) {
      case CompletionStatus::Canceled:
        return true;
      case CompletionStatus::Completed:
        return true;
      case CompletionStatus::Pending:
        return false;
    }
  }

  bool is_valid() const { return state != nullptr; }

 private:
  template <typename Operation>
  stx::Result<T, FutureError> acquire_completed_state(Operation&& operation) {
    switch (acquire_completion_status()) {
      case CompletionStatus::Pending:
        return stx::Err(FutureError::Pending);
      case CompletionStatus::Completed:
        return stx::Ok(static_cast<Operation&&>(operation)());
      case CompletionStatus::Canceled:
        return stx::Err(FutureError::Canceled);
    }
  }

  std::shared_ptr<FutureState<T>> state;
};

template <>
struct Future<void> {
  template <typename P>
  friend struct Promise;

  friend struct FutureToken;
  friend struct CancelationProxy;

  static auto create() {
    Future future;
    future.state = std::shared_ptr<FutureState<void>>{new FutureState<void>};
    return future;
  }

  CompletionStatus acquire_completion_status() const {
    return state->acquire_completion_status();
  }

  CancelationStatus fetch_cancelation_status() const {
    return state->fetch_cancelation_status();
  }

  SuspendStatus fetch_suspend_status() const {
    return state->fetch_suspend_status();
  }

  bool is_done() const {
    switch (acquire_completion_status()) {
      case CompletionStatus::Canceled:
        return true;
      case CompletionStatus::Completed:
        return true;
      case CompletionStatus::Pending:
        return false;
    }
  }

  void request_cancel() const { state->request_cancel(); }

  void request_suspend() const { state->request_suspend(); }

  void request_resume() const { state->request_resume(); }

  bool is_valid() const { return state != nullptr; }

 private:
  std::shared_ptr<FutureState<void>> state;
};

struct FutureToken {
  VLK_DEFAULT_CONSTRUCTOR(FutureToken)

  template <typename T>
  explicit FutureToken(Future<T> const& future)
      : state{std::weak_ptr<SharedBaseState>{
            std::static_pointer_cast<SharedBaseState>(future.state)}} {}

  CompletionStatus acquire_completion_status() const {
    return state->acquire_completion_status();
  }

  CancelationStatus fetch_cancelation_status() const {
    return state->fetch_cancelation_status();
  }

  SuspendStatus fetch_suspend_status() const {
    return state->fetch_suspend_status();
  }

  bool is_done() const {
    switch (acquire_completion_status()) {
      case CompletionStatus::Canceled:
        return true;
      case CompletionStatus::Completed:
        return true;
      case CompletionStatus::Pending:
        return false;
    }
  }

  void request_cancel() const { state->request_cancel(); }

  void request_suspend() const { state->request_suspend(); }

  void request_resume() const { state->request_resume(); }

  bool is_valid() const { return state != nullptr; }

 private:
  // this is aligned to the cache line size via heap allocation which should
  // make allocations chunked to std::max_align_t granularity. so we shouldn't
  // have cache coherence issues.
  std::shared_ptr<SharedBaseState> state;
};

struct CancelationProxy {
  template <typename T>
  explicit CancelationProxy(Future<T> const& future)
      : state{std::static_pointer_cast<SharedBaseState>(future.state)} {}

  // returns true if the associated future is alive and the task has been
  // requested to cancel
  [[nodiscard]] bool try_acknowledge_cancel() const {
    auto shared_state = state.lock();
    if (shared_state == nullptr) {
      // the task is canceled since the associated
      // future has been discarded
      return true;
    } else {
      return shared_state->try_acknowledge_cancel();
    }
  }

 private:
  std::weak_ptr<SharedBaseState> state;
};

template <typename T>
struct Promise {
  explicit Promise(Future<T> const& future)
      : state{future.state}, cancelation_proxy{future} {}

  void finish(T&& value) const {
    auto shared_state = state.lock();

    if (shared_state == nullptr) {
      // the user is no longer interested in the result as they've discarded the
      // future and the future is no longer existing
      return;
    } else {
      switch (shared_state->fetch_cancelation_status()) {
        case CancelationStatus::Canceling: {
          shared_state->release_canceled_state();
        } break;
        case CancelationStatus::CancelRequested:
        case CancelationStatus::Uncanceled: {
          shared_state->unsafe_init(std::move(value));
          return;
        }
      }
    }
  }

  // returns true if the associated future is alive and the task has been
  // requested to suspend.
  // the task is still executed but requested to cancel if the associated
  // future is discarded.
  [[nodiscard]] bool try_acknowledge_suspend() const {
    auto shared_state = state.lock();
    if (shared_state == nullptr) {
      return false;
    } else {
      return shared_state->try_acknowledge_suspend();
    }
  }

  // returns true if the associated future is alive and the task has been
  // requested to resume.
  // the task is still executed but requested to cancel if the associated
  // future is discarded. the task is forced into a resumed state to enable
  // proper completion or cancelation (if any).
  [[nodiscard]] bool try_acknowledge_resume() const {
    auto shared_state = state.lock();
    if (shared_state == nullptr) {
      return true;
    } else {
      return shared_state->try_acknowledge_resume();
    }
  }

  CancelationProxy const& as_cancelation_proxy() const {
    return cancelation_proxy;
  }

 private:
  std::weak_ptr<FutureState<T>> state;
  CancelationProxy cancelation_proxy;
};

template <>
struct Promise<void> {
  explicit Promise(Future<void> const& future)
      : state{future.state}, cancelation_proxy{future} {}

  void finish() const {
    auto shared_state = state.lock();

    if (shared_state == nullptr) {
      // the user is no longer interested in the result as they've discarded the
      // future and the future is no longer existing
      return;
    } else {
      switch (shared_state->fetch_cancelation_status()) {
        case CancelationStatus::Canceling: {
          shared_state->release_canceled_state();
        } break;
        case CancelationStatus::CancelRequested:
        case CancelationStatus::Uncanceled: {
          shared_state->release_completed_state();
          return;
        }
      }
    }
  }

  [[nodiscard]] bool try_acknowledge_suspend() const {
    auto shared_state = state.lock();
    if (shared_state == nullptr) {
      return false;
    } else {
      return shared_state->try_acknowledge_suspend();
    }
  }

  [[nodiscard]] bool try_acknowledge_resume() const {
    auto shared_state = state.lock();
    if (shared_state == nullptr) {
      return true;
    } else {
      return shared_state->try_acknowledge_resume();
    }
  }

  CancelationProxy const& as_cancelation_proxy() const {
    return cancelation_proxy;
  }

 private:
  std::weak_ptr<FutureState<void>> state;
  CancelationProxy cancelation_proxy;
};

struct ThreadInfo {
  uint32_t index = 0;
};

struct TaskIdentifier {
  std::shared_ptr<std::string const> ptr;

  std::string_view get_string_view() const {
    if (ptr == nullptr) return "Unnamed Task";
    return *ptr;
  }
};

// TODO(lamarrr): document weak_ptr recommendation
template <typename ReturnType>
struct Task {
  // used to store the function that needs execution. this function needs to be
  // thread-safe.
  std::function<ReturnType(CancelationProxy const&)> function =
      [](CancelationProxy const&) {};
  TaskIdentifier identifier;
};

// result of one is passed to the other
template <typename... FunctionTypes>
struct TaskChain {
  std::tuple<FunctionTypes...> chain;
};

// Parallel?????????????????????

// all executed sequentially without result dependency
template <typename... SubtaskTypes>
struct TaskSequence {
  std::tuple<SubtaskTypes...> tasks;
};

struct PackagedTask {
  // function to be executed in the execution context
  std::function<void()> function = []() {};
  TaskIdentifier identifier;

  VLK_DEFAULT_CONSTRUCTOR(PackagedTask)
  STX_DISABLE_COPY(PackagedTask)
  STX_DEFAULT_MOVE(PackagedTask)

  template <typename T>
  PackagedTask(Task<T>&& task, Future<T> const& future)
      : function{[promise = Promise<T>(future),
                  task_function = std::move(task.function)]() {
          // it might take time before the executor context's threads are
          // available for executing tasks, so we still have to check for
          // cancelation before beginning to execute any work
          CancelationProxy const& cancelation_proxy =
              promise.as_cancelation_proxy();
          if (cancelation_proxy.try_acknowledge_cancel()) {
          } else {
            auto completion_result = task_function(cancelation_proxy);
            promise.finish(std::move(completion_result));
          }
        }},
        identifier{std::move(task.identifier)} {}

  PackagedTask(Task<void>&& task, Future<void> const& future)
      : function{[promise = Promise<void>(future),
                  task_function = std::move(task.function)]() {
          CancelationProxy const& cancelation_proxy =
              promise.as_cancelation_proxy();
          if (cancelation_proxy.try_acknowledge_cancel()) {
          } else {
            task_function(cancelation_proxy);
            promise.finish();
          }
          //
          // TODO(lamarrr): use task_moved.identifier for tracing
          // we need a weak ptr to the trace context needed for logging so in
          // case the scheduler is destroyed, we'll be able to detect
        }},
        identifier{std::move(task.identifier)} {}
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

  virtual ~ThreadExecutionContext() {}

  virtual void trace_begin();
  virtual void trace_end();

  // std::min(1, <int32_t>(num_hw_threads)-1 )
  auto get_num_hardware_threads() const { return num_hardware_threads; }

 private:
  uint32_t num_hardware_threads = 0;
};

struct ThreadTaskTrace {};

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

// advisable to never block any thread
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
inline void block_on_acquire_lock(std::atomic<LockStatus>& lock) {
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
  impl::block_on([&future]() { return future.is_done(); });
}

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
  Future<T> schedule(Task<T>&& task) {
    auto future = Future<T>::create();

    execution_context_->submit(PackagedTask{std::move(task), future});

    return std::move(future);
  }

  // of-course, the long names are meant to discourage you from using them
  //
  // the executor will still have cancelation tokens to execute on the worker
  // threads
  //

  template <typename T, typename DurationRep, typename DurationPeriod>
  Future<T> schedule_every(
      Task<T>&& task,
      std::chrono::duration<DurationRep, DurationPeriod> duration);

  template <typename DurationRep, typename DurationPeriod>
  Future<void> schedule_every(
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
  // we should probably have a weak_ptr to the execution context
  std::shared_ptr<ThreadExecutionContext> execution_context_;
};

struct ThreadTaskExecutor {};

// we need to be able to notify the worker thread to shutdown
struct WorkerThreadInfo {
  std::thread thread;
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
      return impl::try_acquire_then(
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
  std::mutex task_queue_mutex_;

  void shutdown_worker_threads() {
    for (WorkerThreadInfo& worker_thread : worker_threads_) {
      worker_thread.task.request_cancel();
      block_on(worker_thread.task);
    }
  }

  void launch_worker_threads() {
    uint32_t num_threads = std::thread::hardware_concurrency();
  }

  /*
    void worker_thread_task(uint8_t execution_trace_context,
                            CancelationProxy & cancelation_proxy,
                            ThreadInfo info) {
      uint64_t taskless_iterations = 0;
      // sleeping procedure if no task available
      while (!cancelation_proxy.try_acknowledge_cancel()) {
        PackagedTask task;
        {
          task_queue_mutex_.lock();
          if (true) {
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
    */
};
}  // namespace vlk
