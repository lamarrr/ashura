/// SPDX-License-Identifier: MIT
#include "ashura/std/uri.h"
#include "gtest/gtest.h"

using namespace ash;

TEST(UriTest, BasicParse)
{
  auto uri = "file:///C:/path/to/file.extension?query=example#fragment"_str;

  auto res = UriView::parse(uri).unwrap();

  EXPECT_TRUE(str_eq(res.scheme, "file"_str));
  EXPECT_TRUE(str_eq(res.hier_part, "///C:/path/to/file.extension"_str));
  EXPECT_TRUE(res.queries.is_some());
  EXPECT_TRUE(str_eq(res.queries.unwrap(), "query=example"_str));
  EXPECT_TRUE(res.fragments.is_some());
  EXPECT_TRUE(str_eq(res.fragments.unwrap(), "fragment"_str));
}
