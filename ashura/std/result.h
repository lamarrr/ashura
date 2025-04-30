/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/error.h"

namespace ash
{

template <typename T = Void>
struct [[nodiscard]] Ok
{
  using Type = T;

  T v{};
};

template <typename T>
Ok(T) -> Ok<T>;

template <typename E = Void>
struct [[nodiscard]] Err
{
  using Type = E;

  E v{};
};

template <typename T>
Err(T) -> Err<T>;

template <typename T = Void, typename E = Void>
struct [[nodiscard]] Result
{
  using Type    = T;
  using ErrType = E;

  bool is_ok_;

  union
  {
    T v0_;
    E v1_;
  };

  constexpr Result() = delete;

  constexpr ~Result()
  {
    if (is_ok_)
    {
      v0_.~T();
    }
    else
    {
      v1_.~E();
    }
  }

  constexpr Result(Ok<T> ok) : is_ok_{true}, v0_{static_cast<T &&>(ok.v)}
  {
  }

  constexpr Result(Err<E> err) : is_ok_{false}, v1_{static_cast<E &&>(err.v)}
  {
  }

  constexpr Result(Result && other) : is_ok_{other.is_ok_}
  {
    if (other.is_ok_)
    {
      new (&v0_) T{static_cast<T &&>(other.v0_)};
    }
    else
    {
      new (&v1_) E{static_cast<E &&>(other.v1_)};
    }
  }

  constexpr Result & operator=(Result && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    if (is_ok_)
    {
      v0_.~T();
    }
    else
    {
      v1_.~E();
    }

    is_ok_ = other.is_ok_;

    if (other.is_ok_)
    {
      new (&v0_) T{static_cast<T &&>(other.v0_)};
    }
    else
    {
      new (&v1_) E{static_cast<E &&>(other.v1_)};
    }

    return *this;
  }

  constexpr Result & operator=(Ok<T> other)
  {
    if (is_ok_)
    {
      v0_.~T();
    }
    else
    {
      v1_.~E();
    }

    is_ok_ = true;
    new (&v0_) T{static_cast<T &&>(other.v)};
    return *this;
  }

  constexpr Result & operator=(Err<T> other)
  {
    if (is_ok_)
    {
      v0_.~T();
    }
    else
    {
      v1_.~E();
    }

    is_ok_ = false;
    new (&v1_) E{static_cast<E &&>(other.v)};
    return *this;
  }

  constexpr Result(Result const & other) : is_ok_{other.is_ok_}
  {
    if (other.is_ok_)
    {
      new (&v0_) T{other.v0_};
    }
    else
    {
      new (&v1_) E{other.v1_};
    }
  }

