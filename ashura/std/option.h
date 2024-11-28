/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include "ashura/std/v.h"

namespace ash
{

template <typename T = Void>
struct [[nodiscard]] Some
{
  using Type = T;
  T value{};
};

template <typename T>
Some(T) -> Some<T>;

struct NoneType
{
};

constexpr NoneType None;

template <typename T = Void>
struct [[nodiscard]] Option
{
  using Type = T;

  usize is_some_;

  union
  {
    T value_;
  };

  constexpr Option() : is_some_{false}
  {
  }

  constexpr ~Option()
  {
    if (is_some_)
    {
      value_.~T();
    }
  }

  constexpr Option(Some<T> some) : is_some_{true}, value_{(T &&) some.value}
  {
  }

  template <typename... Args>
  explicit constexpr Option(V<0>, Args &&...args) :
      is_some_{true}, value_{((Args &&) args)...}
  {
  }

  constexpr Option(NoneType) : is_some_{false}
  {
  }

  constexpr Option &operator=(Some<T> other)
  {
    if (is_some_)
    {
      value_.~T();
    }
    is_some_ = true;
    new (&value_) T{(T &&) other.value};
    return *this;
  }

  constexpr Option &operator=(NoneType)
  {
    if (is_some_)
    {
      value_.~T();
    }
    is_some_ = false;
    return *this;
  }

  constexpr Option(Option &&other) : is_some_{other.is_some_}
  {
    if (other.is_some_)
    {
      new (&value_) T{(T &&) other.value_};
    }
  }

  constexpr Option &operator=(Option &&other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    if (is_some_)
    {
      value_.~T();
    }

    is_some_ = other.is_some_;

    if (other.is_some_)
    {
      new (&value_) T{(T &&) other.value_};
    }

    return *this;
  }

  constexpr Option(Option const &other) : is_some_{other.is_some_}
  {
    if (other.is_some_)
    {
      new (&value_) T{other.value_};
    }
  }

  constexpr Option &operator=(Option const &other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    if (is_some_)
    {
      value_.~T();
    }

    is_some_ = other.is_some_;

    if (other.is_some_)
    {
      new (&value_) T{other.value_};
    }

    return *this;
  }

  [[nodiscard]] constexpr bool is_some() const
  {
    return is_some_;
  }

  [[nodiscard]] constexpr bool is_none() const
  {
    return !is_some_;
  }

  [[nodiscard]] explicit constexpr operator bool() const
  {
    return is_some_;
  }

  template <typename CmpType>
  [[nodiscard]] constexpr bool contains(CmpType const &cmp) const
  {
    if (is_some_)
    {
      return value_ == cmp;
    }
    else
    {
      return false;
    }
  }

  constexpr T &value(SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_some_, "Expected Value in Option but got None");
    return value_;
  }

  constexpr T const &value(SourceLocation loc = SourceLocation::current()) const
  {
    CHECK_DESC_SRC(loc, is_some_, "Expected Value in Option but got None");
    return value_;
  }

  constexpr Option<T const *> as_ptr() const
  {
    if (is_some_)
    {
      return Some(&value_);
    }
    return None;
  }

  constexpr Option<T *> as_ptr()
  {
    if (is_some_)
    {
      return Some(&value_);
    }
    return None;
  }

