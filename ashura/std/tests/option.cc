/// SPDX-License-Identifier: MIT
#include "gtest/gtest.h"

#include "ashura/std/option.h"
#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

using ash::None;
using ash::Option;
using ash::Some;

template <typename T>
constexpr auto make_some(T &&value) -> Option<T>
{
  return Some<T>(std::forward<T>(value));
}

template <typename T>
constexpr auto make_none() -> Option<T>
{
  return None;
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
  MoveOnly &operator=(MoveOnly const &)
  {
    abort();
    return *this;
  }
  MoveOnly &operator=(MoveOnly &&) = default;
  ~MoveOnly()                      = default;

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
  int operator()(int &x)
  {
    mut_call_times++;
    return x;
  }

  int operator()(int &x) const
  {
    return x;
  }
};

struct FnConst
{
  int operator()(int &x) const
  {
    return x;
  }
};

TEST(OptionTest, Misc)
{
  EXPECT_EQ(Option<Option<int>>(Some(Option(Some(899)))).unwrap().unwrap(),
            899);
}

TEST(OptionTest, ObjectConstructionTest)
{
  Option<int> a = None;
  Option      b = Some(89);
  EXPECT_DEATH_IF_SUPPORTED(std::move(a).unwrap(), ".*");
  EXPECT_TRUE(b.is_some());
  EXPECT_EQ(Option(Some(89)).unwrap(), 89);

  auto fn_a = []() -> Option<MoveOnly<0>> {
    return Some(make_mv<0>());        // NOLINT
  };
  auto a_   = fn_a();
  auto fn_b = []() -> Option<MoveOnly<1>> { return None; };
  auto b_   = fn_b();

  auto d = fn_a();
  d      = Some(make_mv<0>());
  d      = None;
  d      = Some(make_mv<0>());
}

