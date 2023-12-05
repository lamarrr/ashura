

#include "ashura/bitvec.h"
#include "stx/vec.h"

#include "gtest/gtest.h"

TEST(BitVecTest, Init)
{
  stx::BitVec vec;

  EXPECT_EQ(vec.begin(), vec.end());
  vec.resize(11, ~0ULL).unwrap();
  EXPECT_EQ(vec.num_bits, 11);
  EXPECT_GT(vec.num_bits, vec.vec.size());
  EXPECT_NE(vec.begin(), vec.end());
  EXPECT_EQ(*vec.begin(), 1);
  stx::Bit bit = *(vec.end() - 1);
  EXPECT_EQ(bit, 1);
  vec.push(0).unwrap();
  bit = *(vec.end() - 1);
  EXPECT_EQ(bit, 0);

  for (auto i = 0; i < 12; i++)
  {
    std::cout << vec.get(i).unwrap() << std::endl;
  }
}
