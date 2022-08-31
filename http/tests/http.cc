#include "vlk/http.h"
#include "gtest/gtest.h"
#include "stx/scheduler.h"
#include "stx/scheduler/scheduling/await.h"

#include <iostream>

TEST(Http, Pr) {
  vlk::http::Client client{stx::os_allocator};

  stx::TaskScheduler scheduler{stx::os_allocator,
                               std::chrono::steady_clock::now()};

  auto [response, monitor] =
      client.get(stx::string::make_static("https://github.com"));

  stx::Future a = stx::sched::await(
      scheduler,
      [](stx::Future<vlk::http::Response> response) {
        auto& vec = response.ref().unwrap()->content;

        for (uint8_t c : vec) {
          std::cout << (char)c;
        }
        std::cout << std::endl;
      },
      stx::NORMAL_PRIORITY, {}, response.share());

  while (!a.is_done()) {
    client.tick();
    scheduler.tick({});
  }
}