  constexpr Result & operator=(Result const & other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    if (is_ok_)
    {
      v0_.~T();
    }
    else
    {
      v1_.~E();
    }

    is_ok_ = other.is_ok_;

    if (other.is_ok_)
    {
      new (&v0_) T{other.v0_};
    }
    else
    {
      new (&v1_) E{other.v1_};
    }

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

  [[nodiscard]] explicit constexpr operator bool() const
  {
    return is_ok();
  }

  template <typename U>
  [[nodiscard]] constexpr bool contains(U const & cmp) const
  {
    if (is_ok())
    {
      return v0_ == cmp;
    }
    return false;
  }

  template <typename F>
  [[nodiscard]] constexpr bool contains_err(F const & cmp) const
  {
    if (is_ok())
    {
      return false;
    }
    return v1_ == cmp;
  }

  constexpr T & v(SourceLocation loc = SourceLocation::current())
  {
    CHECK_SLOC(loc, is_ok(), ".v() called on Result with Err = {}", v1_);
    return v0_;
  }

  constexpr T const & v(SourceLocation loc = SourceLocation::current()) const
  {
    CHECK_SLOC(loc, is_ok(), ".v() called on Result with Err = {}", v1_);
    return v0_;
  }

  constexpr E & err(SourceLocation loc = SourceLocation::current())
  {
    CHECK_SLOC(loc, is_err(), ".err() called on Result with Ok = {}", v0_);
    return v1_;
  }

  constexpr E const & err(SourceLocation loc = SourceLocation::current()) const
  {
    CHECK_SLOC(loc, is_err(), ".err() called on Result with Ok = {}", v0_);
    return v1_;
  }

  constexpr void discard()
  {
  }

  constexpr Result<T const *, E const *> as_ptr() const
  {
    if (is_ok())
    {
      return Ok{&v0_};
    }
    return Err{&v1_};
  }

  constexpr Result<T *, E *> as_ptr()
  {
    if (is_ok())
    {
      return Ok{&v0_};
    }
    return Err{&v1_};
  }

  template <typename Fn>
  constexpr auto map(Fn && op)
  {
    using U = decltype(static_cast<Fn &&>(op)(v0_));
    if (is_ok())
    {
      return Result<U, E>{Ok<U>{static_cast<Fn &&>(op)(v0_)}};
    }
    return Result<U, E>{Err{v1_}};
  }

  template <typename Fn, typename U>
  constexpr auto map_or(Fn && op, U && alt)
  {
    if (is_ok())
    {
      return static_cast<Fn &&>(op)(v0_);
    }
    return static_cast<U &&>(alt);
  }

  template <typename Fn, typename AltFn>
  constexpr decltype(auto) map_or_else(Fn && op, AltFn && alt_op)
  {
    if (is_ok())
    {
      return static_cast<Fn &&>(op)(v0_);
    }
    return static_cast<AltFn &&>(alt_op)(v1_);
  }

  template <typename Fn>
  constexpr auto and_then(Fn && op)
  {
    using OutResult = decltype(static_cast<Fn &&>(op)(v0_));
    if (is_ok())
    {
      return static_cast<Fn &&>(op)(v0_);
    }
    return OutResult{Err{v1_}};
  }

  template <typename Fn>
  constexpr auto or_else(Fn && op)
  {
    using OutResult = decltype(static_cast<Fn &&>(op)(v1_));
    if (is_ok())
    {
      return OutResult{Ok{v0_}};
    }
    return static_cast<Fn &&>(op)(v1_);
  }

  template <typename U>
  constexpr T unwrap_or(U && alt)
  {
    if (is_ok())
    {
      return static_cast<T &&>(v0_);
    }
    return static_cast<U &&>(alt);
  }

  template <typename Fn>
  constexpr T unwrap_or_else(Fn && op)
  {
    if (is_ok())
    {
      return static_cast<T &&>(v0_);
    }
    return static_cast<Fn &&>(op)(v1_);
  }

  constexpr T unwrap(Str            msg = ""_str,
                     SourceLocation loc = SourceLocation::current())
  {
    CHECK_SLOC(loc, is_ok(), "Expected Value in Result but got Err = {}. {}",
               v1_, msg);
    return static_cast<T &&>(v0_);
  }

  constexpr E unwrap_err(Str            msg = ""_str,
                         SourceLocation loc = SourceLocation::current())
  {
    CHECK_SLOC(loc, is_err(), "Expected Err in Result but got Value = {}. {}",
               v0_, msg);
    return static_cast<E &&>(v1_);
  }

  template <typename OkFn, typename ErrFn>
  constexpr decltype(auto) match(OkFn && ok, ErrFn && err)
  {
    if (is_ok())
    {
      return ok(v0_);
    }
    return err(v1_);
  }

  template <typename OkFn, typename ErrFn>
  constexpr decltype(auto) match(OkFn && ok, ErrFn && err) const
  {
    if (is_ok())
    {
      return ok(v0_);
    }
    return err(v1_);
  }
};

template <typename T, typename E>
struct IsTriviallyRelocatable<Result<T, E>>
{
  static constexpr bool value =
    TriviallyRelocatable<T> && TriviallyRelocatable<E>;
};

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(Ok<T> const & a, Ok<U> const & b)
{
  return a.v == b.v;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator!=(Ok<T> const & a, Ok<U> const & b)
{
  return a.v != b.v;
}

template <typename E, typename F>
[[nodiscard]] constexpr bool operator==(Err<E> const & a, Err<F> const & b)
{
  return a.v == b.v;
}

template <typename E, typename F>
[[nodiscard]] constexpr bool operator!=(Err<E> const & a, Err<F> const & b)
{
  return a.v != b.v;
}

template <typename T, typename E, typename U>
[[nodiscard]] constexpr bool operator==(Result<T, E> const & a, Ok<U> const & b)
{
  if (a.is_ok())
  {
    return a.v0_ == b.v;
  }
  return false;
}

template <typename T, typename E, typename U>
[[nodiscard]] constexpr bool operator!=(Result<T, E> const & a, Ok<U> const & b)
{
  if (a.is_ok())
  {
    return a.v0_ != b.v;
  }
  return true;
}

template <typename U, typename T, typename E>
[[nodiscard]] constexpr bool operator==(Ok<U> const & a, Result<T, E> const & b)
{
  if (b.is_ok())
  {
    return a.v == b.v0_;
  }
  return false;
}

template <typename U, typename T, typename E>
[[nodiscard]] constexpr bool operator!=(Ok<U> const & a, Result<T, E> const & b)
{
  if (b.is_ok())
  {
    return a.v != b.v0_;
  }
  return true;
}

template <typename T, typename E, typename U>
[[nodiscard]] constexpr bool operator==(Result<T, E> const & a,
                                        Err<U> const &       b)
{
  if (a.is_err())
  {
    return a.v1_ == b.v;
  }
  return false;
}

template <typename T, typename E, typename U>
[[nodiscard]] constexpr bool operator!=(Result<T, E> const & a,
                                        Err<U> const &       b)
{
  if (a.is_err())
  {
    return a.v1_ != b.v;
  }
  return true;
}

template <typename U, typename T, typename E>
[[nodiscard]] constexpr bool operator==(Err<U> const &       a,
                                        Result<T, E> const & b)
{
  if (b.is_err())
  {
    return a.v == b.v1_;
  }
  return false;
}

template <typename U, typename T, typename E>
[[nodiscard]] constexpr bool operator!=(Err<U> const &       a,
                                        Result<T, E> const & b)
{
  if (b.is_err())
  {
    return a.v != b.v1_;
  }
  return true;
}

template <typename T, typename E, typename U, typename F>
[[nodiscard]] constexpr bool operator==(Result<T, E> const & a,
                                        Result<U, F> const & b)
{
  if (a.is_ok() && b.is_ok())
  {
    return a.v0_ == b.v0_;
  }
  if (a.is_err() && b.is_err())
  {
    return a.v1_ == b.v1_;
  }
  return false;
}

template <typename T, typename E, typename U, typename F>
[[nodiscard]] constexpr bool operator!=(Result<T, E> const & a,
                                        Result<U, F> const & b)
{
  if (a.is_ok() && b.is_ok())
  {
    return a.v0_ != b.v0_;
  }
  if (a.is_err() && b.is_err())
  {
    return a.v1_ != b.v1_;
  }
  return true;
}
}    // namespace ash
