/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/range.h"

using namespace ash;

TEST(RunBatchView, RunEndEncoding)
{
  static constexpr u32 data[]           = {0, 1, 2,  3,  4,  5,  6,  7,
                                           8, 9, 10, 11, 12, 13, 14, 15};
  static constexpr u32 prefix_indices[] = {0, 8, 16};

  constexpr auto runs = RunBatchView(span(prefix_indices), span(data));

  auto iter = runs.begin();

  ASSERT_NE(iter, runs.end());
  EXPECT_EQ((*iter).v0.size(), 8);
  EXPECT_EQ((*iter).v0[0], 0);
  ++iter;
  ASSERT_NE(iter, runs.end());
  EXPECT_EQ((*iter).v0.size(), 8);
  EXPECT_EQ((*iter).v0[0], 8);
}

TEST(RunItemView, RunEndEncoding)
{
  static constexpr u32 data[]           = {0, 10, 20, 30};
  static constexpr u32 prefix_indices[] = {0, 8, 16, 24};

  constexpr auto runs = RunItemView(span(prefix_indices), span(data));

  auto iter = runs.begin();

  EXPECT_EQ(runs.num_runs(), 3);
  EXPECT_EQ(runs.size(), 24);
  EXPECT_EQ(iter.run(), 0);
  EXPECT_EQ(iter.item(), 0);
  EXPECT_EQ((*iter).v0, 0);
  iter.seek(9);

  EXPECT_EQ(iter.run(), 1);
  EXPECT_EQ(iter.item(), 9);
  EXPECT_EQ((*iter).v0, 10);

  iter.seek(16);
  EXPECT_EQ(iter.run(), 2);
  EXPECT_EQ(iter.item(), 16);
  EXPECT_EQ((*iter).v0, 20);

  iter.seek(24);
  EXPECT_FALSE(iter != runs.end());
}

TEST(BinaryFind, Find)
{
  static constexpr u32 data[] = {1, 1, 2, 3, 4, 5, 6, 7, 8};

  {
    constexpr auto s = binary_find(span(data), gt, 4U);
    EXPECT_EQ(s[0], 5);
  }

  {
    constexpr auto s = binary_find(span(data), geq, 4U);
    EXPECT_EQ(s[0], 4);
  }

  {
    constexpr auto s = binary_find(span(data), gt, 7U);
    EXPECT_EQ(s[0], 8);
  }

  {
    constexpr auto s = binary_find(span(data), geq, 7U);
    EXPECT_EQ(s[0], 7);
  }

  {
    constexpr auto s = binary_find(span(data), lt, 7U);
    EXPECT_EQ(s[0], 1);
  }

  {
    constexpr auto s = binary_find(span(data), leq, 7U);
    EXPECT_EQ(s[0], 1);
  }
}
