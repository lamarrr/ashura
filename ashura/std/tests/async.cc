/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/async.h"
#include "ashura/std/error.h"
#include "ashura/std/rc.h"
#include <chrono>
#include <thread>

TEST(AsyncTest, Basic)
{
  using namespace ash;

  RcSemaphore sem = semaphore({}).unwrap();

  Dyn<Scheduler> sched = IScheduler::create(
    SchedulerInfo{.allocator              = default_allocator,
                  .dedicated_thread_sleep = span({1ns, 2ns}),
                  .worker_thread_sleep    = span({2ns, 5ns}),
                  .main_thread_id         = std::this_thread::get_id()});

  hook_scheduler(sched);

  defer sched_{[&] {
    sched->shutdown();
    hook_scheduler(nullptr);
  }};

  Stream<int> s = stream({}, 1, 20).unwrap();

  sched->once([]() { info("Hi"); }, AwaitStreams{{s.alias()}, {0}});
  sched->once([]() { info("Hello"); });
  sched->once([]() { info("Sshh"); });
  info("scheduled");
  sched->once([]() { info("Timer passed"); },
              Delay{.from = steady_clock::now(), .delay = 1ms});

  auto fut = future<int>({}).unwrap();

  sched->loop(
    [x = (u64) 0, f = fut.alias(), s = s.alias()]() mutable -> bool {
      x++;
      info("iteration: {}", x);
      info("future value: {}", f.get());
      s.yield_unsequenced([x](int & v) { v = x; }, 1);
      if (x == 10)
      {
        info("loop exited");
        return false;
      }

      return true;
    },
    AwaitFutures{fut.alias()});
  fut.yield(69).unwrap();

  sched->shard<int *>(
    rc<int>(inplace, {}, 0).unwrap(),
    [](TaskInstance shard, int * pcount) {
      std::atomic_ref count_ref{*pcount};
      int             count = count_ref.fetch_add(1);
      info("shard: {}  of {}, sync i: {}", shard.idx, shard.n, count);
    },
    10);

  sched->request_drain();
  sched->await_drain(nanoseconds::max());
}
