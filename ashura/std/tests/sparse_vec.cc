/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/vec.h"
#include <bitset>
#include <iostream>

using namespace ash;

TEST(SparseVecTest, Start)
{
  static_assert(TriviallyRelocatable<Vec<int>>);
  static_assert(TriviallyRelocatable<Vec<int>>);
  static_assert(TriviallyRelocatable<Vec<Vec<int>>>);
  static_assert(TriviallyRelocatable<InplaceVec<Span<int>, 10>>);
  static_assert(!TriviallyRelocatable<InplaceVec<std::string, 10>>);
  static_assert(TriviallyRelocatable<InplaceVec<Vec<int>, 10>>);

  Vec<int> f{default_allocator};

  ASSERT_TRUE(f.push(1));
  ASSERT_EQ(f[0], 1);
  ASSERT_EQ(f.size(), 1);
  ASSERT_TRUE(f.extend(span({2, 3, 4, 5, 6})));
  ASSERT_EQ(f.size(), 6);
  ASSERT_EQ(f[5], 6);
  ASSERT_TRUE(f.fit());
  ASSERT_EQ(f.size(), 6);
  ASSERT_EQ(f.size(), f.capacity());
  ASSERT_EQ(f[5], 6);
  ASSERT_TRUE(f.insert(f.size(), 7).is_ok());
  ASSERT_EQ(f.size(), 7);
  ASSERT_EQ(f[0], 1);
  ASSERT_EQ(f[5], 6);
  ASSERT_EQ(f[6], 7);
  ASSERT_TRUE(f.insert_span(f.size(), span({8, 9, 0})));
  ASSERT_EQ(f.size(), 10);
  ASSERT_EQ(f[7], 8);
  ASSERT_EQ(f[9], 0);
  ASSERT_TRUE(f.extend(5));
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

  SparseVec<Vec<u64>, Vec<u64>, BitVec<u64>> set;

  for (auto [a, b, c] : set)
  {
    a += 2;
    b += 3;
    (void) c;
  }

  ASSERT_EQ(set.push(69U, 67U, true).unwrap(), 0);
  ASSERT_EQ(set.size(), 1);
  ASSERT_EQ(set.size(), set.dense.v0.size());
  ASSERT_EQ(set.push(42U, 32U, false).unwrap(), 1);
  ASSERT_EQ(set.size(), 2);
  ASSERT_EQ(set.size(), set.dense.v0.size());
  ASSERT_TRUE(set.is_valid_id(0));
  ASSERT_TRUE(set.is_valid_id(1));
  ASSERT_EQ(set[0].v0, 69U);
  ASSERT_EQ(set[1].v0, 42U);
  ASSERT_EQ(set[0].v2, true);
  ASSERT_EQ(set[1].v2, false);
  ASSERT_TRUE(set.try_erase(0));
  ASSERT_EQ(set.size(), 1);
  ASSERT_EQ(set.size(), set.dense.v0.size());
  ASSERT_TRUE(set.try_erase(1));
  ASSERT_EQ(set.size(), 0);
  ASSERT_EQ(set.size(), set.dense.v0.size());

  f.reset();
  set.reset();
}
