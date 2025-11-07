/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/edit_str.h"

using namespace ash;

TEST(EditStrTest, Extend)
{
  EditStr8 edit_str{default_allocator, default_allocator, default_allocator,
                    1'024};

  edit_str.insert(0, u8"Hi"_str).unwrap();
}
