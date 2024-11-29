/// SPDX-License-Identifier: MIT
///
/// Stage-based Asynchrony
///
#pragma once
#include "ashura/std/allocator.h"
#include "ashura/std/cfg.h"
#include "ashura/std/error.h"
#include "ashura/std/functional.h"
#include "ashura/std/mem.h"
#include "ashura/std/rc.h"
#include "ashura/std/result.h"
#include "ashura/std/time.h"
#include "ashura/std/types.h"
#include "ashura/std/v.h"

#include <atomic>
#include <thread>

#if ASH_CFG(ARCH, X86) || ASH_CFG(ARCH, X86_64)
#  include <emmintrin.h>
#endif

namespace ash
{

inline void yielding_backoff(u64 poll)
{
  if (poll < 8)
  {
    return;
  }

  if (poll < 16)
  {
#if ASH_CFG(ARCH, X86) || ASH_CFG(ARCH, X86_64)
    _mm_pause();
#else
#  if ASH_CFG(ARCH, ARM32) || ASH_CFG(ARCH, ARM64)
    __asm("yield");
#  endif
#endif

    return;
  }

  std::this_thread::yield();
  return;
}

inline void sleepy_backoff(u64 poll, nanoseconds sleep)
{
  if (poll < 8)
  {
    return;
  }

  if (poll < 16)
  {
#if ASH_CFG(ARCH, X86) || ASH_CFG(ARCH, X86_64)
    _mm_pause();
#else
#  if ASH_CFG(ARCH, ARM32) || ASH_CFG(ARCH, ARM64)
    __asm("yield");
#  endif
#endif

    return;
  }

  if (poll < 64)
  {
    std::this_thread::yield();
    return;
  }

  std::this_thread::sleep_for(sleep);
  return;
}

struct SpinLock
{
  usize flag_ = false;

  void lock()
  {
    usize           expected = false;
    usize           target   = true;
    u64             poll     = 0;
    std::atomic_ref flag{flag_};
    while (!flag.compare_exchange_weak(
        expected, target, std::memory_order_acquire, std::memory_order_relaxed))
    {
      expected = false;
      yielding_backoff(poll);
      poll++;
    }
  }

  [[nodiscard]] bool try_lock()
  {
    usize           expected = false;
    usize           target   = true;
    std::atomic_ref flag{flag_};
    flag.compare_exchange_weak(expected, target, std::memory_order_acquire,
                               std::memory_order_relaxed);
    return expected;
  }

  void unlock()
  {
    std::atomic_ref flag{flag_};
    flag.store(false, std::memory_order_release);
  }
};

template <typename L>
struct LockGuard
{
  L *lock_;

  explicit LockGuard(L &lock) : lock_{&lock}
  {
    lock_->lock();
  }

  LockGuard(LockGuard const &) = delete;

  LockGuard &operator=(LockGuard const &) = delete;

  LockGuard(LockGuard &&) = delete;

  LockGuard &operator=(LockGuard &&) = delete;

  ~LockGuard()
  {
    lock_->unlock();
  }
};

struct ReadWriteLock
{
  SpinLock lock_{};
  usize    num_writers_ = 0;
  usize    num_readers_ = 0;

  void lock_read()
  {
    u64 poll = 0;
    while (true)
    {
      LockGuard guard{lock_};
      if (num_writers_ == 0)
      {
        num_readers_++;
        return;
      }
      yielding_backoff(poll);
      poll++;
    }
  }

  void lock_write()
  {
    u64 poll = 0;

    while (true)
    {
      LockGuard guard{lock_};
      if (num_writers_ == 0 && num_readers_ == 0)
      {
        num_writers_++;
        return;
      }
      yielding_backoff(poll);
      poll++;
    }
  }

  void unlock_read()
  {
    LockGuard guard{lock_};
    num_readers_--;
  }

  void unlock_write()
  {
    LockGuard guard{lock_};
    num_writers_--;
  }
};

struct ReadGuard
{
  ReadWriteLock *lock_;

