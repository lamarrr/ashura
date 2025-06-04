/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/engine/render_text.h"

TEST(RenderText, RunManagement)
{
  using namespace ash;

  RenderText text{default_allocator};

  ASSERT_EQ(text.runs_.size(), 0);

  text.run({}, {}, 0, 1);
  ASSERT_EQ(text.runs_.size(), 1);

  text.run({}, {}, 0, 1);
  ASSERT_EQ(text.runs_.size(), 2);

  text.run({}, {}, 0, 1);
  ASSERT_EQ(text.runs_.size(), 2);

  text.run({}, {}, 0, 8);
  ASSERT_EQ(text.runs_.size(), 2);

  text.run({}, {}, 0, 2);
  ASSERT_EQ(text.runs_.size(), 3);

  text.run({}, {}, 0, 8);
  ASSERT_EQ(text.runs_.size(), 2);

  text.run({}, {}, 1, 7);
  ASSERT_EQ(text.runs_.size(), 3);

  text.run({}, {}, 1, 4);
  ASSERT_EQ(text.runs_.size(), 4);

  text.run({}, {}, 0, USIZE_MAX);
  ASSERT_EQ(text.runs_.size(), 1);
}
