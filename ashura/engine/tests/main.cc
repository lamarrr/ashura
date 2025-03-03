/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/engine/window.h"
#include "ashura/std/allocator.h"
#include "ashura/std/error.h"
#include "ashura/std/log.h"

int main(int argc, char ** argv)
{
  using namespace ash;
  Logger logger{&stdio_sink};
  hook_logger(&logger);
  defer logger_{[&] { hook_logger(nullptr); }};
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
