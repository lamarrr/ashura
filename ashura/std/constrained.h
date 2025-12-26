#pragma once
#include "ashura/std/error.h"

namespace ash
{

struct NonZeroConstraint
{
  static constexpr void check(auto v)
  {
    CHECK(v != static_cast<decltype(v)>(0), "Value must be non-zero");
  }
};

struct NonNullConstraint
{
  static constexpr void check(auto * p)
  {
    CHECK(p != nullptr, "Pointer must be non-null");
  }
};

struct NonZeroPow2Constraint
{
  static constexpr void check(auto v)
  {
    CHECK(v != static_cast<decltype(v)>(0) &&
            ((v & (v - 1)) == static_cast<decltype(v)>(0)),
          "Value must be non-zero power of 2");
  }
};

struct AssumeConstrained
{
};

inline constexpr AssumeConstrained assume_constrained{};

template <typename T, typename Constraint>
struct Constrained
{
  T value_;

  constexpr Constrained(T value) : value_{value}
  {
    Constraint::check(value_);
  }

  constexpr Constrained(AssumeConstrained, T value) : value_{value}
  {
  }

  constexpr Constrained(Constrained const &)             = default;
  constexpr Constrained(Constrained &&)                  = default;
  constexpr Constrained & operator=(Constrained const &) = default;
  constexpr Constrained & operator=(Constrained &&)      = default;
  constexpr ~Constrained()                               = default;

  constexpr operator T const &() const
  {
    return value_;
  }

  constexpr void set(T value)
  {
    Constraint::check(value);
    value_ = value;
  }

  constexpr void set(AssumeConstrained, T value)
  {
    value_ = value;
  }

  constexpr T const & value() const
  {
    return value_;
  }
};

template <typename T>
using NonZero = Constrained<T, NonZeroConstraint>;

template <typename T>
using NonNull = Constrained<T, NonNullConstraint>;

template <typename T>
using NonZeroPow2 = Constrained<T, NonZeroPow2Constraint>;

}    // namespace ash