  explicit ReadGuard(ReadWriteLock &lock) : lock_{&lock}
  {
    lock_->lock_read();
  }

  ReadGuard(ReadGuard const &) = delete;

  ReadGuard &operator=(ReadGuard const &) = delete;

  ReadGuard(ReadGuard &&) = delete;

  ReadGuard &operator=(ReadGuard &&) = delete;

  ~ReadGuard()
  {
    lock_->unlock_read();
  }
};

struct WriteGuard
{
  ReadWriteLock *lock_;

  explicit WriteGuard(ReadWriteLock &lock) : lock_{&lock}
  {
    lock_->lock_write();
  }

  WriteGuard(WriteGuard const &) = delete;

  WriteGuard &operator=(WriteGuard const &) = delete;

  WriteGuard(WriteGuard &&) = delete;

  WriteGuard &operator=(WriteGuard &&) = delete;

  ~WriteGuard()
  {
    lock_->unlock_write();
  }
};

enum class AtomicInitStage : usize
{
  Uninit  = 0,
  Init    = 1,
  Pending = 2
};

/// @brief An atomically initialized value, can only be initialized once.
/// Multiple threads can attempt to initialize the value but only one thread
/// will be successful. This means we don't need to use locks to guard the
/// object.
template <typename T>
struct AtomicInit
{
  AtomicInitStage stage_;
  union
  {
    T v_;
  };

  constexpr AtomicInit() : stage_{AtomicInitStage::Uninit}
  {
  }

  template <typename... Args>
  constexpr AtomicInit(V<0>, Args &&...args) :
      stage_{AtomicInitStage::Init}, v_{static_cast<Args &&>(args)...}
  {
  }

  constexpr AtomicInit(AtomicInit const &)            = delete;
  constexpr AtomicInit(AtomicInit &&)                 = delete;
  constexpr AtomicInit &operator=(AtomicInit const &) = delete;
  constexpr AtomicInit &operator=(AtomicInit &&)      = delete;

  ~AtomicInit()
  {
    // this is the last reference to the object at this point, but we still need
    // to acquire write side effects from other writers (if any)
    std::atomic_ref stage{stage_};

    if (stage.load(std::memory_order_acquire) == AtomicInitStage::Init)
        [[likely]]
    {
      v_.~T();
    }
  }

  template <typename... Args>
  [[nodiscard]] bool init(Args &&...args)
  {
    std::atomic_ref stage{stage_};
    AtomicInitStage expected = AtomicInitStage::Uninit;
    AtomicInitStage target   = AtomicInitStage::Pending;

    /// no side-effects need to be observed
    if (!stage.compare_exchange_strong(expected, target,
                                       std::memory_order_relaxed,
                                       std::memory_order_relaxed))
    {
      return false;
    }

    new (&v_) T{static_cast<Args &&>(args)...};

    stage.store(AtomicInitStage::Init, std::memory_order_release);
    return true;
  }

  /// @brief Get the wrapped value
  /// @return null if value is not initialized yet
  T *get()
  {
    std::atomic_ref stage{stage_};
    if (stage.load(std::memory_order_acquire) != AtomicInitStage::Init)
    {
      return nullptr;
    }
    return &v_;
  }
};

template <typename T>
struct [[nodiscard]] Sync
{
  T data_;

  ReadWriteLock lock_;

  template <typename... Args>
  constexpr Sync(Args &&...args) : data_{static_cast<Args &&>(args)...}, lock_{}
  {
  }

  constexpr Sync(Sync const &) = delete;

  constexpr Sync(Sync &&) = delete;

  constexpr Sync &operator=(Sync const &) = delete;

  constexpr Sync &operator=(Sync &&) = delete;

  constexpr ~Sync() = default;

  template <Callable<T &> Op>
  void read(Op &&op)
  {
    ReadGuard guard{lock_};
    static_cast<Op &&>(op)(data_);
  }

