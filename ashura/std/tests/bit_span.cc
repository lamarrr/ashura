#include "ashura/std/range.h"
#include "ashura/std/types.h"
#include "gtest/gtest.h"

using namespace ash;

TEST(BitSpanTest, Misc)
{
  u32          x[20];
  BitSpan<u32> span{x, 20 * 32};

  span[0] = true;
  EXPECT_TRUE(span[0]);
  *span.begin() = false;
  EXPECT_FALSE(span[0]);
  span[1] = true;
  EXPECT_FALSE(span[0]);
  EXPECT_TRUE(span[1]);
  fill(span, false);
  EXPECT_FALSE(contains(span, true));
  EXPECT_TRUE(contains(span, false));
}