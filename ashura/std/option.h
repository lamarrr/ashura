/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/error.h"
#include "ashura/std/log.h"
#include "ashura/std/v.h"

namespace ash
{

struct None
{
};

inline constexpr None none;

template <typename T = Void>
struct [[nodiscard]] Option
{
  using Type = T;

  bool32 is_some_;

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

  constexpr Option(T some) : is_some_{true}, value_{static_cast<T &&>(some)}
  {
  }

  template <typename... Args>
  explicit constexpr Option(V<0>, Args &&... args) :
    is_some_{true},
    value_{static_cast<Args &&>(args)...}
  {
  }

  constexpr Option(None) : is_some_{false}
  {
  }

  constexpr Option & operator=(T other)
  {
    if (is_some_)
    {
      value_.~T();
    }
    is_some_ = true;
    new (&value_) T{static_cast<T &&>(other)};
    return *this;
  }

  constexpr Option & operator=(None)
  {
    if (is_some_)
    {
      value_.~T();
    }
    is_some_ = false;
    return *this;
  }

  constexpr Option(Option && other) : is_some_{other.is_some_}
  {
    if (other.is_some_)
    {
      new (&value_) T{static_cast<T &&>(other.value_)};
    }
  }

  constexpr Option & operator=(Option && other)
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
      new (&value_) T{static_cast<T &&>(other.value_)};
    }

    return *this;
  }

  constexpr Option(Option const & other) : is_some_{other.is_some_}
  {
    if (other.is_some_)
    {
      new (&value_) T{other.value_};
    }
  }

  constexpr Option & operator=(Option const & other)
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
    return is_some();
  }

  template <typename CmpType>
  [[nodiscard]] constexpr bool contains(CmpType const & cmp) const
  {
    if (is_some())
    {
      return value_ == cmp;
    }
    else
    {
      return false;
    }
  }

  constexpr T & value(SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_some(), "Expected Value in Option but got None");
    return value_;
  }

  constexpr T const &
    value(SourceLocation loc = SourceLocation::current()) const
  {
    CHECK_DESC_SRC(loc, is_some(), "Expected Value in Option but got None");
    return value_;
  }

  constexpr Option<T const *> as_ptr() const
  {
    if (is_some())
    {
      return &value_;
    }
    return none;
  }

  constexpr Option<T *> as_ptr()
  {
    if (is_some())
    {
      return &value_;
    }
    return none;
  }

  constexpr T expect(char const *   msg,
                     SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_some(), msg);
    return static_cast<T &&>(value_);
  }

  constexpr T unwrap(SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_some(), "Expected Value in Option but got None");
    return static_cast<T &&>(value_);
  }

  template <typename U>
  constexpr T unwrap_or(U && alt)
  {
    if (is_some())
    {
      return static_cast<T &&>(value_);
    }
    return static_cast<U &&>(alt);
  }

  template <typename Fn>
  constexpr T unwrap_or_else(Fn && op)
  {
    if (is_some())
    {
      return static_cast<T &&>(value_);
    }
    return op();
  }

  template <typename Fn>
  constexpr auto map(Fn && op)
  {
    using U = decltype(op(value_));
    if (is_some())
    {
      return Option<U>{op(value_)};
    }
    return Option<U>{none};
  }

  template <typename Fn, typename U>
  constexpr auto map_or(Fn && op, U && alt)
  {
    if (is_some())
    {
      return op(value_);
    }
    return static_cast<U &&>(alt);
  }

  template <typename Fn, typename AltFn>
  constexpr decltype(auto) map_or_else(Fn && op, AltFn && alt_fn)
  {
    if (is_some())
    {
      return op(value_);
    }
    return alt_fn();
  }

  template <typename Fn>
  constexpr auto and_then(Fn && op)
  {
    using OutOption = decltype(op(value_));
    if (is_some())
    {
      return op(value_);
    }
    return OutOption{none};
  }

  template <typename Fn>
  constexpr auto or_else(Fn && op)
  {
    using OutOption = decltype(op());
    if (is_some())
    {
      return OutOption{value_};
    }
    return op();
  }

  constexpr void expect_none(char const *   msg,
                             SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_none(), msg, " ", value_);
  }

  constexpr void unwrap_none(SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_none(),
                   "Expected None in Option but got Value = ", value_);
  }

  template <typename SomeFn, typename NoneFn = Noop>
  constexpr decltype(auto) match(SomeFn && some_fn, NoneFn && none_fn = {})
  {
    if (is_some())
    {
      return some_fn(value_);
    }
    return none_fn();
  }

  template <typename SomeFn, typename NoneFn = Noop>
  constexpr decltype(auto) match(SomeFn && some_fn,
                                 NoneFn && none_fn = {}) const
  {
    if (is_some())
    {
      return some_fn(value_);
    }
    return none_fn();
  }
};

template <typename T>
Option(T) -> Option<T>;