  template <Callable<T &> Op>
  void write(Op &&op)
  {
    WriteGuard guard{lock_};
    static_cast<Op &&>(op)(data_);
  }
};

template <typename T>
Sync(T &&) -> Sync<T>;

template <typename T>
Sync(T const &) -> Sync<T>;

/// @brief A CPU Timeline Semaphore (a.k.a. Sequence Barrier) used for
/// synchronization in multi-stage cooperative multitasking jobs. Unlike typical
/// Binary/Counting Semaphores, A timeline semaphores a monotonic counter
/// representing the stages of an operation.
/// - Guarantees Forward Progress
/// - Scatter-gather operations only require one primitive
/// - Primitive can encode state of multiple operations and also be awaited by
/// multiple operations at once.
/// - Task ordering is established by the `state` which describes the number of
/// steps needed to complete a task, and can be awaited by other tasks.
/// - It is use and increment once, hence no deadlocks can occur. This also
/// enables cooperative synchronization between systems processing different
/// stages of an operation without explicit sync between them.
///
/// Semaphore can only move from state `i` to state `i+n` where n > 1.
///
/// Semaphore should ideally not be destroyed before completion as there could
/// possibly be other tasks awaiting it.
///
/// Semaphores never overflow. so it can have a maximum of U64_MAX stages.
struct SemaphoreState
{
  u64 num_stages_;

  u64 stage_;

  explicit constexpr SemaphoreState(u64 num_stages) :
      num_stages_{num_stages}, stage_{0}
  {
  }

  /// @brief Get the current semaphore stage. This represents the current stage
  /// being worked on.
  /// @param sem non-null
  /// @return
  [[nodiscard]] u64 stage()
  {
    std::atomic_ref stage{stage_};
    return stage.load(std::memory_order_acquire);
  }

  /// @brief Get the number of stages in the semaphore
  /// @param sem non-null
  /// @return
  [[nodiscard]] constexpr u64 num_stages() const
  {
    return num_stages_;
  }

  /// @brief returns true if the semaphore has been completed. i.e. reached its
  /// last declared stage.
  /// @param sem non-null
  /// @return
  [[nodiscard]] bool is_completed()
  {
    std::atomic_ref stage{stage_};
    return stage.load(std::memory_order_acquire) == num_stages_;
  }

  /// @brief Signal the semaphore to move to stage `next`. This implies a
  /// sequence ordering of the semaphore stages.
  ///
  /// @param next stage of the semaphore to move to. stage >=
  /// num_stages or U64_MAX means completion of the last stage of the operation.
  /// must be monotonically increasing for each call.
  ///
  /// @returns returns true if the signaled stage has not been passed yet
  /// (successful). otherwise returns false (failed).
  ///
  [[nodiscard]] bool signal(u64 next)
  {
    next                    = min(next, num_stages_);
    u64             current = 0;
    std::atomic_ref stage{stage_};
    while (!stage.compare_exchange_weak(
        current, next, std::memory_order_release, std::memory_order_relaxed))
        [[unlikely]]
    {
      if (current >= next)
      {
        return false;
      }
    }
    return true;
  }

  /// @brief
  /// @param inc stage increment of semaphore. increment of >= num_stages is
  /// equivalent to driving it to completion.
  void increment(u64 inc)
  {
    inc                     = min(inc, num_stages_);
    u64             current = 0;
    u64             target  = inc;
    std::atomic_ref stage{stage_};
    while (!stage.compare_exchange_weak(
        current, target, std::memory_order_release, std::memory_order_relaxed))
        [[unlikely]]
    {
      target = min(sat_add(current, inc), num_stages_);
    }
  }
};

typedef Rc<SemaphoreState *> Semaphore;

/// @brief A Stop Sequence Token
struct StopTokenState
{
  /// @brief stage to stop execution before.
  /// this means the stage represented by `stop_before_` and all proceeding
  /// stages are canceled.
  u64 stop_point_ = U64_MAX;

  constexpr StopTokenState() = default;

