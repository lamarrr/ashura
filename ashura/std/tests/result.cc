/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/result.h"
#include <algorithm>
#include <numeric>
#include <type_traits>
#include <utility>

using ash::Err;
using ash::Ok;
using ash::Result;

// static_assert(std::is_trivially_move_constructible_v<Result<int, int>>);
// static_assert(std::is_trivially_move_assignable_v<Result<int, int>>);
// static_assert(!std::is_trivially_move_constructible_v<Result<int,
// NonTrivial>>); static_assert(!std::is_trivially_move_assignable_v<Result<int,
// NonTrivial>>);

// static_assert(!std::is_trivially_move_constructible_v<Result<NonTrivial,
// int>>); static_assert(!std::is_trivially_move_assignable_v<Result<NonTrivial,
// int>>);

// static_assert(
//     !std::is_trivially_move_constructible_v<Result<NonTrivial, NonTrivial>>);
// static_assert(
//     !std::is_trivially_move_assignable_v<Result<NonTrivial, NonTrivial>>);

template <typename T, typename E>
constexpr auto make_ok(T &&value) -> Result<T, E>
{
  return Ok<T>(std::forward<T>(value));
}

template <typename T, typename E>
constexpr auto make_err(E &&err) -> Result<T, E>
{
  return Err<E>(std::forward<E>(err));
}

TEST(ResultTest, Equality)
{
  //
  EXPECT_EQ((make_ok<int, int>(78)), Ok{78});
  EXPECT_NE((make_ok<int, int>(7)), Ok(78));
  EXPECT_NE((make_ok<int, int>(78)), Err(78));

  EXPECT_NE((make_err<int, int>(78)), Ok(78));
  EXPECT_NE((make_err<int, int>(7)), Ok(78));
  EXPECT_NE((make_err<int, int>(78)), Err(-78));
  EXPECT_EQ((make_err<int, int>(78)), Err(78));

  EXPECT_EQ((make_ok<std::vector<int>, int>(std::vector{1, 2, 3, 4, 5})),
            Ok(std::vector{1, 2, 3, 4, 5}));
  EXPECT_EQ((make_ok<std::vector<int>, int>(std::vector{1, 2, 3, 4, 5})),
            (make_ok<std::vector<int>, int>(std::vector{1, 2, 3, 4, 5})));

  EXPECT_NE(
      (make_ok<std::vector<int>, std::vector<int>>(std::vector{1, 2, 3, 4, 5})),
      Err(std::vector{1, 2, 3, 4, 5}));
  EXPECT_NE(
      (make_ok<std::vector<int>, std::vector<int>>(std::vector{1, 2, 3, 4, 5})),
      (make_err<std::vector<int>, std::vector<int>>(
          std::vector{1, 2, 3, 4, 5})));
}

TEST(ResultTest, IsOk)
{
  EXPECT_TRUE((make_ok<int, int>(0).is_ok()));
  EXPECT_FALSE((make_err<int, int>(9).is_ok()));

  EXPECT_TRUE(
      (make_ok<std::vector<int>, int>(std::vector{1, 2, 3, 4}).is_ok()));
  EXPECT_FALSE((make_err<std::vector<int>, int>(89).is_ok()));

  EXPECT_TRUE((make_ok<int, std::vector<int>>(-78).is_ok()));
  EXPECT_FALSE(
      (make_err<int, std::vector<int>>(std::vector{1, 2, 3, 4}).is_ok()));

  EXPECT_TRUE(
      (make_ok<std::vector<int>, std::vector<int>>(std::vector{1, 2, 3, 4})
           .is_ok()));
  EXPECT_FALSE(
      (make_err<std::vector<int>, std::vector<int>>(std::vector{1, 2, 3, 4})
           .is_ok()));
}

TEST(ResultTest, IsErr)
{
  EXPECT_TRUE((make_err<int, int>(9).is_err()));
  EXPECT_FALSE((make_ok<int, int>(0).is_err()));

  EXPECT_TRUE((make_err<std::vector<int>, int>(89).is_err()));
  EXPECT_FALSE(
      (make_ok<std::vector<int>, int>(std::vector{1, 2, 3, 4}).is_err()));

  EXPECT_TRUE(
      (make_err<int, std::vector<int>>(std::vector{1, 2, 3, 4}).is_err()));
  EXPECT_FALSE((make_ok<int, std::vector<int>>(99).is_err()));

  EXPECT_TRUE(
      (make_err<std::vector<int>, std::vector<int>>(std::vector{5, 6, 7, 8})
           .is_err()));
  EXPECT_FALSE(
      (make_ok<std::vector<int>, std::vector<int>>(std::vector{1, 2, 3, 4})
           .is_err()));
}

