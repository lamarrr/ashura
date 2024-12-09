/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/range.h"

using namespace ash;

TEST(Range, RunEndEncoding)
{
  static constexpr u32 data[]           = {0, 1, 2,  3,  4,  5,  6,  7,
                                           8, 9, 10, 11, 12, 13, 14, 15};
  static constexpr u32 prefix_indices[] = {0, 8, 16};

  constexpr RunRange runs = prefix_run(span(prefix_indices), span(data));

  auto iter = runs.begin();

  ASSERT_NE(iter, runs.end());
  EXPECT_EQ((*iter).v0.size(), 8);
  EXPECT_EQ((*iter).v0[0], 0);
  ++iter;
  ASSERT_NE(iter, runs.end());
  EXPECT_EQ((*iter).v0.size(), 8);
  EXPECT_EQ((*iter).v0[0], 8);
}
