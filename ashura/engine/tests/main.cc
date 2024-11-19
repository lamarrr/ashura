/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/std/allocator.h"
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include "ashura/engine/window.h"

int main(int argc, char **argv)
{
  using namespace ash;
  Logger::init();
  CHECK(logger->add_sink(&stdio_sink));
  WindowSystem::init();
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
