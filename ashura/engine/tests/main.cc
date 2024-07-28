/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/std/allocator.h"
#include "ashura/std/log.h"

int main(int argc, char **argv)
{
  using namespace ash;
  logger->add_sink(&stdio_sink);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
