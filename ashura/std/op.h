#pragma once

namespace ash
{

struct Noop
{
  constexpr void operator()(auto &&...) const
  {
  }
};

struct Add
{
  constexpr auto operator()(auto const &a, auto const &b) const
  {
    return a + b;
  }
};

struct Sub
{
  constexpr auto operator()(auto const &a, auto const &b) const
  {
    return a - b;
  }
};

struct Mul
{
  constexpr auto operator()(auto const &a, auto const &b) const
  {
    return a * b;
  }
};

struct Div
{
  constexpr auto operator()(auto const &a, auto const &b) const
  {
    return a / b;
  }
};

struct Equal
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a == b;
  }
};

struct NotEqual
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a != b;
  }
};

struct Lesser
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a < b;
  }
};

struct LesserOrEqual
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a <= b;
  }
};

struct Greater
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a > b;
  }
};

struct GreaterOrEqual
{
  constexpr bool operator()(auto const &a, auto const &b) const
  {
    return a >= b;
  }
};

struct Compare
{
  constexpr int operator()(auto const &a, auto const &b) const
  {
    if (a == b)
    {
      return 0;
    }
    if (a > b)
    {
      return -1;
    }
    return 1;
  }
};

struct Min
{
  template <typename T>
  constexpr T const &operator()(T const &a, T const &b) const
  {
    return a < b ? a : b;
  }
};

struct Max
{
  template <typename T>
  constexpr auto const &operator()(T const &a, T const &b) const
  {
    return a > b ? a : b;
  }
};

struct Swap
{
  template <typename T>
  constexpr void operator()(T &a, T &b) const
  {
    T a_tmp{(T &&) a};
    a = (T &&) b;
    b = (T &&) a_tmp;
  }
};

struct Clamp
{
  template <typename T>
  constexpr T const &operator()(T const &value, T const &min,
                                T const &max) const
  {
    return value < min ? min : (value > max ? max : value);
  }
};

constexpr Noop           noop;
constexpr Add            add;
constexpr Sub            sub;
constexpr Mul            mul;
constexpr Div            div;
constexpr Equal          equal;
constexpr NotEqual       not_equal;
constexpr Lesser         lesser;
constexpr LesserOrEqual  lesser_or_equal;
constexpr Greater        greater;
constexpr GreaterOrEqual greater_or_equal;
constexpr Compare        compare;
constexpr Min            min;
constexpr Max            max;
constexpr Swap           swap;
constexpr Clamp          clamp;

}        // namespace ash
