#include "ashura/context.h"
#include "ashura/subsystems/http_client.h"
#include "stx/scheduler.h"
#include "stx/scheduler/scheduling/await.h"
#include "gtest/gtest.h"

using namespace ash;

Context _ctx;

TEST(Http, HttpClient)
{
  HttpClient         client{stx::os_allocator};
  stx::TaskScheduler scheduler{stx::os_allocator,
                               std::chrono::steady_clock::now()};
  auto [response, monitor] =
      client.get(stx::string::make_static("https://github.com"));

  stx::Future<void> a = stx::sched::await(
      scheduler,
      [](stx::Future<HttpResponse> response) {
        auto httpResponse = response.ref().unwrap();
        EXPECT_EQ(httpResponse->code, 200);
        EXPECT_FALSE(httpResponse->header.is_empty());
        EXPECT_FALSE(httpResponse->content.is_empty());
        EXPECT_EQ(httpResponse->uploaded, 0);
        EXPECT_GT(httpResponse->downloaded, 0);
      },
      stx::NORMAL_PRIORITY, {}, response.share());

  // Wait for the completion of the future
  while (!a.is_done())
  {
    std::chrono::nanoseconds interval{1000000};
    client.tick(_ctx, interval);
    scheduler.tick(interval);
  }
}