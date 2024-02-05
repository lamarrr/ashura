#include "gtest/gtest.h"

#include "ashura/std/sparse_vec.h"
#include <bitset>
#include <iostream>

using namespace ash;

TEST(SparseSetTest, Start)
{
  Vec<int> f{heap_allocator};

  ASSERT_TRUE(f.push(1));
  ASSERT_EQ(f[0], 1);
  ASSERT_EQ(f.size(), 1);
  ASSERT_TRUE(f.extend_copy(to_span({2, 3, 4, 5, 6})));
  ASSERT_EQ(f.size(), 6);
  ASSERT_EQ(f[5], 6);
  ASSERT_TRUE(f.fit());
  ASSERT_EQ(f.size(), 6);
  ASSERT_EQ(f.size(), f.capacity());
  ASSERT_EQ(f[5], 6);
  ASSERT_TRUE(f.insert(f.size(), 7));
  ASSERT_EQ(f.size(), 7);
  ASSERT_EQ(f[0], 1);
  ASSERT_EQ(f[5], 6);
  ASSERT_EQ(f[6], 7);
  ASSERT_TRUE(f.insert_span_copy(f.size(), to_span({8, 9, 0})));
  ASSERT_EQ(f.size(), 10);
  ASSERT_EQ(f[7], 8);
  ASSERT_EQ(f[9], 0);
  ASSERT_TRUE(f.extend_defaulted(5));
  ASSERT_EQ(f.size(), 15);
  ASSERT_EQ(f[10], 0);
  ASSERT_EQ(f[12], 0);
  ASSERT_EQ(f[14], 0);
  f.clear();
  ASSERT_TRUE(f.is_empty());
  ASSERT_NE(f.data(), nullptr);
  ASSERT_GT(f.capacity(), 0);
  f.reset();
  ASSERT_TRUE(f.is_empty());
  ASSERT_EQ(f.data(), nullptr);
  ASSERT_EQ(f.capacity(), 0);

  SparseVec<u64> set;
  // ASSERT_TRUE(set.reserve_new_ids(heap_allocator, 100));
  // ASSERT_EQ(set.num_slots, 100);
  // ASSERT_EQ(set.num_free, 100);
  //  uid64 id;
  //  u64   index;
  //  for (u32 i = 0; i < 100; i++)
  //  {
  //    ASSERT_TRUE(set.allocate_id(id, index));
  //    ASSERT_LT(id, set.num_slots);
  //    ASSERT_GE(id, 0);
  //  }
  //  ASSERT_FALSE(set.allocate_id(id, index));
  //  ASSERT_TRUE((set.try_release(id, [&](u64 a, u64 b) {
  //    ASSERT_EQ(a, set.num_valid() - 1);
  //    ASSERT_LT(a, set.num_valid());
  //    ASSERT_LT(b, set.num_valid());
  //  })));
  //
  //  indirect_sort(set.index_to_id, Span{set.id_to_index, 100});
  //  ASSERT_TRUE(std::is_sorted(set.index_to_id, set.index_to_id + 100));
  //  stable_indirect_sort(set.index_to_id, Span{set.id_to_index, 100});
  // ASSERT_TRUE(std::is_sorted(set.index_to_id, set.index_to_id + 100));
}
