#include "ashura/std/hash_map.h"
#include "gtest/gtest.h"

TEST(HashMapTest, Insertion)
{
  using namespace ash;
  StrHashMap<int> map;
  EXPECT_FALSE(map.has("A"_span));

  bool exists;
  ASSERT_TRUE(map.insert(exists, nullptr, "A"_span, 0));
  EXPECT_TRUE(map.has("A"_span));
  EXPECT_EQ(*map["A"_span], 0);
  ASSERT_TRUE(map.insert(exists, nullptr, "B"_span, 1));
  EXPECT_TRUE(map.has("A"_span));
  EXPECT_TRUE(map.has("B"_span));
  EXPECT_EQ(*map["A"_span], 0);
  EXPECT_EQ(*map["B"_span], 1);
}