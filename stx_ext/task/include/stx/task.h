// schedule_pending_tasks
// ??????????????????????
// returns true if the associated future is alive and the task has been
// requested to resume.
// the task is still executed but requested to cancel if the associated
// future is discarded. the task is forced into a resumed state to enable
// proper completion or cancelation (if any).
// for scheduling, we need to ensure we reset force suspension

#pragma once

#include "vlk/ui/subsystems/async.h"

namespace stx {

namespace ui {

struct ThreadInfo {
  uint32_t index = 0;
};

// we need to know the reason the function returned
// not going to work since there might be no request for it
// too complex to use
enum class ExitError : uint8_t { UserCanceled, UserSuspended, User };

struct TaskTraceInfo {
  TaskTraceInfo(std::string_view context, std::string_view purpose,
                std::shared_ptr<void const> handle)
      : context_{context}, purpose_{purpose}, handle_{handle} {}

  std::string_view context() const { return context_; }

  std::string_view purpose() const { return purpose_; }

 private:
  // the context of the task spawn. must be a static string.
  // i.e. AssetManager.ResourceLoading
  std::string_view context_ = "Unspecified";
  // the purpose of the task itself. this is usually gotten from the task
  // itself.
  // i.e. "LoadImage{url: 'http://foo.bar/image.jpg'}"
  std::string_view purpose_ = "Unspecified";
  //  if the task's purpose has an associated handle (shared_ptr) that needs
  //  to be kept alive for either of the string views to be valid before this
  //  trace info goes out of scope.
  std::shared_ptr<void const> handle_;
};

struct PackagedTask {
  // function to be executed in the execution context
  std::function<void()> function = []() {};
  TaskTraceInfo info;
};

template <typename ReturnType>
using TaskFunction =
    std::function<stx::Result<ReturnType, Service>(RequestProxy const&)>;

struct void_tag_t {};
constexpr void_tag_t void_tag{};

// TODO(lamarrr): document weak_ptr recommendation
//
// almost every user interface task should be interruptible, our task
// callbacks therefore have them by default. the function arguments can be
// ignored and the user can just return stx::Ok(value);
//
template <typename ReturnType>
struct Task {
  VLK_DEFAULT_CONSTRUCTOR(Task)

  Task(TaskFunction<ReturnType>&& task_function,
       TaskTraceInfo&& task_trace_info, TaskPriority task_priority)
      : function{std::move(task_function)},
        trace_info{std::move(task_trace_info)},
        priority{task_priority} {}

  TaskFunction<ReturnType> function = [](RequestProxy const&) {};
  TaskTraceInfo trace_info;
  TaskPriority priority = background_task_priority;
};

// all callbacks are executed one after the other with the result of one
// passed onto the other. they are implented such that they can be interrupted
// along the execution path.
template <typename... FunctionTypes>
struct TaskChain {
  std::tuple<FunctionTypes...> chain;
  TaskTraceInfo info;
  TaskPriority priority = background_task_priority;

 private:
  uint64_t chain_next_execute_index = 0;
};

template <typename ReturnType>
PackagedTask package(Task<ReturnType>&& task, Promise<ReturnType>&& promise) {
  return PackagedTask{
      [task_function = std::move(task.function),
       task_promise = std::move(promise)]() { task_promise.task_function() },
      std::move(task.trace_info)

  };
}

template <typename... FunctionTypes>
PackagedTask package(TaskChain<FunctionTypes...>&& chain) {}

// Parallel?????????????????????

struct PackagedTask {
  // function to be executed in the execution context
  std::function<void()> function = []() {};
  TaskIdentifier identifier;

  VLK_DEFAULT_CONSTRUCTOR(PackagedTask)
  STX_DISABLE_COPY(PackagedTask)
  STX_DEFAULT_MOVE(PackagedTask)

  // TODO(lamarrr): the user might assume fire and forget, cancelation needs
  // to be explicit or by the executor.
  //
  // force cancelation of the task if it is not alive
  //
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
//
// this should never be used anywhere in user code
//
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
  // even if the user requests for cancelation and we are unable to service
  // the request, we still have to
  //
  //
  //
  // we can't stop the function from running to completion even if a
  // cancelation is requested
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

// should contain ring buffer of tasks taht need executing.
// unlike scheduler. tasks are submitted in execution order.
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
    // the worker threads must not hold on to them while executing the tasks.
    // we also want to be able to send cancelation requests to the worker
    // threads
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

}  // namespace ui
}  // namespace vlk
