/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/vec.h"
#include "ashura/std/range.h"
#include <bitset>
#include <iostream>

using namespace ash;

template <typename T>
struct VecTest : testing::Test
{
};

using Types = testing::Types<Vec<i32>, SmallVec<i32, 10>, InplaceVec<i32, 256>>;

TYPED_TEST_SUITE(VecTest, Types);

TYPED_TEST(VecTest, Push)
{
  TypeParam a;

  for (auto i : range<i32>(256))
  {
    a.push(i).unwrap();
  }

  EXPECT_EQ(a.size(), 256);

  for (auto [i, e] : enumerate<i32>(a))
  {
    EXPECT_EQ(i, e);
  }
}

TEST(MemVecTest, Leak)
{
  Vec<f32> a;
  a.resize(200).unwrap();

  auto prev = a.view();

  auto leaked = a.leak();

  EXPECT_EQ(prev.data(), leaked.data());
  EXPECT_EQ(prev.size(), leaked.size());
  EXPECT_EQ(a.data(), nullptr);
  EXPECT_EQ(a.size(), 0);
}
