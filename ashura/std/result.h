#pragma once
#include "ashura/std/panic.h"
#include <new>

namespace ash
{

template <typename T>
struct [[nodiscard]] Ok
{
  using type = T;
  T value    = {};
};

template <typename T>
Ok(T) -> Ok<T>;

template <typename E>
struct [[nodiscard]] Err
{
  using type = E;
  E value    = {};
};

template <typename T>
Err(T) -> Err<T>;

template <typename T, typename E>
struct [[nodiscard]] Result
{
  using value_type = T;
  using error_type = E;

  union
  {
    T    value_;
    E    error_;
    char stub_;
  };

  bool is_ok_ = false;

  constexpr Result() = delete;

  constexpr ~Result()
  {
    if (is_ok_)
    {
      value_.~T();
    }
    else
    {
      error_.~E();
    }
  }

  constexpr Result(Ok<T> ok) : value_{(T &&) ok.value}, is_ok_{true}
  {
  }

  constexpr Result(Err<E> err) : error_{(E &&) err.value}, is_ok_{false}
  {
  }

  constexpr Result(Result &&other) : stub_{}, is_ok_{}
  {
    if (other.is_ok_)
    {
      new (&value_) T{(T &&) other.value_};
    }
    else
    {
      new (&error_) E{(E &&) other.error_};
    }
    is_ok_ = other.is_ok_;
  }

  constexpr Result &operator=(Result &&other)
  {
    if (is_ok_)
    {
      value_.~T();
    }
    else
    {
      error_.~E();
    }

    if (other.is_ok_)
    {
      new (&value_) T{(T &&) other.value_};
    }
    else
    {
      new (&error_) E{(E &&) other.error_};
    }

    is_ok_ = other.is_ok_;
    return *this;
  }

  constexpr Result &operator=(Ok<T> other)
  {
    if (is_ok_)
    {
      value_.~T();
    }
    else
    {
      error_.~E();
    }

    new (&value_) T{(T &&) other.value};
    is_ok_ = true;
    return *this;
  }

  constexpr Result &operator=(Err<T> other)
  {
    if (is_ok_)
    {
      value_.~T();
    }
    else
    {
      error_.~E();
    }

    new (&error_) E{(E &&) other.value};
    is_ok_ = false;
    return *this;
  }

  constexpr Result(Result const &other) : stub_{}, is_ok_{}
  {
    if (other.is_ok_)
    {
      new (&value_) T{other.value_};
    }
    else
    {
      new (&error_) E{other.error_};
    }
    is_ok_ = other.is_ok_;
  }

  constexpr Result &operator=(Result const &other)
  {
    if (is_ok_)
    {
      value_.~T();
    }
    else
    {
      error_.~E();
    }

    if (other.is_ok_)
    {
      new (&value_) T{other.value_};
    }
    else
    {
      new (&error_) E{other.error_};
    }

    is_ok_ = other.is_ok_;
    return *this;
  }

  [[nodiscard]] constexpr bool is_ok() const
  {
    return is_ok_;
  }

  [[nodiscard]] constexpr bool is_err() const
  {
    return !is_ok_;
  }

  [[nodiscard]] constexpr operator bool() const
  {
    return is_ok_;
  }

  template <typename CmpType>
  [[nodiscard]] constexpr bool contains(CmpType const &cmp) const
  {
    if (is_ok_)
    {
      return value_ == cmp;
    }
    return false;
  }

  template <typename ErrCmp>
  [[nodiscard]] constexpr bool contains_err(ErrCmp const &cmp) const
  {
    if (is_ok_)
    {
      return false;
    }
    return error_ == cmp;
  }

  [[nodiscard]] constexpr T &value()
  {
    if (is_ok_)
    {
      return value_;
    }
    panic_logger.panic(".value() called on result with Error{=", error_, "}");
  }

  [[nodiscard]] constexpr T const &value() const
  {
    if (is_ok_)
    {
      return value_;
    }
    panic_logger.panic(".value() called on result with Error{=", error_, "}");
  }

  [[nodiscard]] constexpr E &err()
  {
    if (!is_ok_)
    {
      return error_;
    }
    panic_logger.panic(".err() called on result with Value{=", value_, "}");
  }

