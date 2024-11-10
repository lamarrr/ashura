/// SPDX-License-Identifier: MIT
#include "ashura/std/enum.h"
#include "ashura/std/hash_map.h"
#include "gtest/gtest.h"

TEST(EnumTest, Basic)
{
  using namespace ash;
  Enum<StrHashMap<int>, int, float, int *> f{nullptr};
  EXPECT_EQ(f.index(), 3);

  static constexpr Enum<int, char, float> e{0.0F};
}