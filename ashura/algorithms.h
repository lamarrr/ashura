#pragma once
#include "ashura/types.h"

namespace ash
{

namespace op
{

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
  constexpr i8 operator()(auto const &a, auto const &b) const
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
  constexpr auto const &operator()(auto const &a, auto const &b) const
  {
    return a < b ? a : b;
  }
};

struct Max
{
  constexpr auto const &operator()(auto const &a, auto const &b) const
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
  constexpr T const &operator()(T const &value, T const &low,
                                T const &high) const
  {
    return value <= low ? low : (value >= high ? high : value);
  }
};

}        // namespace op

namespace alg
{
// it's important we don't specialize any of this functions
// no default parameters
template <typename T>
constexpr void default_construct(Span<T> span)
{
  for (T *iter = span.begin(); iter < span.end(); iter++)
  {
    new (iter) T{};
  }
}

template <typename Src, typename Dst>
constexpr void move_construct(Span<Src> src, Span<Dst> dst)
{
  Src *input  = src.begin();
  Dst *output = dst.begin();
  for (; input < src.end(); output++, input++)
  {
    new (*output) Dst{(Src &&) (*input)};
  }
}

template <typename Src, typename Dst>
constexpr void copy_construct(Span<Src> src, Span<Dst> dst)
{
  Src *input  = src.begin();
  Dst *output = dst.begin();
  for (; input < src.end(); output++, input++)
  {
    new (*output) Dst{*input};
  }
}

template <typename T>
constexpr void destruct(Span<T> span)
{
  for (T *iter = span.begin(); iter < span.end(); iter++)
  {
    iter->~T();
  }
}

template <typename Src, typename Dst>
constexpr void move(Span<Src> src, Span<Dst> dst)
{
  Src *input  = src.begin();
  Dst *output = dst.begin();
  for (; input < src.end(); output++, input++)
  {
    *output = (Src &&) *input;
  }
}

template <typename Src, typename Dst>
constexpr void copy(Span<Src> src, Span<Dst> dst)
{
  Src *input  = src.begin();
  Dst *output = dst.begin();
  for (; input < src.end(); output++, input++)
  {
    *output = *input;
  }
}

template <typename A, typename B, typename Swap = op::Swap>
constexpr void swap(Span<A> a, Span<B> b, Swap swap_op = {})
{
  A *a_iter = a.begin();
  B *b_iter = b.begin();
  for (; a_iter < a.end(); a_iter++, b_iter++)
  {
    swap_op(*a_iter, *b_iter);
  }
}

// destroys elements that don't match a predicate,
// the uninitialized range, no-op
// for trivially destructible types
template <typename T, typename Predicate>
constexpr Span<T> destruct_if(Span<T> span, usize &destroy_start);

// moves the elements to the end of the range if predicate is true
constexpr void relocate_if();

template <typename T, typename U>
constexpr void fill(Span<T> dst, T const &value)
{
  for (T &element : dst)
  {
    element = value;
  }
}

template <typename T, typename Predicate>
constexpr bool all_of(Span<T> span, Predicate predicate)
{
  for (T const &element : span)
  {
    if (!predicate(element))
    {
      return false;
    }
  }
  return true;
}

template <typename T, typename Predicate>
constexpr bool any_of(Span<T> span, Predicate predicate)
{
  for (T const &element : span)
  {
    if (predicate(element))
    {
      return true;
    }
  }
  return false;
}

template <typename T, typename U, typename Cmp = op::Equal>
constexpr bool contains(Span<T const> span, U const &value, Cmp cmp = {})
{
  for (T const &element : span)
  {
    if (cmp(element, value))
    {
      return true;
    }
  }
  return false;
}

template <typename B, typename H, typename Cmp = op::Equal>
constexpr bool begins_with(Span<B const> body, Span<H const> header,
                           Cmp cmp = {})
{
  if (header.size > body.size)
  {
    return false;
  }
  for (usize i = 0; i < header.size; i++)
  {
    if (!cmp(body.data[i], header.data[i]))
    {
      return false;
    }
  }
  return true;
}

template <typename B, typename F, typename Cmp = op::Equal>
constexpr bool ends_with(Span<B const> body, Span<F const> footer, Cmp cmp = {})
{
  if (footer.size > body.size)
  {
    return false;
  }
  for (usize ibody = body.size - footer.size, ifooter = 0;
       ifooter < footer.size; ifooter++)
  {
    if (!cmp(body.data[ibody], footer.data[ifooter]))
    {
      return false;
    }
  }
  return true;
}

// size is 0 if not found, size is 1 if found
template <typename T, typename Cmp = op::Equal>
constexpr Span<T> find(Span<T> span, T const &value, Cmp cmp = {})
{
  usize offset = 0;
  for (; offset < span.size; offset++)
  {
    if (cmp(span.data[offset], value))
    {
      break;
    }
  }
  return span.slice(offset, 1);
}

template <typename T, typename Predicate>
constexpr Span<T> find_if(Span<T> span, Predicate predicate)
{
  usize offset = 0;
  for (; offset < span.size; offset++)
  {
    if (predicate(span.data[offset]))
    {
      break;
    }
  }
  return span.slice(offset, 1);
}

// points to end with 0 size if not found
template <typename T, typename U, typename Cmp = op::Equal>
constexpr Span<T> find_last(Span<T> span, U const &);

template <typename T, typename Predicate>
constexpr Span<T> find_last_if(Span<T> span, Predicate predicate);
// advance points to the first element and the length is
// with the length of the remaining elements

template <typename T, typename Predicate>
constexpr Span<T> skip_until(Span<T>, Predicate predicate);
template <typename T, typename Predicate>
constexpr Span<T> skip_while(Span<T>, Predicate predicate);
template <typename T, typename Predicate>
constexpr Span<T> skip_to_last(Span<T>, Predicate predicate);
template <typename T, typename U, typename Cmp = op::Equal>
constexpr void find_mismatch(Span<T>, Span<U>, Span<T> &, Span<U> &,
                             Cmp cmp = {});
template <typename T, typename Predicate>
constexpr usize count(Span<T const>, Predicate predicate);
template <typename T, typename Predicate>
constexpr usize count_if(Span<T const>, Predicate predicate);
template <typename A, typename B, typename Cmp = op::Equal>
constexpr void equal(Span<A const>, Span<B const>, Cmp cmp = {});
template <typename Input, typename Output, typename Map>
constexpr void map(Span<Input const>, Span<Output>, Map);

template <typename Input, typename Init, typename Reduce>
constexpr Init reduce(Span<Input const>, Init init, Reduce);

template <typename Input, typename Output, typename Init, typename Map,
          typename Reduce>
constexpr Init map_reduce(Span<Input const>, Span<Output>, Init init, Map map,
                          Reduce reduce);

constexpr void replace();

constexpr void replace_if();

constexpr void replace_copy_if();

constexpr void generate();

constexpr void partition();

template <typename T, typename Cmp = op::Equal>
constexpr void unique(Span<T>, Cmp cmp = {});        // destroy? retain?

template <typename T, typename Swap = op::Swap>
constexpr void reverse(Span<T>, Swap swap = {});

constexpr void reversed_copy();

template <typename T, typename Swap = op::Swap>
constexpr void rotate(Span<T>, Swap swap = {});

template <typename T, typename Ord = op::Min>
constexpr Span<T> min(Span<T> span, Ord ord = {});        // return Span
template <typename T, typename Ord = op::Max>
constexpr Span<T> max(Span<T> span, Ord ord = {});        // return Span

template <typename T, typename LexOrd = op::Compare>
constexpr void min_max(Span<T> span, Span<T> &min, Span<T> &max,
                       LexOrd ord = {});

template <typename T>
constexpr void stable_sort(Span<T>);
template <typename T>
constexpr void unstable_sort(Span<T>);
template <typename T>
constexpr bool is_sorted();

template <typename T, typename Cmp = op::Compare>
constexpr Span<T> binary_search(Span<T>, T const &, Cmp cmp = {});

// once gotten, it will call op(span) for each range
// TODO(lamarrr)
template <typename T, typename U, typename Op, typename Cmp = op::Equal>
constexpr void split(Span<T> span, Span<U const> delimeter, Op op,
                     Cmp cmp = {});

// first check if src begins with other
// keep advancing whilst src begins with other
// once it doesn't, store present offset,
// slice from present offset to end, and compare for other
// if equal, move back from end - other.size
// if equal again, move back
// move back until it is no longer equal
template <typename T, typename U, typename Cmp = op::Equal>
constexpr Span<T> strip(Span<T> src, Span<U const> other, Cmp cmp = {});
// title() span, no string_view types

// returns the
template <typename T, typename Predicate>
constexpr void find_reflection(Span<T> span, Span<T> &head, Span<T> &body,
                               Span<T> &tail, Predicate predicate);

}        // namespace alg
}        // namespace  ash