  [[nodiscard]] constexpr E const &err() const
  {
    if (!is_ok_)
    {
      return error_;
    }
    panic_logger.panic(".err() called on result with Value{=", value_, "}");
  }

  constexpr Result<T const *, E const *> as_ref() const
  {
    if (is_ok_)
    {
      return Ok{&value_};
    }
    return Err{&error_};
  }

  constexpr Result<T *, E *> as_ref()
  {
    if (is_ok_)
    {
      return Ok{&value_};
    }
    return Err{&error_};
  }

  template <typename Fn>
  [[nodiscard]] constexpr auto map(Fn op)
  {
    using U = decltype(op(value_));
    if (is_ok_)
    {
      return Result<U, E>{Ok<U>{op(value_)}};
    }
    return Result<U, E>{Err{error_}};
  }

  template <typename Fn, typename AltType>
  [[nodiscard]] constexpr auto map_or(Fn op, AltType alt)
  {
    if (is_ok_)
    {
      return op(value_);
    }
    return (AltType &&) alt;
  }

  template <typename Fn, typename AltFn>
  [[nodiscard]] constexpr auto map_or_else(Fn op, AltFn alt_op)
  {
    if (is_ok_)
    {
      return op(value_);
    }
    return alt_op(error_);
  }

  template <typename Fn>
  [[nodiscard]] constexpr auto map_err(Fn op)
  {
    using F = decltype(op(error_));
    if (!is_ok_)
    {
      return Result<T, F>{Err<F>{op(error_)}};
    }
    return Result<T, F>{Ok<T>{value_}};
  }

  template <typename Fn>
  [[nodiscard]] constexpr auto and_then(Fn op)
  {
    using U = decltype(op(value_));
    if (is_ok_)
    {
      return Result<U, E>{Ok{op(value_)}};
    }
    return Result<U, E>{Err{error_}};
  }

  template <typename Fn>
  [[nodiscard]] constexpr auto or_else(Fn op)
  {
    using OutResult = decltype(op(error_));
    if (is_ok_)
    {
      return OutResult{Ok{value_}};
    }
    return op(error_);
  }

  [[nodiscard]] constexpr T unwrap_or(T alt)
  {
    if (is_ok_)
    {
      return (T &&) value_;
    }
    return (T &&) alt;
  }

  template <typename Fn>
  [[nodiscard]] constexpr T unwrap_or_else(Fn op)
  {
    if (is_ok_)
    {
      return (T &&) value_;
    }
    return op(error_);
  }

  [[nodiscard]] constexpr T unwrap()
  {
    if (is_ok_)
    {
      return (T &&) value_;
    }
    panic_logger.panic("Expected Value in Result but got Error{=", error_, "}");
  }

  [[nodiscard]] constexpr T expect(char const *msg)
  {
    if (is_ok_)
    {
      return (T &&) value_;
    }
    panic_logger.panic(msg);
  }

  [[nodiscard]] constexpr E unwrap_err()
  {
    if (!is_ok_)
    {
      return (E &&) error_;
    }
    panic_logger.panic("Expected Error in Result but got Value{=", value_, "}");
  }

  [[nodiscard]] constexpr E expect_err(char const *msg)
  {
    if (!is_ok_)
    {
      return (E &&) error_;
    }
    panic_logger.panic(msg);
  }

  template <typename OkFn, typename ErrFn>
  [[nodiscard]] constexpr auto match(OkFn &&ok_fn, ErrFn &&err_fn)
  {
    if (is_ok_)
    {
      return ok_fn(value_);
    }
    return err_fn(error_);
  }

