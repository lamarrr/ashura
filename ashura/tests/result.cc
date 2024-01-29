
#include <algorithm>
#include <numeric>
#include <type_traits>
#include <utility>

#include "ashura/std/result.h"
#include "gtest/gtest.h"

using namespace std;
using namespace string_literals;
using namespace ash;

ash::Logger panic_logger;

struct NonTrivial
{
  NonTrivial(NonTrivial &&)
  {
  }

  NonTrivial &operator=(NonTrivial &&)
  {
    return *this;
  }
};

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

  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5})),
            Ok(vector{1, 2, 3, 4, 5}));
  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5})),
            (make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5})));

  EXPECT_NE((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})),
            Err(vector{1, 2, 3, 4, 5}));
  EXPECT_NE((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})),
            (make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})));
}

TEST(ResultTest, IsOk)
{
  EXPECT_TRUE((make_ok<int, int>(0).is_ok()));
  EXPECT_FALSE((make_err<int, int>(9).is_ok()));

  EXPECT_TRUE((make_ok<vector<int>, int>(vector{1, 2, 3, 4}).is_ok()));
  EXPECT_FALSE((make_err<vector<int>, int>(89).is_ok()));

  EXPECT_TRUE((make_ok<int, vector<int>>(-78).is_ok()));
  EXPECT_FALSE((make_err<int, vector<int>>(vector{1, 2, 3, 4}).is_ok()));

  EXPECT_TRUE((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4}).is_ok()));
  EXPECT_FALSE(
      (make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4}).is_ok()));
}

TEST(ResultTest, IsErr)
{
  EXPECT_TRUE((make_err<int, int>(9).is_err()));
  EXPECT_FALSE((make_ok<int, int>(0).is_err()));

  EXPECT_TRUE((make_err<vector<int>, int>(89).is_err()));
  EXPECT_FALSE((make_ok<vector<int>, int>(vector{1, 2, 3, 4}).is_err()));

  EXPECT_TRUE((make_err<int, vector<int>>(vector{1, 2, 3, 4}).is_err()));
  EXPECT_FALSE((make_ok<int, vector<int>>(99).is_err()));

  EXPECT_TRUE(
      (make_err<vector<int>, vector<int>>(vector{5, 6, 7, 8}).is_err()));
  EXPECT_FALSE(
      (make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4}).is_err()));
}

/*
TEST(ResultTest, Ok) {
  EXPECT_EQ((make_ok<int, int>(20).ok().unwrap()), 20);
  EXPECT_EQ((make_err<int, int>(90).ok()), None);

  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4}).ok().unwrap()),
            (vector{1, 2, 3, 4}));
  EXPECT_EQ((make_err<vector<int>, int>(90).ok()), None);

  EXPECT_EQ((make_ok<int, vector<int>>(90).ok().unwrap()), 90);
  EXPECT_EQ((make_err<int, vector<int>>(vector{1, 2, 3, 4}).ok()), None);

  EXPECT_EQ(
      (make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4}).ok().unwrap()),
      (vector{1, 2, 3, 4}));
  EXPECT_EQ((make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4}).ok()),
            None);
}

TEST(ResultTest, Err) {
  EXPECT_EQ((make_err<int, int>(20).err().unwrap()), 20);
  EXPECT_EQ((make_ok<int, int>(90).err()), None);

  EXPECT_EQ((make_err<vector<int>, int>(10).err().unwrap()), 10);
  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4}).err()), None);

  EXPECT_EQ((make_err<int, vector<int>>(vector{1, 2, 3, 4}).err().unwrap()),
            (vector{1, 2, 3, 4}));
  EXPECT_EQ((make_ok<int, vector<int>>(78).err()), None);

  EXPECT_EQ(
      (make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4}).err().unwrap()),
      (vector{1, 2, 3, 4}));
  EXPECT_EQ((make_ok<vector<int>, vector<int>>(vector{78, 67}).err()), None);
}
*/

TEST(ResultTest, Map)
{
  // will it be problematic if the map function is not in scope? No!
  auto a = [](int &value) { return value + 20; };
  EXPECT_EQ((make_ok<int, int>(20).map(a).unwrap()), 40);
  EXPECT_TRUE((make_err<int, int>(-1).map(a).is_err()));

  // will it be problematic if the map function is not in scope? No!
  auto b = [](vector<int> &value) {
    value.push_back(6);
    return std::move(value);
  };
  EXPECT_EQ((make_ok<vector<int>, int>({1, 2, 3, 4, 5}).map(b).unwrap()),
            (vector{1, 2, 3, 4, 5, 6}));
  EXPECT_TRUE((make_err<vector<int>, int>(-1).map(b).is_err()));
}

