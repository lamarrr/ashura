
#include "stx/str.h"

#include "gtest/gtest.h"

using namespace stx;

TEST(StrTest, Init) {
  Str str;

  stx::str::make_static("hello");
  //
}
