/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/error.h"
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
  using Type      = T;
  using Iter      = SpanIter<T>;
  using ConstIter = SpanIter<T const>;
  using View      = Span<T>;
  using ConstView = Span<T const>;

  bool is_some_;

  union
  {
    T v0_;
  };

  constexpr Option() : is_some_{false}
  {
  }

  constexpr ~Option()
  {
    if (is_some_)
    {
      v0_.~T();
    }
  }

  constexpr Option(T some) : is_some_{true}, v0_{static_cast<T &&>(some)}
  {
  }

  template <typename... Args>
  explicit constexpr Option(V<0>, Args &&... args) :
    is_some_{true},
    v0_{static_cast<Args &&>(args)...}
  {
  }

  constexpr Option(None) : is_some_{false}
  {
  }

  constexpr Option & operator=(T other)
  {
    if (is_some_)
    {
      v0_.~T();
    }
    is_some_ = true;
    new (&v0_) T{static_cast<T &&>(other)};
    return *this;
  }

  constexpr Option & operator=(None)
  {
    if (is_some_)
    {
      v0_.~T();
    }
    is_some_ = false;
    return *this;
  }

  constexpr Option(Option && other) : is_some_{other.is_some_}
  {
    if (other.is_some_)
    {
      new (&v0_) T{static_cast<T &&>(other.v0_)};
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
      v0_.~T();
    }

    is_some_ = other.is_some_;

    if (other.is_some_)
    {
      new (&v0_) T{static_cast<T &&>(other.v0_)};
    }

    return *this;
  }

  constexpr Option(Option const & other) : is_some_{other.is_some_}
  {
    if (other.is_some_)
    {
      new (&v0_) T{other.v0_};
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
      v0_.~T();
    }

    is_some_ = other.is_some_;

    if (other.is_some_)
    {
      new (&v0_) T{other.v0_};
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
      return v0_ == cmp;
    }
    else
    {
      return false;
    }
  }

  constexpr T & v(SourceLocation loc = SourceLocation::current())
  {
    CHECK_SLOC(loc, is_some(), "Expected Value in Option but got None");
    return v0_;
  }

  constexpr T const & v(SourceLocation loc = SourceLocation::current()) const
  {
    CHECK_SLOC(loc, is_some(), "Expected Value in Option but got None");
    return v0_;
  }

  constexpr Option<T const *> as_ptr() const
  {
    if (is_some())
    {
      return &v0_;
    }
    return none;
  }

  constexpr Option<T *> as_ptr()
  {
    if (is_some())
    {
      return &v0_;
    }
    return none;
  }

  constexpr T unwrap(Str            msg = ""_str,
                     SourceLocation loc = SourceLocation::current())
  {
    CHECK_SLOC(loc, is_some(), "Expected Value in Option but got None. {}",
               msg);
    return static_cast<T &&>(v0_);
  }

  constexpr T unwrap(Str            msg = ""_str,
                     SourceLocation loc = SourceLocation::current()) const
  {
    CHECK_SLOC(loc, is_some(), "Expected Value in Option but got None. {}",
               msg);
    return v0_;
  }

  template <typename... U>
  constexpr T unwrap_or(U &&... alt)
  {
    if (is_some())
    {
      return static_cast<T &&>(v0_);
    }
    return T{static_cast<U &&>(alt)...};
  }

  template <typename... U>
  constexpr T unwrap_or(U &&... alt) const
  {
    if (is_some())
    {
      return static_cast<T>(v0_);
    }
    return T{static_cast<U &&>(alt)...};
  }

  template <typename Fn>
  constexpr auto map(Fn && op)
  {
    using U = decltype(op(v0_));
    if (is_some())
    {
      return Option<U>{op(v0_)};
    }
    return Option<U>{none};
  }

  template <typename Fn>
  constexpr auto map(Fn && op) const
  {
    using U = decltype(op(v0_));
    if (is_some())
    {
      return Option<U>{op(v0_)};
    }
    return Option<U>{none};
  }

  template <typename Fn, typename... U>
  constexpr auto map_or(Fn && op, U &&... alt)
  {
    if (is_some())
    {
      return op(v0_);
    }
    return T{static_cast<U &&>(alt)...};
  }

  template <typename Fn, typename... U>
  constexpr auto map_or(Fn && op, U &&... alt) const
  {
    if (is_some())
    {
      return op(v0_);
    }
    return T{static_cast<U &&>(alt)...};
  }

  template <typename Fn>
  constexpr auto and_then(Fn && op)
  {
    using OutOption = decltype(op(v0_));
    if (is_some())
    {
      return op(v0_);
    }
    return OutOption{none};
  }

  template <typename Fn>
  constexpr auto and_then(Fn && op) const
  {
    using OutOption = decltype(op(v0_));
    if (is_some())
    {
      return op(v0_);
    }
    return OutOption{none};
  }

  constexpr void unwrap_none(Str            msg = ""_str,
                             SourceLocation loc = SourceLocation::current())
  {
    CHECK_SLOC(loc, is_none(), "Expected None in Option but got Value = {}. {}",
               v0_, msg);
  }

  template <typename Some, typename NoneFn = Noop>
  constexpr decltype(auto) match(Some && some, NoneFn && none = {})
  {
    if (is_some())
    {
      return some(v0_);
    }
    return none();
  }

  template <typename Some, typename NoneFn = Noop>
  constexpr decltype(auto) match(Some && some, NoneFn && none = {}) const
  {
    if (is_some())
    {
      return some(v0_);
    }
    return none();
  }

  constexpr auto begin()
  {
    return Iter{.iter_ = &v0_, .end_ = &v0_ + (is_some() ? 1 : 0)};
  }

  constexpr auto begin() const
  {
    return ConstIter{.iter_ = &v0_, .end_ = &v0_ + (is_some() ? 1 : 0)};
  }

  constexpr auto end() const
  {
    return IterEnd{};
  }

  constexpr View view()
  {
    return View{&v0_, is_some() ? 1 : 0};
  }

  constexpr ConstView view() const
  {
    return ConstView{&v0_, is_some() ? 1 : 0};
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
    return a.v0_ == b.v0_;
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
    return a.v0_ != b.v0_;
  }
  return true;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(Option<T> const & a, U const & b)
{
  if (a.is_some())
  {
    return a.v0_ == b;
  }
  return false;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator!=(Option<T> const & a, U const & b)
{
  if (a.is_some())
  {
    return a.v0_ != b;
  }
  return true;
}

template <typename U, typename T>
[[nodiscard]] constexpr bool operator==(U const & a, Option<T> const & b)
{
  if (b.is_some())
  {
    return a == b.v0_;
  }
  return false;
}

template <typename U, typename T>
[[nodiscard]] constexpr bool operator!=(U const & a, Option<T> const & b)
{
  if (b.is_some())
  {
    return a != b.v0_;
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
struct [[nodiscard]] Option<T &>
{
  using Type = T;
  using Repr = T *;

  T * repr_;

  constexpr Option() : repr_{nullptr}
  {
  }

  constexpr ~Option() = default;

  constexpr Option(T & some) : repr_{&some}
  {
  }

  template <typename... Args>
  explicit constexpr Option(V<0>, T & some) : repr_{&some}
  {
  }

  constexpr Option(None) : repr_{nullptr}
  {
  }

  constexpr Option & operator=(T & other)
  {
    repr_ = &other;
    return *this;
  }

  constexpr Option & operator=(None)
  {
    repr_ = nullptr;
    return *this;
  }

  constexpr Option(Option && other) = default;

  constexpr Option & operator=(Option && other) = default;

  constexpr Option(Option const & other) = default;

  constexpr Option & operator=(Option const & other) = default;

  [[nodiscard]] constexpr bool is_some() const
  {
    return repr_ != nullptr;
  }

  [[nodiscard]] constexpr bool is_none() const
  {
    return repr_ == nullptr;
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
      return *repr_ == cmp;
    }
    else
    {
      return false;
    }
  }

  constexpr T & v(SourceLocation loc = SourceLocation::current()) const
  {
    CHECK_SLOC(loc, is_some(), "Expected Value in Option but got None");
    return *repr_;
  }

  constexpr T & unwrap(Str            msg = ""_str,
                       SourceLocation loc = SourceLocation::current()) const
  {
    CHECK_SLOC(loc, is_some(), "Expected Value in Option but got None. {}",
               msg);
    return *repr_;
  }

  constexpr T & unwrap_or(T & alt) const
  {
    if (is_some())
    {
      return *repr_;
    }
    return alt;
  }

  constexpr Option<T> unref() const
  {
    if (is_some())
    {
      return *repr_;
    }
    return none;
  }

  template <typename Fn>
  constexpr auto map(Fn && op) const
  {
    using U = decltype(op(*repr_));
    if (is_some())
    {
      return Option<U>{op(*repr_)};
    }
    return Option<U>{none};
  }

  template <typename Fn, typename... U>
  constexpr auto map_or(Fn && op, U &&... alt) const
  {
    if (is_some())
    {
      return op(*repr_);
    }
    return T{static_cast<U &&>(alt)...};
  }

  template <typename Fn>
  constexpr auto and_then(Fn && op) const
  {
    using OutOption = decltype(op(*repr_));
    if (is_some())
    {
      return op(*repr_);
    }
    return OutOption{none};
  }

  constexpr void unwrap_none(Str            msg = ""_str,
                             SourceLocation loc = SourceLocation::current())
  {
    CHECK_SLOC(loc, is_none(), "Expected None in Option but got Value = {}. {}",
               *repr_, msg);
  }

  constexpr void discard()
  {
  }

  template <typename SomeFn, typename NoneFn = Noop>
  constexpr decltype(auto) match(SomeFn && some, NoneFn && none = {}) const
  {
    if (is_some())
    {
      return some(*repr_);
    }
    return none();
  }

  constexpr T * operator->() const
  {
    return &v();
  }

  constexpr T & operator*() const
  {
    return v();
  }
};

template <typename T>
struct [[nodiscard]] Option<T &&>
{
  using Type = T;
  using Repr = T *;

  T * repr_;

  constexpr Option() : repr_{nullptr}
  {
  }

  constexpr ~Option() = default;

  constexpr Option(T && some) : repr_{&some}
  {
  }

  template <typename... Args>
  explicit constexpr Option(V<0>, T && some) : repr_{&some}
  {
  }

  template <typename... Args>
  explicit constexpr Option(V<0>, T some) = delete;

  constexpr Option(None) : repr_{nullptr}
  {
  }

  constexpr Option & operator=(T && other)
  {
    repr_ = &other;
    return *this;
  }

  constexpr Option & operator=(None)
  {
    repr_ = nullptr;
    return *this;
  }

  constexpr Option(Option && other) = default;

  constexpr Option & operator=(Option && other) = default;

  constexpr Option(Option const & other) = default;

  constexpr Option & operator=(Option const & other) = default;

  [[nodiscard]] constexpr bool is_some() const
  {
    return repr_ != nullptr;
  }

  [[nodiscard]] constexpr bool is_none() const
  {
    return repr_ == nullptr;
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
      return *repr_ == cmp;
    }
    else
    {
      return false;
    }
  }

  constexpr T && v(SourceLocation loc = SourceLocation::current()) const
  {
    CHECK_SLOC(loc, is_some(), "Expected Value in Option but got None");
    return static_cast<T &&>(*repr_);
  }

  constexpr T && unwrap(Str            msg = ""_str,
                        SourceLocation loc = SourceLocation::current())
  {
    CHECK_SLOC(loc, is_some(), "Expected Value in Option but got None. {}",
               msg);
    return static_cast<T &&>(*repr_);
  }

  constexpr T && unwrap_or(T && alt)
  {
    if (is_some())
    {
      return static_cast<T &&>(*repr_);
    }
    return static_cast<T &&>(alt);
  }

  constexpr Option<T> unref() const
  {
    if (is_some())
    {
      return *repr_;
    }
    return none;
  }

  template <typename Fn>
  constexpr auto map(Fn && op) const
  {
    using U = decltype(op(static_cast<T &&>(*repr_)));
    if (is_some())
    {
      return Option<U>{op(static_cast<T &&>(*repr_))};
    }
    return Option<U>{none};
  }

  template <typename Fn, typename... U>
  constexpr auto map_or(Fn && op, U &&... alt) const
  {
    if (is_some())
    {
      return op(static_cast<T &&>(*repr_));
    }
    return T{static_cast<U &&>(alt)...};
  }

  template <typename Fn>
  constexpr auto and_then(Fn && op) const
  {
    using OutOption = decltype(op(static_cast<T &&>(*repr_)));
    if (is_some())
    {
      return op(static_cast<T &&>(*repr_));
    }
    return OutOption{none};
  }

  constexpr void unwrap_none(Str            msg = ""_str,
                             SourceLocation loc = SourceLocation::current())
  {
    CHECK_SLOC(loc, is_none(), "Expected None in Option but got Value = {}. {}",
               *repr_, msg);
  }

  constexpr void discard()
  {
  }

  template <typename SomeFn, typename NoneFn = Noop>
  constexpr decltype(auto) match(SomeFn && some, NoneFn && none = {}) const
  {
    if (is_some())
    {
      return some(static_cast<T &&>(*repr_));
    }
    return none();
  }

  constexpr T * operator->() const
  {
    return &v();
  }

  constexpr T && operator*() const
  {
    return v();
  }
};

}    // namespace ash