TEST(ResultTest, MapOr)
{
  auto a = [](int &value) { return value + 20; };
  EXPECT_EQ((make_ok<int, int>(20).map_or(a, 100)), 40);
  EXPECT_EQ((make_err<int, int>(-20).map_or(a, 100)), 100);

  auto b = [](vector<int> &value) {
    value.push_back(6);
    return std::move(value);
  };
  EXPECT_EQ(
      (make_ok<vector<int>, int>({1, 2, 3, 4, 5}).map_or(b, vector<int>{})),
      (vector<int>{1, 2, 3, 4, 5, 6}));
  EXPECT_EQ(
      (make_err<vector<int>, int>(-20).map_or(b, vector<int>{6, 7, 8, 9, 10})),
      (vector<int>{6, 7, 8, 9, 10}));
}

TEST(ResultTest, MapOrElse)
{
  auto a      = [](int &value) { return value + 20; };
  auto else_a = [](auto) { return -10; };

  EXPECT_EQ((make_ok<int, int>(20).map_or_else(a, else_a)), 40);
  EXPECT_EQ((make_err<int, int>(-20).map_or_else(a, else_a)), -10);

  auto b = [](vector<int> &value) {
    value.push_back(6);
    return std::move(value);
  };
  auto else_b = [](auto) -> vector<int> {
    return {6, 7, 8, 9, 10};
  };        // NOLINT

  EXPECT_EQ((make_ok<vector<int>, int>({1, 2, 3, 4, 5}).map_or_else(b, else_b)),
            (vector<int>{1, 2, 3, 4, 5, 6}));
  EXPECT_EQ((make_err<vector<int>, int>(-20).map_or_else(b, else_b)),
            (vector<int>{6, 7, 8, 9, 10}));
}

TEST(ResultTest, AndThen)
{
  auto a = [](int v) -> Result<float, int> { return Ok{v * 2.0f}; };
  EXPECT_FLOAT_EQ((make_ok<int, int>(20).and_then(a).unwrap()), 40.0f);

  EXPECT_TRUE((make_err<int, int>(-20).and_then(a).is_err()));

  EXPECT_EQ((make_err<int, int>(-20).and_then(a).unwrap_err()), -20);

  auto b = [](int &v) -> Result<vector<float>, int> {
    vector<float> res;
    res.push_back(static_cast<float>(v));
    return Ok{res};
  };

  EXPECT_EQ((make_ok<int, int>(80).and_then(b).unwrap()), (vector{80.0f}));

  EXPECT_TRUE((make_err<int, int>(-20).and_then(b).is_err()));
  EXPECT_EQ((make_err<int, int>(-20).and_then(b).unwrap_err()), -20);
}

TEST(ResultTest, OrElse)
{
  auto a = [](int &err) -> Result<int, int> { return Ok(err * 100); };
  EXPECT_EQ((make_ok<int, int>(20).or_else(a).unwrap()), 20);
  EXPECT_EQ((make_err<int, int>(10).or_else(a).unwrap()), 1000);

  auto b = [](string &err) -> Result<int, string> {
    return Err("Err: " + err);
  };
  EXPECT_EQ((make_ok<int, string>(20).or_else(b).unwrap()), 20);
  EXPECT_EQ((make_err<int, string>("Max Limit"s).or_else(b).unwrap_err()),
            "Err: Max Limit"s);

  auto c = [](vector<int> &err) -> Result<int, vector<int>> {
    return Ok(err.empty() ? -1 : err[0]);
  };
  EXPECT_EQ((make_ok<int, vector<int>>(40).or_else(c).unwrap()), 40);
  EXPECT_EQ(
      (make_err<int, vector<int>>(vector{10, 20, 30}).or_else(c).unwrap()), 10);
}

TEST(ResultTest, UnwrapOr)
{
  EXPECT_EQ((make_ok<int, int>(89).unwrap_or(90)), 89);
  EXPECT_EQ((make_err<int, int>(89).unwrap_or(90)), 90);

  EXPECT_EQ((make_ok<string, int>("John Doe"s).unwrap_or("Unknown"s)),
            "John Doe"s);
  EXPECT_EQ((make_err<string, int>(-20).unwrap_or("Unknown"s)), "Unknown"s);
}

TEST(ResultTest, Unwrap)
{
  EXPECT_EQ((make_ok<int, int>(89).unwrap()), 89);
  EXPECT_TRUE((make_err<int, int>(89).is_err()));

  EXPECT_EQ((make_ok<string, int>("John Doe"s).unwrap()), "John Doe"s);
  EXPECT_TRUE((make_err<string, int>(-20)).is_err());

  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5}).unwrap()),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_TRUE((make_err<vector<int>, int>(-1)).is_err());
}

TEST(ResultTest, UnwrapOrElse)
{
  auto a = [](int &err) { return err + 20; };
  EXPECT_EQ((make_ok<int, int>(10).unwrap_or_else(a)), 10);
  EXPECT_EQ((make_err<int, int>(20).unwrap_or_else(a)), 40);

  auto b = [](string &err) -> int { return stoi(err) + 20; };
  EXPECT_EQ((make_ok<int, string>(10).unwrap_or_else(b)), 10);
  EXPECT_EQ((make_err<int, string>("40"s).unwrap_or_else(b)), 60);

  auto c = [](vector<int> &vec) {
    vec.push_back(10);
    return std::move(vec);
  };
  EXPECT_EQ((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})
                 .unwrap_or_else(c)),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_EQ((make_err<vector<int>, vector<int>>(vector{6, 7, 8, 9})
                 .unwrap_or_else(c)),
            (vector{6, 7, 8, 9, 10}));
}