  /// @brief check whether the specified stage has been canceled. synchronizes
  /// with the scope
  /// @return
  bool is_stop_requested(u64 stage = 0)
  {
    std::atomic_ref stop_point{stop_point_};
    return stop_point.load(std::memory_order_acquire) <= stage;
  }

  /// @brief stop the execution at the specified stage, all tasks beyond that
  /// stage are also stopped. synchronizes with the scope
  void request_stop(u64 stage = 0)
  {
    std::atomic_ref stop_point{stop_point_};
    u64             expected = U64_MAX;
    u64             target   = min(expected, stage);
    while (!stop_point.compare_exchange_weak(
        expected, target, std::memory_order_release, std::memory_order_relaxed))
        [[unlikely]]
    {
      target = min(expected, stage);
    }
  }
};

typedef Rc<StopTokenState *> StopToken;

///
/// @brief Create an independently allocated semaphore object
///
/// @param num_stages: number of stages represented by this semaphore
/// @return Semaphore
///
inline Result<Semaphore> create_semaphore(AllocatorImpl allocator,
                                          u64           num_stages)
{
  return rc_inplace<SemaphoreState>(allocator, num_stages);
}

inline Result<StopToken> create_stop_token(AllocatorImpl allocator)
{
  return rc_inplace<StopTokenState>(allocator);
}

/// @brief await semaphores at the specified stages.
/// @param sems semaphores to wait for
/// @param stages stages of the semaphores to wait for completion of. must be <
/// semaphore.num_stages or == U64_MAX. U64_MAX meaning waiting for all stages'
/// completion.
/// @param timeout timeout to stop attempting to wait for the semaphores. when
/// timeout is 0, there's an immediate result, and when timeout is
/// nanoseconds::max() it waits for the semaphores forever untilt they are
/// ready.
/// @param any if to wait for all semaphores or atleast 1 semaphore.
/// @returns returns if the semaphore await operation completed successfully
/// based on the `any` criteria.
[[nodiscard]] inline bool await_semaphores(Span<SemaphoreState *const> sems,
                                           Span<u64 const>             stages,
                                           nanoseconds                 timeout)
{
  CHECK(sems.size() == stages.size());
  usize const n = sems.size();
  for (usize i = 0; i < n; i++)
  {
    CHECK((stages[i] == U64_MAX) || (stages[i] <= sems[i]->num_stages()));
  }

  // number of times we've polled so far, counting begins from 0
  u64 poll = 0;

  // avoid sys-calls unless absolutely needed
  steady_clock::time_point poll_begin{};

  // speeds up checks for the 'all' case. points to the next semaphore to be
  // checked
  usize next = 0;

  while (true)
  {
    for (; next < n; next++)
    {
      SemaphoreState *const &s        = sems[next];
      u64 const              stage    = min(stages[next], s->num_stages_ - 1);
      bool const             is_ready = stage < s->stage();

      if (!is_ready)
      {
        break;
      }
    }

    if (next == n)
    {
      return true;
    }

    // fast-path to avoid syscalls
    if (timeout <= nanoseconds{0}) [[likely]]
    {
      return false;
    }

    // fast-path to avoid syscalls
    if (timeout == nanoseconds::max()) [[likely]]
    {
      // infinite timeout
    }
    else
    {
      if (poll_begin == steady_clock::time_point{}) [[unlikely]]
      {
        poll_begin = steady_clock::now();
      }

      nanoseconds const past = steady_clock::now() - poll_begin;

      if (past > timeout) [[unlikely]]
      {
        return false;
      }
    }

    yielding_backoff(poll);
    poll++;
  }

  return false;
}

/// @brief A Stream is a continously mutated value yielding side-effects to
/// consumers. The side-effects are sequenced by the timeline semaphore. There
/// is no locking on the `Stream`, if you need locking consider using
/// `Stream<Sync<T>>`.
///
/// A Stream can yield results in a sequenced or unsequenced manner.
///
template <typename T>
struct [[nodiscard]] Stream
{
  typedef T Type;

