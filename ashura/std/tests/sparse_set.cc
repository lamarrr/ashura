/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/sparse_vec.h"
#include <bitset>
#include <iostream>

using namespace ash;

TEST(SparseSetTest, Start)
{
  Vec<int> f{default_allocator};

  ASSERT_TRUE(f.push(1));
  ASSERT_EQ(f[0], 1);
  ASSERT_EQ(f.size(), 1);
  ASSERT_TRUE(f.extend_copy(span({2, 3, 4, 5, 6})));
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
  ASSERT_TRUE(f.insert_span_copy(f.size(), span({8, 9, 0})));
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

  BitVec<u64> bv{Vec<u64>{default_allocator}, 0};

  EXPECT_TRUE(bv.push(false));
  EXPECT_TRUE(bv.push(true));
  EXPECT_FALSE(bv[0]);
  EXPECT_TRUE(bv[1]);
  EXPECT_EQ(bv.size(), 2);
  bv.erase(0, 1);
  EXPECT_EQ(bv.size(), 1);
  EXPECT_TRUE(bv[0]);

  SparseVec<u64, u64> set;

  ASSERT_TRUE(set.push(69U, 67U));
  ASSERT_EQ(set.size(), 1);
  ASSERT_EQ(set.size(), set.dense.v0.size());
  ASSERT_TRUE(set.push(42U, 32U));
  ASSERT_EQ(set.size(), 2);
  ASSERT_EQ(set.size(), set.dense.v0.size());
  ASSERT_EQ(set.dense.v0[0], 69U);
  ASSERT_EQ(set.dense.v0[1], 42U);
  ASSERT_TRUE(set.try_erase(0));
  ASSERT_EQ(set.size(), 1);
  ASSERT_EQ(set.size(), set.dense.v0.size());
  ASSERT_TRUE(set.try_erase(1));
  ASSERT_EQ(set.size(), 0);
  ASSERT_EQ(set.size(), set.dense.v0.size());

  f.reset();
  bv.reset();
  set.reset();
}
