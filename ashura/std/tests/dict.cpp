/// SPDX-License-Identifier: MIT
#include "ashura/std/dict.hpp"
#include "gtest/gtest.h"

TEST(DictTest, Insertion)
{
    using namespace ash;
    StrDict<int> dict{default_allocator};
    EXPECT_FALSE(dict.has("A"_s));

    ASSERT_TRUE(dict.push("A"_s, 0).is_ok());
    EXPECT_TRUE(dict.has("A"_s));
    EXPECT_EQ(dict["A"_s], 0);
    ASSERT_TRUE(dict.push("B"_s, 1).is_ok());
    EXPECT_TRUE(dict.has("A"_s));
    EXPECT_TRUE(dict.has("B"_s));
    EXPECT_EQ(dict["A"_s], 0);
    EXPECT_EQ(dict["B"_s], 1);
    EXPECT_FALSE(dict.erase("C"_s));
    EXPECT_TRUE(dict.erase("A"_s));
    EXPECT_FALSE(dict.has("A"_s));
    EXPECT_TRUE(dict.erase("B"_s));
    EXPECT_FALSE(dict.has("B"_s));
}