  Rc<T *> data_;

  Semaphore semaphore_;

  Stream(Rc<T *> data, Semaphore semaphore) :
      data_{std::move(data)}, semaphore_{std::move(semaphore)}
  {
  }

  Stream alias() const
  {
    return Stream{data_.alias(), semaphore_.alias()};
  }

  [[nodiscard]] bool is_ready(u64 stage) const
  {
    return semaphore_->stage() > stage;
  }

  [[nodiscard]] bool is_completed() const
  {
    return semaphore_->is_completed();
  }

  template <Callable<T &> F>
  void yield_unseq(F &&op, u64 increment) const
  {
    static_cast<F &&>(op)(*data_.get());
    semaphore_->increment(increment);
  }

  template <Callable<T &> F>
  void yield_seq(F &&op, u64 stage) const
  {
    static_cast<F &&>(op)(*data_.get());
    CHECK_DESC(semaphore_->signal(stage + 1),
               "`Stream` yielded with invalid sequencing");
  }
};

template <typename T, typename... Args>
Result<Stream<T>> stream_inplace(AllocatorImpl allocator, u64 num_stages,
                                 Args &&...args)
{
  Result data = rc_inplace<T>(allocator, static_cast<Args &&>(args)...);
  if (!data)
  {
    return Err{};
  }
  Result sem = create_semaphore(allocator, num_stages);
  if (!sem)
  {
    return Err{};
  }
  return Ok{Stream<T>{std::move(data.value()), std::move(sem.value())}};
}

template <typename T>
Result<Stream<T>> stream(AllocatorImpl allocator, u64 num_stages, T value)
{
  return stream_inplace<T>(allocator, num_stages, std::move(value));
}

template <typename... T>
[[nodiscard]] bool await_streams(nanoseconds timeout, Span<u64 const> stages,
                                 Stream<T> const &...streams)
{
  SemaphoreState *semaphores[] = {(streams.semaphore_.get())...};

  return await_semaphores(span(semaphores), stages, timeout);
}

/// @brief A future is 1-stage Stream that produces a single value. The value is
/// left uninitialized until the future is completed.
template <typename T>
struct [[nodiscard]] Future
{
  typedef T Type;

  Stream<AtomicInit<T>> stream_;

  u64 stage_;

  Future(Stream<AtomicInit<T>> stream, u64 stage) :
      stream_{std::move(stream)}, stage_{stage}
  {
  }

  Future alias() const
  {
    return Future{stream_.alias(), stage_};
  }

  [[nodiscard]] bool is_ready() const
  {
    return stream_.is_ready(stage_);
  }

  T &get() const
  {
    T *data = stream_.data_.get()->get();
    CHECK_DESC(data != nullptr, "Called `Future::get()` on a pending Future");
    return *data;
  }

  template <typename... Args>
  void complete(Args &&...args) const
  {
    stream_.yield_seq(
        [&](AtomicInit<T> &v) {
          bool const init = v.init(static_cast<Args &&>(args)...);
          CHECK_DESC(
              init,
              "Called `Future::complete()` on an already completed `Future`");
        },
        stage_);
  }
};

template <typename T>
Result<Future<T>> future(AllocatorImpl allocator)
{
  Result stream = stream_inplace<AtomicInit<T>>(allocator, 1);
  if (!stream)
  {
    return Err{};
  }
  return Ok{Future<T>{std::move(stream.value()), 0}};
}

template <typename... T>
[[nodiscard]] bool await_futures(nanoseconds timeout,
                                 Future<T> const &...futures)
{
  SemaphoreState *semaphores[] = {(futures.stream_.semaphore_.get())...};
  u64 const       stages[]     = {futures.stage_...};
  return await_semaphores(span(semaphores), span(stages), timeout);
}

constexpr usize MAX_TASK_FRAME_SIZE = PAGE_SIZE >> 4;

template <typename F>
concept TaskFrame = requires(F f) {
  { !f.poll() };
  { !f.run() };
} && (sizeof(F) <= MAX_TASK_FRAME_SIZE);

/// @brief Task Frame layout and dynamic dispatch thunks
/// @param frame_layout memory layout of the task's frame.
/// @param init function to initialize the context data associated with the task
/// to a task stack.
/// @param uninit function to uninitialize the task and the associated data
/// context upon completion of the task.
/// @param poll function to poll for readiness of the task, must be extremely
/// light-weight and non-blocking. it is never called again once it returns
/// true.
/// @param run task to be executed on the executor. must return true if it
/// should be re-queued onto the executor.
/// @param instances number of instances of the task to spawn. all instances
/// share the same state/stack. multi-instanced tasks can not be re-queued for
/// execution. and must return false in their body (unchecked).
/// @note Cancelation is handled within the task itself, as various tasks have
/// various methods/techniques of reacting to cancelation.
struct [[nodiscard]] TaskInfo
{
  typedef Fn<void(void *)> Init;

