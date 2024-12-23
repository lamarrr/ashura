/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/engine/views.h"

TEST(TextCompositor, Main)
{
  using namespace ash;

  TextCompositor cmp;
  TextLayout     layout;
  ClipBoard      clip;

  Span text = U"HELLO, MOTO"_str;

  bool inserted = false;

  auto insert = [&](u32 i, Span<c32 const> str) {
    inserted = true;
    ASSERT_EQ(i, 0);
    ASSERT_TRUE(range_eq(str, text));
  };

  cmp.command(U""_str, layout, 0, 0, TextCommand::InputText, fn(insert), noop,
              text, clip, 1, {});

  ASSERT_TRUE(inserted);
  ASSERT_EQ(cmp.current_record_, 1);
  ASSERT_EQ(cmp.latest_record_, 1);
  ASSERT_EQ(cmp.buffer_pos_, text.size32());

  cmp.command(text, layout, 0, 0, TextCommand::SelectLine, fn(insert), noop, {},
              clip, 1, {});

  ASSERT_EQ(cmp.current_record_, 1);
  ASSERT_EQ(cmp.latest_record_, 1);
  ASSERT_EQ(cmp.buffer_pos_, text.size32());
  ASSERT_EQ(cmp.get_cursor().first, 0);
  ASSERT_EQ(cmp.get_cursor().last, text.size32() - 1);
  ASSERT_EQ(cmp.get_cursor().as_slice(text.size32()).offset, 0);
  ASSERT_EQ(cmp.get_cursor().as_slice(text.size32()).span, text.size32());
}
