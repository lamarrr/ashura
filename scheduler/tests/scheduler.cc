
#include "vlk/scheduler.h"

#include "gtest/gtest.h"
#include "vlk/scheduler/scheduling/deferred.h"
#include "vlk/scheduler/scheduling/delay.h"
#include "vlk/scheduler/scheduling/schedule.h"

TEST(SchedulerTest, Main) {
  using namespace vlk;
  using namespace stx;

  TaskScheduler scheduler{std::chrono::steady_clock::now(), stx::os_allocator};

  scheduler.tick(std::chrono::nanoseconds{1});

  sched::chain(scheduler,
               stx::Chain{[](stx::Void) {
                            VLK_LOG("first");
                            return 2;
                          },
                          [](int a) {
                            VLK_LOG("second");
                            return stx::Void{};
                          }},
               stx::NORMAL_PRIORITY, {});
  sched::fn(scheduler, []() { VLK_LOG("hello"); }, stx::NORMAL_PRIORITY, {});
  sched::fn(scheduler, []() { VLK_LOG("world!"); }, stx::NORMAL_PRIORITY, {});



  scheduler.tick(std::chrono::nanoseconds{1});
}
