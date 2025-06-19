/// SPDX-License-Identifier: MIT
#include "ashura/std/dict.h"
#include "gtest/gtest.h"

TEST(DictTest, Insertion)
{
  using namespace ash;
  StrDict<int> dict;
  EXPECT_FALSE(dict.has("A"_str));

  ASSERT_TRUE(dict.insert("A"_str, 0).is_ok());
  EXPECT_TRUE(dict.has("A"_str));
  EXPECT_EQ(dict["A"_str], 0);
  ASSERT_TRUE(dict.insert("B"_str, 1).is_ok());
  EXPECT_TRUE(dict.has("A"_str));
  EXPECT_TRUE(dict.has("B"_str));
  EXPECT_EQ(dict["A"_str], 0);
  EXPECT_EQ(dict["B"_str], 1);
  EXPECT_FALSE(dict.erase("C"_str));
  EXPECT_TRUE(dict.erase("A"_str));
  EXPECT_FALSE(dict.has("A"_str));
  EXPECT_TRUE(dict.erase("B"_str));
  EXPECT_FALSE(dict.has("B"_str));
}
