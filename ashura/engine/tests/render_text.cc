/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/engine/render_text.h"

TEST(RenderText, RunManagement)
{
  using namespace ash;

  RenderText text{default_allocator};

  ASSERT_EQ(text.runs_.size32(), 0);

  text.run({}, {}, 0, 1);
  ASSERT_EQ(text.runs_.size32(), 1);

  text.run({}, {}, 0, 1);
  ASSERT_EQ(text.runs_.size32(), 2);

  text.run({}, {}, 0, 1);
  ASSERT_EQ(text.runs_.size32(), 2);

  text.run({}, {}, 0, 8);
  ASSERT_EQ(text.runs_.size32(), 2);

  text.run({}, {}, 0, 2);
  ASSERT_EQ(text.runs_.size32(), 3);

  text.run({}, {}, 0, 8);
  ASSERT_EQ(text.runs_.size32(), 2);

  text.run({}, {}, 1, 7);
  ASSERT_EQ(text.runs_.size32(), 3);

  text.run({}, {}, 1, 4);
  ASSERT_EQ(text.runs_.size32(), 4);

  text.run({}, {}, 0, U32_MAX);
  ASSERT_EQ(text.runs_.size32(), 1);
}
