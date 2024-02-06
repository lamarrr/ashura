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

  Vec<u64>    bvsrc{heap_allocator};
  BitVec<u64> bv{&bvsrc, 0};

  EXPECT_TRUE(bv.push(false));
  EXPECT_TRUE(bv.push(true));
  EXPECT_FALSE(bv[0]);
  EXPECT_TRUE(bv[1]);
  EXPECT_EQ(bv.size(), 2);

  SparseVec<u64> set{.index_to_id = {heap_allocator},
                     .id_to_index = {heap_allocator}};

  ASSERT_TRUE(set.push([&](u64 id, u64 index) {
    ASSERT_EQ(id, 0);
    EXPECT_EQ(index, 0);
  }));
  ASSERT_EQ(set.size(), 1);
  ASSERT_TRUE(set.push([&](u64 id, u64 index) {
    ASSERT_EQ(id, 1);
    ASSERT_EQ(index, 1);
  }));
  ASSERT_EQ(set.size(), 2);
  ASSERT_TRUE(set.try_erase(0));
  ASSERT_EQ(set.size(), 1);
  ASSERT_TRUE(set.try_erase(1));
  ASSERT_EQ(set.size(), 0);
}
