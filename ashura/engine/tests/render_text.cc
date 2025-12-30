/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/engine/render_text.h"

using namespace ash;

TEST(RenderText, RunManagement)
{
  TextRunsStyle text{default_allocator};
  ASSERT_EQ(text.run_indices_.size(), 2);
  ASSERT_EQ(text.fonts_.size(), 1);
  ASSERT_EQ(text.styles_.size(), 1);

  text.update(TextStyle{}, FontStyle{}, 0, USIZE_MAX);

  ASSERT_EQ(text.run_indices_.size(), 2);
  ASSERT_EQ(text.fonts_.size(), 1);
  ASSERT_EQ(text.styles_.size(), 1);

  text.update(TextStyle{}, FontStyle{}, 0, 5);

  ASSERT_EQ(text.run_indices_.size(), 3);
  ASSERT_EQ(text.fonts_.size(), 2);
  ASSERT_EQ(text.styles_.size(), 2);

  text.update(TextStyle{}, FontStyle{}, 1, 2);
  ASSERT_EQ(text.run_indices_.size(), 5);
  ASSERT_EQ(text.fonts_.size(), 4);
  ASSERT_EQ(text.styles_.size(), 4);
}