  typedef void (*Uninit)(void *);

  typedef bool (*Poll)(void *);

  typedef bool (*Run)(void *);

  Layout frame_layout{};

  Init init = noop;

  Uninit uninit = noop;

  Poll poll = [](void *) { return true; };

  Run run = [](void *) { return false; };
};

enum class TaskTarget
{
  Worker    = 0,
  Main      = 1,
  Dedicated = 2
};

/// @brief Describes how to schedule the task onto the executor
/// @param target the target execution unit
/// @param thread the thread on the execution unit. U32_MAX means any available
/// thread. This is ignored when target is main thread.
struct TaskSchedule
{
  TaskTarget target = TaskTarget::Worker;
  u32        thread = U32_MAX;
};

/// @brief Wrap a Task frame
/// @return TaskInfo struct to be passed to the scheduler for execution
template <TaskFrame F>
TaskInfo to_task_info(F &frame)
{
  Fn init = fn(&frame, [](F *frame, void *mem) {
    new (mem) F{static_cast<F &&>(*frame)};
  });

  TaskInfo::Uninit uninit = [](void *f) {
    F *frame = reinterpret_cast<F *>(f);
    frame->~F();
  };

  TaskInfo::Poll poll = [](void *f) -> bool {
    F *frame = reinterpret_cast<F *>(f);
    return frame->poll();
  };

  TaskInfo::Run run = [](void *f) -> bool {
    F *frame = reinterpret_cast<F *>(f);
    return frame->run();
  };

  return TaskInfo{.frame_layout = layout<F>,
                  .init         = init,
                  .uninit       = uninit,
                  .poll         = poll,
                  .run          = run};
}

/// @brief Static Thread Pool Scheduler.
///
/// all tasks execute out-of-order.
///
/// it has 2 types of threads: worker threads and dedicated threads.
///
/// dedicated threads are for processing latency-sensitive tasks that need to
/// happen within a deadline, i.e. audio, video. they can spin, sleep, preempt
/// and/or wait for tasks.
///
/// worker threads process any type of tasks, although might not be as
/// responsive as dedicated threads due to their over-susbscription model.
///
///
/// @note work submitted to the main thread MUST be extremely light-weight and
/// non-blocking.
///
struct Scheduler
{
  /// @brief Initialize the scheduler.
  /// @note not thread-safe, typically called at program startup or DLL
  /// loading-time.
  /// @param allocator thread-safe allocator to allocate tasks from, must be
  /// able to allocate page-sized allocations
  /// @param dedicated_thread_sleep max sleep time for the dedicated threads.
  /// enables responsiveness. `.size()` represents the number of dedicated
  /// threads to create.
  /// @param worker_thread_sleep maximum sleep time for the worker threads.
  /// enables responsiveness. `.size()` represents the number of worker threads
  /// to create.
  static void init(AllocatorImpl allocator, std::thread::id main_thread_id,
                   Span<nanoseconds const> dedicated_thread_sleep,
                   Span<nanoseconds const> worker_thread_sleep);

