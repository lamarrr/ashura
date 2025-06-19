/// SPDX-License-Identifier: MIT
#include "ashura/std/enum.h"
#include "ashura/std/dict.h"
#include "gtest/gtest.h"

TEST(EnumTest, Basic)
{
  using namespace ash;
  Enum<StrDict<int>, int, float, int *> f{nullptr};
  EXPECT_EQ(f.index(), 3);

  static constexpr Enum<int, char, float> e{0.2F};

  constexpr auto a = e.match([](f32) { return 1.0F; },        //
                             [](f32) { return 0.5F; },        //
                             [](f32) { return 3.0F; });

  EXPECT_EQ(a, 3.0F);
  EXPECT_EQ(e.v2_, 0.2F);
}
