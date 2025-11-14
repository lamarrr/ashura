/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/edit_str.h"

using namespace ash;

std::u8string_view strv(Span<c8 const> s)
{
  return std::u8string_view{s.data(), s.size()};
}

TEST(EditStrTest, Insert)
{
  EditStr8 str{default_allocator, default_allocator, default_allocator, 8};

  str.insert(0, u8"Hi"_str).unwrap();

  {
    Vec<c8> result{default_allocator};
    str.get_table().compact(Slice::all(), result).unwrap();
    EXPECT_EQ(strv(result.view()), std::u8string_view{u8"Hi"});
  }

  str.insert(0, u8"Konichiwa, minasan! : "_str).unwrap();
  {
    Vec<c8> result{default_allocator};
    str.get_table().compact(Slice::all(), result).unwrap();
    EXPECT_EQ(strv(result.view()),
              std::u8string_view{u8"Konichiwa, minasan! : Hi"});
  }

  str.insert(11, u8"taijobu desu ka? "_str).unwrap();
  {
    Vec<c8> result{default_allocator};
    str.get_table().compact(Slice::all(), result).unwrap();
    EXPECT_EQ(
      strv(result.view()),
      std::u8string_view{u8"Konichiwa, taijobu desu ka? minasan! : Hi"});
  }

}

