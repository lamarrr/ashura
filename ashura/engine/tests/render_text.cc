/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/engine/render_text.h"

TEST(RenderText, RunManagement)
{
  using namespace ash;

  RenderText text;
  defer      text_uninit{[&] { text.reset(); }};

  ASSERT_EQ(text.inner.runs.size32(), 0);

  text.style(0, 1, {}, {});
  ASSERT_EQ(text.inner.runs.size32(), 1);

  text.style(0, 1, {}, {});
  ASSERT_EQ(text.inner.runs.size32(), 2);

  text.style(0, 1, {}, {});
  ASSERT_EQ(text.inner.runs.size32(), 2);

  text.style(0, 8, {}, {});
  ASSERT_EQ(text.inner.runs.size32(), 2);

  text.style(0, 2, {}, {});
  ASSERT_EQ(text.inner.runs.size32(), 3);

  text.style(0, 8, {}, {});
  ASSERT_EQ(text.inner.runs.size32(), 2);

  text.style(1, 7, {}, {});
  ASSERT_EQ(text.inner.runs.size32(), 3);

  text.style(1, 4, {}, {});
  ASSERT_EQ(text.inner.runs.size32(), 4);

  text.style(0, U32_MAX, {}, {});
  ASSERT_EQ(text.inner.runs.size32(), 1);
}
