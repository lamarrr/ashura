/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/constrained.hpp"

using namespace ash;

TEST(Constrained, NonZero)
{
  ASSERT_DEATH({ NonZero<i32> nz{0}; }, ".*");
  ASSERT_NO_FATAL_FAILURE({
    NonZero<i32> nz{5};
    EXPECT_EQ(static_cast<i32>(nz), 5);
  });
}

TEST(Constrained, NonNull)
{
  ASSERT_DEATH({ NonNull<void *> nn{nullptr}; }, ".*");
  ASSERT_NO_FATAL_FAILURE({
    int            value = 10;
    NonNull<int *> nn{&value};
    EXPECT_EQ(*static_cast<int *>(nn), 10);
  });
}

TEST(Constrained, NonZeroPow2)
{
  ASSERT_DEATH({ NonZeroPow2<i32> nz{0}; }, ".*");
  ASSERT_DEATH({ NonZeroPow2<i32> nz{5}; }, ".*");
  ASSERT_NO_FATAL_FAILURE({
    NonZeroPow2<i32> nz{8};
    EXPECT_EQ(static_cast<i32>(nz), 8);
  });
}
