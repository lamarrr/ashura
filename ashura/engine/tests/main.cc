/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/std/allocator.h"
#include "ashura/std/log.h"

namespace ash
{
Logger *default_logger;
}

int main(int argc, char **argv)
{
  ash::StdioSink sink;
  ash::default_logger = ash::create_logger(ash::span<ash::LogSink *>({&sink}),
                                           ash::default_allocator);
  ash::defer default_logger_del{
      [&] { ash::destroy_logger(ash::default_logger); }};
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
