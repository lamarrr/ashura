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

  Rc<Semaphore *> sem = create_semaphore(1, default_allocator);
  defer           sem_{[&] { sem.uninit(); }};

  scheduler->init(span<nanoseconds>({1ns, 2ns}), span({2ns, 5ns}));
  defer scheduler_{[&] { scheduler->uninit(); }};

  for (u32 i = 0; i < 5'000; i++)
  {
    scheduler->schedule_worker({.task = fn([](TaskInstance, void *) {
                                  invocs.fetch_add(1);
                                  return false;
                                })});
    scheduler->schedule_dedicated(0, {.task = fn([](TaskInstance, void *) {
                                        static int x = 0;
                                        x++;
                                        if (x > 10)
                                        {
                                          return false;
                                        }
                                        std::this_thread::sleep_for(8us);
                                        return true;
                                      })});
    scheduler->schedule_dedicated(1, {.task = fn([](TaskInstance, void *) {
                                        invocs.fetch_add(1);
                                        return false;
                                      })});

    if (i % 1'000 == 0)
    {
      std::this_thread::sleep_for(1ms);
    }
  }

  auto poll = [&](void *) {
    return await_semaphores(span({sem.get(), sem.get()}), span<u64>({0, 0}),
                            {});
  };

  scheduler->schedule_main({.task = fn([](TaskInstance, void *) {
                              static int x = 0;
                              x++;
                              if (x > 10)
                              {
                                return false;
                              }
                              std::this_thread::sleep_for(8us);
                              return true;
                            }),
                            .poll = fn(&poll)});
  sem->signal(1);
  scheduler->execute_main_thread_work(5s);
  std::this_thread::sleep_for(500ms);
}