/// SPDX-License-Identifier: MIT
#include "ashura/std/uri.hpp"
#include "gtest/gtest.h"

using namespace ash;

TEST(UriTest, BasicParse)
{
  auto uri = "file:///C:/path/to/file.extension?query=example#fragment"_s;

  auto res = UriView::parse(uri).unwrap();

  EXPECT_TRUE(span_bit_eq(res.scheme, "file"_s));
  EXPECT_TRUE(span_bit_eq(res.hier_part, "///C:/path/to/file.extension"_s));
  EXPECT_TRUE(res.queries.is_some());
  EXPECT_TRUE(span_bit_eq(res.queries.unwrap(), "query=example"_s));
  EXPECT_TRUE(res.fragments.is_some());
  EXPECT_TRUE(span_bit_eq(res.fragments.unwrap(), "fragment"_s));
}
