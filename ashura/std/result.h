#pragma once
#include "ashura/std/log.h"
#include <new>

namespace ash
{

template <typename T>
struct [[nodiscard]] Ok
{
  using type = T;
  T value{};
};

template <typename T>
Ok(T) -> Ok<T>;

template <typename E>
struct [[nodiscard]] Err
{
  using type = E;
  E value{};
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

  template <typename U>
  [[nodiscard]] constexpr bool contains(U const &cmp) const
  {
    if (is_ok_)
    {
      return value_ == cmp;
    }
    return false;
  }

  template <typename F>
  [[nodiscard]] constexpr bool contains_err(F const &cmp) const
  {
    if (is_ok_)
    {
      return false;
    }
    return error_ == cmp;
  }

  constexpr T &value()
  {
    if (is_ok_)
    {
      return value_;
    }
    default_logger->panic(".value() called on result with Error{=", error_, "}");
  }

  constexpr T const &value() const
  {
    if (is_ok_)
    {
      return value_;
    }
    default_logger->panic(".value() called on result with Error{=", error_, "}");
  }

  constexpr E &err()
  {
    if (!is_ok_)
    {
      return error_;
    }
    default_logger->panic(".err() called on result with Value{=", value_, "}");
  }

  constexpr E const &err() const
  {
    if (!is_ok_)
    {
      return error_;
    }
    default_logger->panic(".err() called on result with Value{=", value_, "}");
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
  constexpr auto map(Fn &&op)
  {
    using U = decltype(((Fn &&) op)(value_));
    if (is_ok_)
    {
      return Result<U, E>{Ok<U>{((Fn &&) op)(value_)}};
    }
    return Result<U, E>{Err{error_}};
  }

  template <typename Fn, typename U>
  constexpr auto map_or(Fn &&op, U &&alt)
  {
    if (is_ok_)
    {
      return ((Fn &&) op)(value_);
    }
    return (U &&) alt;
  }

  template <typename Fn, typename AltFn>
  constexpr auto map_or_else(Fn &&op, AltFn &&alt_op)
  {
    if (is_ok_)
    {
      return ((Fn &&) op)(value_);
    }
    return ((AltFn &&) alt_op)(error_);
  }

  template <typename Fn>
  constexpr auto and_then(Fn &&op)
  {
    using OutResult = decltype(((Fn &&) op)(value_));
    if (is_ok_)
    {
      return ((Fn &&) op)(value_);
    }
    return OutResult{Err{error_}};
  }

  template <typename Fn>
  constexpr auto or_else(Fn &&op)
  {
    using OutResult = decltype(((Fn &&) op)(error_));
    if (is_ok_)
    {
      return OutResult{Ok{value_}};
    }
    return ((Fn &&) op)(error_);
  }

  template <typename U>
  constexpr T unwrap_or(U &&alt)
  {
    if (is_ok_)
    {
      return (T &&) value_;
    }
    return (U &&) alt;
  }

  template <typename Fn>
  constexpr T unwrap_or_else(Fn &&op)
  {
    if (is_ok_)
    {
      return (T &&) value_;
    }
    return ((Fn &&) op)(error_);
  }

  constexpr T unwrap()
  {
    if (is_ok_)
    {
      return (T &&) value_;
    }
    default_logger->panic("Expected Value in Result but got Error{=", error_, "}");
  }

  constexpr T expect(char const *msg)
  {
    if (is_ok_)
    {
      return (T &&) value_;
    }
    default_logger->panic(msg);
  }

  constexpr E unwrap_err()
  {
    if (!is_ok_)
    {
      return (E &&) error_;
    }
    default_logger->panic("Expected Error in Result but got Value{=", value_, "}");
  }

  constexpr E expect_err(char const *msg)
  {
    if (!is_ok_)
    {
      return (E &&) error_;
    }
    default_logger->panic(msg);
  }

  template <typename OkFn, typename ErrFn>
  constexpr auto match(OkFn &&ok_fn, ErrFn &&err_fn)
  {
    if (is_ok_)
    {
      return ok_fn(value_);
    }
    return err_fn(error_);
  }

  template <typename OkFn, typename ErrFn>
  constexpr auto match(OkFn &&ok_fn, ErrFn &&err_fn) const
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