TEST(ResultTest, Expect)
{
  EXPECT_EQ((make_ok<int, int>(10).expect("===TEST ERR MSG===")), 10);
  EXPECT_DEATH_IF_SUPPORTED(
      (make_err<int, int>(20).expect("===TEST ERR MSG===")), ".*");

  EXPECT_EQ((make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5})
                 .expect("===TEST ERR MSG===")),
            (vector{1, 2, 3, 4, 5}));
  EXPECT_DEATH_IF_SUPPORTED(
      (make_err<vector<int>, int>(20).expect("===TEST ERR MSG===")), ".*");

  EXPECT_EQ((make_ok<int, vector<int>>(-1).expect("===TEST ERR MSG===")), -1);
  EXPECT_DEATH_IF_SUPPORTED(
      (make_err<int, vector<int>>(vector{-1, -2, -3, -4, -5})
           .expect("===TEST ERR MSG===")),
      ".*");

  EXPECT_EQ((make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4, 5})
                 .expect("===TEST ERR MSG===")),
            (vector{1, 2, 3, 4, 5}));

  EXPECT_DEATH_IF_SUPPORTED(
      (make_err<vector<int>, vector<int>>(vector{-1, -2, -3, -4, -5})
           .expect("===TEST ERR MSG===")),
      ".*");
}

TEST(ResultTest, UnwrapErr)
{
  EXPECT_EQ((make_err<int, int>(20).unwrap_err()), 20);
  EXPECT_DEATH_IF_SUPPORTED(((void) make_ok<int, int>(10).unwrap_err()), ".*");

  EXPECT_EQ((make_err<vector<int>, int>(-40).unwrap_err()), -40);
  EXPECT_DEATH_IF_SUPPORTED(
      ((void) make_ok<vector<int>, int>(vector{10, 20, 30}).unwrap_err()),
      ".*");

  EXPECT_EQ((make_err<int, vector<int>>(vector{1, 2, 3, 4}).unwrap_err()),
            (vector{1, 2, 3, 4}));
  EXPECT_DEATH_IF_SUPPORTED(((void) make_ok<int, vector<int>>(68).unwrap_err()),
                            ".*");

  EXPECT_EQ(
      (make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4}).unwrap_err()),
      (vector{1, 2, 3, 4}));
  EXPECT_DEATH_IF_SUPPORTED(
      ((void) make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4})
           .unwrap_err()),
      ".*");
}

TEST(ResultTest, ExpectErr)
{
  EXPECT_EQ((make_err<int, int>(20).expect_err("===TEST ERR MSG===")), 20);
  EXPECT_DEATH_IF_SUPPORTED(
      ((void) make_ok<int, int>(10).expect_err("===TEST ERR MSG===")), ".*");

  EXPECT_EQ((make_err<vector<int>, int>(-40).expect_err("===TEST ERR MSG===")),
            -40);
  EXPECT_DEATH_IF_SUPPORTED(
      ((void) make_ok<vector<int>, int>(vector{10, 20, 30})
           .expect_err("===TEST ERR MSG===")),
      ".*");

  EXPECT_EQ((make_err<int, vector<int>>(vector{1, 2, 3, 4})
                 .expect_err("===TEST ERR MSG===")),
            (vector{1, 2, 3, 4}));
  EXPECT_DEATH_IF_SUPPORTED(
      ((void) make_ok<int, vector<int>>(68).expect_err("===TEST ERR MSG===")),
      ".*");

  EXPECT_EQ((make_err<vector<int>, vector<int>>(vector{1, 2, 3, 4})
                 .expect_err("===TEST ERR MSG===")),
            (vector{1, 2, 3, 4}));
  EXPECT_DEATH_IF_SUPPORTED(
      ((void) make_ok<vector<int>, vector<int>>(vector{1, 2, 3, 4})
           .expect_err("===TEST ERR MSG===")),
      ".*");
}

TEST(ResultTest, Match)
{
  auto a = make_ok<int, int>(98).match([](auto ok) { return ok + 2; },
                                       [](auto err) { return err + 5; });

  EXPECT_EQ(a, 100);

  auto b =
      make_ok<vector<int>, int>(vector{1, 2, 3, 4, 5})
          .match([](auto ok) { return accumulate(ok.begin(), ok.end(), 0); },
                 [](auto) { return -1; });

  EXPECT_EQ(b, 15);

  auto c = make_err<vector<int>, int>(67).match(
      [](auto ok) { return accumulate(ok.begin(), ok.end(), 0); },
      [](auto) { return -1; });

  EXPECT_EQ(c, -1);
}
