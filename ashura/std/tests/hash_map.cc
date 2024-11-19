/// SPDX-License-Identifier: MIT
#include "ashura/std/map.h"
#include "gtest/gtest.h"

TEST(MapTest, Insertion)
{
  using namespace ash;
  StaticStrMap<int> map;
  EXPECT_FALSE(map.has("A"_span));

  ASSERT_TRUE(map.insert("A"_span, 0).is_ok());
  EXPECT_TRUE(map.has("A"_span));
  EXPECT_EQ(map["A"_span], 0);
  ASSERT_TRUE(map.insert("B"_span, 1).is_ok());
  EXPECT_TRUE(map.has("A"_span));
  EXPECT_TRUE(map.has("B"_span));
  EXPECT_EQ(map["A"_span], 0);
  EXPECT_EQ(map["B"_span], 1);
  EXPECT_FALSE(map.erase("C"_span));
  EXPECT_TRUE(map.erase("A"_span));
  EXPECT_FALSE(map.has("A"_span));
  EXPECT_TRUE(map.erase("B"_span));
  EXPECT_FALSE(map.has("B"_span));
}