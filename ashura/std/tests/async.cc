/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/async.h"
#include "ashura/std/error.h"
#include "ashura/std/list.h"
#include <chrono>
#include <thread>

std::atomic<uint64_t> invocs = 0;

TEST(AsyncTest, Basic)
{
  using namespace ash;
  logger->add_sink(&stdio_sink);

  Semaphore sem = create_semaphore(1, default_allocator);

  scheduler->init(default_allocator, std::this_thread::get_id(),
                  span<nanoseconds>({1ns, 2ns}), span({2ns, 5ns}));
  defer scheduler_{[&] { scheduler->uninit(); }};

  async::once([]() { logger->info("Hi"); });
  async::once([]() { logger->info("Hello"); });
  async::once([]() { logger->info("Sshh"); });
  logger->info("scheduled");
  async::once([]() { logger->info("Timer passed"); },
              async::Delay{.from = steady_clock::now(), .delay = 1ms});
  async::loop([x = (u64) 0]() mutable -> bool {
    x++;
    logger->info(x, " iteration");
    if (x == 10)
    {
      logger->info("loop exited");
      return false;
    }

    return true;
  });
  async::shard(
      [](async::TaskInstance instance) {
        logger->info("shard: ", instance.idx, " of ", instance.instances);
      },
      10);

  std::this_thread::sleep_for(500ms);
  scheduler->join();
}