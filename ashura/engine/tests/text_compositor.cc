/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/engine/views.h"

using namespace ash;

struct TextCompositorTest : testing::Test
{
  TextCompositorTest()
  {
  }

  TextCompositor cmp{default_allocator};
  TextLayout     layout{default_allocator};
  ClipBoard      clip;
};

TEST_F(TextCompositorTest, Main)
{
  Span text = U"HELLO, MOTO"_str;

  bool inserted = false;

  auto insert = [&](usize i, Span<c32 const> str) {
    inserted = true;
    ASSERT_EQ(i, 0);
    ASSERT_TRUE(range_eq(str, text));
  };

  cmp.command(TextBlock{.text = U""_str}, layout, TextBlockStyle{},
              TextCommand::InputText, fn(insert), noop, text, clip, 1, {});

  ASSERT_TRUE(inserted);
  ASSERT_EQ(cmp.current_record_, 1);
  ASSERT_EQ(cmp.latest_record_, 1);
  ASSERT_EQ(cmp.buffer_pos_, text.size32());

  cmp.command(TextBlock{.text = text}, layout, TextBlockStyle{},
              TextCommand::SelectLine, fn(insert), noop, {}, clip, 1, {});

  ASSERT_EQ(cmp.current_record_, 1);
  ASSERT_EQ(cmp.latest_record_, 1);
  ASSERT_EQ(cmp.buffer_pos_, text.size32());
  ASSERT_EQ(cmp.get_cursor().first, 0);
  ASSERT_EQ(cmp.get_cursor().last(), text.size32() - 1);
  ASSERT_EQ(cmp.get_cursor().as_slice()(text.size32()).offset, 0);
  ASSERT_EQ(cmp.get_cursor().as_slice()(text.size32()).span, text.size32());

  cmp.command(TextBlock{.text = text}, layout, TextBlockStyle{},
              TextCommand::Left, fn(insert), noop, {}, clip, 1, {});

  ASSERT_EQ(cmp.get_cursor().first, 0);
  ASSERT_EQ(cmp.get_cursor().last(), 0);
  cmp.command(TextBlock{.text = text}, layout, TextBlockStyle{},
              TextCommand::SelectCodepoint, fn(insert), noop, {}, clip, 1, {});

  ASSERT_EQ(cmp.get_cursor().first, 0);
  ASSERT_EQ(cmp.get_cursor().last(), 1);

  cmp.command(TextBlock{.text = text}, layout, TextBlockStyle{},
              TextCommand::Right, fn(insert), noop, {}, clip, 1, {});

  ASSERT_EQ(cmp.get_cursor().first, 1);
  ASSERT_EQ(cmp.get_cursor().last(), 0);

  cmp.command(TextBlock{.text = text}, layout, TextBlockStyle{},
              TextCommand::SelectCodepoint, fn(insert), noop, {}, clip, 1, {});

  ASSERT_EQ(cmp.get_cursor().first, 1);
  ASSERT_EQ(cmp.get_cursor().last(), 1);
}
