#include "vlk/scheduler/scheduling/schedule.h"

#include "vlk/scheduler/scheduling/await.h"

void brr() {}
int rx() { return 0; }

int first(stx::Void) { return 0; }
int rx_loop(int64_t) { return 0; }

void ff() {
  vlk::TaskScheduler scheduler{std::chrono::steady_clock::now()};

  vlk::sched::loop(scheduler, stx::Loop{[]() {
                     int x;
                     (void)x;
                     return;
                   }},
                   stx::TaskPriority::Background, {});

  vlk::sched::loop(scheduler, stx::Loop{brr}, stx::TaskPriority::Background,
                   {});

  vlk::sched::forloop(scheduler, stx::For{0, 0, rx_loop},
                      stx::TaskPriority::Background, {});

  vlk::sched::fn(scheduler, []() { return 0; }, stx::TaskPriority::Critical,
                 {});

  vlk::Future a =
      vlk::sched::fn(scheduler, rx, stx::TaskPriority::Critical, {});

  vlk::Future b = vlk::sched::chain(scheduler, vlk::Chain{first, rx_loop},
                                    stx::TaskPriority::Interactive, {});

  //
  // auto [a_d, b_d] =  reap/drain(a, b)
  // we need an operator to finialize on the await operation.
  // i.e. drain
  //
  //

  vlk::sched::await_any(
      scheduler, [](vlk::Future<int>, vlk::Future<int>) {},
      stx::TaskPriority::Background, {}, a.share(), b.share());

  vlk::sched::await_all(
      scheduler, [](vlk::Future<int>, vlk::Future<int>) {},
      stx::TaskPriority::Critical, {}, a.share(), b.share());

  // forloop combine with loop combine with others???
}
