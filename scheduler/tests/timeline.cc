#include "stx/memory.h"
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

  auto fn = stx::make_functor_fn(stx::os_allocator, [](int value) {
              VLK_LOG("value {}", value);
            }).unwrap();

  auto g = stx::make_static_fn(rawrrr);

  g.get()(5);

  auto xg = stx::make_static_fn([](float a) { return rawrrr(a); });

  xg.get()(34);

  fn.get()(8);

  auto d = stx::make_static_fn(
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
        .add_task(stx::make_static_fn([]() {}), {}, {},
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
          .push(stx::mem::make_rc_inplace<vlk::ThreadSlot>(
                    stx::os_allocator,
                    stx::make_promise<void>(stx::os_allocator).unwrap())
                    .unwrap())
          .unwrap();

    for (size_t i = 0; i < 20; i++) {
      timeline
          .add_task(stx::make_static_fn([]() {}), {}, {},
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

  stx::Heaped heaped =
      stx::make_heaped(stx::os_allocator, std::array<fuck, 400>{}).unwrap();
  heaped->size();
  heaped->empty();
}
