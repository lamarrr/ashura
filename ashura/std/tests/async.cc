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

  SemaphoreRef sem = create_semaphore(1, default_allocator);
  scheduler->init(span<nanoseconds>({1ns, 2ns}), span({2ns, 5ns}));

  for (u32 i = 0; i < 5'000; i++)
  {
    scheduler->schedule_worker({.task = fn([]() {
                                  invocs.fetch_add(1);
                                  return false;
                                })});
    scheduler->schedule_dedicated(0, {.task = fn([]() {
                                        static int x = 0;
                                        x++;
                                        if (x > 10)
                                        {
                                          return false;
                                        }
                                        std::this_thread::sleep_for(8us);
                                        return true;
                                      })});
    scheduler->schedule_dedicated(1, {.task = fn([]() {
                                        invocs.fetch_add(1);
                                        return false;
                                      })});

    if (i % 1'000 == 0)
    {
      std::this_thread::sleep_for(1ms);
    }
  }

  scheduler->schedule_main({.task = fn([]() {
                              static int x = 0;
                              x++;
                              if (x > 10)
                              {
                                return false;
                              }
                              std::this_thread::sleep_for(8us);
                              return true;
                            })});
  sem->signal(1);
  scheduler->execute_main_thread_work(5s);
  std::this_thread::sleep_for(5ms);
  scheduler->uninit();
  uninit_semaphore(sem, default_allocator);
}