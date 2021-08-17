
#include "stx/str.h"

#include "gtest/gtest.h"

using namespace stx;

TEST(StrTest, Init) {
  Str str;

  Str("HELLO", 5, static_storage_allocator);
  //
}