TEST(ResultTest, Map)
{
  // will it be problematic if the map function is not in scope? No!
  auto a = [](int &value) { return value + 20; };
  EXPECT_EQ((make_ok<int, int>(20).map(a).unwrap()), 40);
  EXPECT_TRUE((make_err<int, int>(-1).map(a).is_err()));

  // will it be problematic if the map function is not in scope? No!
  auto b = [](std::vector<int> &value) {
    value.push_back(6);
    return std::move(value);
  };
  EXPECT_EQ((make_ok<std::vector<int>, int>({1, 2, 3, 4, 5}).map(b).unwrap()),
            (std::vector{1, 2, 3, 4, 5, 6}));
  EXPECT_TRUE((make_err<std::vector<int>, int>(-1).map(b).is_err()));
}

TEST(ResultTest, MapOr)
{
  auto a = [](int &value) { return value + 20; };
  EXPECT_EQ((make_ok<int, int>(20).map_or(a, 100)), 40);
  EXPECT_EQ((make_err<int, int>(-20).map_or(a, 100)), 100);

  auto b = [](std::vector<int> &value) {
    value.push_back(6);
    return std::move(value);
  };
  EXPECT_EQ((make_ok<std::vector<int>, int>({1, 2, 3, 4, 5})
                 .map_or(b, std::vector<int>{})),
            (std::vector<int>{1, 2, 3, 4, 5, 6}));
  EXPECT_EQ((make_err<std::vector<int>, int>(-20).map_or(
                b, std::vector<int>{6, 7, 8, 9, 10})),
            (std::vector<int>{6, 7, 8, 9, 10}));
}

TEST(ResultTest, MapOrElse)
{
  auto a      = [](int &value) { return value + 20; };
  auto else_a = [](auto) { return -10; };

  EXPECT_EQ((make_ok<int, int>(20).map_or_else(a, else_a)), 40);
  EXPECT_EQ((make_err<int, int>(-20).map_or_else(a, else_a)), -10);

  auto b = [](std::vector<int> &value) {
    value.push_back(6);
    return std::move(value);
  };
  auto else_b = [](auto) -> std::vector<int> {
    return {6, 7, 8, 9, 10};
  };        // NOLINT

  EXPECT_EQ(
      (make_ok<std::vector<int>, int>({1, 2, 3, 4, 5}).map_or_else(b, else_b)),
      (std::vector<int>{1, 2, 3, 4, 5, 6}));
  EXPECT_EQ((make_err<std::vector<int>, int>(-20).map_or_else(b, else_b)),
            (std::vector<int>{6, 7, 8, 9, 10}));
}

TEST(ResultTest, AndThen)
{
  auto a = [](int v) -> Result<float, int> { return Ok{v * 2.0F}; };
  EXPECT_FLOAT_EQ((make_ok<int, int>(20).and_then(a).unwrap()), 40.0F);

  EXPECT_TRUE((make_err<int, int>(-20).and_then(a).is_err()));

  EXPECT_EQ((make_err<int, int>(-20).and_then(a).unwrap_err()), -20);

  auto b = [](int &v) -> Result<std::vector<float>, int> {
    std::vector<float> res;
    res.push_back(static_cast<float>(v));
    return Ok{res};
  };

  EXPECT_EQ((make_ok<int, int>(80).and_then(b).unwrap()), (std::vector{80.0F}));

  EXPECT_TRUE((make_err<int, int>(-20).and_then(b).is_err()));
  EXPECT_EQ((make_err<int, int>(-20).and_then(b).unwrap_err()), -20);
}

TEST(ResultTest, OrElse)
{
  auto a = [](int &err) -> Result<int, int> { return Ok(err * 100); };
  EXPECT_EQ((make_ok<int, int>(20).or_else(a).unwrap()), 20);
  EXPECT_EQ((make_err<int, int>(10).or_else(a).unwrap()), 1000);

  auto b = [](std::string &err) -> Result<int, std::string> {
    return Err("Err: " + err);
  };
  EXPECT_EQ((make_ok<int, std::string>(20).or_else(b).unwrap()), 20);
  EXPECT_EQ((make_err<int, std::string>(std::string{"Max Limit"})
                 .or_else(b)
                 .unwrap_err()),
            std::string{"Err: Max Limit"});

  auto c = [](std::vector<int> &err) -> Result<int, std::vector<int>> {
    return Ok(err.empty() ? -1 : err[0]);
  };
  EXPECT_EQ((make_ok<int, std::vector<int>>(40).or_else(c).unwrap()), 40);
  EXPECT_EQ((make_err<int, std::vector<int>>(std::vector{10, 20, 30})
                 .or_else(c)
                 .unwrap()),
            10);
}

