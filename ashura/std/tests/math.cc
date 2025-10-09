// SPDX-License-Identifier: MIT
#include "ashura/std/math.h"
#include "gtest/gtest.h"

using namespace ash;
using namespace ash::math;

TEST(VecTest, Basic)
{
  i32x3 v{1, 2, 3};
  EXPECT_EQ(v.x(), 1);
  EXPECT_EQ(v.y(), 2);
  EXPECT_EQ(v.z(), 3);
  EXPECT_EQ(v.size(), 3);
  EXPECT_FALSE(v.is_zero());
  auto  zero3 = i32x3::zero();
  i32x3 expected_zero3{0, 0, 0};
  EXPECT_EQ(zero3, expected_zero3);
  auto  one3 = i32x3::one();
  i32x3 expected_one3{1, 1, 1};
  EXPECT_EQ(one3, expected_one3);
  auto  xy = v.xy();
  i32x2 expected_xy{1, 2};
  EXPECT_EQ(xy, expected_xy);
}

TEST(VecTest, Ops)
{
  i32x3 a{1, 2, 3};
  i32x3 b{4, 5, 6};
  auto  add = a + b;
  i32x3 expected_add{5, 7, 9};
  EXPECT_EQ(add, expected_add);
  auto  sub = b - a;
  i32x3 expected_sub{3, 3, 3};
  EXPECT_EQ(sub, expected_sub);
  auto  mul = a * b;
  i32x3 expected_mul{4, 10, 18};
  EXPECT_EQ(mul, expected_mul);
  auto  div = b / a;
  i32x3 expected_div{4, 2, 2};
  EXPECT_EQ(div, expected_div);
  a += b;
  i32x3 expected_a1{5, 7, 9};
  EXPECT_EQ(a, expected_a1);
  a -= b;
  i32x3 expected_a2{1, 2, 3};
  EXPECT_EQ(a, expected_a2);
  a *= 2;
  i32x3 expected_a3{2, 4, 6};
  EXPECT_EQ(a, expected_a3);
  a /= 2;
  i32x3 expected_a4{1, 2, 3};
  EXPECT_EQ(a, expected_a4);
}

TEST(VecTest, Utilities)
{
  constexpr i32x4 v{1, 2, 3, 4};
  EXPECT_EQ(v.sum(), 10);
  EXPECT_EQ(v.product(), 24);
  EXPECT_EQ(v.min(), 1);
  EXPECT_EQ(v.max(), 4);
  i32x4 ones{1, 1, 1, 1};
  auto  dot = v.dot(ones);
  EXPECT_EQ(dot, 10);
  auto        appended = v.append(5);
  vec<int, 5> expected_appended{1, 2, 3, 4, 5};
  EXPECT_EQ(appended, expected_appended);
  auto        prepended = v.prepend(0);
  vec<int, 5> expected_prepended{0, 1, 2, 3, 4};
  EXPECT_EQ(prepended, expected_prepended);
}

TEST(VecTest, ClampAbs)
{
  i32x3 v{-5, 0, 5};
  auto  clamped = v.clamp(-2, 2);
  i32x3 expected_clamped{-2, 0, 2};
  EXPECT_EQ(clamped, expected_clamped);
  auto  abs_v = v.abs();
  i32x3 expected_abs{5, 0, 5};
  EXPECT_EQ(abs_v, expected_abs);
}

TEST(MatTest, IdentityDiagonalZero)
{
  auto id = mat<int, 3, 3>::identity();
  EXPECT_EQ(id[0], (i32x3{1, 0, 0}));
  EXPECT_EQ(id[1], (i32x3{0, 1, 0}));
  EXPECT_EQ(id[2], (i32x3{0, 0, 1}));

  auto diag = mat<int, 2, 2>::diagonal(7);
  EXPECT_EQ(diag[0], (i32x2{7, 0}));
  EXPECT_EQ(diag[1], (i32x2{0, 7}));

  auto zero = mat<int, 2, 2>::zero();
  EXPECT_EQ(zero[0], (i32x2{0, 0}));
  EXPECT_EQ(zero[1], (i32x2{0, 0}));
}

TEST(MatTest, BasicOps)
{
  mat<int, 2, 2> a{
    {{1, 2}, {3, 4}}
  };
  mat<int, 2, 2> b{
    {{5, 6}, {7, 8}}
  };
  mat<int, 2, 2> add{
    {a[0] + b[0], a[1] + b[1]}
  };
  EXPECT_EQ(add[0], (i32x2{6, 8}));
  EXPECT_EQ(add[1], (i32x2{10, 12}));

  mat<int, 2, 2> sub{
    {a[0] - b[0], a[1] - b[1]}
  };
  EXPECT_EQ(sub[0], (i32x2{-4, -4}));
  EXPECT_EQ(sub[1], (i32x2{-4, -4}));

  // Scalar multiplication
  mat<int, 2, 2> mul = a;
  mul[0]             = mul[0] * 2;
  mul[1]             = mul[1] * 2;
  EXPECT_EQ(mul[0], (i32x2{2, 4}));
  EXPECT_EQ(mul[1], (i32x2{6, 8}));
}