  static void uninit();

  /// @brief Destroys the scheduler. The scheduler must have been joined.
  virtual ~Scheduler() = default;

  /// @brief Request that the threads stop executing and purges the tasks on the
  /// task queue.
  virtual void join() = 0;

  virtual u32 num_dedicated() = 0;

  virtual u32 num_workers() = 0;

  /// @brief Schedule task to a specific dedicated thread
  /// @param info Task frame information
  /// @param thread the index of the dedicated thread to schedule to
  virtual void schedule_dedicated(TaskInfo const &info, u32 thread) = 0;

  /// @brief Schedule task to a worker thread
  /// @param info Task frame information
  virtual void schedule_worker(TaskInfo const &info) = 0;

  /// @brief Schedule task to the main thread. The tasks are executed once the
  /// main thread loop runs.
  /// @param info Task frame information
  virtual void schedule_main(TaskInfo const &info) = 0;

  /// @brief Execute work on the main thread queue
  /// @param grace_period minimum time (within duration) to wait for tasks when
  /// the task queue is empty
  /// @param duration maximum timeout to spend executing tasks
  virtual void execute_main_thread_loop(nanoseconds grace_period,
                                        nanoseconds duration) = 0;

  template <TaskFrame F>
  void schedule(F &&task, TaskSchedule schedule)
  {
    TaskInfo info = to_task_info(task);

    switch (schedule.target)
    {
      case TaskTarget::Worker:
        schedule_worker(info);
        return;
      case TaskTarget::Dedicated:
        schedule_dedicated(info, schedule.thread);
        return;
      case TaskTarget::Main:
        schedule_main(info);
        return;
      default:
        UNREACHABLE();
    }
  }
};

/// @brief Global scheduler object. Designed for hooking across DLLs. Must be
/// initialized with `Scheduler::init()` and uninitialized with
/// `Scheduler::uninit()`.
ASH_C_LINKAGE ASH_DLL_EXPORT Scheduler *scheduler;

namespace async
{

template <typename P>
concept Poll = requires(P p) {
  { p() && true };
};

template <typename R>
concept Run = requires(R r) {
  { r() && true };
};

template <Run R, Poll P>
struct TaskBody
{
  typedef P Poller;
  typedef R Runner;

  R run{};
  P poll{};
};

struct TaskInstance
{
  u64 n   = 1;
  u64 idx = 0;
};

template <typename... T>
struct [[nodiscard]] AwaitStreams
{
  Tuple<Stream<T>...> streams;

  Array<u64, sizeof...(T)> stages;

  explicit AwaitStreams(Array<u64, sizeof...(T)> const &stages,
                        Stream<T>... streams) :
      streams{std::move(streams)...}, stages{stages}
  {
  }

  bool operator()() const
  {
    return apply(
        [this](auto const &...s) {
          return await_streams(nanoseconds{0}, span(stages), s...);
        },
        streams);
  }
};

template <typename... T>
struct [[nodiscard]] AwaitFutures
{
  Tuple<Future<T>...> futures;

  explicit AwaitFutures(Future<T>... futures) : futures{std::move(futures)...}
  {
  }

  bool operator()() const
  {
    return apply(
        [](auto const &...f) { return await_futures(nanoseconds{0}, f...); },
        futures);
  }
};

struct [[nodiscard]] Delay
{
  steady_clock::time_point from{};

  nanoseconds delay = 0ns;

