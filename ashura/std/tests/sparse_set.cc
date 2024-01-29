#include "gtest/gtest.h"

#include "ashura/std/sparse_set.h"
#include <iostream>

TEST(SparseSetTest, Start)
{
  ash::SparseSet<ash::u64> set;
  set.compact([](ash::u64, ash::u64) {});
  EXPECT_TRUE(set.reserve_new_ids(ash::heap_allocator, 5));
  EXPECT_EQ(set.num_slots, 5);
  EXPECT_EQ(set.num_free, 5);
     std::cout << "a: " << "id"<< ", b:" << set.num_slots
              << ", c:" << set.num_free << ", d:" << set.free_id_head
              << ", e:" << set.free_index_head<<std::endl;
  ash::uid64 id;
  for (ash::u32 i = 0; i < 5; i++)
  {
    ASSERT_TRUE(set.allocate_id(id));
    EXPECT_LT(id, set.num_slots);
    EXPECT_GE(id, 0);
    std::cout << "a: " << id << ", b:" << set.num_slots
              << ", c:" << set.num_free << ", d:" << set.free_id_head
              << ", e:" << set.free_index_head<<std::endl;
  }
  // ASSERT_FALSE(set.allocate_id(id));
  set.reset(ash::heap_allocator);
}