  template <typename OkFn, typename ErrFn>
  [[nodiscard]] constexpr auto match(OkFn ok_fn, ErrFn err_fn) const
  {
    if (is_ok_)
    {
      return ok_fn(value_);
    }
    return err_fn(error_);
  }
};

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(Ok<T> const &a, Ok<U> const &b)
{
  return a.value == b.value;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator!=(Ok<T> const &a, Ok<U> const &b)
{
  return a.value != b.value;
}

template <typename E, typename F>
[[nodiscard]] constexpr bool operator==(Err<E> const &a, Err<F> const &b)
{
  return a.value == b.value;
}

template <typename E, typename F>
[[nodiscard]] constexpr bool operator!=(Err<E> const &a, Err<F> const &b)
{
  return a.value != b.value;
}

template <typename T, typename E, typename U>
[[nodiscard]] constexpr bool operator==(Result<T, E> const &a, Ok<U> const &b)
{
  if (a.is_ok_)
  {
    return a.value_ == b.value;
  }
  return false;
}

template <typename T, typename E, typename U>
[[nodiscard]] constexpr bool operator!=(Result<T, E> const &a, Ok<U> const &b)
{
  if (a.is_ok_)
  {
    return a.value_ != b.value;
  }
  return true;
}

template <typename U, typename T, typename E>
[[nodiscard]] constexpr bool operator==(Ok<U> const &a, Result<T, E> const &b)
{
  if (b.is_ok_)
  {
    return a.value == b.value_;
  }
  return false;
}

template <typename U, typename T, typename E>
[[nodiscard]] constexpr bool operator!=(Ok<U> const &a, Result<T, E> const &b)
{
  if (b.is_ok_)
  {
    return a.value != b.value_;
  }
  return true;
}

template <typename T, typename E, typename U>
[[nodiscard]] constexpr bool operator==(Result<T, E> const &a, Err<U> const &b)
{
  if (a.is_err())
  {
    return a.error_ == b.value;
  }
  return false;
}

template <typename T, typename E, typename U>
[[nodiscard]] constexpr bool operator!=(Result<T, E> const &a, Err<U> const &b)
{
  if (a.is_err())
  {
    return a.error_ != b.value;
  }
  return true;
}

template <typename U, typename T, typename E>
[[nodiscard]] constexpr bool operator==(Err<U> const &a, Result<T, E> const &b)
{
  if (b.is_err())
  {
    return a.value == b.error_;
  }
  return false;
}

template <typename U, typename T, typename E>
[[nodiscard]] constexpr bool operator!=(Err<U> const &a, Result<T, E> const &b)
{
  if (b.is_err())
  {
    return a.value != b.error_;
  }
  return true;
}

template <typename T, typename E, typename U, typename F>
[[nodiscard]] constexpr bool operator==(Result<T, E> const &a,
                                        Result<U, F> const &b)
{
  if (a.is_ok_ && b.is_ok_)
  {
    return a.value_ == b.value_;
  }
  if (a.is_err() && b.is_err())
  {
    return a.error_ == b.error_;
  }
  return false;
}

template <typename T, typename E, typename U, typename F>
[[nodiscard]] constexpr bool operator!=(Result<T, E> const &a,
                                        Result<U, F> const &b)
{
  if (a.is_ok_ && b.is_ok_)
  {
    return a.value_ != b.value_;
  }
  if (a.is_err() && b.is_err())
  {
    return a.error_ != b.error_;
  }
  return true;
}
}        // namespace ash

#define STX_TRY_OK_IMPL_(STX_ARG_UNIQUE_PLACEHOLDER, qualifier_identifier,    \
                         ...)                                                 \
  static_assert(!::std::is_const_v<decltype((__VA_ARGS__))>,                  \
                "the expression: ' " #__VA_ARGS__                             \
                " ' evaluates to a const and is not mutable");                \
  static_assert(                                                              \
      !::std::is_lvalue_reference_v<decltype((__VA_ARGS__))>,                 \
      "the expression: ' " #__VA_ARGS__                                       \
      " ' evaluates to an l-value reference, 'TRY_OK' only accepts r-values " \
      "and r-value references ");                                             \
  decltype((__VA_ARGS__)) &&STX_ARG_UNIQUE_PLACEHOLDER = (__VA_ARGS__);       \
                                                                              \
  if (STX_ARG_UNIQUE_PLACEHOLDER.is_err())                                    \
  {                                                                           \
    return std::move(STX_ARG_UNIQUE_PLACEHOLDER.err_);                        \
  }                                                                           \
                                                                              \
  typename std::remove_reference_t<decltype((__VA_ARGS__))>::value_type       \
      qualifier_identifier = STX_ARG_UNIQUE_PLACEHOLDER.val_.move();

#define TRY_OK(qualifier_identifier, ...)                           \
  STX_TRY_OK_IMPL_(                                                 \
      STX_WITH_UNIQUE_SUFFIX_(STX_TRY_OK_PLACEHOLDER, __COUNTER__), \
      qualifier_identifier, __VA_ARGS__)