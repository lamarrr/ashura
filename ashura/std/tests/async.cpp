/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/async.hpp"
#include "ashura/std/rc.hpp"
#include <chrono>
#include <thread>

TEST(AsyncTest, Basic)
{
  using namespace ash;

  RcTimelineSemaphore sem = semaphore(default_allocator).unwrap();

  Dyn<Scheduler> sched = IScheduler::create(SchedulerInfo{
    .allocator = default_allocator,
    .dedicated_threads =
      span({SchedulerThreadInfo{"0"_s}, SchedulerThreadInfo{"1"_s}}),
    .worker_threads =
      span({SchedulerThreadInfo{"2"_s}, SchedulerThreadInfo{"3"_s}}),
    .main_thread_id = std::this_thread::get_id()});

  hook_scheduler(sched.get());

  defer sched_{[&] {
    sched->shutdown();
    hook_scheduler(nullptr);
  }};

  Stream<int> s = stream(default_allocator, 1, 20).unwrap();

  sched->once(
    WorkerThread::Any, []() { info("Hi"_s); }, AwaitStreams{{s.alias()}, {0}});
  sched->once(WorkerThread::Any, []() { info("Hello"_s); }, ready);
  sched->once(WorkerThread::Any, []() { info("World"_s); }, ready);
  info("Scheduled"_s);
  sched->once(
    WorkerThread::Any, []() { info("Timer passed"_s); },
    Delay{.from = steady_clock::now(), .delay = 1ms});

  auto fut = future<int>(default_allocator).unwrap();

  sched->loop(
    WorkerThread::Any,
    [x = (u64) 0, f = fut.alias(), s = s.alias()]() mutable -> bool {
      x++;
      info("Iteration: {}"_s, x);
      info("Future value: {}"_s, f.get());
      s.yield_unsequenced([x](int & v) { v = x; }, 1);
      if (x == 10)
      {
        info("Loop exited"_s);
        return false;
      }

      return true;
    },
    AwaitFutures{fut.alias()});
  fut.yield(69).unwrap();

  sched->shard<int *>(
    WorkerThread::Any, rc<int>(inplace, default_allocator, 0).unwrap(),
    [](TaskInstance shard, int * pcount) {
      std::atomic_ref count_ref{*pcount};
      int             count = count_ref.fetch_add(1);
      info("Shard: {}  of {}, sync i: {}"_s, shard.idx, shard.dim, count);
    },
    20, ready);

  sched->run_main_loop(10ms, 500s);

  sched->shutdown();
}
