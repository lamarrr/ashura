/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/std/allocator.h"
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include "ashura/std/log_sinks.h"

namespace ash
{
void init_sync_runtime();
}

int main(int argc, char ** argv)
{
  using namespace ash;

  init_sync_runtime();
  ILogger logger{span<LogSink>({&stdio_sink})};
  hook_logger(&logger);
  defer logger_{[&] { hook_logger(nullptr); }};
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
