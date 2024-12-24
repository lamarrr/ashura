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
  CHECK(logger->add_sink(&stdio_sink));

  Semaphore sem = create_semaphore({}, 1).unwrap();

  scheduler->init({}, std::this_thread::get_id(), span<nanoseconds>({1ns, 2ns}),
                  span({2ns, 5ns}));
  defer       scheduler_{[&] { scheduler->uninit(); }};
  Stream<int> s = stream({}, 1, 20).unwrap();

  async::once([]() { logger->info("Hi"); }, AwaitStreams{{0}, s.alias()});
  async::once([]() { logger->info("Hello"); });
  async::once([]() { logger->info("Sshh"); });
  logger->info("scheduled");
  async::once([]() { logger->info("Timer passed"); },
              Delay{.from = steady_clock::now(), .delay = 1ms});

  auto fut = future<int>({}).unwrap();

  async::loop(
    [x = (u64) 0, f = fut.alias(), s = s.alias()]() mutable -> bool {
      x++;
      logger->info(x, " iteration");
      logger->info("future value: ", f.get());
      s.yield_unsequenced([x](int & v) { v = x; }, 1);
      if (x == 10)
      {
        logger->info("loop exited");
        return false;
      }

      return true;
    },
    AwaitFutures{fut.alias()});
  fut.yield(69).unwrap();

  async::shard<std::atomic<int> *>(
    [](TaskInstance shard, std::atomic<int> * pcount) {
      int count = pcount->fetch_add(1);
      logger->info("shard: ", shard.idx, " of ", shard.n, ", sync i: ", count);
    },
    rc_inplace<std::atomic<int>>({}, 0).unwrap(), 10);

  std::this_thread::sleep_for(500ms);
  scheduler->join();
}
