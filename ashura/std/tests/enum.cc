/// SPDX-License-Identifier: MIT
#include "ashura/std/enum.h"
#include "ashura/std/map.h"
#include "gtest/gtest.h"

TEST(EnumTest, Basic)
{
  using namespace ash;
  Enum<StrMap<int>, int, float, int *> f{nullptr};
  EXPECT_EQ(f.index(), 3);

  // [ ] map(   )
  // [ ]

  static constexpr Enum<int, char, float> e{0.0F};
}