  constexpr bool operator()() const
  {
    if (delay == 0ns)
    {
      return true;
    }
    auto const past = steady_clock::now() - from;
    return past >= delay;
  }
};

struct [[nodiscard]] Ready
{
  constexpr bool operator()() const
  {
    return true;
  }
};

/// @brief Launch a one-shot task
/// @tparam F Task Functor type
/// @tparam P Poller Functor type
/// @param fn Task functor
/// @param poll Poller functor that returns true when ready
/// @param schedule How to schedule the task
template <Callable F, Poll P = Ready>
void once(F fn, P poll = {}, TaskSchedule schedule = {})
{
  TaskBody body{[fn = std::move(fn)]() mutable -> bool {
                  fn();
                  return false;
                },
                std::move(poll)};

  scheduler->schedule(std::move(body), schedule);
}

template <Callable F, Callable... F1, Poll P = Ready>
void once(Tuple<F, F1...> fns, P poll = {}, TaskSchedule schedule = {})
{
  TaskBody body{[fns = std::move(fns)]() mutable -> bool {
                  ash::fold(fns);
                  return false;
                },
                std::move(poll)};

  scheduler->schedule(std::move(body), schedule);
}

/// @brief Launch a task that is repeatedly called until it is done
/// @tparam F Task Functor type
/// @tparam P Poller Functor type
/// @param fn Task functor that returns false when it is done
/// @param poll Poller functor that returns true when ready
/// @param schedule How to schedule the task
template <Callable F, Poll P = Ready>
  requires(Convertible<CallResult<F>, bool>)
void loop(F fn, P poll = {}, TaskSchedule schedule = {})
{
  TaskBody body{[fn = std::move(fn)]() mutable -> bool { return fn(); },
                std::move(poll)};

  scheduler->schedule(std::move(body), schedule);
}

/// @brief Launch a task that is repeatedly called n times
/// @tparam F Functor type
/// @tparam P Poller Functor type
/// @param fn Task functor to be called, can terminate early by returning a
/// boolean
/// @param n Number of times to execute the task
/// @param poll Poller functor that returns true when ready
/// @param schedule How to schedule the task
template <Callable<u64> F, Poll P = Ready>
  requires(Same<CallResult<F, u64>, void> ||
           Convertible<CallResult<F, u64>, bool>)
void repeat(F fn, u64 n, P poll = {}, TaskSchedule schedule = {})
{
  if (n == 0)
  {
    return;
  }

  TaskBody body{[fn = std::move(fn), n, i = (u64) 0]() mutable -> bool {
                  if constexpr (Same<CallResult<F, u64>, void>)
                  {
                    fn(i);
                    i++;
                    return n == i;
                  }
                  else
                  {
                    bool const done = fn(i);
                    i++;
                    return done || (n == i);
                  }
                },
                std::move(poll)};

  scheduler->schedule(std::move(body), schedule);
}

/// @brief Launch shards of tasks, All shards share the same state and task
/// frame and run concurrently. Typically used for SPMD
/// (https://en.wikipedia.org/wiki/Single_program,_multiple_data)
/// @tparam F Shard functor type
/// @tparam P Poller Functor type
/// @param fn Shard body
/// @param n Number of shard instances of the task to launch
/// @param poll Poller functor that returns true when ready
/// @param schedule How to schedule the shards
template <typename State, Poll P = Ready>
void shard(Fn<void(TaskInstance, State &)> fn, Rc<State *> const &state, u64 n,
           P poll = {}, TaskSchedule schedule = {})
{
  if (n == 0)
  {
    return;
  }

  // we need to first dispatch a task that will poll for readiness, and once the
  // shard is ready for dispatch we dispatch the shards. we can avoid this
  // intermediate process if we know the task is immediately available (Ready
  // type) but that's really not a good idea for a generic type. we also need
  // the dispatch as we don't expect the polling function to be thread-safe when
  // called across all instances.

  TaskBody body{
      [fn, state = state.alias(), schedule, n]() mutable -> bool {
        for (u64 i = 0; i < n; i++)
        {
          scheduler->schedule(
              TaskBody{[fn, i, n, state = state.alias()]() mutable -> bool {
                         fn(TaskInstance{.n = n, .idx = i}, *state.get());
                         return false;
                       },
                       Ready{}},
              schedule);
        }
        return false;
      },
      std::move(poll)};

  scheduler->schedule(std::move(body), schedule);
}

};        // namespace async

}        // namespace ash
