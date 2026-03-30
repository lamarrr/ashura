/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/std/allocator.hpp"
#include "ashura/std/error.hpp"
#include "ashura/std/log.hpp"
#include "ashura/std/log_sinks.hpp"

int main(int argc, char ** argv)
{
  using namespace ash;
  ILogger logger{span<LogSink>({&stdio_sink})};
  hook_logger(&logger);
  defer logger_{[&] { hook_logger(nullptr); }};
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