TEST(ResultTest, UnwrapOr)
{
  EXPECT_EQ((make_ok<int, int>(89).unwrap_or(90)), 89);
  EXPECT_EQ((make_err<int, int>(89).unwrap_or(90)), 90);

  EXPECT_EQ(
      (make_ok<std::string, int>(std::string{"John Doe"}).unwrap_or("Unknown")),
      "John Doe");
  EXPECT_EQ((make_err<std::string, int>(-20).unwrap_or(std::string{"Unknown"})),
            "Unknown");
}

TEST(ResultTest, Unwrap)
{
  EXPECT_EQ((make_ok<int, int>(89).unwrap()), 89);
  EXPECT_TRUE((make_err<int, int>(89).is_err()));

  EXPECT_EQ((make_ok<std::string, int>("John Doe").unwrap()), "John Doe");
  EXPECT_TRUE((make_err<std::string, int>(-20)).is_err());

  EXPECT_EQ(
      (make_ok<std::vector<int>, int>(std::vector{1, 2, 3, 4, 5}).unwrap()),
      (std::vector{1, 2, 3, 4, 5}));
  EXPECT_TRUE((make_err<std::vector<int>, int>(-1)).is_err());
}

TEST(ResultTest, UnwrapOrElse)
{
  auto a = [](int &err) { return err + 20; };
  EXPECT_EQ((make_ok<int, int>(10).unwrap_or_else(a)), 10);
  EXPECT_EQ((make_err<int, int>(20).unwrap_or_else(a)), 40);

  auto b = [](std::string &err) -> int { return stoi(err) + 20; };
  EXPECT_EQ((make_ok<int, std::string>(10).unwrap_or_else(b)), 10);
  EXPECT_EQ((make_err<int, std::string>("40").unwrap_or_else(b)), 60);

  auto c = [](std::vector<int> &vec) {
    vec.push_back(10);
    return std::move(vec);
  };
  EXPECT_EQ(
      (make_ok<std::vector<int>, std::vector<int>>(std::vector{1, 2, 3, 4, 5})
           .unwrap_or_else(c)),
      (std::vector{1, 2, 3, 4, 5}));
  EXPECT_EQ(
      (make_err<std::vector<int>, std::vector<int>>(std::vector{6, 7, 8, 9})
           .unwrap_or_else(c)),
      (std::vector{6, 7, 8, 9, 10}));
}

TEST(ResultTest, UnwrapErr)
{
  EXPECT_EQ((make_err<int, int>(20).unwrap_err()), 20);
  EXPECT_DEATH_IF_SUPPORTED(((void) make_ok<int, int>(10).unwrap_err()), ".*");

  EXPECT_EQ((make_err<std::vector<int>, int>(-40).unwrap_err()), -40);
  EXPECT_DEATH_IF_SUPPORTED(
      ((void) make_ok<std::vector<int>, int>(std::vector{10, 20, 30})
           .unwrap_err()),
      ".*");

  EXPECT_EQ(
      (make_err<int, std::vector<int>>(std::vector{1, 2, 3, 4}).unwrap_err()),
      (std::vector{1, 2, 3, 4}));
  EXPECT_DEATH_IF_SUPPORTED(
      ((void) make_ok<int, std::vector<int>>(68).unwrap_err()), ".*");

  EXPECT_EQ(
      (make_err<std::vector<int>, std::vector<int>>(std::vector{1, 2, 3, 4})
           .unwrap_err()),
      (std::vector{1, 2, 3, 4}));
  EXPECT_DEATH_IF_SUPPORTED(((void) make_ok<std::vector<int>, std::vector<int>>(
                                 std::vector{1, 2, 3, 4})
                                 .unwrap_err()),
                            ".*");
}

TEST(ResultTest, Match)
{
  auto a = make_ok<int, int>(98).match([](auto ok) { return ok + 2; },
                                       [](auto err) { return err + 5; });

  EXPECT_EQ(a, 100);

  auto b =
      make_ok<std::vector<int>, int>(std::vector{1, 2, 3, 4, 5})
          .match([](auto ok) { return accumulate(ok.begin(), ok.end(), 0); },
                 [](auto) { return -1; });

  EXPECT_EQ(b, 15);

  auto c = make_err<std::vector<int>, int>(67).match(
      [](auto ok) { return accumulate(ok.begin(), ok.end(), 0); },
      [](auto) { return -1; });

  EXPECT_EQ(c, -1);
}
