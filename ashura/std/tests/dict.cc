#include "ashura/std/dict.h"
#include "ashura/std/range.h"
#include "ashura/std/types.h"
#include "gtest/gtest.h"

using namespace ash;

TEST(DictTest, Misc)
{
  StrDict<int> res;
  ASSERT_TRUE(res.init());
  ASSERT_TRUE(res.insert(nullptr, "A"_span, 0));
  ASSERT_NE(res["A"_span], nullptr);
  ASSERT_TRUE(res.insert(nullptr, "B"_span, 1));
  ASSERT_NE(res["A"_span], nullptr);
  ASSERT_NE(res["B"_span], nullptr);
  ASSERT_TRUE(res.insert(nullptr, "C"_span, 2));
  ASSERT_NE(res["A"_span], nullptr);
  ASSERT_NE(res["B"_span], nullptr);
  ASSERT_NE(res["C"_span], nullptr);
  ASSERT_TRUE(res.insert(nullptr, "D"_span, 3));
  ASSERT_NE(res["A"_span], nullptr);
  ASSERT_NE(res["B"_span], nullptr);
  ASSERT_NE(res["C"_span], nullptr);
  ASSERT_NE(res["D"_span], nullptr);
  ASSERT_TRUE(res.insert(nullptr, "E"_span, 4));
  ASSERT_NE(res["A"_span], nullptr);
  ASSERT_NE(res["B"_span], nullptr);
  ASSERT_NE(res["C"_span], nullptr);
  ASSERT_NE(res["D"_span], nullptr);
  ASSERT_NE(res["E"_span], nullptr);
  ASSERT_EQ(*res["A"_span], 0);
  ASSERT_EQ(*res["B"_span], 1);
  ASSERT_EQ(*res["C"_span], 2);
  ASSERT_EQ(*res["D"_span], 3);
  ASSERT_EQ(*res["E"_span], 4);

  for (int i = 2000; i < 8000; i++)
  {
    ASSERT_TRUE(res.insert(nullptr, Span{&i, 1}.as_char(), i));
    ASSERT_EQ(*(res[Span{&i, 1}.as_char()]), i);
  }

  for (int i = 2000; i < 8000; i++)
  {
    ASSERT_NE((res[Span{&i, 1}.as_char()]), nullptr) << i;
    ASSERT_EQ(*(res[Span{&i, 1}.as_char()]), i);
  }
  res.destroy();
}
