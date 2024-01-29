#include "gtest/gtest.h"

#include "ashura/std/sparse_set.h"
#include <bitset>
#include <iostream>

TEST(SparseSetTest, Start)
{
  ash::SparseSet<ash::u64> set;
  set.compact([](ash::u64, ash::u64) {});
  ASSERT_TRUE(set.reserve_new_ids(ash::heap_allocator, 100));
  ASSERT_EQ(set.num_slots, 100);
  ASSERT_EQ(set.num_free, 100);
  ash::uid64 id;
  for (ash::u32 i = 0; i < 100; i++)
  {
    ASSERT_TRUE(set.allocate_id(id));
    ASSERT_LT(id, set.num_slots);
    ASSERT_GE(id, 0);
  }
  ASSERT_FALSE(set.allocate_id(id));
  for (ash::u32 i = 0; i < 100; i += 2)
  {
    ASSERT_TRUE(set.release(i));
  }
  set.compact([](auto src, auto dst) {
    ASSERT_TRUE(src % 2 == 1);
    ASSERT_TRUE(dst % 2 == 0);
  });
  ASSERT_EQ(set.num_free, 100 / 2);

  for (ash::u32 i = 0; i < 100; i++)
  {
    if (i % 2)
    {
      ASSERT_TRUE(set.is_valid_id(i));
    }
    else
    {
      ASSERT_FALSE(set.is_valid_id(i));
    }
  }
  for (ash::u32 i = 0; i < 100 / 2; i++)
  {
    ASSERT_TRUE(set.allocate_id(id));
    ASSERT_LT(id, set.num_slots);
    ASSERT_GE(id, 0);
    ASSERT_TRUE(id % 2 == 0);
  }
  ASSERT_FALSE(set.allocate_id(id));
  set.release_ids();
  EXPECT_EQ(set.num_free, set.num_slots);
  EXPECT_TRUE(set.allocate_id(id));
  EXPECT_EQ(id, 0);
  set.reset(ash::heap_allocator);
}
