/// SPDX-License-Identifier: MIT
#include "ashura/std/map.h"
#include "gtest/gtest.h"

TEST(MapTest, Insertion)
{
  using namespace ash;
  StrMap<int> map;
  EXPECT_FALSE(map.has("A"_str));

  ASSERT_TRUE(map.insert("A"_str, 0).is_ok());
  EXPECT_TRUE(map.has("A"_str));
  EXPECT_EQ(map["A"_str], 0);
  ASSERT_TRUE(map.insert("B"_str, 1).is_ok());
  EXPECT_TRUE(map.has("A"_str));
  EXPECT_TRUE(map.has("B"_str));
  EXPECT_EQ(map["A"_str], 0);
  EXPECT_EQ(map["B"_str], 1);
  EXPECT_FALSE(map.erase("C"_str));
  EXPECT_TRUE(map.erase("A"_str));
  EXPECT_FALSE(map.has("A"_str));
  EXPECT_TRUE(map.erase("B"_str));
  EXPECT_FALSE(map.has("B"_str));
}