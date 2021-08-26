#include "stx/dynamic.h"
#include "vlk/scheduler.h"

using namespace std::chrono_literals;

float rawrrr(float arg) {
  VLK_LOG("rawwwrrrrrrr!!!!!!!!!!!!! {}", arg);
  return arg;
}

void gggg() {
  // vlk::TaskScheduler sched;

  stx::Promise promise = stx::make_promise<int>(stx::os_allocator).unwrap();
  stx::Future future = promise.get_future();
  // auto [future2, promise2] = stx::make_future<void>();

  auto fn = stx::fn::rc::make_functor(stx::os_allocator, [](int value) {
              VLK_LOG("value {}", value);
            }).unwrap();

  auto g = stx::fn::rc::make_static(rawrrr);

  g.handle(5);

  auto xg = stx::fn::rc::make_static([](float a) { return rawrrr(a); });

  xg.handle(34);

  fn.handle(8);

  auto d = stx::fn::rc::make_static(
      [](stx::Future<int>, stx::Future<void>) { VLK_LOG("all ready!"); });

  vlk::Chain{[](vlk::Void) -> int {
               VLK_LOG("executing 1 ...");
               std::this_thread::sleep_for(1s);
               return 0;
             },
             [](int x) {
               VLK_LOG("executing 2 ...");
               std::this_thread::sleep_for(1s);
               return x + 1;
             },
             [](int x) {
               VLK_LOG("executing 3 ...");
               std::this_thread::sleep_for(1s);
               return x + 2.5;
             },
             [](float y) {
               VLK_LOG("executing 4 ...");
               std::this_thread::sleep_for(1s);
               return y + 5;
             },
             rawrrr};

  // d.get()();
  /*
  stx::TaskPriority::Background, vlk::TaskTraceInfo{}, future,
  future2);

promise.notify_completed(8);
promise2.notify_completed();
*/
  /*
    sched.schedule(stx::make_static_fn([]() { return; }),
                   stx::TaskPriority::Background, {});
    sched.schedule_chain(vlk::Chain{[](vlk::Void) -> int {
                                      VLK_LOG("executing 1 ...");
                                      std::this_thread::sleep_for(1s);
                                      return 0;
                                    },
                                    [](int x) {
                                      VLK_LOG("executing 2 ...");
                                      std::this_thread::sleep_for(1s);
                                      return x + 1;
                                    },
                                    [](int x) {
                                      VLK_LOG("executing 3 ...");
                                      std::this_thread::sleep_for(1s);
                                      return x + 2.5;
                                    },
                                    [](float y) {
                                      VLK_LOG("executing 4 ...");
                                      std::this_thread::sleep_for(1s);
                                      return y + 5;
                                    },
                                    rawrrr},
                         stx::TaskPriority::Critical, vlk::TaskTraceInfo{});

    for (auto& task : sched.entries) {
      if (task.state == vlk::TaskScheduler::EntryState::Scheduled &&
          task.is_ready.get()()) {
        task.state = vlk::TaskScheduler::EntryState::Executing;
        task.packaged_task.get()();
      } else if (true) {
        // ... and others
      }
    }
    */
}

#include "gtest/gtest.h"
#include "vlk/scheduler/timeline.h"

struct alignas(64) fuck {
  int y;
};

TEST(ScheduleTimelineTest, Tick) {
  auto timepoint = std::chrono::steady_clock::now();
  {
    vlk::ScheduleTimeline timeline{stx::os_allocator};

    stx::Vec<stx::Rc<vlk::ThreadSlot *>> slots{stx::noop_allocator};

    timeline.tick(slots, timepoint);

    timeline
        .add_task(stx::fn::rc::make_static([]() {}), {}, {},
                  stx::PromiseAny{
                      stx::make_promise<void>(stx::os_allocator).unwrap()},
                  timepoint)
        .unwrap();

    timeline.tick(slots, timepoint);

    EXPECT_EQ(timeline.thread_slots_capture.size(), 0);
  }

  {
    vlk::ScheduleTimeline timeline{stx::os_allocator};

    stx::Vec<stx::Rc<vlk::ThreadSlot *>> slots{stx::os_allocator};

    for (size_t i = 0; i < 10; i++)
      slots
          .push(stx::dyn::rc::make_inplace<vlk::ThreadSlot>(
                    stx::os_allocator,
                    stx::make_promise<void>(stx::os_allocator).unwrap())
                    .unwrap())
          .unwrap();

    for (size_t i = 0; i < 20; i++) {
      timeline
          .add_task(stx::fn::rc::make_static([]() {}), {}, {},
                    stx::PromiseAny{
                        stx::make_promise<void>(stx::os_allocator).unwrap()},
                    timepoint)
          .unwrap();
    }

    timeline.tick(slots, timepoint);
    EXPECT_EQ(slots.size(), 10);
    EXPECT_EQ(timeline.thread_slots_capture.size(), slots.size());

    EXPECT_EQ(timeline.starvation_timeline.size(), 20);
  }

  stx::Dynamic dyn_array =
      stx::dyn::make(stx::os_allocator, std::array<fuck, 400>{}).unwrap();
  dyn_array->size();
  dyn_array->empty();

  stx::Str h = stx::str::make_static("Hello boy");
  stx::Str y = stx::str::make(stx::os_allocator, "Hello boy").unwrap();
  EXPECT_EQ(h, "Hello boy");
  EXPECT_NE(h, "Hello Boy");
  EXPECT_EQ(h, y);

  EXPECT_TRUE(h.starts_with("Hello world"));
}

#include "vlk/scheduler/scheduling/await.h"
#include "vlk/scheduler/scheduling/delay.h"
#include "vlk/scheduler/scheduling/schedule.h"

void brr() {}
int rx() { return 0; }

int first(stx::Void) { return 0; }
int rx_loop(int64_t) { return 0; }

TEST(SchedulerTest, HH) {
  namespace sched = vlk::sched;
  vlk::TaskScheduler scheduler{std::chrono::steady_clock::now()};

  sched::loop(scheduler, stx::Loop{[]() {
                int x;
                (void)x;
                return;
              }},
              stx::SERVICE_PRIORITY, {});

  sched::loop(scheduler, stx::Loop{brr}, stx::SERVICE_PRIORITY, {});

  sched::forloop(scheduler, stx::For{0, 0, rx_loop}, stx::SERVICE_PRIORITY, {});

  sched::fn(scheduler, []() { return 0; }, stx::CRITICAL_PRIORITY, {});

  vlk::Future a = sched::fn(scheduler, rx, stx::CRITICAL_PRIORITY, {});
  vlk::Future b = sched::chain(scheduler, vlk::Chain{first, rx_loop},
                               stx::INTERACTIVE_PRIORITY, {});

  //
  // auto [a_d, b_d] =  reap/drain(a, b)
  // we need an operator to finialize on the await operation.
  // i.e. drain
  //
  //

  using namespace std::chrono_literals;

  vlk::Future<float> c = sched::await_any(
      scheduler,
      [](vlk::Future<int> a, vlk::Future<int> b) {
        return (a.copy().unwrap_or(0) + b.copy().unwrap_or(0)) * 20.0f;
      },
      stx::NORMAL_PRIORITY, {}, a.share(), b.share());

  sched::await(
      scheduler, [](vlk::Future<int>, vlk::Future<int>) {},
      stx::CRITICAL_PRIORITY, {}, a.share(), b.share());

  sched::delay(
      scheduler, []() {}, stx::SERVICE_PRIORITY, {}, 500ms);

  // forloop combine with loop combine with others???
}
