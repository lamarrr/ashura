#pragma once
#include "ashura/std/log.h"
#include <new>

namespace ash
{

template <typename T>
struct [[nodiscard]] Some
{
  using type = T;
  T value{};
};

template <typename T>
Some(T) -> Some<T>;

struct NoneType
{
};

constexpr NoneType None;

template <typename T>
struct [[nodiscard]] Option
{
  using type = T;

  union
  {
    T    value_;
    char stub_;
  };

  bool is_some_ = false;

  constexpr Option() : stub_{}, is_some_{false}
  {
  }

  constexpr ~Option()
  {
    if (is_some_)
    {
      value_.~T();
    }
  }

  constexpr Option(Some<T> some) : value_{(T &&) some.value}, is_some_{true}
  {
  }

  constexpr Option(NoneType) : stub_{}, is_some_{false}
  {
  }

  constexpr Option &operator=(Some<T> other)
  {
    if (is_some_)
    {
      value_.~T();
    }
    new (&value_) T{(T &&) other.value};
    is_some_ = true;
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

  constexpr Option(Option &&other) : stub_{}, is_some_{false}
  {
    if (other.is_some_)
    {
      new (&value_) T{(T &&) other.value_};
      is_some_ = true;
    }
  }

  constexpr Option &operator=(Option &&other)
  {
    if (is_some_)
    {
      value_.~T();
    }

    if (other.is_some_)
    {
      new (&value_) T{(T &&) other.value_};
      is_some_ = true;
    }
    else
    {
      is_some_ = false;
    }
    return *this;
  }

  constexpr Option(Option const &other) : stub_{}, is_some_{false}
  {
    if (other.is_some_)
    {
      new (&value_) T{other.value_};
      is_some_ = true;
    }
  }

  constexpr Option &operator=(Option const &other)
  {
    if (is_some_)
    {
      value_.~T();
    }
    if (other.is_some_)
    {
      new (&value_) T{other.value_};
      is_some_ = true;
    }
    else
    {
      is_some_ = false;
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

  [[nodiscard]] constexpr operator bool() const
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

  constexpr T &value()
  {
    if (is_some_)
    {
      return value_;
    }
    default_logger->panic("Expected value in Option but got None");
  }

  constexpr T const &value() const
  {
    if (is_some_)
    {
      return value_;
    }
    default_logger->panic("Expected value in Option but got None");
  }

  constexpr Option<T const *> as_ref() const
  {
    if (is_some_)
    {
      return Some(&value_);
    }
    return None;
  }

  constexpr Option<T *> as_ref()
  {
    if (is_some_)
    {
      return Some(&value_);
    }
    return None;
  }

  constexpr T expect(char const *msg)
  {
    if (is_some_)
    {
      return (T &&) value_;
    }
    default_logger->panic(msg);
  }

  constexpr T unwrap()
  {
    if (is_some_)
    {
      return (T &&) value_;
    }
    default_logger->panic("Expected value in Option but got None");
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
  constexpr auto map_or_else(Fn op, AltFn alt_fn)
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

  constexpr void expect_none(char const *msg)
  {
    if (is_some_)
    {
      default_logger->panic(msg);
    }
  }

  constexpr void unwrap_none()
  {
    if (is_some_)
    {
      default_logger->panic("Expected value in Option but got None");
    }
  }

  template <typename SomeFn, typename NoneFn>
  constexpr auto match(SomeFn &&some_fn, NoneFn &&none_fn)
  {
    if (is_some_)
    {
      return some_fn(value_);
    }
    return none_fn();
  }

  template <typename SomeFn, typename NoneFn>
  constexpr auto match(SomeFn &&some_fn, NoneFn &&none_fn) const
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
