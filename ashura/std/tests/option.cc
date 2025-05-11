/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/option.h"
#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

using ash::none;
using ash::Option;

template <typename T>
constexpr auto make_some(T && value) -> Option<T>
{
  return std::forward<T>(value);
}

template <typename T>
constexpr auto make_none() -> Option<T>
{
  return none;
}

template <size_t ID>
struct MoveOnly
{
  explicit MoveOnly(int)
  {
  }

  // no implicit defaul construction
  MoveOnly()
  {
    abort();
  }

  MoveOnly(MoveOnly const &)
  {
    abort();
  }

  MoveOnly(MoveOnly &&) = default;

  MoveOnly & operator=(MoveOnly const &)
  {
    abort();
    return *this;
  }

  MoveOnly & operator=(MoveOnly &&) = default;
  ~MoveOnly()                       = default;

  void done() const
  {
    // cout << "\t>> MoveOnly<" << to_string(ID) << "> Done!" << std::endl;
  }

  bool operator==(MoveOnly const &) const
  {
    return true;
  }

  bool operator!=(MoveOnly const &) const
  {
    return false;
  }
};

template <size_t id>
MoveOnly<id> make_mv()
{
  return MoveOnly<id>(id);
}

struct FnMut
{
  int mut_call_times;

  FnMut() : mut_call_times{0}
  {
  }

  int operator()(int & x)
  {
    mut_call_times++;
    return x;
  }

  int operator()(int & x) const
  {
    return x;
  }
};

struct FnConst
{
  int operator()(int & x) const
  {
    return x;
  }
};

TEST(OptionTest, Misc)
{
  EXPECT_EQ(Option<Option<int>>(Option(899)).unwrap().unwrap(), 899);
}

TEST(OptionTest, ObjectConstructionTest)
{
  Option<int> a = none;
  Option      b = 89;
  EXPECT_DEATH_IF_SUPPORTED(std::move(a).unwrap(), ".*");
  EXPECT_TRUE(b.is_some());
  EXPECT_EQ(Option(89).unwrap(), 89);

  auto fn_a = []() -> Option<MoveOnly<0>> {
    return make_mv<0>();    // NOLINT
  };
  auto a_   = fn_a();
  auto fn_b = []() -> Option<MoveOnly<1>> { return none; };
  auto b_   = fn_b();

  auto d = fn_a();
  d      = make_mv<0>();
  d      = none;
  d      = make_mv<0>();
}