template <typename T>
struct IsTriviallyRelocatable<Option<T>>
{
  static constexpr bool value = TriviallyRelocatable<T>;
};

[[nodiscard]] constexpr bool operator==(None, None)
{
  return true;
}

[[nodiscard]] constexpr bool operator!=(None, None)
{
  return false;
}

template <typename T>
[[nodiscard]] constexpr bool operator==(T const &, None)
{
  return false;
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(T const &, None)
{
  return true;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(Option<T> const & a,
                                        Option<U> const & b)
{
  if (a.is_none() && b.is_none())
  {
    return true;
  }
  if (a.is_some() && b.is_some())
  {
    return a.value_ == b.value_;
  }
  return false;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator!=(Option<T> const & a,
                                        Option<U> const & b)
{
  if (a.is_none() && b.is_none())
  {
    return false;
  }
  if (a.is_some() && b.is_some())
  {
    return a.value_ != b.value_;
  }
  return true;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(Option<T> const & a, U const & b)
{
  if (a.is_some())
  {
    return a.value_ == b;
  }
  return false;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator!=(Option<T> const & a, U const & b)
{
  if (a.is_some())
  {
    return a.value_ != b;
  }
  return true;
}

template <typename U, typename T>
[[nodiscard]] constexpr bool operator==(U const & a, Option<T> const & b)
{
  if (b.is_some())
  {
    return a == b.value_;
  }
  return false;
}

template <typename U, typename T>
[[nodiscard]] constexpr bool operator!=(U const & a, Option<T> const & b)
{
  if (b.is_some())
  {
    return a != b.value_;
  }
  return true;
}

template <typename T>
[[nodiscard]] constexpr bool operator==(Option<T> const & a, None)
{
  return a.is_none();
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(Option<T> const & a, None)
{
  return a.is_some();
}

template <typename T>
[[nodiscard]] constexpr bool operator==(None, Option<T> const & a)
{
  return a.is_none();
}

template <typename T>
[[nodiscard]] constexpr bool operator!=(None, Option<T> const & a)
{
  return a.is_some();
}

template <typename T>
struct [[nodiscard]] OptionRef
{
  using Type = T;

  T * rep_;

  constexpr OptionRef() : rep_{nullptr}
  {
  }

  constexpr ~OptionRef() = default;

  constexpr OptionRef(T & some) : rep_{&some}
  {
  }

  template <typename... Args>
  explicit constexpr OptionRef(V<0>, T & some) : rep_{&some}
  {
  }

  constexpr OptionRef(None) : rep_{nullptr}
  {
  }

  constexpr OptionRef & operator=(T & other)
  {
    rep_ = &other;
    return *this;
  }

  constexpr OptionRef & operator=(None)
  {
    rep_ = nullptr;
    return *this;
  }

  constexpr OptionRef(OptionRef && other) = default;

  constexpr OptionRef & operator=(OptionRef && other) = default;

  constexpr OptionRef(OptionRef const & other) = default;

  constexpr OptionRef & operator=(OptionRef const & other) = default;

  [[nodiscard]] constexpr bool is_some() const
  {
    return rep_ != nullptr;
  }

  [[nodiscard]] constexpr bool is_none() const
  {
    return rep_ == nullptr;
  }

  [[nodiscard]] explicit constexpr operator bool() const
  {
    return is_some();
  }

  template <typename CmpType>
  [[nodiscard]] constexpr bool contains(CmpType const & cmp) const
  {
    if (is_some())
    {
      return *rep_ == cmp;
    }
    else
    {
      return false;
    }
  }

  constexpr T & value(SourceLocation loc = SourceLocation::current()) const
  {
    CHECK_DESC_SRC(loc, is_some(), "Expected Value in OptionRef but got None");
    return *rep_;
  }

  constexpr T & expect(char const *   msg,
                       SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_some(), msg);
    return *rep_;
  }

  constexpr T & unwrap(SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_some(), "Expected Value in OptionRef but got None");
    return *rep_;
  }

  constexpr T & unwrap_or(T & alt)
  {
    if (is_some())
    {
      return *rep_;
    }
    return alt;
  }

  constexpr void expect_none(char const *   msg,
                             SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_none(), msg, " ", *rep_);
  }

  constexpr void unwrap_none(SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_none(),
                   "Expected None in OptionRef but got Value = ", *rep_);
  }

  template <typename SomeFn, typename NoneFn = Noop>
  constexpr decltype(auto) match(SomeFn && some_fn, NoneFn && none_fn = {})
  {
    if (is_some())
    {
      return some_fn(*rep_);
    }
    return none_fn();
  }

  template <typename SomeFn, typename NoneFn = Noop>
  constexpr decltype(auto) match(SomeFn && some_fn,
                                 NoneFn && none_fn = {}) const
  {
    if (is_some())
    {
      return some_fn(*rep_);
    }
    return none_fn();
  }
};

template <typename T>
OptionRef(T &) -> OptionRef<T>;

}    // namespace ash
