#pragma once
#include "ashura/std/panic.h"
#include <new>

namespace ash
{

template <typename T>
struct [[nodiscard]] Some
{
  using type = T;
  T value    = {};
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
      value_->~T();
    }
    new (&value_) T{(T &&) other.value};
    is_some_ = true;
    return *this;
  }

  constexpr Option &operator=(NoneType)
  {
    if (is_some_)
    {
      value_->~T();
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
      value_->~T();
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
      return some_.value == cmp;
    }
    else
    {
      return false;
    }
  }

  [[nodiscard]] constexpr T &value()
  {
    if (is_some_)
    {
      return value_;
    }
    panic_logger.panic("Expected value in Option but got None");
  }

  [[nodiscard]] constexpr T const &value() const
  {
    if (is_some_)
    {
      return value_;
    }
    panic_logger.panic("Expected value in Option but got None");
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

  [[nodiscard]] constexpr T expect(char const *msg)
  {
    if (is_some_)
    {
      return (T &&) value_;
    }
    panic_logger.panic(msg);
  }

  [[nodiscard]] constexpr T unwrap()
  {
    if (is_some_)
    {
      return (T &&) value_;
    }
    panic_logger.panic("Expected value in Option but got None");
  }

  [[nodiscard]] constexpr T unwrap_or(T alt)
  {
    if (is_some_)
    {
      return (T &&) value_;
    }
    return (T &&) alt;
  }

  template <typename Fn>
  [[nodiscard]] constexpr T unwrap_or_else(Fn op)
  {
    if (is_some_)
    {
      return (T &&) value_;
    }
    return op();
  }

  template <typename Fn>
  [[nodiscard]] constexpr auto map(Fn op)
  {
    using return_type = decltype(op(value_));
    if (is_some_)
    {
      return Option<return_type>{Some(op(value_))};
    }
    return None;
  }

  template <typename Fn, typename Alt>
  [[nodiscard]] constexpr auto map_or(Fn op, Alt alt)
  {
    if (is_some_)
    {
      return op(value_);
    }
    return (Alt &&) alt;
  }

  template <typename Fn, typename AltFn>
  [[nodiscard]] constexpr auto map_or_else(Fn op, AltFn alt_fn)
  {
    if (is_some_)
    {
      return op(value_);
    }
    return alt_fn();
  }

  constexpr void expect_none(char const *msg)
  {
    if (is_some_)
    {
      panic_logger.panic(msg);
    }
  }

  constexpr void unwrap_none()
  {
    if (is_some_)
    {
      panic_logger.panic("Expected value in Option but got None");
    }
  }

  template <typename SomeFn, typename NoneFn>
  [[nodiscard]] constexpr auto match(SomeFn some_fn, NoneFn none_fn)
  {
    if (is_some_)
    {
      return some_fn(value_);
    }
    return none_fn();
  }

  template <typename SomeFn, typename NoneFn>
  [[nodiscard]] constexpr auto match(SomeFn some_fn, NoneFn none_fn) const
  {
    if (is_some_)
    {
      return some_fn(value_);
    }
    return none_fn();
  }
};

template <typename T>
Option(T) -> Option<T>;

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
    return a.value_ == b;
  }
  return false;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator!=(Option<T> const &a, Some<U> const &b)
{
  if (a.is_some_)
  {
    return a.value_ != b;
  }
  return true;
}

template <typename U, typename T>
[[nodiscard]] constexpr bool operator==(Some<U> const &a, Option<T> const &b)
{
  if (b.is_some_)
  {
    return a == b.value_;
  }
  return false;
}

template <typename U, typename T>
[[nodiscard]] constexpr bool operator!=(Some<U> const &a, Option<T> const &b)
{
  if (b.is_some_)
  {
    return a != b.value_;
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

#define STX_TRY_SOME_IMPL_(STX_ARG_UNIQUE_PLACEHOLDER, qualifier_identifier, \
                           ...)                                              \
  static_assert(!::std::is_const_v<decltype((__VA_ARGS__))>,                 \
                "the expression: ' " #__VA_ARGS__                            \
                " ' evaluates to a const and is not mutable");               \
  static_assert(!::std::is_lvalue_reference_v<decltype((__VA_ARGS__))>,      \
                "the expression: ' " #__VA_ARGS__                            \
                " ' evaluates to an l-value reference, 'TRY_SOME' only "     \
                "accepts r-values "                                          \
                "and r-value references ");                                  \
  decltype((__VA_ARGS__)) &&STX_ARG_UNIQUE_PLACEHOLDER = (__VA_ARGS__);      \
                                                                             \
  if (STX_ARG_UNIQUE_PLACEHOLDER.is_none())                                  \
  {                                                                          \
    return ::stx::None;                                                      \
  }                                                                          \
                                                                             \
  typename std::remove_reference_t<decltype((__VA_ARGS__))>::type            \
      qualifier_identifier = STX_ARG_UNIQUE_PLACEHOLDER.val_.move();

#define TRY_SOME(qualifier_identifier, ...)                           \
  STX_TRY_SOME_IMPL_(                                                 \
      STX_WITH_UNIQUE_SUFFIX_(STX_TRY_SOME_PLACEHOLDER, __COUNTER__), \
      qualifier_identifier, __VA_ARGS__)