  constexpr T expect(char const    *msg,
                     SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_some_, msg);
    return (T &&) value_;
  }

  constexpr T unwrap(SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_some_, "Expected Value in Option but got None");
    return (T &&) value_;
  }

  template <typename U>
  constexpr T unwrap_or(U &&alt)
  {
    if (is_some_)
    {
      return (T &&) value_;
    }
    return (U &&) alt;
  }

  template <typename Fn>
  constexpr T unwrap_or_else(Fn &&op)
  {
    if (is_some_)
    {
      return (T &&) value_;
    }
    return op();
  }

  template <typename Fn>
  constexpr auto map(Fn &&op)
  {
    using U = decltype(op(value_));
    if (is_some_)
    {
      return Option<U>{Some(op(value_))};
    }
    return Option<U>{None};
  }

  template <typename Fn, typename U>
  constexpr auto map_or(Fn &&op, U &&alt)
  {
    if (is_some_)
    {
      return op(value_);
    }
    return (U &&) alt;
  }

  template <typename Fn, typename AltFn>
  constexpr decltype(auto) map_or_else(Fn &&op, AltFn &&alt_fn)
  {
    if (is_some_)
    {
      return op(value_);
    }
    return alt_fn();
  }

  template <typename Fn>
  constexpr auto and_then(Fn &&op)
  {
    using OutOption = decltype(op(value_));
    if (is_some_)
    {
      return op(value_);
    }
    return OutOption{None};
  }

  template <typename Fn>
  constexpr auto or_else(Fn &&op)
  {
    using OutOption = decltype(op());
    if (is_some_)
    {
      return OutOption{Some{value_}};
    }
    return op();
  }

  constexpr void expect_none(char const    *msg,
                             SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, !is_some_, msg, " ", value_);
  }

  constexpr void unwrap_none(SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, !is_some_,
                   "Expected None in Option but got Value = ", value_);
  }

  template <typename SomeFn, typename NoneFn>
  constexpr decltype(auto) match(SomeFn &&some_fn, NoneFn &&none_fn)
  {
    if (is_some_)
    {
      return some_fn(value_);
    }
    return none_fn();
  }

  template <typename SomeFn, typename NoneFn>
  constexpr decltype(auto) match(SomeFn &&some_fn, NoneFn &&none_fn) const
  {
    if (is_some_)
    {
      return some_fn(value_);
    }
    return none_fn();
  }
};

template <typename T>
Option(Some<T>) -> Option<T>;

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(Some<T> const &a, Some<U> const &b)
{
  return a.value == b.value;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator!=(Some<T> const &a, Some<U> const &b)
{
  return a.value != b.value;
}

[[nodiscard]] constexpr bool operator==(NoneType, NoneType)
{
  return true;
}

[[nodiscard]] constexpr bool operator!=(NoneType, NoneType)
{
  return false;
}

template <typename T>
[[nodiscard]] constexpr bool operator==(Some<T> const &, NoneType)
{
  return false;
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(Some<T> const &, NoneType)
{
  return true;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(Option<T> const &a, Option<U> const &b)
{
  if (a.is_none() && b.is_none())
  {
    return true;
  }
  if (a.is_some_ && b.is_some_)
  {
    return a.value_ == b.value_;
  }
  return false;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator!=(Option<T> const &a, Option<U> const &b)
{
  if (a.is_none() && b.is_none())
  {
    return false;
  }
  if (a.is_some_ && b.is_some_)
  {
    return a.value_ != b.value_;
  }
  return true;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(Option<T> const &a, Some<U> const &b)
{
  if (a.is_some_)
  {
    return a.value_ == b.value;
  }
  return false;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator!=(Option<T> const &a, Some<U> const &b)
{
  if (a.is_some_)
  {
    return a.value_ != b.value;
  }
  return true;
}

template <typename U, typename T>
[[nodiscard]] constexpr bool operator==(Some<U> const &a, Option<T> const &b)
{
  if (b.is_some_)
  {
    return a.value == b.value_;
  }
  return false;
}

template <typename U, typename T>
[[nodiscard]] constexpr bool operator!=(Some<U> const &a, Option<T> const &b)
{
  if (b.is_some_)
  {
    return a.value != b.value_;
  }
  return true;
}

template <typename T>
[[nodiscard]] constexpr bool operator==(Option<T> const &a, NoneType)
{
  return a.is_none();
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(Option<T> const &a, NoneType)
{
  return a.is_some_;
}

template <typename T>
[[nodiscard]] constexpr bool operator==(NoneType, Option<T> const &a)
{
  return a.is_none();
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(NoneType, Option<T> const &a)
{
  return a.is_some_;
}

}        // namespace ash