TEST(OptionTest, CopyConstructionTest)
{
  {
    Option<int> a = None;
    Option<int> b = a;
    EXPECT_EQ(a, b);

    Option<int> c = Some(98);
    b             = c;
    EXPECT_EQ(b, c);
    EXPECT_NE(a, c);
    EXPECT_NE(a, b);

    Option<std::vector<int>> d = None;
    Option<std::vector<int>> e = d;
    EXPECT_EQ(d, e);

    Option f =
        Some(std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    e = f;
    EXPECT_EQ(e, f);
    EXPECT_NE(d, e);
    EXPECT_NE(d, f);
  }

  {
    Option<int> a = None;
    Option<int> b = a;
    EXPECT_EQ(a, b);

    Option<int> c = Some(98);
    b             = c;
    EXPECT_EQ(b, c);
    EXPECT_NE(a, c);
    EXPECT_NE(a, b);

    Option<std::vector<int>> d = None;
    Option<std::vector<int>> e = d;
    EXPECT_EQ(d, e);

    Option f =
        Some(std::vector{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    e = f;
    EXPECT_EQ(e, f);
    EXPECT_NE(d, e);
    EXPECT_NE(d, f);
  }
}

TEST(OptionTest, ObjectForwardingTest)
{
  auto fn_a = []() -> Option<MoveOnly<0>> {
    return Some(make_mv<0>());        // NOLINT
  };
  EXPECT_TRUE(fn_a().is_some());
  auto fn_b = []() -> Option<std::unique_ptr<int[]>> {
    return Some(std::make_unique<int[]>(1024));
  };
  EXPECT_TRUE(fn_b().is_some());

  Option g = Some(std::vector{1, 2, 3, 4, 5});

  g = Some(std::vector{5, 6, 7, 8, 9});

  EXPECT_EQ(g, Some(std::vector{5, 6, 7, 8, 9}));

  g = None;

  EXPECT_EQ(g, None);

  g = Some(std::vector{1, 2, 3, 4, 5});

  EXPECT_EQ(g, Some(std::vector{1, 2, 3, 4, 5}));

  g = None;

  EXPECT_EQ(g, None);
}

TEST(OptionTest, Equality)
{
  constexpr Option<int> h{};
  static_assert(h == None);

  // EXPECT_NE(Some(0), None);
  EXPECT_EQ(Some(90), Some(90));
  EXPECT_NE(Some(90), Some(70));
  // EXPECT_NE(Some<Option<int>>(None), None);
  EXPECT_EQ(None, None);
  EXPECT_EQ(Option(Some(90)), Some(90));
  EXPECT_NE(Option(Some(90)), Some(70));
  EXPECT_EQ(Option(Some(90)), Option(Some(90)));
  EXPECT_NE(Option(Some(90)), Option(Some(20)));
  EXPECT_NE(Option(Some(90)), None);
  EXPECT_EQ(Option<int>(None), None);
  EXPECT_NE(Option<Option<int>>(Some(Option<int>(None))), None);

  // EXPECT_NE(None, Some(0));
  // EXPECT_NE(None, Some<Option<int>>(None));
  EXPECT_EQ(Some(90), Option(Some(90)));
  EXPECT_NE(Some(70), Option(Some(90)));
  EXPECT_NE(None, Option(Some(90)));
  EXPECT_EQ(None, Option<int>(None));
  EXPECT_NE(None, Option<Option<int>>(Some(Option<int>(None))));
}

TEST(OptionTest, Contains)
{
  EXPECT_TRUE(
      Option(Some(std::vector{1, 2, 3, 4})).contains(std::vector{1, 2, 3, 4}));
  EXPECT_FALSE(Option(Some(std::vector{1, 2, 3, 4}))
                   .contains(std::vector{1, 2, 3, 4, 5}));

  EXPECT_TRUE(Option(Some(8)).contains(8));
  EXPECT_FALSE(Option(Some(8)).contains(88));
}

TEST(OptionLifetimeTest, Contains)
{
  EXPECT_TRUE(Option(Some(make_mv<0>())).contains(make_mv<0>()));
  EXPECT_FALSE(Option<MoveOnly<1>>(None).contains(make_mv<1>()));
}

TEST(OptionTest, AsConstRef)
{
  Option const a = Some(68);
  EXPECT_EQ(*a.as_ref().unwrap(), 68);

  Option<int> const b = None;
  EXPECT_EQ(b.as_ref(), None);

  Option const c = Some(std::vector{1, 2, 3, 4});
  EXPECT_EQ(*c.as_ref().unwrap(), (std::vector{1, 2, 3, 4}));

  Option<std::vector<int>> const d = None;
  EXPECT_EQ(d.as_ref(), None);
}

TEST(OptionTest, AsRef)
{
  Option a             = Some(68);
  *a.as_ref().unwrap() = 99;
  EXPECT_EQ(a, Some(99));

  Option<int> b = None;
  EXPECT_EQ(b.as_ref(), None);

  auto c               = Option(Some(std::vector{1, 2, 3, 4}));
  *c.as_ref().unwrap() = std::vector{5, 6, 7, 8, 9, 10};
  EXPECT_EQ(c, Some(std::vector{5, 6, 7, 8, 9, 10}));

  auto d = Option<std::vector<int>>(None);
  EXPECT_EQ(d.as_ref(), None);
}

TEST(OptionLifeTimeTest, AsRef)
{
  auto a = Option(Some(make_mv<0>()));
  EXPECT_TRUE(a.as_ref().is_some());

  auto b = Option<MoveOnly<1>>(None);
  EXPECT_TRUE(b.as_ref().is_none());
}

TEST(OptionTest, Unwrap)
{
  EXPECT_EQ(Option(Some(0)).unwrap(), 0);
  EXPECT_DEATH_IF_SUPPORTED(Option<int>(None).unwrap(), ".*");

  EXPECT_EQ(Option(Some(std::vector{1, 2, 3, 4, 5})).unwrap(),
            (std::vector{1, 2, 3, 4, 5}));
  EXPECT_DEATH_IF_SUPPORTED(Option<std::vector<int>>(None).unwrap(), ".*");
}

TEST(OptionTest, Expect)
{
  Option(Some(0)).expect("No Value Received");
  // how does it behave with unique_ptr?
  EXPECT_DEATH_IF_SUPPORTED(
      Option<std::unique_ptr<int>>(None).expect("No Value Received"), ".*");
}

TEST(OptionLifetimeTest, Expect)
{
  auto a = Option(Some(make_mv<0>()));
  std::move(a).expect("Yahoooo!").done();
}

TEST(OptionTest, UnwrapOr)
{
  EXPECT_EQ(Option(Some(0)).unwrap_or(90), 0);
  EXPECT_EQ(Option<int>(None).unwrap_or(90), 90);

  EXPECT_EQ(Option(Some(std::vector{1, 2, 3, 4, 5}))
                .unwrap_or(std::vector{6, 7, 8, 9, 10}),
            (std::vector{1, 2, 3, 4, 5}));
  EXPECT_EQ(
      Option<std::vector<int>>(None).unwrap_or(std::vector{6, 7, 8, 9, 10}),
      (std::vector{6, 7, 8, 9, 10}));
}

TEST(OptionLifetimeTest, UnwrapOr)
{
  auto a = Option(Some(make_mv<0>()));
  std::move(a).unwrap_or(make_mv<0>()).done();

  auto b = Option<MoveOnly<1>>(None);
  std::move(b).unwrap_or(make_mv<1>()).done();
}

TEST(OptionTest, UnwrapOrElse)
{
  auto &&a = Option(Some(0)).unwrap_or_else([]() { return 90; });
  EXPECT_EQ(a, 0);
  auto &&b = Option<int>(None).unwrap_or_else([]() { return 90; });
  EXPECT_EQ(b, 90);

  auto &&c = Option(Some(std::vector{1, 2, 3, 4, 5})).unwrap_or_else([]() {
    return std::vector{6, 7, 8, 9, 10};
  });
  EXPECT_EQ(c, (std::vector{1, 2, 3, 4, 5}));

  auto &&d = Option<std::vector<int>>(None).unwrap_or_else(
      []() { return std::vector{6, 7, 8, 9, 10}; });
  EXPECT_EQ(d, (std::vector{6, 7, 8, 9, 10}));
}

TEST(OptionLifetimeTest, UnwrapOrElse)
{
  auto a  = Option(Some(make_mv<0>()));
  auto fn = []() { return make_mv<0>(); };
  std::move(a).unwrap_or_else(fn).done();

  auto b    = Option<MoveOnly<1>>(None);
  auto fn_b = []() { return make_mv<1>(); };
  std::move(b).unwrap_or_else(fn_b).done();
}

TEST(OptionTest, Map)
{
  auto &&a = Option(Some(90)).map([](int &x) -> int { return x + 90; });
  EXPECT_EQ(a, Some(180));

  auto &&b = Option<int>(None).map([](int &x) { return x + 90; });
  EXPECT_EQ(b, None);

  auto &&c =
      Option(Some(std::vector{1, 2, 3, 4, 5})).map([](std::vector<int> &vec) {
        vec.push_back(6);
        return std::move(vec);
      });

  EXPECT_EQ(c, Some(std::vector{1, 2, 3, 4, 5, 6}));

  auto &&d = Option<std::vector<int>>(None).map([](std::vector<int> &vec) {
    vec.push_back(6);
    return std::move(vec);
  });
  EXPECT_EQ(d, None);
}

TEST(OptionLifetimeTest, Map)
{
  auto a = Option(Some(make_mv<0>()));
  std::move(a).map([](auto &r) { return std::move(r); }).unwrap().done();
}

TEST(OptionTest, FnMutMap)
{
  auto fnmut_a = FnMut();
  auto a1_     = Option(Some(90)).map(fnmut_a);
  auto a2_     = Option(Some(90)).map(fnmut_a);

  (void) a1_;
  (void) a2_;

  EXPECT_EQ(fnmut_a.mut_call_times, 2);

  auto const fnmut_b = FnMut();
  auto       b1_     = Option(Some(90)).map(fnmut_b);
  auto       b2_     = Option(Some(90)).map(fnmut_b);

  (void) b1_;
  (void) b2_;

  EXPECT_EQ(fnmut_b.mut_call_times, 0);

  auto fnconst = FnConst();
  auto c       = Option(Some(90)).map(fnconst);

  (void) c;
}

TEST(OptionTest, MapOrElse)
{
  auto &&a = Option(Some(90)).map_or_else([](int &x) -> int { return x + 90; },
                                          []() -> int { return 90; });
  EXPECT_EQ(a, 180);

  auto &&b = Option<int>(None).map_or_else([](int &x) -> int { return x + 90; },
                                           []() -> int { return 90; });
  EXPECT_EQ(b, 90);
}

TEST(OptionLifetimeTest, MapOrElse)
{
  auto a    = Option(Some(make_mv<0>()));
  auto fn   = [](auto &) { return make_mv<0>(); };        // NOLINT
  auto fn_b = []() { return make_mv<0>(); };              // NOLINT
  std::move(a).map_or_else(fn, fn_b).done();
}

TEST(OptionTest, AndThen)
{
  auto &&a = Option(Some(90)).and_then(
      [](int &x) { return Option(Some(static_cast<float>(x) + 90.0f)); });

  EXPECT_FLOAT_EQ(std::move(a).unwrap(), 180.0f);

  auto &&b = make_none<int>().and_then(
      [](int &x) { return Option(Some(static_cast<float>(x) + 90.0f)); });
  EXPECT_EQ(b, None);
}

TEST(OptionTest, OrElse)
{
  auto &&a = Option(Some(90.0f)).or_else([]() { return Option(Some(0.5f)); });
  EXPECT_FLOAT_EQ(std::move(a).unwrap(), 90.0f);

  auto &&b = Option<float>(None).or_else([]() { return Option(Some(0.5f)); });
  EXPECT_FLOAT_EQ(std::move(b).unwrap(), 0.5f);

  auto &&c = Option<float>(None).or_else([]() { return Option<float>(None); });
  EXPECT_EQ(c, None);
  //
  //
  auto &&d = Option(Some(std::vector{1, 2, 3, 4, 5})).or_else([]() {
    return Option(Some(std::vector{6, 7, 8, 9, 10}));
  });
  EXPECT_EQ(std::move(d).unwrap(), (std::vector{1, 2, 3, 4, 5}));

  auto &&e = Option<std::vector<int>>(None).or_else(
      []() { return Option(Some(std::vector{6, 7, 8, 9, 10})); });
  EXPECT_EQ(std::move(e).unwrap(), (std::vector{6, 7, 8, 9, 10}));

  auto &&f = Option<std::vector<int>>(None).or_else(
      []() { return Option<std::vector<int>>(None); });
  EXPECT_EQ(f, None);
}

TEST(OptionTest, ExpectNone)
{
  EXPECT_DEATH_IF_SUPPORTED(Option(Some(56)).expect_none("===TEST==="), ".*");
  Option<int>(None).expect_none("===TEST===");

  EXPECT_DEATH_IF_SUPPORTED(
      Option(Some(std::vector<int>{1, 2, 3, 4, 5})).expect_none("===TEST==="),
      ".*");
  Option<std::vector<int>>(None).expect_none("===TEST===");
}

TEST(OptionTest, Match)
{
  auto v = Option(Some(98)).match([](auto some) { return some + 2; },
                                  []() { return 5; });
  EXPECT_EQ(v, 100);

  auto &&a = Option(Some(90)).match([](int &x) { return x + 10; },
                                    []() { return -1; });
  EXPECT_EQ(a, 100);

  auto &&b = Option<int>(None).match([](auto &x) { return x + 10; },
                                     []() { return -1; });
  EXPECT_EQ(b, -1);

  auto &&c =
      Option(Some(std::vector{1, 2, 3, 4, 5}))
          .match([](auto &x) { return accumulate(x.begin(), x.end(), 0); },
                 []() { return -1; });
  EXPECT_EQ(c, 15);

  auto &&d = Option<std::vector<int>>(None).match(
      [](auto &x) { return accumulate(x.begin(), x.end(), 0); },
      []() { return -1; });
  EXPECT_EQ(d, -1);
}
