/// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <cstdio>

#include "ashura/engine/views/text_box.h"

TEST(TextCompositor, Main)
{
  using namespace ash;

  TextCompositor cmp;
  defer          cmp_uninit{[&] { cmp.reset(); }};
  TextLayout     layout;
  ClipBoard      clip;

  cmp.init(1024, 256);

  Span text = U"HELLO, MOTO"_utf;

  bool inserted = false;

  auto insert = [&](u32 i, Span<u32 const> str) {
    inserted = true;
    ASSERT_EQ(i, 0);
    ASSERT_TRUE(range_equal(str, text));
  };

  cmp.command(U""_utf, layout, 0, 0, TextCommand::InputText, fn(&insert),
              fn([](Slice32) {}), text, clip, 1, {});

  ASSERT_TRUE(inserted);
  ASSERT_EQ(cmp.inner.current_record, 1);
  ASSERT_EQ(cmp.inner.latest_record, 1);
  ASSERT_EQ(cmp.inner.buffer_pos, text.size32());

  cmp.command(text, layout, 0, 0, TextCommand::SelectLine, fn(&insert),
              fn([](Slice32) {}), {}, clip, 1, {});

  ASSERT_EQ(cmp.inner.current_record, 1);
  ASSERT_EQ(cmp.inner.latest_record, 1);
  ASSERT_EQ(cmp.inner.buffer_pos, text.size32());
  ASSERT_EQ(cmp.get_cursor().first, 0);
  ASSERT_EQ(cmp.get_cursor().last, text.size32() - 1);
  ASSERT_EQ(cmp.get_cursor().as_slice(text.size32()).offset, 0);
  ASSERT_EQ(cmp.get_cursor().as_slice(text.size32()).span, text.size32());
}
