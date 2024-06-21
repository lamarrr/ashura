#include "gtest/gtest.h"

#include "ashura/std/async.h"
#include "ashura/std/error.h"
#include "ashura/std/list.h"

std::atomic<uint64_t> invocs = 0;

TEST(AsyncTest, Basic)
{
  using namespace ash;
  StdioSink sink;
  default_logger = create_logger(to_span<LogSink *>({&sink}), heap_allocator);

  Semaphore sem = create_semaphore(1);
  scheduler->init(to_span<nanoseconds>({1ns, 2ns}), to_span({2ns, 5ns}));

  for (u32 i = 0; i < 5'000; i++)
  {
    scheduler->schedule_worker({.task = to_fn([](void *) {
                                  invocs.fetch_add(1);
                                  return false;
                                })});
    scheduler->schedule_dedicated(0, {.task = to_fn([](void *) {
                                        static int x = 0;
                                        x++;
                                        if (x > 10)
                                        {
                                          return false;
                                        }
                                        std::this_thread::sleep_for(7ms);
                                        return true;
                                      })});
    scheduler->schedule_dedicated(1, {.task = to_fn([](void *) {
                                        invocs.fetch_add(1);
                                        return false;
                                      })});

    if (i % 1'000 == 0)
    {
      std::this_thread::sleep_for(1ms);
    }
  }

  scheduler->schedule_main({.task = to_fn([](void *) {
                              static int x = 0;
                              x++;
                              if (x > 10)
                              {
                                return false;
                              }
                              std::this_thread::sleep_for(8ms);
                              return true;
                            })});
  signal_semaphore(sem, 1);
  scheduler->execute_main_thread_work(5s);
  std::this_thread::sleep_for(1min);
  scheduler->uninit();
  destroy_semaphore(sem);
}