TEST(OptionTest, CopyConstructionTest)
{
  {
    Option<int> a = none;
    Option<int> b = a;
    EXPECT_EQ(a, b);

    Option<int> c(98);
    b = c;
    EXPECT_EQ(b, c);
    EXPECT_NE(a, c);
    EXPECT_NE(a, b);

    Option<std::vector<int>> d = none;
    Option<std::vector<int>> e = d;
    EXPECT_EQ(d, e);

    Option f(std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    e = f;
    EXPECT_EQ(e, f);
    EXPECT_NE(d, e);
    EXPECT_NE(d, f);
  }

  {
    Option<int> a = none;
    Option<int> b = a;
    EXPECT_EQ(a, b);

    Option<int> c(98);
    b = c;
    EXPECT_EQ(b, c);
    EXPECT_NE(a, c);
    EXPECT_NE(a, b);

    Option<std::vector<int>> d = none;
    Option<std::vector<int>> e = d;
    EXPECT_EQ(d, e);

    Option f(std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    e = f;
    EXPECT_EQ(e, f);
    EXPECT_NE(d, e);
    EXPECT_NE(d, f);
  }
}

TEST(OptionTest, ObjectForwardingTest)
{
  auto fn_a = []() -> Option<MoveOnly<0>> {
    return make_mv<0>();    // NOLINT
  };
  EXPECT_TRUE(fn_a().is_some());
  auto fn_b = []() -> Option<std::unique_ptr<int[]>> {
    return std::make_unique<int[]>(1'024);
  };
  EXPECT_TRUE(fn_b().is_some());

  Option g = std::vector{1, 2, 3, 4, 5};

  g = std::vector{5, 6, 7, 8, 9};

  EXPECT_EQ(g, (std::vector{5, 6, 7, 8, 9}));

  g = none;

  EXPECT_EQ(g, none);

  g = std::vector{1, 2, 3, 4, 5};

  EXPECT_EQ(g, (std::vector{1, 2, 3, 4, 5}));

  g = none;

  EXPECT_EQ(g, none);
}

TEST(OptionTest, Equality)
{
  constexpr Option<int> h{};
  static_assert(h == none);

  EXPECT_EQ(none, none);
  EXPECT_NE(Option(90), 70);
  EXPECT_EQ(Option(90), Option(90));
  EXPECT_NE(Option(90), Option(20));
  EXPECT_NE(Option(90), none);
  EXPECT_EQ(Option<int>(none), none);
  EXPECT_NE(Option<Option<int>>(Option<int>(none)), none);

  EXPECT_EQ(90, Option(90));
  EXPECT_NE(70, Option(90));
  EXPECT_NE(none, Option(90));
  EXPECT_EQ(none, Option<int>(none));
  EXPECT_NE(none, Option<Option<int>>(Option<int>(none)));
}

TEST(OptionTest, Contains)
{
  EXPECT_TRUE(
    Option(std::vector{1, 2, 3, 4}).contains(std::vector{1, 2, 3, 4}));
  EXPECT_FALSE(
    Option(std::vector{1, 2, 3, 4}).contains(std::vector{1, 2, 3, 4, 5}));

  EXPECT_TRUE(Option(8).contains(8));
  EXPECT_FALSE(Option(8).contains(88));
}

TEST(OptionLifetimeTest, Contains)
{
  EXPECT_TRUE(Option(make_mv<0>()).contains(make_mv<0>()));
  EXPECT_FALSE(Option<MoveOnly<1>>(none).contains(make_mv<1>()));
}

TEST(OptionTest, AsConstRef)
{
  Option const a = 68;
  EXPECT_EQ(*a.as_ptr().unwrap(), 68);

  Option<int> const b = none;
  EXPECT_EQ(b.as_ptr(), none);

  Option const c = std::vector{1, 2, 3, 4};
  EXPECT_EQ(*c.as_ptr().unwrap(), (std::vector{1, 2, 3, 4}));

  Option<std::vector<int>> const d = none;
  EXPECT_EQ(d.as_ptr(), none);
}

TEST(OptionTest, AsRef)
{
  Option a             = 68;
  *a.as_ptr().unwrap() = 99;
  EXPECT_EQ(a, 99);

  Option<int> b = none;
  EXPECT_EQ(b.as_ptr(), none);

  auto c               = Option(std::vector{1, 2, 3, 4});
  *c.as_ptr().unwrap() = std::vector{5, 6, 7, 8, 9, 10};
  EXPECT_EQ(c, (std::vector{5, 6, 7, 8, 9, 10}));

  auto d = Option<std::vector<int>>(none);
  EXPECT_EQ(d.as_ptr(), none);
}

TEST(OptionLifeTimeTest, AsRef)
{
  auto a = Option(make_mv<0>());
  EXPECT_TRUE(a.as_ptr().is_some());

  auto b = Option<MoveOnly<1>>(none);
  EXPECT_TRUE(b.as_ptr().is_none());
}

TEST(OptionTest, Unwrap)
{
  EXPECT_EQ(Option(0).unwrap(), 0);
  EXPECT_DEATH_IF_SUPPORTED(Option<int>(none).unwrap(), ".*");

  EXPECT_EQ(Option(std::vector{1, 2, 3, 4, 5}).unwrap(),
            (std::vector{1, 2, 3, 4, 5}));
  EXPECT_DEATH_IF_SUPPORTED(Option<std::vector<int>>(none).unwrap(), ".*");
}

TEST(OptionTest, UnwrapOr)
{
  EXPECT_EQ(Option(0).unwrap_or(90), 0);
  EXPECT_EQ(Option<int>(none).unwrap_or(90), 90);

  EXPECT_EQ(
    Option(std::vector{1, 2, 3, 4, 5}).unwrap_or(std::vector{6, 7, 8, 9, 10}),
    (std::vector{1, 2, 3, 4, 5}));
  EXPECT_EQ(
    Option<std::vector<int>>(none).unwrap_or(std::vector{6, 7, 8, 9, 10}),
    (std::vector{6, 7, 8, 9, 10}));
}

TEST(OptionLifetimeTest, UnwrapOr)
{
  auto a = Option(make_mv<0>());
  std::move(a).unwrap_or(make_mv<0>()).done();

  auto b = Option<MoveOnly<1>>(none);
  std::move(b).unwrap_or(make_mv<1>()).done();
}

TEST(OptionTest, Map)
{
  auto && a = Option(90).map([](int & x) -> int { return x + 90; });
  EXPECT_EQ(a, 180);

  auto && b = Option<int>(none).map([](int & x) { return x + 90; });
  EXPECT_EQ(b, none);

  auto && c =
    Option(std::vector{1, 2, 3, 4, 5}).map([](std::vector<int> & vec) {
      vec.push_back(6);
      return std::move(vec);
    });

  EXPECT_EQ(c, (std::vector{1, 2, 3, 4, 5, 6}));

  auto && d = Option<std::vector<int>>(none).map([](std::vector<int> & vec) {
    vec.push_back(6);
    return std::move(vec);
  });
  EXPECT_EQ(d, none);
}

TEST(OptionLifetimeTest, Map)
{
  auto a = Option(make_mv<0>());
  std::move(a).map([](auto & r) { return std::move(r); }).unwrap().done();
}

TEST(OptionTest, FnMutMap)
{
  auto fnmut_a = FnMut();
  auto a1_     = Option(90).map(fnmut_a);
  auto a2_     = Option(90).map(fnmut_a);

  (void) a1_;
  (void) a2_;

  EXPECT_EQ(fnmut_a.mut_call_times, 2);

  auto const fnmut_b = FnMut();
  auto       b1_     = Option(90).map(fnmut_b);
  auto       b2_     = Option(90).map(fnmut_b);

  (void) b1_;
  (void) b2_;

  EXPECT_EQ(fnmut_b.mut_call_times, 0);

  auto fnconst = FnConst();
  auto c       = Option(90).map(fnconst);

  (void) c;
}

TEST(OptionTest, AndThen)
{
  auto && a = Option(90).and_then(
    [](int & x) { return Option(static_cast<float>(x + 90.0F)); });

  EXPECT_FLOAT_EQ(std::move(a).unwrap(), 180.0F);

  auto && b = make_none<int>().and_then(
    [](int & x) { return Option(static_cast<float>(x + 90.0F)); });
  EXPECT_EQ(b, none);
}

TEST(OptionTest, Match)
{
  auto v =
    Option(98).match([](auto some) { return some + 2; }, []() { return 5; });
  EXPECT_EQ(v, 100);

  auto && a =
    Option(90).match([](int & x) { return x + 10; }, []() { return -1; });
  EXPECT_EQ(a, 100);

  auto && b = Option<int>(none).match([](auto & x) { return x + 10; },
                                      []() { return -1; });
  EXPECT_EQ(b, -1);

  auto && c =
    Option(std::vector{1, 2, 3, 4, 5})
      .match([](auto & x) { return accumulate(x.begin(), x.end(), 0); },
             []() { return -1; });
  EXPECT_EQ(c, 15);

  auto && d = Option<std::vector<int>>(none).match(
    [](auto & x) { return accumulate(x.begin(), x.end(), 0); },
    []() { return -1; });
  EXPECT_EQ(d, -1);
}
